# Flashes the RP2040 .uf2 over USB via picotool running inside WSL.
#
# Sequence:
#   attach run-mode (COM) device to WSL ->
#   picotool reboot into BOOTSEL ->
#   attach BOOTSEL (RP2 Boot) device to WSL ->
#   picotool load (the -x flag reboots the board into the app afterwards).
#
# Requires the device to be bound first (run "Setup: Bind USB device (admin)"
# once). attach is idempotent and does not require admin.

$ErrorActionPreference = 'Stop'

$RunPattern     = '\(COM\d+\)'
$BootselPattern = 'RP2 Boot'
$Uf2Path        = '~/gui.ll/build/gui.ll.uf2'

function Get-Device([string]$devicePattern) {
    # Only match lines from the "Connected:" section, which start with a busid
    # like "3-3". The "Persisted:" section starts with a GUID and is ignored.
    $match = usbipd list | Select-String -Pattern "^\s*(?<busid>\d+-\d+)\b.*$devicePattern"
    if (-not $match) { return $null }
    $line = $match[0].Line
    return [PSCustomObject]@{
        Busid    = $match[0].Matches[0].Groups['busid'].Value
        # The STATE column reads "Shared", "Not shared" or "Attached".
        Shared   = ($line -notmatch 'Not shared')
        Attached = ($line -match 'Attached')
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

function Invoke-Attach($dev) {
    if ($dev.Attached) {
        Write-Host "Bus ID $($dev.Busid) is already attached to WSL."
        return
    }
    if (-not $dev.Shared) {
        Write-Error "Bus ID $($dev.Busid) is not shared yet. Run the ""Setup: Bind USB device (admin)"" task once, then retry."
        exit 1
    }
    Write-Host "Attaching bus ID $($dev.Busid) to WSL..."
    usbipd attach --wsl --busid=$($dev.Busid)
    # Give WSL a moment to enumerate the device before picotool talks to it.
    Start-Sleep -Seconds 2
}

# --- Attach run mode (COM) ---
$dev = Get-Device $RunPattern
if (-not $dev) {
    Write-Error 'RP2040 not found in run mode. Make sure the board is connected and try again.'
    exit 1
}
Write-Host "Found RP2040 (run mode) at bus ID: $($dev.Busid)"
Invoke-Attach $dev

# --- Reboot into BOOTSEL ---
Write-Host 'Rebooting RP2040 into BOOTSEL...'
wsl -e bash -c "picotool reboot -u -f"

# --- Attach BOOTSEL (RP2 Boot) ---
$dev = Wait-Device $BootselPattern 'BOOTSEL / RP2 Boot'
Write-Host "Found RP2040 (BOOTSEL) at bus ID: $($dev.Busid)"
Invoke-Attach $dev

# --- Load the firmware (-x reboots into the app afterwards) ---
Write-Host 'Loading firmware...'
wsl -e bash -c "picotool load -v -x $Uf2Path"

Write-Host 'Done. Firmware flashed and RP2040 rebooted into the application.'
