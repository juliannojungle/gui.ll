# One-time setup: shares the RP2040 with WSL via usbipd, in BOTH USB modes.
#
# The RP2040 enumerates as two different USB devices:
#   - Run mode:     "...USB (COMx)..."                  -> matched by '\(COM\d+\)'
#   - BOOTSEL mode: "USB Mass Storage Device, RP2 Boot" -> matched by 'RP2 Boot'
#
# Each has a distinct VID:PID, so each needs its own usbipd bind. picotool runs
# inside WSL and can only see the board once it is attached, so the flow is:
#   bind+attach (run mode) -> picotool reboot into BOOTSEL ->
#   bind+attach (BOOTSEL)  -> picotool reboot back to run mode.
#
# bind requires administrator rights and is persistent across reboots, so it is
# skipped when the device is already shared. attach does not require admin.

$ErrorActionPreference = 'Stop'

$RunPattern     = '\(COM\d+\)'
$BootselPattern = 'RP2 Boot'

function Get-Device([string]$devicePattern) {
    # Only match lines from the "Connected:" section, which start with a busid
    # like "3-3". The "Persisted:" section starts with a GUID and is ignored.
    $match = usbipd list | Select-String -Pattern "^\s*(?<busid>\d+-\d+)\b.*$devicePattern"
    if (-not $match) { return $null }
    $line = $match[0].Line
    return [PSCustomObject]@{
        Busid  = $match[0].Matches[0].Groups['busid'].Value
        # The STATE column reads "Shared" or "Not shared".
        Shared = ($line -notmatch 'Not shared')
    }
}

function Wait-Device([string]$pattern, [string]$label, [int]$timeoutSec = 20) {
    Write-Host "Waiting for device ($label)..."
    for ($i = 0; $i -lt $timeoutSec; $i++) {
        $dev = Get-Device $pattern
        if ($dev) { return $dev }
        Start-Sleep -Seconds 1
    }
    Write-Error "Timed out waiting for device ($label)."
    exit 1
}

function Invoke-Bind([string]$busid) {
    Write-Host "Binding bus ID $busid (requires administrator)..."
    Start-Process powershell -Verb RunAs -Wait -ArgumentList @(
        '-NoProfile',
        '-Command',
        "usbipd bind --busid=$busid"
    )
}

function Ensure-Shared($dev) {
    if ($dev.Shared) {
        Write-Host "Bus ID $($dev.Busid) is already shared; skipping bind."
    } else {
        Invoke-Bind $dev.Busid
    }
}

function Invoke-Attach([string]$busid) {
    Write-Host "Attaching bus ID $busid to WSL..."
    usbipd attach --wsl --busid=$busid
    # Give WSL a moment to enumerate the device before picotool talks to it.
    Start-Sleep -Seconds 2
}

# --- Run mode (COM) ---
$dev = Get-Device $RunPattern
if (-not $dev) {
    Write-Error 'RP2040 not found in run mode. Make sure the board is connected and try again.'
    exit 1
}
Write-Host "Found RP2040 (run mode) at bus ID: $($dev.Busid)"
Ensure-Shared $dev
Invoke-Attach $dev.Busid

# --- Reboot into BOOTSEL ---
Write-Host 'Rebooting RP2040 into BOOTSEL...'
wsl -e bash -c "picotool reboot -u -f"

# --- BOOTSEL mode (RP2 Boot) ---
$dev = Wait-Device $BootselPattern 'BOOTSEL / RP2 Boot'
Write-Host "Found RP2040 (BOOTSEL) at bus ID: $($dev.Busid)"
Ensure-Shared $dev
Invoke-Attach $dev.Busid

# --- Reboot back to run mode ---
Write-Host 'Rebooting RP2040 back to run mode...'
wsl -e bash -c "picotool reboot -a -f"

Write-Host 'Done. The RP2040 is now shared with WSL in both run and BOOTSEL modes.'
