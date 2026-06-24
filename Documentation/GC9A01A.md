# GC9A01A Datasheet

> a-Si TFT LCD Single Chip Driver — 240RGBx240 Resolution
>
> Rev.1.0 Preliminary — 2019-07-02 — LCD-DST-3014 — Source: www.buydisplay.com
>
> Markdown conversion of `GC9A01A.pdf` (192 pages). Tables with merged cells in the
> original have been flattened/split so that every row/column value is unambiguous.
> Timing waveform diagrams (which are graphical) are summarized; their associated
> parameter tables are reproduced in full.

## Generation Revision History

| Rev. | Effective Date | Description of Changes | Prepared By |
|------|----------------|------------------------|-------------|
| 1.0  | 2019-07-02     | New Created            | June        |

---

## 1. Introduction

GC9A01 is a 262,144-color single-chip SOC driver for an a-TFT liquid crystal display with
resolution of 240RGBx240 dots, comprising a 360-channel source driver, a 32-channel gate
driver, 129,600 bytes GRAM for graphic display data of 240RGBx240 dots, and a power supply
circuit.

GC9A01 supports parallel 8-/9-/12-/16-/18-bit data bus MCU interface, 6-/12-/16-/18-bit data
bus RGB interface and 3-/4-line serial peripheral interface (SPI). The moving picture area can
be specified in internal GRAM by window address function. The specified window area can be
updated selectively, so that a moving picture can be displayed simultaneously independent of
the still picture area.

GC9A01 supports full color, 8-color display mode and sleep mode for precise power control by
software, making it an ideal LCD driver for medium or small size portable products such as
digital cellular phones, smart phones, MP3 and PMP where long battery life is a major concern.

---

## 2. Features

- No need for external electronic component
- Display resolution: 240xRGB (H) x 240 (V)
- Output:
  - 360 source outputs
  - 32 gate outputs
- Resolution (source mapping):
  - 80x160: S121–S240
  - 120x120 / 120x240: S91–S270
  - 128x128: S85–S276
  - 160x160: S61–S300
  - 240x240: S1–S360
- a-TFT LCD driver with on-chip full display RAM: 129,600 bytes
- System Interface:
  - 8-bit, 9-bit, 12-bit, 16-bit, 18-bit interface with 8080-I / 8080-II series MCU
  - 6-bit, 12-bit, 16-bit, 18-bit RGB interface with graphic controller
  - 8-bit, 9-bit, 24-bit Serial Peripheral Interface (SPI) and 2-data-lane SPI
- Display mode:
  - Full color mode (Idle mode OFF): 262K-color (selectable color depth mode by software)
  - Reduce color mode (Idle mode ON): 8-color
- Power saving mode: Sleep mode
- On chip functions: Timing generator, Oscillator, DC/DC converter, Dot/column inversion
- Low-power consumption architecture:
  - Low operating power supplies: IOVCC = 1.65V ~ 3.3V (logic); VCI = 2.5V ~ 3.3V (analog)
- LCD Voltage drive:
  - Source/Gamma power supply voltage: GVDD - GVCL = 6.4V ~ -4.6V
  - Gate driver output voltage: VGH - GND = 12.0V ~ 13.0V; VGL - GND = -11.0V ~ -8.0V; VGH - VGL ≤ 27V
  - VCOM connect to GND
- Operating temperature range: -40°C to 80°C
- a-Si TFT LCD storage capacitor: Cst on Common structure only

---

## 3. Block Diagram

### 3.1. Block diagram

(Graphical block diagram in the original PDF, page 8. It shows the MCU/RGB/SPI interface logic
feeding the GRAM, the address/index registers, the source/gate drivers, the gamma circuit, the
oscillator/timing generator and the DC/DC power supply circuit driving the 240RGBx240 panel.)

### 3.2. Pin Description

#### Table 1 — Power Supply Pins

| Pin Name  | I/O | Type          | Description |
|-----------|-----|---------------|-------------|
| VDDI      | I   | Digital Power | Low voltage power supply for interface logic circuits (1.65~3.3V) |
| VDDB/VDDI | I   | Analog Power  | High voltage power supply for analog circuit blocks (2.5~3.3V) |
| DVDD      | O   | Digital Power | Regulated low voltage level for interface circuits. Don't apply any external power to this pad |
| VSSB      | I   | Analog Ground | System ground level for dynamic state. Connect to VSSB on the FPC to prevent noise. |
| VSSR      | I   | Digital Ground| System ground level for static state. Connect to VSSR on the FPC to prevent noise. |

#### Table 2 — Interface Logic Signals

**IM[3:0] MCU interface selection** (pin `IM[3:0]`, I, IOVCC/GND). MPU parallel interface bus
and serial interface select. If using the RGB Interface, a serial interface must be selected.
Fix this pin at IOVCC or GND.

| IM3 | IM2 | IM1 | IM0 | MCU-Interface | Register pins | GRAM pins |
|-----|-----|-----|-----|---------------|---------------|-----------|
| 0 | 1 | 0 | 0 | 8080 MCU 8-bit bus interface I  | D[7:0] | D[7:0] |
| 0 | 1 | 1 | 0 | 8080 MCU 16-bit bus interface I | D[7:0] | D[15:0] |
| 0 | 1 | 0 | 1 | 8080 MCU 9-bit bus interface I  | D[7:0] | D[8:0] |
| 0 | 1 | 1 | 1 | 8080 MCU 18-bit bus interface I | D[7:0] | D[17:0] |
| 1 | 1 | 0 | 1 | 3-wire 9-bit data serial interface I | — | SDA: In/OUT |
| 1 | 1 | 0 | 1 | 2 data line serial interface I | — | SDA: In/OUT; DCX: In |
| 1 | 1 | 1 | 1 | 4-wire 8-bit data serial interface I | — | SDA: In/OUT |
| 0 | 0 | 1 | 0 | 8080 MCU 16-bit bus interface II | D[8:1]   | D[17:10], D[8:1] |
| 0 | 0 | 0 | 0 | 8080 MCU 8-bit bus interface II  | D[17:10] | D[17:10] |
| 0 | 0 | 1 | 1 | 8080 MCU 18-bit bus interface II | D[8:1]   | D[17:0] |
| 0 | 0 | 0 | 1 | 8080 MCU 9-bit bus interface II  | D[17:10] | D[17:9] |
| 1 | 0 | 0 | 1 | 3-wire 9-bit data serial interface II | — | SDA: In / SDO: OUT |
| 1 | 0 | 1 | 1 | 4-wire 8-bit data serial interface II | — | SDA: In / SDO: OUT |

**Other interface logic signals:**

| Pin Name | I/O | Type | Description |
|----------|-----|------|-------------|
| RESX | I | MCU (IOVCC/GND) | Resets the device; must be applied to properly initialize the chip. Active low. |
| CSX | I | MCU (IOVCC/GND) | Chip select input ("Low" enable). Can be permanently fixed "Low" in MPU interface mode only. |
| D/CX (SCL) | I | MCU (IOVCC/GND) | Selects "Data or Command" in the parallel interface. DCX='1' → data; DCX='0' → command. Also serves as serial interface clock in 3-wire 9-bit / 4-wire 8-bit serial data interface. If not used, connect to IOVCC or GND. |
| RDX | I | MCU (IOVCC/GND) | 8080-I/8080-II (RDX): read signal; MCU reads data at the rising edge. Fix to IOVCC level when not in use. |
| WRX (D/CX) | I | MCU (IOVCC/GND) | 8080-I/8080-II (WRX): write signal; writes data at the rising edge. 4-line system (D/CX): command or parameter select. Fix to IOVCC level when not in use. |
| D[17:0] | I/O | MCU (IOVCC/GND) | 18-bit parallel bi-directional data bus for MCU system and RGB interface mode. Fix to VSSR level when not in use. |
| SDA | I/O | MCU (IOVCC/GND) | When IM[3]:Low, serial in/out signal in 3-wire 9-bit/4-wire 8-bit serial interface. When IM[3]:High, serial input signal. Data is applied on the rising edge of SCL. If not used, fix at IOVCC or GND. |
| SDO | O | MCU (IOVCC/GND) | Serial output signal. Data is output on the falling edge of SCL. If not used, leave open. |
| TE | O | MCU (IOVCC/GND) | Tearing effect output pin to synchronize MPU to frame writing, activated by S/W command. When not activated, this pin is low. If not used, leave open. |
| DOTCLK | I | MCU (IOVCC/GND) | Dot clock signal for RGB interface operation. Fix to IOVCC or VSSR level when not in use. |
| VSYNC | I | MCU (IOVCC/GND) | Frame synchronizing signal for RGB interface operation. Fix to IOVCC or VSSR level when not in use. |
| HSYNC | I | MCU (IOVCC/GND) | Line synchronizing signal for RGB interface operation. Fix to IOVCC or VSSR level when not in use. |
| DE | I | MCU (IOVCC/GND) | Data enable signal for RGB interface operation. Fix to IOVCC or GND level when not in use. |

**Notes:**
1. If CSX is connected to GND in Parallel interface mode, there is no abnormal visible effect to
   the display module, no restriction on Parallel Read/Write protocols / Power On/Off sequences /
   other functions, and no influence on power consumption.
2. When CSX='1', there is no influence to the parallel and serial interface.

#### Table 3 — LCD Driver Input/Output Pins

| Pin Name | I/O | Type | Description |
|----------|-----|------|-------------|
| S360~S1 | O | Source | Source output signals. Leave open when not in use. |
| G36~G1 | O | Gate | Gate output signals. Leave open when not in use. |
| VCOM | O | GND | Connect to GND. |
| VRDD | O | Power | Power supply for AVDD |
| VREE | O | Power | Power supply for AVEE |
| VRCL | O | Power | Power supply for VCL. |
| AVDD | O | Power | Output voltage of 1st step-up circuit (3*VRDD). Input voltage to 2nd step-up circuit. Generated power output pad for source driver block. |
| AVEE | O | Power | Output voltage of 1st step-up circuit (-2*VREE). Input voltage to 2nd step-up circuit. Generated power output pad for source driver block. |
| VGH | O | Power | Power supply for the gate driver (Positive). |
| VGL | O | Power | Power supply for the gate driver (Negative). |
| VCL | O | Power | Power supply for VGH and VGL. VCL = 0 ~ -VCI |
| VREG1A | O | Ref | Internal generated stable power for source driver unit. Highest positive grayscale reference voltage of source driver. |
| VREG_VREF | O | Ref | Internal generated stable power for source driver unit. Lowest positive grayscale reference voltage of source driver. |
| VREGP | O | Ref | Internal generated stable power for source driver unit. Highest negative grayscale reference voltage of source driver. |
| VREGN | O | Ref | Internal generated stable power for source driver unit. Highest negative grayscale reference voltage of source driver. |
| BC | O | Dig IO | Output pin for PWM (Pulse Width Modulation) signal of LED driving. If not used, leave open. |

#### Table 4 — Test Pins

| Pin Name | I/O | Type | Description |
|----------|-----|------|-------------|
| OSC_IN | I/O | Open | Galaxycore internal test pins. |
| OSC_TEST | I/O | Open | Galaxycore internal test pins. |
| VPP | I/O | Open | Galaxycore internal test pins. |
| DUMMY | - | Open | Input pads used only for test purpose at IC-side. During normal operation, leave these pads open. |

#### Table 5 — Liquid Crystal Power Supply Specifications

| No. | Item | Description |
|-----|------|-------------|
| 1 | TFT Source Driver | 240 pins (240*RGB) |
| 2 | TFT Gate Driver | 32 pins |
| 3 | TFT Display's Capacitor Structure | Cst structure only (Cs on Common) |
| 4 | Liquid Crystal Drive Output | S1~S360: V0~V63 grayscales; G1~G36: VGH-VGL |
| 5 | Input Voltage | IOVCC: 1.65~3.30V; VCI: 2.50~3.30V |
| 6 | Liquid Crystal Drive Voltages | AVDD: 6.5~7.5V; AVEE: -5.5V~-4.5V; VGH: 12.0~13.0V; VGL: -11.0~-8.0V; VCL: -3.0~-1.5V; VGH-VGL: Max. 27.0V |
| 7 | Internal Step-up Circuits | AVDD: VCI*3; AVEE: VCI*-2; VGH: VCI*5; VGL: VCI*-5; VCL: VCI*-1 |

### 3.3. PAD coordinates

Chip size (including scribe line): 750µm x 10800µm. Chip thickness: 300µm. Bump height: 9µm.
Pad widths: 1–80: 35µm; 81–94: 38µm; 95–116: 50µm; 117–184: 35µm; 185–226 & 595–636
(GOUT pads): 23µm. Alignment marks: left mark (-5000, 230), right mark (5000, 230).

All 636 pad coordinates (X, Y in µm, IC origin at center). Pads 1–318 are listed in the left
group, 319–636 in the right group of each row.

| No. | Pad | X | Y | No. | Pad | X | Y |
|----:|-----|---:|---:|----:|-----|---:|---:|
| 1 | VCOM | -4904.5 | -306 | 319 | S91 | 1846 | 166 |
| 2 | VCOM | -4854.5 | -306 | 320 | S92 | 1832 | 291 |
| 3 | VCOM | -4804.5 | -306 | 321 | S93 | 1818 | 166 |
| 4 | VCOM | -4754.5 | -306 | 322 | S94 | 1804 | 291 |
| 5 | VCOM | -4704.5 | -306 | 323 | S95 | 1790 | 166 |
| 6 | DUM0 | -4654.5 | -306 | 324 | S96 | 1776 | 291 |
| 7 | VGH | -4604.5 | -306 | 325 | S97 | 1762 | 166 |
| 8 | VGH | -4554.5 | -306 | 326 | S98 | 1748 | 291 |
| 9 | VGH | -4504.5 | -306 | 327 | S99 | 1734 | 166 |
| 10 | VGH | -4454.5 | -306 | 328 | S100 | 1720 | 291 |
| 11 | VGH | -4404.5 | -306 | 329 | S101 | 1706 | 166 |
| 12 | VGL | -4354.5 | -306 | 330 | S102 | 1692 | 291 |
| 13 | VGL | -4304.5 | -306 | 331 | S103 | 1678 | 166 |
| 14 | VGL | -4254.5 | -306 | 332 | S104 | 1664 | 291 |
| 15 | VGL | -4204.5 | -306 | 333 | S105 | 1650 | 166 |
| 16 | VGL | -4154.5 | -306 | 334 | S106 | 1636 | 291 |
| 17 | VCL | -4104.5 | -306 | 335 | S107 | 1622 | 166 |
| 18 | VCL | -4054.5 | -306 | 336 | S108 | 1608 | 291 |
| 19 | VCL | -4004.5 | -306 | 337 | S109 | 1594 | 166 |
| 20 | VRCL | -3954.5 | -306 | 338 | S110 | 1580 | 291 |
| 21 | VRCL | -3904.5 | -306 | 339 | S111 | 1566 | 166 |
| 22 | VRCL | -3854.5 | -306 | 340 | S112 | 1552 | 291 |
| 23 | AVDD | -3804.5 | -306 | 341 | S113 | 1538 | 166 |
| 24 | AVDD | -3754.5 | -306 | 342 | S114 | 1524 | 291 |
| 25 | AVDD | -3704.5 | -306 | 343 | S115 | 1510 | 166 |
| 26 | AVDD | -3654.5 | -306 | 344 | S116 | 1496 | 291 |
| 27 | VRDD | -3604.5 | -306 | 345 | S117 | 1482 | 166 |
| 28 | VRDD | -3554.5 | -306 | 346 | S118 | 1468 | 291 |
| 29 | VRDD | -3504.5 | -306 | 347 | S119 | 1454 | 166 |
| 30 | VSSB | -3454.5 | -306 | 348 | S120 | 1440 | 291 |
| 31 | VSSB | -3404.5 | -306 | 349 | S121 | 1426 | 166 |
| 32 | VSSB | -3354.5 | -306 | 350 | S122 | 1412 | 291 |
| 33 | VSSB | -3304.5 | -306 | 351 | S123 | 1398 | 166 |
| 34 | VSSB | -3254.5 | -306 | 352 | S124 | 1384 | 291 |
| 35 | VDDB | -3204.5 | -306 | 353 | S125 | 1370 | 166 |
| 36 | VDDB | -3154.5 | -306 | 354 | S126 | 1356 | 291 |
| 37 | VDDB | -3104.5 | -306 | 355 | S127 | 1342 | 166 |
| 38 | VDDB | -3054.5 | -306 | 356 | S128 | 1328 | 291 |
| 39 | VDDB | -3004.5 | -306 | 357 | S129 | 1314 | 166 |
| 40 | DUM1 | -2954.5 | -306 | 358 | S130 | 1300 | 291 |
| 41 | BVDD | -2904.5 | -306 | 359 | S131 | 1286 | 166 |
| 42 | BVDD | -2854.5 | -306 | 360 | S132 | 1272 | 291 |
| 43 | BVDD | -2804.5 | -306 | 361 | S133 | 1258 | 166 |
| 44 | BVEE | -2754.5 | -306 | 362 | S134 | 1244 | 291 |
| 45 | BVEE | -2704.5 | -306 | 363 | S135 | 1230 | 166 |
| 46 | BVEE | -2654.5 | -306 | 364 | S136 | 1216 | 291 |
| 47 | DUM2 | -2604.5 | -306 | 365 | S137 | 1202 | 166 |
| 48 | DUM2 | -2554.5 | -306 | 366 | S138 | 1188 | 291 |
| 49 | DUM2 | -2504.5 | -306 | 367 | S139 | 1174 | 166 |
| 50 | DUM3 | -2454.5 | -306 | 368 | S140 | 1160 | 291 |
| 51 | DUM3 | -2404.5 | -306 | 369 | S141 | 1146 | 166 |
| 52 | DUM3 | -2354.5 | -306 | 370 | S142 | 1132 | 291 |
| 53 | DUM4 | -2304.5 | -306 | 371 | S143 | 1118 | 166 |
| 54 | DUM4 | -2254.5 | -306 | 372 | S144 | 1104 | 291 |
| 55 | DUM4 | -2204.5 | -306 | 373 | S145 | 1090 | 166 |
| 56 | DUM5 | -2154.5 | -306 | 374 | S146 | 1076 | 291 |
| 57 | DUM5 | -2104.5 | -306 | 375 | S147 | 1062 | 166 |
| 58 | DUM5 | -2054.5 | -306 | 376 | S148 | 1048 | 291 |
| 59 | DUM6 | -2004.5 | -306 | 377 | S149 | 1034 | 166 |
| 60 | DVDD | -1954.5 | -306 | 378 | S150 | 1020 | 291 |
| 61 | DVDD | -1904.5 | -306 | 379 | S151 | 1006 | 166 |
| 62 | DVDD | -1854.5 | -306 | 380 | S152 | 992 | 291 |
| 63 | VDDSF | -1804.5 | -306 | 381 | S153 | 978 | 166 |
| 64 | VDDSF | -1754.5 | -306 | 382 | S154 | 964 | 291 |
| 65 | VDDSF | -1704.5 | -306 | 383 | S155 | 950 | 166 |
| 66 | VSSB | -1654.5 | -306 | 384 | S156 | 936 | 291 |
| 67 | VSSB | -1604.5 | -306 | 385 | S157 | 922 | 166 |
| 68 | VSSB | -1554.5 | -306 | 386 | S158 | 908 | 291 |
| 69 | VSSB | -1504.5 | -306 | 387 | S159 | 894 | 166 |
| 70 | VSSB | -1454.5 | -306 | 388 | S160 | 880 | 291 |
| 71 | VDDB | -1404.5 | -306 | 389 | S161 | 866 | 166 |
| 72 | VDDB | -1354.5 | -306 | 390 | S162 | 852 | 291 |
| 73 | VDDB | -1304.5 | -306 | 391 | S163 | 838 | 166 |
| 74 | VDDB | -1254.5 | -306 | 392 | S164 | 824 | 291 |
| 75 | VDDB | -1204.5 | -306 | 393 | S165 | 810 | 166 |
| 76 | VDDI | -1154.5 | -306 | 394 | S166 | 796 | 291 |
| 77 | VDDI | -1104.5 | -306 | 395 | S167 | 782 | 166 |
| 78 | VDDI | -1054.5 | -306 | 396 | S168 | 768 | 291 |
| 79 | VDDI | -1004.5 | -306 | 397 | S169 | 754 | 166 |
| 80 | VDDI | -954.5 | -306 | 398 | S170 | 740 | 291 |
| 81 | REF_TEST | -896 | -306 | 399 | S171 | 726 | 166 |
| 82 | REF_TEST | -836 | -306 | 400 | S172 | 712 | 291 |
| 83 | RESX | -776 | -306 | 401 | S173 | 698 | 166 |
| 84 | WRX | -716 | -306 | 402 | S174 | 684 | 291 |
| 85 | CSX | -656 | -306 | 403 | S175 | 670 | 166 |
| 86 | DCX | -596 | -306 | 404 | S176 | 656 | 291 |
| 87 | RDX | -536 | -306 | 405 | S177 | 642 | 166 |
| 88 | DOTCLK | -476 | -306 | 406 | S178 | 628 | 291 |
| 89 | ENABLE | -416 | -306 | 407 | S179 | 614 | 166 |
| 90 | VSYNC | -356 | -306 | 408 | S180 | 600 | 291 |
| 91 | HSYNC | -296 | -306 | 409 | DUM26 | 586 | 166 |
| 92 | BC | -236 | -306 | 410 | DUM27 | 572 | 291 |
| 93 | TE | -176 | -306 | 411 | DUM28 | -572 | 291 |
| 94 | SDO | -116 | -306 | 412 | DUM29 | -586 | 166 |
| 95 | SDA | -50 | -306 | 413 | S181 | -600 | 291 |
| 96 | DB17 | 22 | -306 | 414 | S182 | -614 | 166 |
| 97 | DB16 | 94 | -306 | 415 | S183 | -628 | 291 |
| 98 | DB15 | 166 | -306 | 416 | S184 | -642 | 166 |
| 99 | DB14 | 238 | -306 | 417 | S185 | -656 | 291 |
| 100 | DB13 | 310 | -306 | 418 | S186 | -670 | 166 |
| 101 | DB12 | 382 | -306 | 419 | S187 | -684 | 291 |
| 102 | DB11 | 454 | -306 | 420 | S188 | -698 | 166 |
| 103 | DUM7 | 526 | -306 | 421 | S189 | -712 | 291 |
| 104 | DB10 | 598 | -306 | 422 | S190 | -726 | 166 |
| 105 | DB9 | 670 | -306 | 423 | S191 | -740 | 291 |
| 106 | DUM8 | 742 | -306 | 424 | S192 | -754 | 166 |
| 107 | DUM9 | 843 | -306 | 425 | S193 | -768 | 291 |
| 108 | DB8 | 915 | -306 | 426 | S194 | -782 | 166 |
| 109 | DB7 | 987 | -306 | 427 | S195 | -796 | 291 |
| 110 | DB6 | 1059 | -306 | 428 | S196 | -810 | 166 |
| 111 | DB5 | 1131 | -306 | 429 | S197 | -824 | 291 |
| 112 | DB4 | 1203 | -306 | 430 | S198 | -838 | 166 |
| 113 | DB3 | 1275 | -306 | 431 | S199 | -852 | 291 |
| 114 | DB2 | 1347 | -306 | 432 | S200 | -866 | 166 |
| 115 | DB1 | 1419 | -306 | 433 | S201 | -880 | 291 |
| 116 | DB0 | 1491 | -306 | 434 | S202 | -894 | 166 |
| 117 | DUM10 | 1554.5 | -306 | 435 | S203 | -908 | 291 |
| 118 | IM0 | 1604.5 | -306 | 436 | S204 | -922 | 166 |
| 119 | IM1 | 1654.5 | -306 | 437 | S205 | -936 | 291 |
| 120 | IM2 | 1704.5 | -306 | 438 | S206 | -950 | 166 |
| 121 | IM3 | 1754.5 | -306 | 439 | S207 | -964 | 291 |
| 122 | OSC_IN | 1804.5 | -306 | 440 | S208 | -978 | 166 |
| 123 | OSC_TEST | 1854.5 | -306 | 441 | S209 | -992 | 291 |
| 124 | DUM11 | 1904.5 | -306 | 442 | S210 | -1006 | 166 |
| 125 | VSSB | 1954.5 | -306 | 443 | S211 | -1020 | 291 |
| 126 | VSSB | 2004.5 | -306 | 444 | S212 | -1034 | 166 |
| 127 | VSSB | 2054.5 | -306 | 445 | S213 | -1048 | 291 |
| 128 | VSSB | 2104.5 | -306 | 446 | S214 | -1062 | 166 |
| 129 | VSSB | 2154.5 | -306 | 447 | S215 | -1076 | 291 |
| 130 | VSSR | 2204.5 | -306 | 448 | S216 | -1090 | 166 |
| 131 | VSSR | 2254.5 | -306 | 449 | S217 | -1104 | 291 |
| 132 | VSSR | 2304.5 | -306 | 450 | S218 | -1118 | 166 |
| 133 | VSSR | 2354.5 | -306 | 451 | S219 | -1132 | 291 |
| 134 | VSSR | 2404.5 | -306 | 452 | S220 | -1146 | 166 |
| 135 | DUM12 | 2454.5 | -306 | 453 | S221 | -1160 | 291 |
| 136 | VPP | 2504.5 | -306 | 454 | S222 | -1174 | 166 |
| 137 | VREG1A | 2554.5 | -306 | 455 | S223 | -1188 | 291 |
| 138 | VREG1A | 2604.5 | -306 | 456 | S224 | -1202 | 166 |
| 139 | VREGP | 2654.5 | -306 | 457 | S225 | -1216 | 291 |
| 140 | VREGP | 2704.5 | -306 | 458 | S226 | -1230 | 166 |
| 141 | VREG_VREF | 2754.5 | -306 | 459 | S227 | -1244 | 291 |
| 142 | VREG_VREF | 2804.5 | -306 | 460 | S228 | -1258 | 166 |
| 143 | DUM13 | 2854.5 | -306 | 461 | S229 | -1272 | 291 |
| 144 | DUM13 | 2904.5 | -306 | 462 | S230 | -1286 | 166 |
| 145 | DUM14 | 2954.5 | -306 | 463 | S231 | -1300 | 291 |
| 146 | DUM14 | 3004.5 | -306 | 464 | S232 | -1314 | 166 |
| 147 | DUM15 | 3054.5 | -306 | 465 | S233 | -1328 | 291 |
| 148 | VDDB | 3104.5 | -306 | 466 | S234 | -1342 | 166 |
| 149 | VDDB | 3154.5 | -306 | 467 | S235 | -1356 | 291 |
| 150 | VDDB | 3204.5 | -306 | 468 | S236 | -1370 | 166 |
| 151 | VDDB | 3254.5 | -306 | 469 | S237 | -1384 | 291 |
| 152 | VDDB | 3304.5 | -306 | 470 | S238 | -1398 | 166 |
| 153 | DUM15 | 3354.5 | -306 | 471 | S239 | -1412 | 291 |
| 154 | DUM16 | 3404.5 | -306 | 472 | S240 | -1426 | 166 |
| 155 | DUM16 | 3454.5 | -306 | 473 | S241 | -1440 | 291 |
| 156 | DUM17 | 3504.5 | -306 | 474 | S242 | -1454 | 166 |
| 157 | DUM17 | 3554.5 | -306 | 475 | S243 | -1468 | 291 |
| 158 | DUM17 | 3604.5 | -306 | 476 | S244 | -1482 | 166 |
| 159 | DUM18 | 3654.5 | -306 | 477 | S245 | -1496 | 291 |
| 160 | DUM18 | 3704.5 | -306 | 478 | S246 | -1510 | 166 |
| 161 | DUM19 | 3754.5 | -306 | 479 | S247 | -1524 | 291 |
| 162 | DUM19 | 3804.5 | -306 | 480 | S248 | -1538 | 166 |
| 163 | VSSB | 3854.5 | -306 | 481 | S249 | -1552 | 291 |
| 164 | VSSB | 3904.5 | -306 | 482 | S250 | -1566 | 166 |
| 165 | VSSB | 3954.5 | -306 | 483 | S251 | -1580 | 291 |
| 166 | VSSB | 4004.5 | -306 | 484 | S252 | -1594 | 166 |
| 167 | VSSB | 4054.5 | -306 | 485 | S253 | -1608 | 291 |
| 168 | VREGN | 4104.5 | -306 | 486 | S254 | -1622 | 166 |
| 169 | VREGN | 4154.5 | -306 | 487 | S255 | -1636 | 291 |
| 170 | VREGN | 4204.5 | -306 | 488 | S256 | -1650 | 166 |
| 171 | AVEE | 4254.5 | -306 | 489 | S257 | -1664 | 291 |
| 172 | AVEE | 4304.5 | -306 | 490 | S258 | -1678 | 166 |
| 173 | AVEE | 4354.5 | -306 | 491 | S259 | -1692 | 291 |
| 174 | VREE | 4404.5 | -306 | 492 | S260 | -1706 | 166 |
| 175 | VREE | 4454.5 | -306 | 493 | S261 | -1720 | 291 |
| 176 | VREE | 4504.5 | -306 | 494 | S262 | -1734 | 166 |
| 177 | DUM20 | 4554.5 | -306 | 495 | S263 | -1748 | 291 |
| 178 | DUM20 | 4604.5 | -306 | 496 | S264 | -1762 | 166 |
| 179 | DUM20 | 4654.5 | -306 | 497 | S265 | -1776 | 291 |
| 180 | VCOM | 4704.5 | -306 | 498 | S266 | -1790 | 166 |
| 181 | VCOM | 4754.5 | -306 | 499 | S267 | -1804 | 291 |
| 182 | VCOM | 4804.5 | -306 | 500 | S268 | -1818 | 166 |
| 183 | VCOM | 4854.5 | -306 | 501 | S269 | -1832 | 291 |
| 184 | VCOM | 4904.5 | -306 | 502 | S270 | -1846 | 166 |
| 185 | DUM21 | 4908 | 306 | 503 | S271 | -1860 | 291 |
| 186 | DUM22 | 4870 | 306 | 504 | S272 | -1874 | 166 |
| 187 | DUM22 | 4832 | 306 | 505 | S273 | -1888 | 291 |
| 188 | DUM22 | 4794 | 306 | 506 | S274 | -1902 | 166 |
| 189 | DUM22 | 4756 | 306 | 507 | S275 | -1916 | 291 |
| 190 | VGL | 4718 | 306 | 508 | S276 | -1930 | 166 |
| 191 | VGL | 4680 | 306 | 509 | S277 | -1944 | 291 |
| 192 | VGL | 4642 | 306 | 510 | S278 | -1958 | 166 |
| 193 | VGL | 4604 | 306 | 511 | S279 | -1972 | 291 |
| 194 | GOUT1 | 4566 | 306 | 512 | S280 | -1986 | 166 |
| 195 | GOUT1 | 4528 | 306 | 513 | S281 | -2000 | 291 |
| 196 | GOUT2 | 4490 | 306 | 514 | S282 | -2014 | 166 |
| 197 | GOUT2 | 4452 | 306 | 515 | S283 | -2028 | 291 |
| 198 | GOUT3 | 4414 | 306 | 516 | S284 | -2042 | 166 |
| 199 | GOUT3 | 4376 | 306 | 517 | S285 | -2056 | 291 |
| 200 | GOUT4 | 4338 | 306 | 518 | S286 | -2070 | 166 |
| 201 | GOUT4 | 4300 | 306 | 519 | S287 | -2084 | 291 |
| 202 | GOUT5 | 4262 | 306 | 520 | S288 | -2098 | 166 |
| 203 | GOUT5 | 4224 | 306 | 521 | S289 | -2112 | 291 |
| 204 | GOUT6 | 4186 | 306 | 522 | S290 | -2126 | 166 |
| 205 | GOUT6 | 4148 | 306 | 523 | S291 | -2140 | 291 |
| 206 | GOUT7 | 4110 | 306 | 524 | S292 | -2154 | 166 |
| 207 | GOUT7 | 4072 | 306 | 525 | S293 | -2168 | 291 |
| 208 | GOUT8 | 4034 | 306 | 526 | S294 | -2182 | 166 |
| 209 | GOUT8 | 3996 | 306 | 527 | S295 | -2196 | 291 |
| 210 | GOUT9 | 3958 | 306 | 528 | S296 | -2210 | 166 |
| 211 | GOUT9 | 3920 | 306 | 529 | S297 | -2224 | 291 |
| 212 | GOUT10 | 3882 | 306 | 530 | S298 | -2238 | 166 |
| 213 | GOUT10 | 3844 | 306 | 531 | S299 | -2252 | 291 |
| 214 | GOUT11 | 3806 | 306 | 532 | S300 | -2266 | 166 |
| 215 | GOUT11 | 3768 | 306 | 533 | S301 | -2280 | 291 |
| 216 | GOUT12 | 3730 | 306 | 534 | S302 | -2294 | 166 |
| 217 | GOUT12 | 3692 | 306 | 535 | S303 | -2308 | 291 |
| 218 | GOUT13 | 3654 | 306 | 536 | S304 | -2322 | 166 |
| 219 | GOUT13 | 3616 | 306 | 537 | S305 | -2336 | 291 |
| 220 | GOUT14 | 3578 | 306 | 538 | S306 | -2350 | 166 |
| 221 | GOUT14 | 3540 | 306 | 539 | S307 | -2364 | 291 |
| 222 | GOUT15 | 3502 | 306 | 540 | S308 | -2378 | 166 |
| 223 | GOUT15 | 3464 | 306 | 541 | S309 | -2392 | 291 |
| 224 | GOUT16 | 3426 | 306 | 542 | S310 | -2406 | 166 |
| 225 | GOUT16 | 3388 | 306 | 543 | S311 | -2420 | 291 |
| 226 | DUM23 | 3350 | 306 | 544 | S312 | -2434 | 166 |
| 227 | DUM24 | 3134 | 166 | 545 | S313 | -2448 | 291 |
| 228 | DUM25 | 3120 | 291 | 546 | S314 | -2462 | 166 |
| 229 | S1 | 3106 | 166 | 547 | S315 | -2476 | 291 |
| 230 | S2 | 3092 | 291 | 548 | S316 | -2490 | 166 |
| 231 | S3 | 3078 | 166 | 549 | S317 | -2504 | 291 |
| 232 | S4 | 3064 | 291 | 550 | S318 | -2518 | 166 |
| 233 | S5 | 3050 | 166 | 551 | S319 | -2532 | 291 |
| 234 | S6 | 3036 | 291 | 552 | S320 | -2546 | 166 |
| 235 | S7 | 3022 | 166 | 553 | S321 | -2560 | 291 |
| 236 | S8 | 3008 | 291 | 554 | S322 | -2574 | 166 |
| 237 | S9 | 2994 | 166 | 555 | S323 | -2588 | 291 |
| 238 | S10 | 2980 | 291 | 556 | S324 | -2602 | 166 |
| 239 | S11 | 2966 | 166 | 557 | S325 | -2616 | 291 |
| 240 | S12 | 2952 | 291 | 558 | S326 | -2630 | 166 |
| 241 | S13 | 2938 | 166 | 559 | S327 | -2644 | 291 |
| 242 | S14 | 2924 | 291 | 560 | S328 | -2658 | 166 |
| 243 | S15 | 2910 | 166 | 561 | S329 | -2672 | 291 |
| 244 | S16 | 2896 | 291 | 562 | S330 | -2686 | 166 |
| 245 | S17 | 2882 | 166 | 563 | S331 | -2700 | 291 |
| 246 | S18 | 2868 | 291 | 564 | S332 | -2714 | 166 |
| 247 | S19 | 2854 | 166 | 565 | S333 | -2728 | 291 |
| 248 | S20 | 2840 | 291 | 566 | S334 | -2742 | 166 |
| 249 | S21 | 2826 | 166 | 567 | S335 | -2756 | 291 |
| 250 | S22 | 2812 | 291 | 568 | S336 | -2770 | 166 |
| 251 | S23 | 2798 | 166 | 569 | S337 | -2784 | 291 |
| 252 | S24 | 2784 | 291 | 570 | S338 | -2798 | 166 |
| 253 | S25 | 2770 | 166 | 571 | S339 | -2812 | 291 |
| 254 | S26 | 2756 | 291 | 572 | S340 | -2826 | 166 |
| 255 | S27 | 2742 | 166 | 573 | S341 | -2840 | 291 |
| 256 | S28 | 2728 | 291 | 574 | S342 | -2854 | 166 |
| 257 | S29 | 2714 | 166 | 575 | S343 | -2868 | 291 |
| 258 | S30 | 2700 | 291 | 576 | S344 | -2882 | 166 |
| 259 | S31 | 2686 | 166 | 577 | S345 | -2896 | 291 |
| 260 | S32 | 2672 | 291 | 578 | S346 | -2910 | 166 |
| 261 | S33 | 2658 | 166 | 579 | S347 | -2924 | 291 |
| 262 | S34 | 2644 | 291 | 580 | S348 | -2938 | 166 |
| 263 | S35 | 2630 | 166 | 581 | S349 | -2952 | 291 |
| 264 | S36 | 2616 | 291 | 582 | S350 | -2966 | 166 |
| 265 | S37 | 2602 | 166 | 583 | S351 | -2980 | 291 |
| 266 | S38 | 2588 | 291 | 584 | S352 | -2994 | 166 |
| 267 | S39 | 2574 | 166 | 585 | S353 | -3008 | 291 |
| 268 | S40 | 2560 | 291 | 586 | S354 | -3022 | 166 |
| 269 | S41 | 2546 | 166 | 587 | S355 | -3036 | 291 |
| 270 | S42 | 2532 | 291 | 588 | S356 | -3050 | 166 |
| 271 | S43 | 2518 | 166 | 589 | S357 | -3064 | 291 |
| 272 | S44 | 2504 | 291 | 590 | S358 | -3078 | 166 |
| 273 | S45 | 2490 | 166 | 591 | S359 | -3092 | 291 |
| 274 | S46 | 2476 | 291 | 592 | S360 | -3106 | 166 |
| 275 | S47 | 2462 | 166 | 593 | DUM30 | -3120 | 291 |
| 276 | S48 | 2448 | 291 | 594 | DUM31 | -3134 | 166 |
| 277 | S49 | 2434 | 166 | 595 | DUM32 | -3350 | 306 |
| 278 | S50 | 2420 | 291 | 596 | GOUT17 | -3388 | 306 |
| 279 | S51 | 2406 | 166 | 597 | GOUT17 | -3426 | 306 |
| 280 | S52 | 2392 | 291 | 598 | GOUT18 | -3464 | 306 |
| 281 | S53 | 2378 | 166 | 599 | GOUT18 | -3502 | 306 |
| 282 | S54 | 2364 | 291 | 600 | GOUT19 | -3540 | 306 |
| 283 | S55 | 2350 | 166 | 601 | GOUT19 | -3578 | 306 |
| 284 | S56 | 2336 | 291 | 602 | GOUT20 | -3616 | 306 |
| 285 | S57 | 2322 | 166 | 603 | GOUT20 | -3654 | 306 |
| 286 | S58 | 2308 | 291 | 604 | GOUT21 | -3692 | 306 |
| 287 | S59 | 2294 | 166 | 605 | GOUT21 | -3730 | 306 |
| 288 | S60 | 2280 | 291 | 606 | GOUT22 | -3768 | 306 |
| 289 | S61 | 2266 | 166 | 607 | GOUT22 | -3806 | 306 |
| 290 | S62 | 2252 | 291 | 608 | GOUT23 | -3844 | 306 |
| 291 | S63 | 2238 | 166 | 609 | GOUT23 | -3882 | 306 |
| 292 | S64 | 2224 | 291 | 610 | GOUT24 | -3920 | 306 |
| 293 | S65 | 2210 | 166 | 611 | GOUT24 | -3958 | 306 |
| 294 | S66 | 2196 | 291 | 612 | GOUT25 | -3996 | 306 |
| 295 | S67 | 2182 | 166 | 613 | GOUT25 | -4034 | 306 |
| 296 | S68 | 2168 | 291 | 614 | GOUT26 | -4072 | 306 |
| 297 | S69 | 2154 | 166 | 615 | GOUT26 | -4110 | 306 |
| 298 | S70 | 2140 | 291 | 616 | GOUT27 | -4148 | 306 |
| 299 | S71 | 2126 | 166 | 617 | GOUT27 | -4186 | 306 |
| 300 | S72 | 2112 | 291 | 618 | GOUT28 | -4224 | 306 |
| 301 | S73 | 2098 | 166 | 619 | GOUT28 | -4262 | 306 |
| 302 | S74 | 2084 | 291 | 620 | GOUT29 | -4300 | 306 |
| 303 | S75 | 2070 | 166 | 621 | GOUT29 | -4338 | 306 |
| 304 | S76 | 2056 | 291 | 622 | GOUT30 | -4376 | 306 |
| 305 | S77 | 2042 | 166 | 623 | GOUT30 | -4414 | 306 |
| 306 | S78 | 2028 | 291 | 624 | GOUT31 | -4452 | 306 |
| 307 | S79 | 2014 | 166 | 625 | GOUT31 | -4490 | 306 |
| 308 | S80 | 2000 | 291 | 626 | GOUT32 | -4528 | 306 |
| 309 | S81 | 1986 | 166 | 627 | GOUT32 | -4566 | 306 |
| 310 | S82 | 1972 | 291 | 628 | VGL | -4604 | 306 |
| 311 | S83 | 1958 | 166 | 629 | VGL | -4642 | 306 |
| 312 | S84 | 1944 | 291 | 630 | VGL | -4680 | 306 |
| 313 | S85 | 1930 | 166 | 631 | VGL | -4718 | 306 |
| 314 | S86 | 1916 | 291 | 632 | DUM33 | -4756 | 306 |
| 315 | S87 | 1902 | 166 | 633 | DUM33 | -4794 | 306 |
| 316 | S88 | 1888 | 291 | 634 | DUM33 | -4832 | 306 |
| 317 | S89 | 1874 | 166 | 635 | DUM33 | -4870 | 306 |
| 318 | S90 | 1860 | 291 | 636 | DUM34 | -4908 | 306 |

---

## 4. Interface setting

### 4.1. MCU interfaces

GC9A01 provides 8-/9-/12-/16-/18-bit parallel system interfaces for 8080-I / 8080-II series, and
3-/4-line serial system interfaces for serial data input. The input system interface is selected
by external pins IM[3:0], and the bit-format-per-pixel color order is selected by the DBI[2:0]
3 bits of the 3Ah register.

#### 4.1.1. MCU interface selection — Table 6

Selection of interface is done by setting external pins IM[3:0].

| IM3 | IM2 | IM1 | IM0 | MCU-Interface Mode | Register/Content | GRAM (pins in use) |
|-----|-----|-----|-----|--------------------|------------------|--------------------|
| 0 | 1 | 0 | 0 | 8080 MCU 8-bit bus interface I  | D[7:0] | D[7:0], WRX, RDX, CSX, D/CX |
| 0 | 1 | 1 | 0 | 8080 MCU 16-bit bus interface I | D[7:0] | D[15:0], WRX, RDX, CSX, D/CX |
| 0 | 1 | 0 | 1 | 8080 MCU 9-bit bus interface I  | D[7:0] | D[8:0], WRX, RDX, CSX, D/CX |
| 0 | 1 | 1 | 1 | 8080 MCU 18-bit bus interface I | D[7:0] | D[17:0], WRX, RDX, CSX, D/CX |
| 1 | 1 | 0 | 1 | 3-wire 9-bit data serial interface I | — | SCL, SDA, CSX |
| 1 | 1 | 0 | 1 | 2 data line serial interface I       | — | SCL, SDA, CSX, DCX |
| 1 | 1 | 1 | 1 | 4-wire 8-bit data serial interface I | — | SCL, SDA, D/CX, CSX |
| 0 | 0 | 1 | 0 | 8080 MCU 16-bit bus interface II | D[8:1]   | D[17:10], D[8:1], WRX, RDX, CSX, D/CX |
| 0 | 0 | 0 | 0 | 8080 MCU 8-bit bus interface II  | D[17:10] | D[17:10], WRX, RDX, CSX, D/CX |
| 0 | 0 | 1 | 1 | 8080 MCU 18-bit bus interface II | D[8:1]   | D[17:0], WRX, RDX, CSX, D/CX |
| 0 | 0 | 0 | 1 | 8080 MCU 9-bit bus interface II  | D[17:10] | D[17:9], WRX, RDX, CSX, D/CX |
| 1 | 0 | 0 | 1 | 3-wire 9-bit data serial interface II | — | SCL, SDA, CSX, SDO |
| 1 | 0 | 1 | 1 | 4-wire 8-bit data serial interface II | — | SCL, SDA, D/CX, CSX, SDO |

#### 4.1.2. 8080-I Series Parallel Interface

GC9A01 can be accessed via 8-/9-/12-/16-/18-bit MCU 8080-I series parallel interface. CSX (active
low) enables/disables the chip; RESX (active low) is the external reset; WRX is the write strobe;
RDX is the read strobe; D[17:0] is the parallel data bus. Input data is latched on the rising edge
of WRX. D/CX selects data/command: D/CX='1' → display RAM data or command parameters; D/CX='0' →
commands. The 8080-I interface is selected when IM3 is low (VSSR level); bus width is selected by
IM[2:0].

#### Table 7 — 8080-I series parallel interface control

For every listed bus width (8080 MCU 8/16/9/18-bit bus interface I), the control signals behave
identically:

| CSX | WRX | RDX | D/CX | Function |
|-----|-----|-----|------|----------|
| "L" | ↑ (active) | "H" | "L" | Write command code. |
| "L" | "H" | ↑ (active) | "H" | Read internal status. |
| "L" | ↑ (active) | "H" | "H" | Write parameter or display data. |
| "L" | "H" | ↑ (active) | "H" | Read parameter or display data. |

(IM3,IM2 = 0,1; IM1,IM0 select width: 00=8-bit, 10=16-bit, 01=9-bit, 11=18-bit.)

#### 4.1.3. Write Cycle Sequence (8080-I)

WRX is driven high→low→high during the write cycle. The display module captures information on the
rising edge of WRX. When D/CX is low the data is interpreted as command information; when D/CX is
high the data is SRAM data or a command parameter. WRX is an unsynchronized signal (it can be
stopped). Signals on D[17:0], D/CX, RDX and WRX are ignored while CSX="H".
(Figures 2 and 3 in the original are waveform diagrams.)

#### 4.1.4. Read Cycle Sequence (8080-I)

RDX is driven high→low→high during the read cycle. The host reads display module information on the
rising edge of RDX. D/CX low → command; D/CX high → RAM data or command parameter. RDX is an
unsynchronized signal (it can be stopped).
Note: Read data is only valid when D/CX is pulled high. If D/CX is driven low during read, the
display information outputs are High-Z. (Figures 4 and 5 are waveform diagrams.)

#### 4.1.5. 8080-II Series Parallel Interface

GC9A01 can be accessed via 8-/9-/16-/18-bit MCU 8080-II series parallel interface. Signals and
latching behave as in 8080-I. The 8080-II interface is selected when IM3 is high (IOVCC level);
bus width is selected by IM[2:0].

#### Table 8 — 8080-II series parallel interface control

For every listed bus width (8080 MCU 16/8/18/9-bit bus interface II), the control signals behave
identically:

| CSX | WRX | RDX | D/CX | Function |
|-----|-----|-----|------|----------|
| "L" | ↑ (active) | "H" | "L" | Write command code. |
| "L" | "H" | ↑ (active) | "H" | Read internal status. |
| "L" | ↑ (active) | "H" | "H" | Write parameter or display data. |
| "L" | "H" | ↑ (active) | "H" | Read parameter or display data. |

(IM3,IM2 = 0,0; IM1,IM0 select width: 10=16-bit, 00=8-bit, 11=18-bit, 01=9-bit.)

#### 4.1.6. Write Cycle Sequence (8080-II)

Same behavior as 8080-I: WRX is driven high→low→high; the display captures information on the
rising edge of WRX. D/CX low → command; D/CX high → RAM data or command parameter. WRX is
unsynchronized (can be stopped). Signals are ignored while CSX="H". (Figures 6, 7 are waveforms.)

#### 4.1.7. Read Cycle Sequence (8080-II)

Same behavior as 8080-I: RDX high→low→high; host reads on the rising edge of RDX. Read data is
valid only when D/CX is high; if D/CX is low during read, outputs are High-Z. (Figures 8, 9 are
waveforms.)

#### 4.1.8. Serial Interface

#### Table 8 (serial) — serial interface selection

| IM3 | IM2 | IM1 | IM0 | MCU-Interface Mode | CSX | D/CX | SCL | Function |
|-----|-----|-----|-----|--------------------|-----|------|-----|----------|
| 1 | 1 | 0 | 1 | 3-line serial interface | "L" | X (—) | "L" | Read/Write command, parameter or display data. |
| 1 | 1 | 1 | 1 | 4-line serial interface | "L" | "H/L" | "L" | Read/Write command, parameter or display data. |
| 1 | 0 | 0 | 1 | 3-line serial interface | "L" | X (—) | "L" | Read/Write command, parameter or display data. |
| 1 | 0 | 1 | 1 | 4-line serial interface | "L" | "H/L" | "L" | Read/Write command, parameter or display data. |

GC9A01 supplies 3-line/9-bit and 4-line/8-bit bi-directional serial interfaces. The 3-line serial
mode consists of CSX, SCL and SDA (SDI/SDO). The 4-line serial mode adds the D/CX selection input.
Unused D[17:0] data bus lines must be connected to GND. SCL is used only for MCU interface, so it
can be stopped when no communication is needed.

#### 4.1.9. Write Cycle Sequence (Serial)

The host writes commands or data to GC9A01. In the 3-line serial packet, each transmission unit
contains a D/CX bit followed by an 8-bit byte; D/CX low → command byte, D/CX high → display data
RAM (Memory Write) or command parameter. MSB is transmitted first. The interface is initialized
while CSX is high (SCL/SDA have no effect); a falling edge on CSX starts data transmission.

- **3-line serial format (Figure 10):** `[D/CX][D7..D0]` repeated — D/CX bit prepended to each
  8-bit transmission byte.
- **4-line serial format (Figure 11):** `[D7..D0]` bytes; the D/CX line selects command/data.

The host drives CSX low and sets the D/CX bit on SDA, read by GC9A01 on the first rising edge of
SCL. On each next falling edge of SCL, the host sets the next data bit (D7, then D6, …). CSX may go
high between bytes; SDA/SCL are invalid while CSX is high. (Figures 12, 13 are protocol waveforms.)

#### 4.1.10. Read Cycle Sequence (Serial)

The host reads a register parameter: it sends a command (Read ID or register command) and the
following byte(s) are transmitted in the opposite direction. GC9A01 latches SDA on rising edges of
SCL and shifts SDA out on falling edges. After a read status command, SDA must be tri-stated no
later than the falling edge of SCL of the last bit. Three transmitted command-data widths exist
(8-/24-/32-bit) depending on the command code:

- 8-bit read: RDID1/RDID2/RDID3/0Ah/0Bh/0Ch/0Dh/0Eh/0Fh (Figures 14, 17).
- 24-bit read: RDDID (Figure 15) — includes a dummy clock cycle before the multi-byte output.
- 32-bit read: RDDST (Figure 16) — includes a dummy clock cycle before the multi-byte output.

In Interface I the read data is shifted out on SDA; in Interface II it is shifted out on SDO.

#### 4.1.11. Data Transfer Break and Recovery

- **Break by RESX pulse:** if a command/parameter byte is interrupted before bit D0 completes, the
  driver rejects the previous bits and resets the interface, ready to receive command data again
  once CSX is activated after RESX returns high. SCL/SDA during RESX="L" are invalid and the next
  byte becomes a command. (Wait > 10µs — Figure 20.)
- **Break by CSX pulse:** if a command/frame-memory/parameter byte is interrupted before D0
  completes, the driver rejects the previous bits and is ready to receive the same byte
  re-transmitted when CSX is next activated. (Figure 21.)
- **Break during a multi-parameter command, then a new command:** successfully sent parameters are
  stored; the parameter where the break occurred is rejected. The original command (with its first
  parameter) must be executed again to write the remaining parameters. (Figures 22, 23.)

#### 4.1.12. Data Transfer Pause

A pause can be invoked while transferring a command, frame memory data, or multiple parameters. If
CSX is released high after a whole byte of frame memory/parameter data, GC9A01 waits and continues
from the paused point. If CSX is released after a whole command byte, the next enable receives
either the command's parameters or a new command. Applies to 4 conditions: (1) Command-Pause-Command,
(2) Command-Pause-Parameter, (3) Parameter-Pause-Command, (4) Parameter-Pause-Parameter. (Figure 24.)

#### 4.1.13. Serial Interface Pause (3-wire)

CSX may be high between data and the next command; SDA and SCL are invalid while CSX is high.
(Figure 25.)

#### 4.1.14. Parallel Interface Pause

A pause may occur between Command/Parameter transfers on the parallel bus while CSX is held.
(Figure 26.)

#### 4.1.15. Data Transfer Mode

GC9A01 provides two color depths (16-bit/pixel and 18-bit/pixel) of display data to the graphic
RAM. Data can be downloaded to the frame memory by 2 methods.

#### 4.1.16. Data Transfer Method 1

Successive frame writing: each time the frame memory is filled, the pointer resets to the start
point and the next frame is written. Sequence: Start Frame Memory Write → Frame 1 → Frame 2 →
Frame 3 → (any command stops). (Figure 27.)

#### 4.1.17. Data Transfer Method 2

After each frame download, a command is sent to stop frame memory writing, then a new Start Memory
Write command begins a new frame. (Figure 28.)

> Note 1: These methods apply to all data transfer color modes on both serial and parallel
> interfaces.
> Note 2: The frame memory can contain odd or even number of pixels for both methods. Only complete
> pixel data is stored.

### 4.2. RGB Interface

#### 4.2.1. RGB Interface Selection

GC9A01 has two RGB interface modes, selected by RCM[1:0]: "10" = DE mode (uses VSYNC, HSYNC,
DOTCLK, DE, D[17:0]); "11" = SYNC mode (uses VSYNC, HSYNC, DOTCLK, D[17:0]). Using the RGB
interface requires selecting a serial interface. Pixel formats are selected by the RIM bit of the
F6h command.

#### Table 9 — RGB interface selection (RCM[1:0], RIM, DPI[1:0])

| RCM[1] | RCM[0] | RIM | DPI[1] | DPI[0] | RGB interface Mode | RGB Mode | Used Pins |
|---|---|---|---|---|---|---|---|
| 1 | 0 | 0 | 1 | 0 | 18-bit RGB interface (262K colors) | DE Mode (valid data determined by DE signal) | VSYNC, HSYNC, DE, DOTCLK, D[17:0] |
| 1 | 0 | 0 | 0 | 1 | 16-bit RGB interface (65K colors) | DE Mode | VSYNC, HSYNC, DE, DOTCLK, D[17:13] & D[11:1] |
| 1 | 0 | 1 | — | — | 6-bit RGB interface (262K colors) | DE Mode | VSYNC, HSYNC, DE, DOTCLK, D[5:0] |
| 1 | 1 | 0 | 1 | 0 | 18-bit RGB interface (262K colors) | SYNC Mode (DE ignored; blanking porch set by B5h) | VSYNC, HSYNC, DOTCLK, D[17:0] |
| 1 | 1 | 0 | 0 | 1 | 16-bit RGB interface (65K colors) | SYNC Mode | VSYNC, HSYNC, DOTCLK, D[17:13] & D[11:1] |
| 1 | 1 | 1 | — | — | 6-bit RGB interface (262K colors) | SYNC Mode | VSYNC, HSYNC, DOTCLK, D[5:0] |

**Frame memory write bit mapping:**

- 18-bit bus (D[17:0]), RIM=0 (Figure 29):
  `R[5:0]=D17..D12, G[5:0]=D11..D6, B[5:0]=D5..D0`
- 16-bit bus (D[17:13] & D[11:1]), DPI[2:0]=101, RIM=0 (Figure 30):
  `R[4:0]=D17..D13, G[5:0]=D11..D6, B[4:0]=D5..D1`. The LSB of red/blue equals the MSB.
- 6-bit bus (D[5:0]), RIM=1 (Figure 31): three DOTCLKs per pixel —
  `R[5:0]`, then `G[5:0]`, then `B[5:0]` on D[5:0].

DOTCLK runs continuously; VSYNC/HSYNC/DE/D[17:0] are sampled on its rising edge. VSYNC (low enable)
marks a new frame; HSYNC (low enable) marks a new line; in DE mode, DE (high enable) marks valid
RGB data. In SYNC mode, valid data is input per pixel via D[17:0] according to HFP/HBP (HSYNC) and
VFP/VBP (VSYNC). In both modes input display data is written to GRAM first, then the source
voltage is output from the gray data in GRAM.

#### Table 10 — RGB interface timing parameters

| Parameter | Symbol | Min. | Typ. | Max. | Units |
|-----------|--------|------|------|------|-------|
| Horizontal Synchronization | Hsync | 2 | 10 | 16 | DOTCLK |
| Horizontal Back Porch | HBP | 2 | 20 | 24 | DOTCLK |
| Horizontal Address | HAdr | - | 320 | - | DOTCLK |
| Horizontal Front Porch | HFP | 2 | 10 | 16 | DOTCLK |
| Vertical Synchronization | Vsync | 1 | 2 | 4 | Line |
| Vertical Back Porch | VBP | 1 | 2 | - | Line |
| Vertical Address | VAdr | - | 240 | - | Line |
| Vertical Front Porch | VFP | 3 | 4 | - | Line |

> Notes: (1) Vertical period (one frame) = VBP + VAdr + VFP. (2) Horizontal period (one line) =
> HBP + HAdr + HFP. (3) Hsync must be transmitted as specified at all times while valid pixels are
> transferred.

#### 4.2.2. RGB Interface Timing

- **SYNC mode timing (Figures 33):** Mode 1 (RCM[1:0]="11", SYNC) and Mode 2 (RCM[1:0]="10",
  SYNC+DE). Active area is framed by VBP/VFP (vertical) and HBP/HFP (horizontal). DE is not needed
  in SYNC mode. Settings: VSPL=0, HSPL=0, DPL=0, EPL=0 of "Interface Mode Control (B0h)".
- **6-bit RGB mode timing (Figure 34):** Used only in the DE interface. Each dot (R, G, B) is
  transferred in sync with DOTCLK. VLW = VSYNC low width (≥1H); HLW = HSYNC low width (≥2 DOTCLKs);
  DTST = data transfer startup time (≥HLW). Set VSYNC, HSYNC and DE cycles to multiples of 3 DOTCLKs.

### 4.3. VSYNC Interface

GC9A01 supports a VSYNC interface synchronized with the frame-sync signal VSYNC to display moving
pictures over the 8080-I/8080-II system interface. Enable it by setting DM[1:0]="10" and RM="0".
In VSYNC mode the ENABLE pin must connect to IOVCC. The frame rate is determined by the VSYNC pulse
rate; all display data is stored in GRAM to minimize transfer for moving picture display.

Notes:
1. The minimum GRAM write speed must be satisfied; account for frequency variation.
2. The VSYNC period must be longer than the scan period of an entire display.
3. Switching between internal clock mode (DM[1:0]="00") and VSYNC mode starts from the next VSYNC
   cycle (after the current frame completes).
4. Partial display, vertical scroll and interlaced scan are not available in VSYNC interface mode.

Transition procedure (Figure 38): to enter VSYNC mode set GRAM address, set DM[1:0]="10"/FM=0, set
index register to 2Ch, wait > 1 frame, then write data through the VSYNC interface. To return, set
DM[1:0]="00"/FM=0, wait > 1 frame (input VSYNC for > 1 frame period after setting DM/RM).

### 4.4. Display Data RAM (DDRAM)

GC9A01 has an integrated 320x240x18-bit graphic-type static RAM (172,800 bytes), storing a
320xRGBx240 image at 18-bit resolution (262K-color). Simultaneous panel display read and interface
read/write to the same frame-memory location cause no abnormal visible effect.

### 4.5. Display Data Format

GC9A01 supplies 18-/16-/9-/8-bit parallel MCU interface (8080-I/8080-II), 3-/4-line serial
interface and 6-/16-/18-bit parallel RGB interface. Parallel MCU and serial modes are selected by
IM[3:0]; RGB mode is selected by RCM[1:0].

#### 4.5.1. 3-line Serial Interface (IM[3:0]="1101")

Three color depths supported (MSB transmitted first; each 9-bit unit = `[D/CX][8 data bits]`):

- **12 bit/pixel, RGB 4,4,4 (4096 colors) — Figure 41:** Pixel order R[3:0], G[3:0], B[3:0]. MSBs
  are Rx3/Gx3/Bx3; LSBs are Rx0/Gx0/Bx0.
- **16 bit/pixel, RGB 5,6,5 (65,536 colors) — Figure 41:** Order R[4:0], G[5:0], B[4:0]. A 16→18-bit
  look-up table maps to frame memory. MSBs Rx4/Gx5/Bx4; LSBs Rx0/Gx0/Bx0.
- **18 bit/pixel, RGB 6,6,6 (262,144 colors) — Figure 42:** Order R[5:0], (2 don't-care), G[5:0],
  (2 don't-care), B[5:0], (2 don't-care). MSBs Rx5/Gx5/Bx5; LSBs Rx0/Gx0/Bx0.

('-' = don't care, may be 0 or 1.)

#### 4.5.2. 4-line Serial Interface (IM[3:0]="1111")

Same three color depths (4k RGB444, 65k RGB565, 262k RGB666) as the 3-line interface, but the D/CX
line carries the command/data select instead of a prefixed bit, so each transmission unit is a
plain 8-bit byte. (Figures 43, 44 show Interface I/II wiring.)

**4-line SPI pixel formats (Figures 44–46):** identical color orders to the 3-line interface —
RGB444 (12bpp), RGB565 (16bpp, with 16→18-bit LUT), RGB666 (18bpp). Bytes are sent MSB-first with
D/CX as a separate line.

#### 4.5.3. 2-data-line mode

Active when `2data_en (E9h[3]) = 1` in 3-wire mode. Only frame pixel-data writes use 2 data lines;
register write/read still use 3-wire. nCS (active low) enables the interface; SCL is the clock; SDA
and DCX(RS) are the two serial data lines. Data is read on the rising edge of SCL. The first bit A0
is the data/command flag and must be "1" for display data. SDA carries `A0, D15..D8…`; DCX(RS)
carries `A0, D7..D0…`. Five formats, selected by `2data_mdt (E9h[2:0])`:

| 2data_mdt | Format | Throughput |
|-----------|--------|------------|
| 000 | RGB565 (65K color) | 1 pixel / transition |
| 001 | RGB666 (262K color) | 1 pixel / transition |
| 010 | RGB666 (262K color) | 2/3 pixel / transition |
| 100 | RGB888 (4M color) | 1 pixel / transition |
| 110 | RGB888 (4M color) | 2/3 pixel / transition |

- RGB565 1px/trans (Fig 48): SDA = A0,R4,R3,R2,R1,R0,G5,G4,G3; RS = A0,G2,G1,G0,B4,B3,B2,B1,B0.
- RGB666 1px/trans (Fig 49): SDA = A0,R5,R4,R3,R2,R1,R0,G5,G4,G3; RS = A0,G2,G1,G0,B5,B4,B3,B2,B1,B0.
- RGB666 2/3px/trans (Fig 50) and RGB888 variants (Figs 51, 52) interleave sub-pixels across SDA/RS
  in repeating 7-bit (A0 + 6 data) groups.

#### 4.5.4. 8-bit Parallel MCU Interface

- **8080-I, IM[3:0]="0000"** — D[7:0] used.
- **8080-II, IM[3:0]="1001"** — D[17:10] used.

The first transfer (Count 0, D/CX=0) is the command byte C7..C0. Subsequent transfers (D/CX=1) are
pixel data. Per-pixel structure:

- **65K RGB565, DBI[2:0]=101 — 2 bytes/pixel** (Tables 11/13). For pixel `n` the two bytes map (MSB→LSB):
  - Byte A: `nR4, nR3, nR2, nR1, nR0, nG5, nG4, nG3`
  - Byte B: `nG2, nG1, nG0, nB4, nB3, nB2, nB1, nB0`
- **262K RGB666, DBI[2:0]=110 — 3 bytes/pixel** (Tables 12/14). For pixel `n` (each byte uses the top
  6 bits D7..D2 in 8080-I, D17..D12 in 8080-II; lower 2 bits unused on data bytes):
  - Byte A: `nR5, nR4, nR3, nR2, nR1, nR0`
  - Byte B: `nG5, nG4, nG3, nG2, nG1, nG0`
  - Byte C: `nB5, nB4, nB3, nB2, nB1, nB0`

#### 4.5.5. 9-bit Parallel MCU Interface

- **8080-I, IM[3:0]="0010"** — D[8:0] used.
- **8080-II, IM[3:0]="1011"** — D[17:9] used.

262K RGB666, DBI[2:0]=110: 2 pixels (6 sub-pixels) sent in 4 transfers (Tables 15/16). For 8080-I
the 9 lines D8..D0 are loaded; the command byte (Count 0) occupies D7..D0 (C7..C0). The repeating
4-transfer group encodes pixel n then n+1 sub-pixels R5..R0, G5..G0, B5..B0 packed across D8..D0.

#### 4.5.6. 16-bit Parallel MCU Interface

- **8080-I, IM[3:0]="0001"** — D[15:0] used; D[17:16] unused.
- **8080-II, IM[3:0]="1000"** — D[17:2] used.

The command (Count 0) loads C7..C0 on the low byte (D7..D0 / D9..D2).

- **65K RGB565, DBI[2:0]=101 — 1 transfer/pixel** (Table 17). For pixel `n`, D15..D0 =
  `nR4,nR3,nR2,nR1,nR0,nG5,nG4,nG3,nG2,nG1,nG0,nB4,nB3,nB2,nB1,nB0`.
- **262K RGB666, DBI[2:0]=110 — 2 transfers/pixel**, with packing controlled by MDT[1:0]:
  - MDT="00" (Table 18): 3 pixels in 6 transfers (interleaved R/G/B halves across two words).
  - MDT="01" (Table 19): 2 transfers/pixel; word1 D15..D10 = R5..R0, D7..D2 = G5..G0; word2
    D15..D10 = B5..B0.
  - MDT="10" (Table 20): R on D15..D10, G on D9..D4, B on D3..D0+D15..D14 of next — packed tight.
  - MDT="11" (Table 21): R on D15..D12, G on D11..D6, B on D5..D2, plus R5/R4 & B1/B0 on D1/D0.

> These MDT variants differ only in how the 18 bits per pixel are distributed across the 16-bit
> bus over 2 transfers; all encode the same RGB666 data.

*(8080-II 16-bit RGB666 MDT variants — Tables 22–26 — follow the same scheme as the 8080-I MDT
tables above, with the command byte on the low byte and R/G/B sub-pixels distributed across
D17..D1 over 2 transfers per pixel.)*

#### 4.5.7. 18-bit Parallel MCU Interface

- **8080-I, IM[3:0]="0011"** — D[17:0] used.
- **8080-II, IM[3:0]="1010"** — D[17:0] used.

One pixel sent in **1 transfer** for both color depths. Command byte (Count 0) on D7..D0 (8080-I)
or D8..D1 (8080-II).

- **65K RGB565, DBI[2:0]=101** (Tables 27/29): per pixel D15..D0 =
  `R4,R3,R2,R1,R0, G5,G4,G3,G2,G1,G0, B4,B3,B2,B1,B0`. (8080-I: D17..D16 unused. 8080-II shifts up
  one: data on D15..D0 with command on D8..D1.)
- **262K RGB666, DBI[2:0]=110** (Tables 28/30): per pixel D17..D0 =
  `R5..R0, G5..G0, B5..B0`.

#### 4.5.8. 6-bit Parallel RGB Interface

Selected by RIM="1". With RCM[1:0]="10" (DE mode) display syncs with VSYNC/HSYNC/DOTCLK; data
enters GRAM via D[5:0] gated by DE. With RCM[1:0]="11" (SYNC mode) valid data is input per pixel per
VFP/VBP and HFP/HBP. Unused pins → GND. Registers set via SPI.

- **262K RGB666 (Figure 60):** 3 DOTCLK transfers per pixel on D[5:0] —
  1st = `R5..R0`, 2nd = `G5..G0`, 3rd = `B5..B0`.

A transfer counter (1st/2nd/3rd) resets to "1st" on the falling edge of VSYNC. The number of DOTCLK
inputs per frame must be a multiple of 3, or the current and next frame are affected.

#### 4.5.9. 16-bit Parallel RGB Interface

Selected by DPI[2:0]="101". DE mode (RCM="10") or SYNC mode (RCM="11"). Data on D[17:13] & D[11:0];
unused D12 and D0 → GND. A 16→18-bit look-up table maps to GRAM (Figure 62):
input `D17,D16,D15,D14,D13, D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1` → `R5..R0, G5..G0, B5..B0`.

#### 4.5.10. 18-bit Parallel RGB Interface

Selected by DPI[2:0]="110". DE mode (RCM="10") or SYNC mode (RCM="11"). Data on D[17:0] (Figure 63):
`D17..D0` → `R5..R0, G5..G0, B5..B0`. Registers set via SPI.

---

## 5. Function Description

### 5.1. Display data GRAM mapping

The display data RAM stores display dots: 1,382,400 bits (320 x 18 x 240 bits). RAM can be accessed
even while the same address is loaded to the DAC, with no abnormal visible effect on simultaneous
panel read and interface read/write to the same location.

Every 18-bit pixel in GRAM is located by a (Page, Column) address (Y, X). By specifying an arbitrary
window (SC, EC and SP, EP bits) the GRAM is accessed via RAMWR/RAMRD from the window start position.

**Table 31 — GRAM address vs display panel position:** addresses run from (Page,Column) = (00,00)h
at top-left to (EF,13F)h at bottom-right. Columns 00h..13Fh (0..319), Pages 00h..EFh (0..239).

### 5.2. MCU to memory write/read direction

The counter that dictates the physical memory write location is controlled by the "Memory Access
Control" command (36h) bits D5 (MV), D6 (MX), D7 (MY):

| D5 | D6 | D7 | CASET (column) | PASET (page) |
|----|----|----|----------------|--------------|
| 0 | 0 | 0 | Direct to Physical Column Pointer | Direct to Physical Page Pointer |
| 0 | 0 | 1 | Direct to Physical Column Pointer | Direct to (239 - Physical Page Pointer) |
| 0 | 1 | 0 | Direct to (239 - Physical Column Pointer) | Direct to Physical Page Pointer |
| 0 | 1 | 1 | Direct to (239 - Physical Column Pointer) | Direct to (239 - Physical Page Pointer) |
| 1 | 0 | 0 | Direct to Physical Page Pointer | Direct to Physical Column Pointer |
| 1 | 0 | 1 | Direct to (239 - Physical Page Pointer) | Direct to Physical Column Pointer |
| 1 | 1 | 0 | Direct to Physical Page Pointer | Direct to (239 - Physical Column Pointer) |
| 1 | 1 | 1 | Direct to (239 - Physical Page Pointer) | Direct to (239 - Physical Column Pointer) |

**Counter behavior:**

| Condition | Column Counter | Page Counter |
|-----------|----------------|--------------|
| When RAMWR/RAMRD command is accepted | Return to "Start column" | Return to "Start Page" |
| Complete pixel read/write action | Increment by 1 | No change |
| Column value is larger than "End Column" | Return to "Start column" | Increment by 1 |
| Page counter is larger than "End Page" | Return to "Start column" | Return to "Start Page" |

### 5.3. GRAM to display address mapping

Setting SS reverses the source-output-channel to GRAM-address relation (horizontal flip). Setting GS
reverses the gate-output-channel to GRAM-address relation (vertical flip). Setting BGR reverses the
R/G/B dot allocation for different LCD color filter arrangements. GC9A01 supports Normal, Partial and
Scrolling display modes.

#### 5.3.1. Normal display on / partial mode on, vertical scroll off

Frame memory in the area where column pointer is 0000h..00EFh and page pointer is 0000h..00EFh is
displayed. The leftmost-top dot is at (column, page) = (0,0).

#### 5.3.2. Vertical scroll display mode

Setting R37h activates scrolling mode; vertical scrolling is specified by TFA, VSA, BFA bits (R33h)
and VSP bits (R37h).

#### 5.3.3. Updating order on display active area in RGB interface mode

For RGB interface (RCM[1:0]="1x"), the host-to-display data stream order is controlled by MY and MX:
MY=0/MX=0, MY=0/MX=1, MY=1/MX=0, MY=1/MX=1 set the start/end corners accordingly (Figures 75–78).

**Table 37 — Updating order rules:**

| Condition | Horizontal Counter | Vertical Counter |
|-----------|--------------------|--------------------|
| An active VS signal is received | Return to 0 | Return to 0 |
| Single pixel of the active area is received | Increment by 1 | No change |
| An active HS signal between two active-area lines | Return to 0 | Increment by 1 |
| Horizontal counter > X and Vertical counter > Y | Return to 0 "Start Column" | Return to "Start Page" |

> Note: Pixel order is RGB on the display.

### 5.4. Tearing effect output line

The Tearing Effect (TE) output line supplies a panel synchronization signal to the MPU. It is
enabled/disabled by the Tearing Effect Line Off (34h) & On (35h) commands; its mode is defined by
the parameter of the On command. The MPU uses it to synchronize frame memory writing for video.

#### 5.4.1. Tearing effect line modes

- **Mode 1 (M=0):** TE signal consists of V-Blanking information only. tVdh = LCD not updated from
  frame memory; tVdl = LCD updated from frame memory (except invisible line).
- **Mode 2 (M=1):** TE signal consists of V-Blanking and H-Blanking information — one V-sync and 240
  H-sync pulses per field. thdh = not updated; thdl = updated.

#### 5.4.2. Tearing effect line timing — Table 38

Idle Mode Off (Frame Rate = 20~40 Hz):

| Symbol | Parameter | Min. | Max. | Unit |
|--------|-----------|------|------|------|
| tvdl | Vertical Timing Low Duration | TBD | - | ms |
| tvdh | Vertical Timing High Duration | 1000 | - | µs |
| thdl | Horizontal Timing Low Duration | TBD | - | µs |
| thdh | Horizontal Timing High Duration | TBD | 500 | µs |

> Note: At Idle Mode Off (20~40 Hz), the signal's rise/fall times (tf, tr) must be ≤ 15ns (measured
> between 0.2*IOVCC and 0.8*IOVCC). The TE line is fed back to the MCU to avoid tearing.

### 5.5. Source driver

GC9A01 contains 360 source driver channels (S1~S360) for driving the TFT LCD source lines. It
converts digital GRAM data into analog voltage for 360 channels, generating the corresponding
grayscale voltage output for a 262K-color display. The output circuit incorporates an operational
amplifier so each channel can alternately output positive and negative voltage.

### 5.6. Gate driver

GC9A01 contains 32 gate driver channels (G1~G32). The gate level is VGH when scanning a line and VGL
for other lines.

### 5.7. Scan mode setting

- **GS:** sets gate driver scan direction; GS=0 can be reversed by GS=1.
- **SM:** sets the gate driver pin arrangement, combined with GS, to select the optimal scan mode.
  (Table 39 in the original is a graphical scan-mode arrangement diagram.)

### 5.8. LCD power generation circuit

#### 5.8.1. Power supply circuit

The power circuit generates the LCD panel driving supply voltages from VCI/VSSB/VSSR via reference
voltage generation circuits and three step-up circuits, producing AVDD, AVEE, BVDD, BVEE, VGH, VGL
(Figure 83).

#### 5.8.2. LCD power generation scheme

Boost voltages generated (from VCI = 2.4~3.3V; VSSB/VSSR = 0V) — Figure 84:

| Rail | Value |
|------|-------|
| VGH | 5 × VCI |
| AVDD | 2 × VCI |
| AVEE | -2 × VCI |
| BVEE | -2 × VCI |
| VCL | -VCI |
| VGL | -4 × VCI |

VREGP and VREGN are derived as the highest positive/negative grayscale reference voltages.

### 5.9. Gamma Correction

GC9A01 incorporates γ-correction to display 262,144 colors. It uses 3 groups of registers
determining eight reference grayscale levels (gradient adjustment, amplitude adjustment, and
fine-adjustment registers for positive and negative polarities), to suit panels of various
characteristics. Grayscale DACs map register points VP0..VP63 / VN0..VN63 to output levels V0..V63
between VREG1A/VREG1B (and VREGP/VREGN) — Figures 85, 86. Source output uses dot inversion relative to
a DC VCOM (Figures 87, 88): V0 corresponds to GRAM data 000000, V63 to 111111.

### 5.10. Power Level Definition

#### 5.10.1. Power Levels

Six modes, from maximum to minimum power consumption:
1. Normal Mode On (full display), Idle Off, Sleep Out — up to 262,144 colors.
2. Partial Mode On, Idle Off, Sleep Out — part of display, up to 262,144 colors.
3. Normal Mode On (full display), Idle On, Sleep Out — full display, 8 colors.
4. Partial Mode On, Idle On, Sleep Out — part of display, 8 colors.
5. Sleep In Mode — DC/DC converter, internal oscillator and panel driver stopped; only MCU interface
   and memory work (IOVCC). Memory contents are safe.
6. Power Off Mode — both VCI and IOVCC removed.

> Note: Transitions between modes 1–5 are controllable by MCU commands; mode 6 is entered only when
> both power supplies are removed.

#### 5.10.2. Power Flow Chart

Transitions (Figure 89) between Normal/Partial (NORON/PLTON), Idle On/Off (IDMON/IDMOFF) and Sleep
In/Out (SLPIN/SLPOUT) states, reached from the Power-On sequence / HW reset / SW reset. No abnormal
visual effect occurs when changing power modes.

#### 5.10.3. Brightness control block

An external LEDPWM output controls the LED driver IC for display brightness. Register bits DBV[7:0]
(R51h) set manual brightness. LEDPWM duty = DBV[7:0]/255 × period (period affected by OSC frequency).
Example: period = 3ms, DBV = 200 → duty = 200/255 = 78.1% → high level 2.344ms, low level 0.656ms
(Figure 90).

### 5.11. Input/output pin state

#### 5.11.1. Output pins — Table 40

| Output / Bi-directional pins | After Power On | After Hardware Reset |
|------------------------------|----------------|----------------------|
| DB17 to DB0 (output driver) | High-Z (Inactive) | High-Z (Inactive) |
| SDA | High-Z (Inactive) | High-Z (Inactive) |
| SDO | High-Z (Inactive) | High-Z (Inactive) |
| TE | Low | Low |
| LEDPWM | Low | Low |

#### 5.11.2. Input pins — Table 41

| Input pins | During Power On Process | After Power On | After Hardware Reset | During Power Off Process |
|------------|-------------------------|----------------|----------------------|--------------------------|
| RESX | Input valid | Input valid | Input valid | Input valid |
| CSX | Input invalid | Input valid | Input valid | Input invalid |
| WRX | Input invalid | Input valid | Input valid | Input invalid |
| RDX | Input invalid | Input valid | Input valid | Input invalid |
| D/CX | Input invalid | Input valid | Input valid | Input invalid |
| SDA | Input invalid | Input valid | Input valid | Input invalid |
| VSYNC | Input invalid | Input valid | Input valid | Input invalid |
| HSYNC | Input invalid | Input valid | Input valid | Input invalid |
| DE | Input invalid | Input valid | Input valid | Input invalid |
| DOTCLK | Input invalid | Input valid | Input valid | Input invalid |
| D[17:0] | Input invalid | Input valid | Input valid | Input invalid |
| IM[3:0] | Input invalid | Input valid | Input valid | Input invalid |

---

## 6. Command

For all commands: D/CX=0 marks the command byte (written on rising WRX); D/CX=1 marks
parameters. RDX rising edge clocks reads. "↑" = active (rising) edge. The HEX column gives each
parameter's reset default. In the per-command bit tables, the **D17-8** column ("XX") covers data
bits D17..D8 (unused for 8-bit serial transfers); multi-bit fields are expanded one bit per column
so any value can be read directly off the row/column intersection.

**Register Availability (applies to every command in 6.2/6.3/6.4 unless noted):** each command is
available ("Yes") in all five operating states — Normal Mode On/Idle Off/Sleep Out, Normal Mode
On/Idle On/Sleep Out, Partial Mode On/Idle Off/Sleep Out, Partial Mode On/Idle On/Sleep Out, and
Sleep In. **Exception:** Vertical Scrolling Start Address (37h) is **not** available in the two
Partial Mode states.

**Defaults:** the per-command bit table's HEX column lists each parameter's reset value (Power On /
SW Reset / HW Reset are identical unless stated otherwise in the command's notes).

### 6.1. Command List

#### Regulative (Level 1) Command Set

| Command | Hex | Parameters (with default) |
|---------|-----|---------------------------|
| Read Display Identification Information | 04h | dummy; ID1[7:0]=00; ID2[7:0]=9A; ID3[7:0]=01 (read) |
| Read Display Status | 09h | dummy; D[31:25]; D[22:16]=61; D[10:8]; D[7:5] (read, 32-bit) |
| Enter Sleep Mode | 10h | none |
| Sleep Out | 11h | none |
| Partial Mode ON | 12h | none |
| Normal Display Mode ON | 13h | none |
| Display Inversion OFF | 20h | none |
| Display Inversion ON | 21h | none |
| Display OFF | 28h | none |
| Display ON | 29h | none |
| Column Address Set | 2Ah | SC[15:8]=00, SC[7:0]=00, EC[15:8]=01, EC[7:0]=3F |
| Page (Row) Address Set | 2Bh | SP[15:8]=00, SP[7:0]=00, EP[15:8]=00, EP[7:0]=EF |
| Memory Write | 2Ch | D[17:0] pixel data (repeated) |
| Partial Area | 30h | SR[15:8]=00, SR[7:0]=00, ER[15:8]=00, ER[7:0]=EF |
| Vertical Scrolling Definition | 33h | TFA[15:0]=0000, VSA[15:0]=00F0, BFA[15:0] |
| Tearing Effect Line OFF | 34h | none |
| Tearing Effect Line ON | 35h | bit0 M=0 (TE mode) |
| Memory Access Control | 36h | MY,MX,MV,ML,BGR,MH,-,- = 00 |
| Vertical Scrolling Start Address | 37h | VSP[15:8]=00, VSP[7:0]=00 |
| Idle Mode OFF | 38h | none |
| Idle Mode ON | 39h | none |
| Pixel Format Set (COLMOD) | 3Ah | DPI[2:0] (RGB i/f), DBI[2:0] (MCU i/f) = 66 |
| Write Memory Continue | 3Ch | D[17:0] pixel data (repeated) |
| Set Tear Scanline | 44h | STS[8]=0, STS[7:0]=00 |
| Get Scanline | 45h | dummy; GTS[8]; GTS[7:0] (read) |
| Write Display Brightness | 51h | DBV[7:0]=00 |
| Write CTRL Display | 53h | -,-,BCTRL,-,DD,BL,-,- = 00 |
| Read ID1 | DAh | dummy; module/driver ID[7:0]=00 (read) |
| Read ID2 | DBh | dummy; module/driver ID[7:0]=9A (read) |
| Read ID3 | DCh | dummy; module/driver ID[7:0]=01 (read) |

#### Extended (Level 2 & 3) Command Set

| Command | Hex | Parameters (with default) |
|---------|-----|---------------------------|
| RGB Interface Signal Control | B0h | RCM[1:0], VSPL, HSPL, DPL, EPL = 01 |
| Blanking Porch Control | B5h | VFP[6:0]=08, VBP[6:0]=02, HBP[4:0]=14 |
| Display Function Control | B6h | GS, SS, SM (=00); NL[5:0]=1D |
| Tearing Effect Control | BAh | te_pol, te_width[6:0] = 00 |
| Interface Control | F6h | (see 6.3.5) |
| Power Criterion Control | (A7h Power Control 7) | see 6.4.7 |
| Vreg1a voltage Control | C3h | vreg1_vbp_d[6:0] = 3C |
| Vreg1b voltage Control | C4h | vreg1_vbn_d[6:0] = 3C |
| Vreg2a voltage Control | C9h | vrh[5:0] = 28 |
| Frame Rate | E8h | DINV[2:0], RTN1[3:0]=11; RTN2[7:0]=40 |
| SPI 2data control | E9h | 2data_en (bit3), 2data_mdt[2:0] = 00 |
| Charge Pump Frequent Control | ECh | avdd_clk_ad/avee_clk_ad=33; vcl_clk_ad=02; vgh_clk_ad/vgl_clk_ad=88 |
| Power Control 1 | C1h | see 6.4.3 |
| Power Control 2 | C3h | see 6.4.4 |
| Power Control 3 | C4h | see 6.4.5 |
| Power Control 4 | C9h | see 6.4.6 |
| Inter Register Enable 1 | FEh | none (enable) |
| Inter Register Enable 2 | EFh | none (enable) |
| SET_GAMMA1 | F0h | 6 bytes (negative gamma group, default 80 03 08 06 05 2B) |
| SET_GAMMA2 | F1h | 6 bytes (negative gamma group, default 41 97 98 13 17 CD) |
| SET_GAMMA3 | F2h | 6 bytes (positive gamma group, default 40 03 08 0B 08 2E) |
| SET_GAMMA4 | F3h | 6 bytes (positive gamma group, default 3F 98 B4 14 18 CD) |

### 6.2. Description of Level 1 Commands

#### 6.2.1. Read Display Identification Information (04h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 04h |
| 1st Param | 1 | ↑ | 1 | XX | X | X | X | X | X | X | X | X | XX |
| 2nd Param | 1 | ↑ | 1 | XX | ID1.7 | ID1.6 | ID1.5 | ID1.4 | ID1.3 | ID1.2 | ID1.1 | ID1.0 | 00 |
| 3rd Param | 1 | ↑ | 1 | XX | ID2.7 | ID2.6 | ID2.5 | ID2.4 | ID2.3 | ID2.2 | ID2.1 | ID2.0 | 9A |
| 4th Param | 1 | ↑ | 1 | XX | ID3.7 | ID3.6 | ID3.5 | ID3.4 | ID3.3 | ID3.2 | ID3.1 | ID3.0 | 01 |

**Description:**

This read byte returns 24 bits display identification information. The 1st parameter is dummy data. The 2nd parameter (ID2_1 [7:0]): LCD module’s manufacturer ID. The 3rd parameter (ID2_2 [7:0]): LCD module/driver version ID. The 4th parameter (ID2_3 [7:0]): LCD module/driver ID.

**Restriction:** None

#### 6.2.2. Read Display Status (09h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 09h |
| 1st Param | 1 | ↑ | 1 | XX | X | X | X | X | X | X | X | X | XX |
| 2nd Param | 1 | ↑ | 1 | XX | D31 | D30 | D29 | D28 | D27 | D26 | D25 | X | 00 |
| 3rd Param | 1 | ↑ | 1 | XX | 0 | D22 | D21 | D20 | D19 | D18 | D17 | D16 | 61 |
| 4th Param | 1 | ↑ | 1 | XX | 0 | 0 | 0 | 0 | 0 | D10 | D9 | D8 | 00 |
| 5th Param | 1 | ↑ | 1 | XX | D7 | D6 | D5 | 0 | 0 | 0 | 0 | 0 | 00 |

**Description:**

This command indicates the current status of the display as described in the table below:

| Bit | Description | Value | Status |
|-----|-------------|-------|--------|
| D31 | Booster voltage status | 0 | Booster OFF |
| D31 | Booster voltage status | 1 | Booster ON |
| D30 | Row address order | 0 | Top to Bottom (When MADCTL B7='0') |
| D30 | Row address order | 1 | Bottom to Top (When MADCTL B7='1') |
| D29 | Column address order | 0 | Left to Right (When MADCTL B6='0') |
| D29 | Column address order | 1 | Right to Left (When MADCTL B6='1') |
| D28 | Row/column exchange | 0 | Normal Mode (When MADCTL B5='0') |
| D28 | Row/column exchange | 1 | Reverse Mode (When MADCTL B5='1') |
| D27 | Vertical refresh | 0 | LCD Refresh Top to Bottom (When MADCTL B4='0') |
| D27 | Vertical refresh | 1 | LCD Refresh Bottom to Top (When MADCTL B4='1') |
| D26 | RGB/BGR order | 0 | RGB (When MADCTL B3='0') |
| D26 | RGB/BGR order | 1 | BGR (When MADCTL B3='1') |
| D25 | Horizontal refresh order | 0 | LCD Refresh Left to Right (When MADCTL B2='0') |
| D25 | Horizontal refresh order | 1 | LCD Refresh Right to Left (When MADCTL B2='1') |
| D24 | Not used | 0 | - |
| D23 | Not used | 0 | - |
| D22 D21 D20 | Interface color pixel format definition | 101 | 16-bit/pixel |
| D22 D21 D20 | Interface color pixel format definition | 110 | 18-bit/pixel |
| D19 | Idle mode ON/OFF | 0 | Idle Mode OFF |
| D19 | Idle mode ON/OFF | 1 | Idle Mode ON |
| D18 | Partial mode ON/OFF | 0 | Partial Mode OFF |
| D18 | Partial mode ON/OFF | 1 | Partial Mode ON |
| D17 | Sleep IN/OUT | 0 | Sleep IN Mode |
| D17 | Sleep IN/OUT | 1 | Sleep OUT Mode |
| D16 | Display normal mode ON/OFF | 0 | Display Normal Mode OFF |
| D16 | Display normal mode ON/OFF | 1 | Display Normal Mode ON |
| D15 | Vertical scrolling status | 0 | Scroll OFF |
| D14 | Not used | 0 | - |
| D13 | Inversion status | 0 | Not defined |
| D12 | All pixel ON | 0 | Not defined |
| D11 | All pixel OFF | 0 | Not defined |
| D10 | Display ON/OFF | 0 | - |
| D10 | Display ON/OFF | 1 | Display is ON |
| D9 | Tearing effect line ON/OFF | 0 | Tearing Effect Line OFF |
| D9 | Tearing effect line ON/OFF | 1 | Tearing Effect ON |
| D5 | Tearing effect line mode | 0 | Mode 1, V-Blanking only |
| D5 | Tearing effect line mode | 1 | Mode 2, both H-Blanking and V-Blanking |
| D4 | Not used | 0 | - |
| D3 | Not used | 0 | - |
| D2 | Not used | 0 | - |
| D1 | Not used | 0 | - |
| D0 | Not used | 0 | - |

**Restriction:** None

#### 6.2.3. Enter Sleep Mode (10h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 10h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command causes the LCD module to enter the minimum power consumption mode. In this mode e.g. the DC/DC converter is stopped, Internal oscillator is stopped, and panel scanning is stopped Out Blank STOP MCU interface and memory are still working and the memory keeps its contents. X = Don’t care

**Restriction:** This command has no effect when module is already in sleep in mode. Sleep In Mode can only be left by the Sleep Out Command (11h). It will be necessary to wait 5msec before sending next to command, this is to allow time for the supply voltages and clock circuits to stabilize. It will be necessary to wait 120msec after sending Sleep Out command (when in Sleep In Mode) before Sleep In command can be sent.

#### 6.2.4. Sleep Out Mode (11h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 11h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command turns off sleep mode. the DC/DC converter is enabled, Internal oscillator is started, and panel scanning is started. X = Don’t care

**Restriction:** This command has no effect when module is already in sleep out mode. Sleep Out Mode can only be left by the Sleep In Command (10h). It will be necessary to wait 5msec before sending next command, this is to allow time for the supply voltages and clock circuits stabilize. The display module loads all display supplier’s factory default values to the registers during this 5msec and there cannot be any abnormal visual effect on the display image if factory default and register values are same when this load is done and when the display module is already Sleep Out –mode. The display module is doing self-diagnostic functions during this 5msec. It will be necessary to wait 120msec after sending Sleep In command (when in Sleep Out mode) before Sleep Out command can be sent.

#### 6.2.5. Partial Mode ON (12h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 0 | 12h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command turns on partial mode The partial mode window is described by the Partial Area command (30H). To leave Partial mode, the Normal Display Mode On command (13H) should be written. X = Don’t care

**Restriction:** This command has no effect when Partial mode is active.

#### 6.2.6. Normal Display Mode ON (13h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 1 | 13h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command returns the display to normal mode. Normal display mode on means Partial mode off. Exit from NORON by the Partial mode On command (12h) X = Don’t care

**Restriction:** This command has no effect when Normal Display mode is active.

#### 6.2.7. Display Inversion OFF (20h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 20h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to recover from display inversion mode. This command makes no change of the content of frame memory. This command doesn’t change any other status. memory Display Panel X = Don’t care

**Restriction:** This command has no effect when module already is inversion OFF mode.

#### 6.2.8. Display Inversion ON (21h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 | 21h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to enter into display inversion mode. This command makes no change of the content of frame memory. Every bit is inverted from the frame memory to the display. This command doesn’t change any other status. To exit Display inversion mode, the Display inversion OFF command (20h) should be written.. memory Display Panel X = Don’t care

**Restriction:** This command has no effect when module already is inversion ON mode.

#### 6.2.9. Display OFF (28h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 28h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to enter into DISPLAY OFF mode. In this mode, the output from Frame Memory is disabled and blank page inserted. This command makes no change of contents of frame memory. This command does not change any other status. There will be no abnormal visible effect on the display. memory Display Panel X = Don’t care

**Restriction:** This command has no effect when module is already in display off mode.

#### 6.2.10. Display ON (29h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 1 | 29h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to recover from DISPLAY OFF mode. Output from the Frame Memory is enabled. This command makes no change of contents of frame memory. This command does not change any other status. memory Display Panel X = Don’t care

**Restriction:** This command has no effect when module is already in display on mode.

#### 6.2.11. Column Address Set (2Ah)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 2Ah |
| 1st Param | 1 | 1 | ↑ | XX | SC15 | SC14 | SC13 | SC12 | SC11 | SC10 | SC9 | SC8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | SC7 | SC6 | SC5 | SC4 | SC3 | SC2 | SC1 | SC0 | 00 |
| 3rd Param | 1 | 1 | ↑ | XX | EC15 | EC14 | EC13 | EC12 | EC11 | EC10 | EC9 | EC8 | 01 |
| 4th Param | 1 | 1 | ↑ | XX | EC7 | EC6 | EC5 | EC4 | EC3 | EC2 | EC1 | EC0 | 3F |

**Description:**

This command is used to define area of frame memory where MCU can access. This command makes no change on the other driver status. The values of SC [15:0] and EC [15:0] are referred when RAMWR command comes. Each value represents one column line in the Frame Memory.. SC[15:0] EC[15:0] X = Don’t care

**Restriction:** SC [15:0] always must be equal to or less than EC [15:0]. Note 1: When SC [15:0] or EC [15:0] is greater than 013Fh (When MADCTL’s B5 = 0) or 00EFh (When MADCTL’s B5 = 1), data of out of range will be ignored

#### 6.2.12. Row (Page) Address Set (2Bh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 1 | 0 | 1 | 1 | 2Bh |
| 1st Param | 1 | 1 | ↑ | XX | SP15 | SP14 | SP13 | SP12 | SP11 | SP10 | SP9 | SP8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | SP7 | SP6 | SP5 | SP4 | SP3 | SP2 | SP1 | SP0 | 00 |
| 3rd Param | 1 | 1 | ↑ | XX | EP15 | EP14 | EP13 | EP12 | EP11 | EP10 | EP9 | EP8 | 00 |
| 4th Param | 1 | 1 | ↑ | XX | EP7 | EP6 | EP5 | EP4 | EP3 | EP2 | EP1 | EP0 | EF |

**Description:**

This command is used to define area of frame memory where MCU can access. This command makes no change on the other driver status. The values of SP [15:0] and EP [15:0] are referred when RAMWR command comes. Each value represents one Page line in the Frame Memory. Sc[15:0] EC[15:0] X = Don’t care

**Restriction:** SP [15:0] always must be equal to or less than EP [15:0] Note 1: When SP [15:0] or EP [15:0] is greater than 00EFh (When MADCTL’s B5 = 0) or 013Fh (When MADCTL’s B5 = 1), data of out of range will be ignored.

#### 6.2.13. Memory Write (2Ch)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 0 | 1 | 1 | 0 | 0 | 2Ch |
| 1st…Nth Param | 1 | 1 | ↑ | D[17:0] = pixel data D1…Dn (repeated) | | | | | | | | | XX |

**Description:**

This command is used to transfer data from MCU to frame memory. This command makes no change to the other driver status. When this command is accepted, the column register and the page register are reset to the Start Column/Start Page positions. The Start Column/Start Page positions are different in accordance with MADCTL setting.) Then D [17:0] is stored in frame memory and the column register and the page register incremented. Sending any other command can stop frame Write. X = Don’t care.

**Restriction:** In all color modes, there is no restriction on length of parameters.

#### 6.2.14. Partial Area (30h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | 30h |
| 1st Param | 1 | 1 | ↑ | XX | SR15 | SR14 | SR13 | SR12 | SR11 | SR10 | SR9 | SR8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | SR7 | SR6 | SR5 | SR4 | SR3 | SR2 | SR1 | SR0 | 00 |
| 3rd Param | 1 | 1 | ↑ | XX | ER15 | ER14 | ER13 | ER12 | ER11 | ER10 | ER9 | ER8 | 00 |
| 4th Param | 1 | 1 | ↑ | XX | ER7 | ER6 | ER5 | ER4 | ER3 | ER2 | ER1 | ER0 | EF |

**Description:**

This command defines the partial mode’s display area. There are 2 parameters associated with this command, the first defines the Start Row (SR) and the second the End Row (ER), as illustrated in the figures below. SR and ER refer to the Frame Memory Line Pointer. If End Row>Start Row when MADCTL B4=0:- Start Row SR[15:0] Partial Area Etart Row ER[15:0] If End Row>Start Row when MADCTL B4=1:- Etart Row ER[15:0] Partial Area Start Row SR[15:0] If End Row<Start Row when MADCTL B4=0:-

**Restriction:** SR [15…0] and ER [15…0] cannot be 0000h nor exceed 013Fh.

#### 6.2.15. Vertical Scrolling Definition (33h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 33h |
| 1st Param | 1 | 1 | ↑ | XX | TFA15 | TFA14 | TFA13 | TFA12 | TFA11 | TFA10 | TFA9 | TFA8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | TFA7 | TFA6 | TFA5 | TFA4 | TFA3 | TFA2 | TFA1 | TFA0 | 00 |
| 3rd Param | 1 | 1 | ↑ | XX | VSA15 | VSA14 | VSA13 | VSA12 | VSA11 | VSA10 | VSA9 | VSA8 | 00 |
| 4th Param | 1 | 1 | ↑ | XX | VSA7 | VSA6 | VSA5 | VSA4 | VSA3 | VSA2 | VSA1 | VSA0 | F0 |

**Description:**

This command defines the Vertical Scrolling Area of the display. When MADCTL B4=0 The 1st & 2nd parameter TFA [15...0] describes the Top Fixed Area (in No. of lines from Top of the Frame Memory and Display). The 3rd & 4th parameter VSA [15...0] describes the height of the Vertical Scrolling Area (in No. of lines of the Frame Memory [not the display] from the Vertical Scrolling Start Address). The first line read from Frame Memory appears immediately after the bottom most line of the Top Fixed Area. Top Fixed Area TFA[15:0] First line read from memory Scroll Area VSA[15:0] Bottom Fixed Area When MADCTL B4=1 The 1st & 2nd parameter TFA [15...0] describes the Top Fixed Area (in No. of lines from Bottom of the Frame Memory and Display). The 3rd & 4th parameter VSA [15...0] describes the height of the Vertical Scrolling Area (in No. of lines of the Frame Memory [not the display] from the Vertical Scrolling Start Address). The first line read from Frame Memory appears immediately after the top most line of the Top Fixed Area.

**Restriction:** None

#### 6.2.16. Tearing Effect Line OFF (34h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 1 | 0 | 0 | 34h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to turn OFF (Active Low) the Tearing Effect output signal from the TE signal line. X = Don’t care.

**Restriction:** This command has no effect when Tearing Effect output is already OFF.

#### 6.2.17. Tearing Effect Line ON (35h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 1 | 0 | 1 | 35h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | 0 | 0 | 0 | 0 | M | 00 |

**Description:**

This command is used to turn ON the Tearing Effect output signal from the TE signal line. This output is not affected by changing MADCTL bit B4. The Tearing Effect Line On has one parameter which describes the mode of the Tearing Effect Output Line. When M=0: The Tearing Effect Output line consists of V-Blanking information only: tvdl tvdh Vertical Time Scale When M=1: The Tearing Effect Output Line consists of both V-Blanking and H-Blanking information: tvdl tvdh Vertical Time Scale Note: During Sleep In Mode with Tearing Effect Line On, Tearing Effect Output pin will be active Low. X = Don’t care.

**Restriction:** This command has no effect when Tearing Effect output is already ON

#### 6.2.18. Memory Access Control (36h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 1 | 1 | 0 | 36h |
| 1st Param | 1 | 1 | ↑ | XX | MY | MX | MV | ML | BGR | MH | 0 | 0 | 00 |

**Description:**

This command defines read/write scanning direction of frame memory. This command makes no change on the other driver status.

| Bit | Name | Description |
|-----|------|-------------|
| MY | Row Address Order | These 3 bits control MCU to memory write/read direction. |
| MX | Column Address Order | These 3 bits control MCU to memory write/read direction. |
| MV | Row / Column Exchange | These 3 bits control MCU to memory write/read direction. |
| ML | Vertical Refresh Order | LCD vertical refresh direction control. |
| BGR | RGB-BGR Order | Color selector switch control (0=RGB color filter panel, 1=BGR color filter panel) |
| MH | Horizontal Refresh ORDER | LCD horizontal refreshing direction control. |

Note: When BGR bit is changed, the new setting is active immediately without update the content in Frame Memory again.

**Restriction:** Note: Top-Left (0,0) means a physical memory location. This command has no effect when Tearing Effect output is already ON

#### 6.2.19. Vertical Scrolling Start Address (37h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 0 | 1 | 1 | 1 | 37h |
| 1st Param | 1 | 1 | ↑ | XX | VSP15 | VSP14 | VSP13 | VSP12 | VSP11 | VSP10 | VSP9 | VSP8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | VSP7 | VSP6 | VSP5 | VSP4 | VSP3 | VSP2 | VSP1 | VSP0 | 00 |

**Description:**

This command is used together with Vertical Scrolling Definition (33h). These two commands describe the scrolling area and the scrolling mode. The Vertical Scrolling Start Address command has one parameter which describes the address of the line in the Frame Memory that will be written as the first line after the last line of the Top Fixed Area on the display as illustrated below:- When MADCTL B4=0 Example: When Top Fixed Area = Bottom Fixed Area = 00, Vertical Scrolling Area = 240 and VSP=’3’. When MADCTL B4=1 Example: When Top Fixed Area = Bottom Fixed Area = 00, Vertical Scrolling Area = 240 and VSP=’3’. Note: (1) When new Pointer position and Picture Data are sent, the result on the display will happen at the next Panel Scan to avoid tearing effect. VSP refers to the Frame Memory line Pointer. (2) This command is ignored when the GC9A01 enters Partial mode. X = Don’t care

**Restriction:** This command has no effect when Tearing Effect output is already ON

#### 6.2.20. Idle Mode OFF (38h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 38h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to recover from Idle mode on. In the idle off mode, LCD can display maximum 262,144 colors. X = Don’t care.

**Restriction:** This command has no effect when module is already in idle off mode.

#### 6.2.21. Idle Mode ON (39h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 1 | 39h |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used to enter into Idle mode on.
In the idle on mode, color expression is reduced. The primary and the secondary colors using MSB of each R, G and B in the Frame Memory, 8 color depth data is displayed.

Memory Contents vs. Display Color:

| Color | R5 R4 R3 R2 R1 R0 | G5 G4 G3 G2 G1 G0 | B5 B4 B3 B2 B1 B0 |
|-------|-------------------|-------------------|-------------------|
| Black | 0XXXXX | 0XXXXX | 0XXXXX |
| Blue | 0XXXXX | 0XXXXX | 1XXXXX |
| Red | 1XXXXX | 0XXXXX | 0XXXXX |
| Magenta | 1XXXXX | 0XXXXX | 1XXXXX |
| Green | 0XXXXX | 1XXXXX | 0XXXXX |
| Cyan | 0XXXXX | 1XXXXX | 1XXXXX |
| Yellow | 1XXXXX | 1XXXXX | 0XXXXX |
| White | 1XXXXX | 1XXXXX | 1XXXXX |

X = Don't care.

**Restriction:** This command has no effect when module is already in idle off mode.

#### 6.2.22. COLMOD: Pixel Format Set (3Ah)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 1 | 0 | 1 | 0 | 3Ah |
| 1st Param | 1 | 1 | ↑ | XX | 0 | DPI2 | DPI1 | DPI0 | 0 | DBI2 | DBI1 | DBI0 | 66 |

**Description:**

This command sets the pixel format for the RGB image data used by the interface. DPI [2:0] is the pixel format select of RGB interface and DBI [2:0] is the pixel format of MCU interface. If a particular interface, either RGB interface or MCU interface, is not used then the corresponding bits in the parameter are ignored. The pixel format is shown in the table below.

| DPI [2:0] | RGB Interface Format | DBI [2:0] | MCU Interface Format |
|-----------|----------------------|-----------|----------------------|
| 0 0 0 | Reserved | 0 0 0 | Reserved |
| 0 0 1 | Reserved | 0 0 1 | Reserved |
| 0 1 0 | Reserved | 0 1 0 | Reserved |
| 0 1 1 | Reserved | 0 1 1 | 12 bits / pixel |
| 1 0 0 | Reserved | 1 0 0 | Reserved |
| 1 0 1 | 16 bits / pixel | 1 0 1 | 16 bits / pixel |
| 1 1 0 | 18 bits / pixel | 1 1 0 | 18 bits / pixel |
| 1 1 1 | Reserved | 1 1 1 | Reserved |

If using RGB Interface must selection serial interface.
X = Don't care.

**Restriction:** This command has no effect when module is already in idle off mode.

#### 6.2.23. Write Memory Continue (3Ch)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 3Ch |
| 1st…Nth Param | 1 | 1 | ↑ | D[17:0] = pixel data D1…Dn (repeated) | | | | | | | | | XX |

**Description:**

This command transfers image data from the host processor to the display module’s frame memory continuing from the pixel location following the previous write_memory_continue or write_memory_start command. If set_address_mode B5 = 0: Data is written continuing from the pixel location after the write range of the previous write_memory_start or write_memory_continue. The column register is then incremented and pixels are written to the frame memory until the column register equals the End Column (EC) value. The column register is then reset to SC and the page register is incremented. Pixels are written to the frame memory until the page register equals the End Page (EP) value and the column register equals the EC value, or the host processor sends another command. If the number of pixels exceeds (EC –SC + 1) * (EP – SP + 1) the extra pixels are ignored. If set_address_mode B5 = 1: Data is written continuing from the pixel location after the write range of the previous write_memory_start or write_memory_continue. The page register is then incremented and pixels are written to the frame memory until the page register equals the End Page (EP) value. The page register is then reset to SP and the column register is incremented. Pixels are written to the frame memory until the column register equals the End column (EC) value and the page register equals the EP value, or the host processor sends another command. If the number of pixels exceeds (EC – SC + 1) * (EP –SP + 1) the extra pixels are ignored. Sending any other command can stop frame Write. Frame Memory Access and Interface setting (B3h), WEMODE=0 When the transfer number of data exceeds (EC-SC+1)*(EP-SP+1), the exceeding data will be ignored. Frame Memory Access and Interface setting (B3h), WEMODE=1

**Restriction:** A write_memory_start should follow a set_column_address, set_page_address or set_address_mode to define the write address. Otherwise, data written with write_memory_continue is written to undefined addresses.

#### 6.2.24. Set_Tear_Scanline (44h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 44h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | 0 | 0 | 0 | 0 | STS8 | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | STS7 | STS6 | STS5 | STS4 | STS3 | STS2 | STS1 | STS0 | 00 |

**Description:**

This command turns on the display Tearing Effect output signal on the TE signal line when the display reaches line equal the value of STS[8:0] tvdl tvdh Vertical Time Scale Note:that set_tear_scanline with STS is equivalent to set_tear_on with 8+GateN(N=1、2、 3...240) eg:when the STS[8:0]=8,the TE will output at the position of Gate1. when the STS[8:0]=9,the TE will output at the position of Gate2. when the STS[8:0]=10,the TE will output at the position of Gate3. ………. The Tearing Effect Output line shall be active low when the display module is in Sleep mode.

**Restriction:** The Tearing Effect Output line shall be active low when the display module is in Sleep mode.

#### 6.2.25. Get_Scanline (45h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 1 | 0 | 0 | 0 | 1 | 0 | 1 | 45h |
| 1st Param | 1 | ↑ | 1 | XX | 0 | 0 | 0 | 0 | 0 | 0 | 0 | GTS8 | 00 |
| 2nd Param | 1 | ↑ | 1 | XX | GTS7 | GTS6 | GTS5 | GTS4 | GTS3 | GTS2 | GTS1 | GTS0 | 00 |

**Description:**

This command returns the setting value of STS[8:0] . When in Sleep Mode, the value returned by get_scanline is undefined.

**Restriction:** None

#### 6.2.26. Write Display Brightness (51h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 1 | 51h |
| 1st Param | 1 | 1 | ↑ | XX | DBV7 | DBV6 | DBV5 | DBV4 | DBV3 | DBV2 | DBV1 | DBV0 | 00 |

**Description:**

This command is used to adjust the brightness value of the display. It should be checked what is the relationship between this written value and output brightness of the display. This relationship is defined on the display module specification. In principle relationship is that 00h value means the lowest brightness and FFh value means the highest brightness.

**Restriction:** None

#### 6.2.27. Write CTRL Display (53h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 0 | 1 | 0 | 1 | 0 | 0 | 1 | 1 | 53h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | 0 | BCTRL | 0 | DD | BL | 0 | 0 | 00 |

**Description:**

This command is used to return brightness setting. BCTRL: Brightness Control Block On/Off, ‘0’ = Off (Brightness registers are 00h) ‘1’ = On (Brightness registers are active, according to the DBV[7..0] parameters.) DD: Display Dimming ‘0’ = Display Dimming is off ‘1’ = Display Dimming is on BL: Backlight On/Off ‘0’ = Off (Completely turn off backlight circuit. Control lines must be low. ) ‘1’ = On


**Default Value (Power On / SW Reset / HW Reset):** BCTRL=1'b0, DD=1'b0, BL=1'b0.

**Restriction:** The display module is sending 2nd parameter value on the data lines if the MCU wants to read more than one parameter (= more than 2 RDX cycle) on DBI. Only 2nd parameter is sent on DSI (The 1st parameter is not sent).

#### 6.2.28. Read ID1 (DAh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 1 | 1 | 0 | 1 | 0 | DAh |
| 1st Param | 1 | ↑ | 1 | XX | X | X | X | X | X | X | X | X | XX |
| 2nd Param | 1 | ↑ | 1 | XX | ID.7 | ID.6 | ID.5 | ID.4 | ID.3 | ID.2 | ID.1 | ID.0 | 00 |

**Description:**

This read byte is used to track the LCD module/driver version. It is defined by display supplier (with User’s agreement) and changes each time a revision is made to the display, material or construction specifications. The 1st parameter is dummy data. The 2nd parameter is LCD module/driver version ID The ID3 can be programmed by MTP function. X = Don’t care

**Restriction:** None

#### 6.2.29. Read ID2 (DBh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 1 | 1 | 0 | 1 | 1 | DBh |
| 1st Param | 1 | ↑ | 1 | XX | X | X | X | X | X | X | X | X | XX |
| 2nd Param | 1 | ↑ | 1 | XX | ID.7 | ID.6 | ID.5 | ID.4 | ID.3 | ID.2 | ID.1 | ID.0 | 9A |

**Description:**

This read byte is used to track the LCD module/driver version. It is defined by display supplier (with User’s agreement) and changes each time a revision is made to the display, material or construction specifications. The 1st parameter is dummy data. The 2nd parameter is LCD module/driver version ID The ID3 can be programmed by MTP function. X = Don’t care

**Restriction:** None

#### 6.2.30. Read ID3 (DCh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 1 | 1 | 1 | 0 | 0 | DCh |
| 1st Param | 1 | ↑ | 1 | XX | X | X | X | X | X | X | X | X | XX |
| 2nd Param | 1 | ↑ | 1 | XX | ID.7 | ID.6 | ID.5 | ID.4 | ID.3 | ID.2 | ID.1 | ID.0 | 01 |

**Description:**

This read byte is used to track the LCD module/driver version. It is defined by display supplier (with User’s agreement) and changes each time a revision is made to the display, material or construction specifications. The 1st parameter is dummy data. The 2nd parameter is LCD module/driver version ID The ID3 can be programmed by MTP function. X = Don’t care

**Restriction:** None

### 6.3. Description of Level 2 Commands

#### 6.3.1. RGB Interface Signal Control (B0h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | B0h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | RCM1 | RCM0 | 0 | VSPL | HSPL | DPL | EPL | 01 |

**Description:**

Sets the operation status of the display interface. The setting becomes effective as soon as the command is received.
EPL: DE polarity ("0"= High enable for RGB interface, "1"= Low enable for RGB interface)
DPL: DOTCLK polarity set ("0"= data fetched at the rising time, "1"= data fetched at the falling time)
HSPL: HSYNC polarity ("0"= Low level sync clock, "1"= High level sync clock)
VSPL: VSYNC polarity ("0"= Low level sync clock, "1"= High level sync clock)
RCM [1:0]: RGB interface selection (refer to the RGB interface section).

| RCM[1:0] | RIM | DPI[1:0] | RGB interface Mode | RGB Mode | Used Pins |
|----------|-----|----------|--------------------|----------|-----------|
| 1 0 | 0 | 1 0 | 18-bit RGB interface (262K colors) | DE Mode. Valid data is determined by the DE signal | VSYNC,HSYNC,DE,DOTCLK,D[17:0] |
| 1 0 | 0 | 0 1 | 16-bit RGB interface (65K colors) | DE Mode. Valid data is determined by the DE signal | VSYNC,HSYNC,DE,DOTCLK,D[17:13] & D[11:1] |
| 1 0 | 1 | - | 6-bit RGB interface (262K colors) | DE Mode. Valid data is determined by the DE signal | VSYNC,HSYNC,DE,DOTCLK,D[5:0] |
| 1 1 | 0 | 1 0 | 18-bit RGB interface (262K colors) | SYNC Mode. In SYNC mode, DE signal is ignored; blanking porch is determined by B5h command | VSYNC,HSYNC,DOTCLK,D[17:0] |
| 1 1 | 0 | 0 1 | 16-bit RGB interface (65K colors) | SYNC Mode. In SYNC mode, DE signal is ignored; blanking porch is determined by B5h command | VSYNC,HSYNC,DOTCLK,D[17:13] & D[11:1] |
| 1 1 | 1 | - | 6-bit RGB interface (262K colors) | SYNC Mode. In SYNC mode, DE signal is ignored; blanking porch is determined by B5h command | VSYNC,HSYNC,DOTCLK,D[5:0] |


**Default Value (Power On / SW Reset / HW Reset):** RCM[1:0]=2'b00, VSPL=1'b0, HSPL=1'b0, DPL=1'b0, EPL=1'b1.

**Restriction:** None

#### 6.3.2. Blanking Porch Control (B5h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 0 | 1 | 1 | 0 | 1 | 0 | 1 | B5h |
| 1st Param | 1 | 1 | ↑ | XX | VFP7 | VFP6 | VFP5 | VFP4 | VFP3 | VFP2 | VFP1 | VFP0 | 08 |
| 2nd Param | 1 | 1 | ↑ | XX | 0 | VBP6 | VBP5 | VBP4 | VBP3 | VBP2 | VBP1 | VBP0 | 08 |
| 3rd Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | HBP4 | HBP3 | HBP2 | HBP1 | HBP0 | 14 |

**Description:**

Note: The Third parameter must write, but it is not valid.
VFP [6:0] / VBP [6:0]: The VFP [6:0] and VBP [6:0] bits specify the line number of vertical front and back porch period respectively.

| VFP[6:0] / VBP[6:0] | Number of HSYNC of front/back porch | VFP[6:0] / VBP[6:0] | Number of HSYNC of front/back porch |
|---------------------|-------------------------------------|---------------------|-------------------------------------|
| 0000000 | Setting inhibited | 1000000 | 64 |
| 0000001 | Setting inhibited | 1000001 | 65 |
| 0000010 | 2 | 1000010 | 66 |
| 0000011 | 3 | 1000011 | 67 |
| 0000100 | 4 | 1000100 | 68 |
| 0000101 | 5 | 1000101 | 69 |
| : | : | : | : |
| 0111101 | 61 | 1111101 | 125 |
| 0111110 | 62 | 1111110 | 109.5 |
| 0111111 | 63 | 1111111 | 127 |

Note: VFP + VBP ≦ 254 HSYNC signals

HBP [4:0]: HBP [4:0] bits specify the line number of horizontal back porch period respectively.

| HBP[4:0] | Number of HSYNC of front/back porch |
|----------|-------------------------------------|
| 00000 | Setting inhibited |
| 00001 | Setting inhibited |
| 00010 | 2 |
| 00011 | 3 |
| 00100 | 4 |
| 00101 | 5 |
| : | : |
| 11101 | 30 |
| 11110 | 31 |
| 11111 | 32 |


**Default Value (Power On / SW Reset / HW Reset):** VFP[6:0]=7'h08, VBP[6:0]=7'h08, HBP[4:0]=5'h14.

**Restriction:** EXTC should be high to enable this command

#### 6.3.3. Display Function Control (B6h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 0 | 1 | 1 | 0 | 1 | 1 | 0 | B6h |
| 1st Param | 1 | 1 | ↑ | XX | X | X | X | X | X | X | X | X | 00 |
| 2nd Param | 1 | 1 | ↑ | XX | X | GS | SS | SM | X | X | X | X | 00 |
| 3rd Param | 1 | 1 | ↑ | XX | X | X | NL5 | NL4 | NL3 | NL2 | NL1 | NL0 | 1D |

**Description:**

note: the first parameter must write, but it is not valid.
SS: Select the shift direction of outputs from the source driver.

| SS | Source Output Scan Direction |
|----|------------------------------|
| 0 | S1 → S360 |
| 1 | S360 → S1 |

In addition to the shift direction, the settings for both SS and BGR bits are required to change the assignment of R, G, and B dots to the source driver pins. To assign R, G, B dots to the source driver pins from S1 to S360, set SS = 0. To assign R, G, B dots to the source driver pins from S360 to S1, set SS = 1.

GS: Sets the direction of scan by the gate driver in the range determined by SCN [4:0] and NL [4:0]. The scan direction determined by GS = 0 can be reversed by setting GS = 1.

| GS | Gate Output Scan Direction |
|----|----------------------------|
| 0 | G1 → G32 |
| 1 | G32 → G1 |

SM: Sets the gate driver pin arrangement in combination with the GS bit to select the optimal scan mode for the module.

NL [5:0]: Sets the number of lines to drive the LCD at an interval of 8 lines. The GRAM address mapping is not affected by the number of lines set by NL [5:0]. The number of lines must be the same or more than the number of lines necessary for the size of the liquid crystal panel.

| NL[5:0] | LCD Drive Line Setting | NL[5:0] | LCD Drive Line Setting |
|---------|------------------------|---------|------------------------|
| 0 0 0 0 0 0 | prohibited | 0 1 0 1 0 1 | 176 lines |
| 0 0 0 0 0 1 | 16 lines | 0 1 0 1 1 0 | 184 lines |
| 0 0 0 0 1 0 | 24 lines | 0 1 0 1 1 1 | 192 lines |
| 0 0 0 0 1 1 | 32 lines | 0 1 1 0 0 0 | 200 lines |
| 0 0 0 1 0 0 | 40 lines | 0 1 1 0 0 1 | 208 lines |
| 0 0 0 1 0 1 | 48 lines | 0 1 1 0 1 0 | 216 lines |
| 0 0 0 1 1 0 | 56 lines | 0 1 1 0 1 1 | 224 lines |
| 0 0 0 1 1 1 | 64 lines | 0 1 1 1 0 0 | 232 lines |
| 0 0 1 0 0 0 | 72 lines | 0 1 1 1 0 1 | 240 lines |
| 0 0 1 0 0 1 | 80 lines | Others | Setting prohibited |
| 0 0 1 0 1 0 | 88 lines | | |
| 0 0 1 0 1 1 | 96 lines | | |
| 0 0 1 1 0 0 | 104 lines | | |
| 0 0 1 1 0 1 | 112 lines | | |
| 0 0 1 1 1 0 | 120 lines | | |
| 0 0 1 1 1 1 | 128 lines | | |
| 0 1 0 0 0 0 | 136 lines | | |
| 0 1 0 0 0 1 | 144 lines | | |
| 0 1 0 0 1 0 | 152 lines | | |
| 0 1 0 0 1 1 | 160 lines | | |
| 0 1 0 1 0 0 | 168 lines | | |


**Default Value (Power On / HW Reset):** GS=1'b0, SS=1'b0, SM=1'b0.

**Restriction:** EXTC should be high to enable this command

#### 6.3.4. Tearing Effect Control (BAh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 0 | 1 | 1 | 1 | 0 | 1 | 0 | BAh |
| 1st Param | 1 | 1 | ↑ | XX | te_pol | tw6 | tw5 | tw4 | tw3 | tw2 | tw1 | tw0 | 00 |

**Description:**

te_pol is used to adjust the Tearing Effect output signal pulse polarity. te_pol Tearing Effect polarity 0 Positive pulse 1 negative pulse te_width[6:0] is used to adjust the Tearing Effect output signal pulse width with display lines in unit te_width[6:0] Tearing Effect width(display line time) 0 1line time 1 2line time … … N N+1 line time … … 7f 128 line time Note: During Sleep In Mode with Tearing Effect Line On, Tearing Effect Output pin will be active Low. X = Don’t care.

**Restriction:** This command has no effect when Tearing Effect output is already ON

#### 6.3.5. Interface Control (F6h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 0 | F6h |
| 1st Param | 1 | 1 | ↑ | XX | 1 | 1 | 0 | 0 | DM1 | DM0 | RM | RIM | C0 |

**Description:**

DM [1:0]: Select the display operation mode.

| DM[1] | DM[0] | Display Operation Mode |
|-------|-------|------------------------|
| 0 | 0 | Internal clock operation |
| 0 | 1 | RGB Interface Mode |
| 1 | 0 | VSYNC interface Mode |
| 1 | 1 | Setting disabled |

RM: Select the interface to access the GRAM. Set RM to "1" when writing display data by the RGB interface.

| RM | Interface for RAM Access |
|----|--------------------------|
| 0 | System interface/VSYNC interface |
| 1 | RGB interface |

RIM: Specify the RGB interface mode when the RGB interface is used. These bits should be set before display operation through the RGB interface and should not be set during operation.

| RIM | COLMOD [6:4] | RGB Interface Mode |
|-----|--------------|--------------------|
| 0 | 110 (262K color) | 18-bit RGB interface (1 transfer/pixel) |
| 0 | 101 (65K color) | 16-bit RGB interface (1 transfer/pixel) |
| 1 | (262K color) | 6-bit RGB interface (3 transfer/pixel) |

**Restriction:** EXTC should be high to enable this command

### 6.4. Description of Level 3 Commands

#### 6.4.1. Frame Rate (E8h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 0 | 1 | 0 | 0 | 0 | E8h |
| 1st Param | 1 | 1 | ↑ | XX | DINV3 | DINV2 | DINV1 | DINV0 | X | X | X | X | 14 |

**Description:**

DINV[3:0] : Set display inversion mode

| DINV[3:0] | Inversion |
|-----------|-----------|
| 0 | column inversion |
| 1 | 1 dot inversion |
| 2 | 2 dot inversion |
| 3 | 4 dot inversion |
| 4 | 8 dot inversion |


**Default Value (Power On / SW Reset / HW Reset):** DINV[3:0]=4'h1.

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.2. SPI 2DATA control (E9h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 0 | 1 | 0 | 0 | 1 | E9h |
| 1st Param | 1 | 1 | ↑ | XX | X | X | X | X | 2data_en | mdt2 | mdt1 | mdt0 | 00 |

**Description:**

2DATA_EN: Set 2_data_line mode in 3-wire/4-wire SPI.
2DATA_MDT[2:0] Set pixel data format in 2_data_line mode.

| 2DATA_MDT[2:0] | Data Format |
|----------------|-------------|
| 000 | 65K color 1pixle/transition |
| 001 | 262K color 1pixle/transition |
| 010 | 262K color 2/3pixle/transition |
| 100 | 4M color 1pixle/transition |
| 110 | 4M color 2/3pixle/transition |

**Restriction:** Inter command should be set high to enable this command

#### 6.4.3. Power Control 1 (C1h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | C1h |
| 1st Param | 1 | 1 | ↑ | XX | X | X | X | X | 0 | 0 | VCIRE | 0 | 00 |

**Description:**

VCIRE: Select the external reference voltage Vci or internal reference voltage VCIR.

| VCIRE | Reference voltage |
|-------|-------------------|
| 0 | Internal reference voltage 2.5V (default) |
| 1 | External reference voltage Vci |

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.4. Power Control 2 (C3h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 0 | 0 | 0 | 1 | 1 | C3h |
| 1st Param | 1 | 1 | ↑ | XX | X | vbp6 | vbp5 | vbp4 | vbp3 | vbp2 | vbp1 | vbp0 | 3C |

**Description:**

Set the voltage level value to output the VREG1A and VREG1B OUT level, which is a reference level for the grayscale voltage level. (Table is valid when vrh=0x28)

`VREG1A = (vrh+vbp_d)*0.02 + 4`  ;  `VREG1B = vbp_d*0.02 + 0.3`

| vreg1_vbp_d[6:0] | VREG1A/V | VREG1B/V |
|------------------|----------|----------|
| 7'h00 | 4.8 | 0.3 |
| … | … | … |
| N | (N+40)*0.02+4 | N*0.02+0.3 |
| … | … | … |
| 7'h3C | 6 | 1.5 |
| … | … | … |
| 7'h7F | 7.34 | 2.84 |

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.5. Power Control 3 (C4h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 0 | 0 | 1 | 0 | 0 | C4h |
| 1st Param | 1 | 1 | ↑ | XX | X | vbn6 | vbn5 | vbn4 | vbn3 | vbn2 | vbn1 | vbn0 | 3C |

**Description:**

Set the voltage level value to output the VREG2A OUT level, which is a reference level for the grayscale voltage level (Table is valid when vrh=0x28)

`VREG2A = (vbn_d-vrh)*0.02 - 3.4`  ;  `VREG2B = vbn_d*0.02 + 0.3`

| vreg1_vbn_d[6:0] | VREG2A/V | VREG2B/V |
|------------------|----------|----------|
| 7'h00 | -4.2 | 0.3 |
| … | … | … |
| N | N*0.02-4.2 | N*0.02+0.3 |
| … | … | … |
| 7'h3C | -3 | 1.5 |
| … | … | … |
| 7'h7F | -1.66 | 2.84 |

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.6. Power Control 4 (C9h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 0 | 0 | 1 | 0 | 0 | 1 | C9h |
| 1st Param | 1 | 1 | ↑ | XX | X | X | vrh5 | vrh4 | vrh3 | vrh2 | vrh1 | vrh0 | 28 |

**Description:**

Set the voltage level value to output the VREG1A OUT level, which is a reference level for the grayscale voltage level. (Table is valid when vbp_d=0x3C and vbn_d=0x3C)

`VREG1A = (vrh+vbp_d)*0.02 + 4`  ;  `VREG2A = (vbn_d-vrh)*0.02 - 3.4`

| vrh[5:0] | VREG1A/V | VREG2A/V |
|----------|----------|----------|
| 6'h00 | 5.2 | -2.2 |
| … | … | … |
| N | (N+60)*0.02+4 | (100-N)*0.02-4.2 |
| … | … | … |
| 6'h28 | 6 | -3 |
| … | … | … |
| 6'h3F | 6.46 | -3.46 |

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.7. Power Control 7 (A7h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 0 | 1 | 0 | 0 | 1 | 1 | 1 | A7h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | 1 | 0 | 0 | vdd3 | vdd2 | vdd1 | vdd0 | 48 |

**Description:**

vdd_ad: Set the voltage level value to output the VCORE level.

| vdd_ad[3:0] | VCORE(V) | vdd_ad[3:0] | VCORE(V) |
|-------------|----------|-------------|----------|
| 4'h00 | 1.483 | 4'h08 | 1.994 |
| 4'h01 | 1.545 | 4'h09 | 2.109 |
| 4'h02 | 1.590 | 4'h0a | 2.193 |
| 4'h03 | 1.638 | 4'h0b | 2.286 |
| 4'h04 | 1.714 | 4'h0c | 2.385 |
| 4'h05 | 1.279 | 4'h0d | 1.713 |
| 4'h06 | 1.859 | 4'h0e | 1.713 |
| 4'h07 | 1.925 | 4'h0f | 1.713 |

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.8. Inter Register Enable 1 (FEh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | FEh |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used for Inter_command controlling. To set Inter_command high ,you should write Inter register enable 1 (FEh) and Inter register enable 2 (EFh) continuously. Once Inter_command is set high, only hardware or software reset can turn it to low. Inter_command is low Command write command Parameter Inter register enable 1 (FEh) Display write command Inter register enable 2 (EFh) Action Mode Inter_command is high Sequential transfer

**Restriction:** None

#### 6.4.9. Inter Register Enable 2 (EFh)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | EFh |
| Parameter | — | — | — | — | — | — | — | — | — | — | — | — | No Parameter |

**Description:**

This command is used for Inter_command controlling. To set Inter_command high ,you should write Inter register enable 1 (FEh) and Inter register enable 2 (EFh) continuously. Once Inter_command is set high, only hardware or software reset can turn it to low. Inter_command is low Command write command Parameter Inter register enable 1 (FEh) Display write command Inter register enable 2 (EFh) Action Mode Inter_command is high Sequential transfer

**Restriction:** None

#### 6.4.10. SET_GAMMA1 (F0h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | F0h |
| 1st Param | 1 | 1 | ↑ | XX | j0n1 | j0n0 | vr1n5 | vr1n4 | vr1n3 | vr1n2 | vr1n1 | vr1n0 | 80 |
| 2nd Param | 1 | 1 | ↑ | XX | j1n1 | j1n0 | vr2n5 | vr2n4 | vr2n3 | vr2n2 | vr2n1 | vr2n0 | 03 |
| 3rd Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | vr4n4 | vr4n3 | vr4n2 | vr4n1 | vr4n0 | 08 |
| 4th Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | vr6n4 | vr6n3 | vr6n2 | vr6n1 | vr6n0 | 06 |
| 5th Param | 1 | 1 | ↑ | XX | vr0n3 | vr0n2 | vr0n1 | vr0n0 | vr13n3 | vr13n2 | vr13n1 | vr13n0 | 05 |
| 6th Param | 1 | 1 | ↑ | XX | 0 | vr20n6 | vr20n5 | vr20n4 | vr20n3 | vr20n2 | vr20n1 | vr20n0 | 2B |

**Description:**

dig2gam_dig2j0_n[1:0]: γ gradient adjustment register for negative polarity dig2gam_dig2j1_n[1:0]: γ gradient adjustment register for negative polarity dig2gam_vr0_n[3:0]: γ gradient adjustment register for negative polarity dig2gam_vr1_n[5:0]: γ gradient adjustment register for negative polarity dig2gam_vr2_n[5:0]: γ gradient adjustment register for negative polarity dig2gam_vr4_n[4:0]: γ gradient adjustment register for negative polarity dig2gam_vr6_n[4:0]: γ gradient adjustment register for negative polarity dig2gam_vr13_n[3:0]: γ gradient adjustment register for negative polarity dig2gam_vr20_n[6:0]: γ gradient adjustment register for negative polarity


**Default Value (Power On / SW Reset / HW Reset):** dig2gam_dig2j0_n[1:0]=2'h02, dig2gam_dig2j1_n[1:0]=2'h00, dig2gam_vr0_n[3:0]=4'h00, dig2gam_vr1_n[5:0]=6'h00, dig2gam_vr2_n[5:0]=6'h03, dig2gam_vr4_n[4:0]=5'h08, dig2gam_vr6_n[4:0]=5'h06, dig2gam_vr13_n[3:0]=4'h05, dig2gam_vr20_n[6:0]=7'h2b.

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.11. SET_GAMMA2 (F1h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 1 | F1h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | vr43n6 | vr43n5 | vr43n4 | vr43n3 | vr43n2 | vr43n1 | vr43n0 | 41 |
| 2nd Param | 1 | 1 | ↑ | XX | vr27n2 | vr27n1 | vr27n0 | vr57n4 | vr57n3 | vr57n2 | vr57n1 | vr57n0 | 97 |
| 3rd Param | 1 | 1 | ↑ | XX | vr36n2 | vr36n1 | vr36n0 | vr59n4 | vr59n3 | vr59n2 | vr59n1 | vr59n0 | 98 |
| 4th Param | 1 | 1 | ↑ | XX | 0 | 0 | vr61n5 | vr61n4 | vr61n3 | vr61n2 | vr61n1 | vr61n0 | 13 |
| 5th Param | 1 | 1 | ↑ | XX | 0 | 0 | vr62n5 | vr62n4 | vr62n3 | vr62n2 | vr62n1 | vr62n0 | 17 |
| 6th Param | 1 | 1 | ↑ | XX | vr50n3 | vr50n2 | vr50n1 | vr50n0 | vr63n3 | vr63n2 | vr63n1 | vr63n0 | CD |

**Description:**

dig2gam_vr43_p[6:0]: γ gradient adjustment register for negative polarity dig2gam_vr27_p[2:0]: γ gradient adjustment register for negative polarity dig2gam_vr57_p[4:0]: γ gradient adjustment register for negative polarity dig2gam_vr59_p[4:0]: γ gradient adjustment register for negative polarity dig2gam_vr36_p[2:0]: γ gradient adjustment register for negative polarity dig2gam_vr61_p[5:0]: γ gradient adjustment register for negative polarity dig2gam_vr62_p[5:0]: γ gradient adjustment register for negative polarity dig2gam_vr50_p[3:0]: γ gradient adjustment register for negative polarity dig2gam_vr63_p[3:0]: γ gradient adjustment register for negative polarity


**Default Value (Power On / SW Reset / HW Reset):** dig2gam_vr43_n[6:0]=7'h41, dig2gam_vr27_n[2:0]=3'h04, dig2gam_vr57_n[4:0]=5'h17, dig2gam_vr59_n[4:0]=5'h18, dig2gam_vr36_n[2:0]=3'h04, dig2gam_vr61_n[5:0]=6'h13, dig2gam_vr62_n[5:0]=6'h17, dig2gam_vr50_n[3:0]=4'h0C, dig2gam_vr63_n[3:0]=4'h0D.

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.12. SET_GAMMA3 (F2h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 0 | 0 | 1 | 0 | F2h |
| 1st Param | 1 | 1 | ↑ | XX | j0p1 | j0p0 | vr1p5 | vr1p4 | vr1p3 | vr1p2 | vr1p1 | vr1p0 | 40 |
| 2nd Param | 1 | 1 | ↑ | XX | j1p1 | j1p0 | vr2p5 | vr2p4 | vr2p3 | vr2p2 | vr2p1 | vr2p0 | 03 |
| 3rd Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | vr4p4 | vr4p3 | vr4p2 | vr4p1 | vr4p0 | 08 |
| 4th Param | 1 | 1 | ↑ | XX | 0 | 0 | 0 | vr6p4 | vr6p3 | vr6p2 | vr6p1 | vr6p0 | 0B |
| 5th Param | 1 | 1 | ↑ | XX | vr0p3 | vr0p2 | vr0p1 | vr0p0 | vr13p3 | vr13p2 | vr13p1 | vr13p0 | 08 |
| 6th Param | 1 | 1 | ↑ | XX | 0 | vr20p6 | vr20p5 | vr20p4 | vr20p3 | vr20p2 | vr20p1 | vr20p0 | 2E |

**Description:**

dig2gam_dig2j0_p[1:0]: γ gradient adjustment register for positive polarity dig2gam_dig2j1_p[1:0]: γ gradient adjustment register for positive polarity dig2gam_vr1_p[5:0]: γ gradient adjustment register for positive polarity dig2gam_vr2_p[5:0]: γ gradient adjustment register for positive polarity dig2gam_vr4_p[4:0]: γ gradient adjustment register for positive polarity dig2gam_vr6_p[4:0]: γ gradient adjustment register for positive polarity dig2gam_vr0_p[3:0]: γ gradient adjustment register for positive polarity dig2gam_vr13_p[3:0]: γ gradient adjustment register for positive polarity dig2gam_vr20_p[6:0]: γ gradient adjustment register for positive polarity


**Default Value (Power On / SW Reset / HW Reset):** dig2gam_dig2j0_p[1:0]=2'h01, dig2gam_dig2j1_p[1:0]=2'h00, dig2gam_vr1_p[5:0]=6'h00, dig2gam_vr2_p[5:0]=6'h03, dig2gam_vr4_p[4:0]=5'h08, dig2gam_vr6_p[4:0]=5'h0B, dig2gam_vr0_p[3:0]=4'h00, dig2gam_vr13_p[3:0]=4'h08, dig2gam_vr20_p[6:0]=7'h2E.

**Restriction:** Inter_command should be set high to enable this command

#### 6.4.13. SET_GAMMA4 (F3h)

| | D/CX | RDX | WRX | D17-8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | HEX |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Command | 0 | 1 | ↑ | XX | 1 | 1 | 1 | 1 | 0 | 0 | 1 | 1 | F3h |
| 1st Param | 1 | 1 | ↑ | XX | 0 | vr43p6 | vr43p5 | vr43p4 | vr43p3 | vr43p2 | vr43p1 | vr43p0 | 3F |
| 2nd Param | 1 | 1 | ↑ | XX | vr27p2 | vr27p1 | vr27p0 | vr57p4 | vr57p3 | vr57p2 | vr57p1 | vr57p0 | 98 |
| 3rd Param | 1 | 1 | ↑ | XX | vr36p2 | vr36p1 | vr36p0 | vr59p4 | vr59p3 | vr59p2 | vr59p1 | vr59p0 | B4 |
| 4th Param | 1 | 1 | ↑ | XX | 0 | 0 | vr61p5 | vr61p4 | vr61p3 | vr61p2 | vr61p1 | vr61p0 | 14 |
| 5th Param | 1 | 1 | ↑ | XX | 0 | 0 | vr62p5 | vr62p4 | vr62p3 | vr62p2 | vr62p1 | vr62p0 | 18 |
| 6th Param | 1 | 1 | ↑ | XX | vr50p3 | vr50p2 | vr50p1 | vr50p0 | vr63p3 | vr63p2 | vr63p1 | vr63p0 | CD |

**Description:**

dig2gam_vr43_p[6:0]: γ gradient adjustment register for positive polarity dig2gam_vr27_p[2:0]: γ gradient adjustment register for positive polarity dig2gam_vr57_p[4:0]: γ gradient adjustment register for positive polarity dig2gam_vr36_p[2:0]: γ gradient adjustment register for positive polarity dig2gam_vr59_p[4:0]: γ gradient adjustment register for positive polarity dig2gam_vr61_p[5:0]: γ gradient adjustment register for positive polarity dig2gam_vr62_p[5:0]: γ gradient adjustment register for positive polarity dig2gam_vr50_p[3:0]: γ gradient adjustment register for positive polarity dig2gam_vr63_p[3:0]: γ gradient adjustment register for positive polarity


**Default Value (Power On / SW Reset / HW Reset):** dig2gam_vr43_p[6:0]=7'h3F, dig2gam_vr27_p[2:0]=3'h04, dig2gam_vr57_p[4:0]=5'h18, dig2gam_vr36_p[2:0]=3'h05, dig2gam_vr59_p[4:0]=5'h14, dig2gam_vr61_p[5:0]=6'h14, dig2gam_vr62_p[5:0]=6'h18, dig2gam_vr50_p[3:0]=4'h0C, dig2gam_vr63_p[3:0]=4'h0D.

**Restriction:** Inter_command should be set high to enable this command

---

## 7. Electrical Characteristics

### 7.1. Absolute Maximum Ratings — Table 43

Exceeding these (even momentarily) may permanently damage or degrade the device.

| Item | Symbol | Unit | Value |
|------|--------|------|-------|
| Supply voltage | VCI | V | -0.3 ~ +4.6 |
| Supply voltage (Logic) | IOVCC | V | -0.3 ~ +4.6 |
| Supply voltage (Digital) | DVDD | V | -0.3 ~ +2.0 |
| Driver supply voltage | VGH-VGL | V | -0.3 ~ +27.0 |
| Logic input voltage range | VIN | V | -0.3 ~ IOVCC+0.3 |
| Logic output voltage range | VO | V | -0.3 ~ IOVCC+0.3 |
| Operation temperature | Topr | °C | -40 ~ +80 |
| Storage temperature | Tstg | °C | -40 ~ +80 |

### 7.2. DC Characteristics — Table 44

| Item | Symbol | Unit | Condition | Min. | Typ. | Max. | Note |
|------|--------|------|-----------|------|------|------|------|
| Analog Operating Voltage | VCI | V | Operating voltage | 2.5 | 2.8 | 3.3 | 2 |
| Logic Operating Voltage | IOVCC | V | I/O supply voltage | 1.65 | 2.8 | 3.3 | 2 |
| Digital Operating Voltage | DVDD | V | Digital supply voltage | - | 1.34 | - | 2 |
| Gate Driver High Voltage | VGH | V | - | 12.0 | - | 13.0 | 3 |
| Gate Driver Low Voltage | VGL | V | - | -11.0 | - | -18.0 | 3 |
| Driver Supply Voltage | \|VGH-VGL\| | V | - | 20 | - | 27 | 3 |
| Logic High Level Input Voltage | VIH | V | - | 0.7*IOVCC | - | IOVCC | 1,2,3 |
| Logic Low Level Input Voltage | VIL | V | - | VSSC | - | 0.3*IOVCC | 1,2,3 |
| Logic High Level Output Voltage | VOH | V | IOL=-1.0mA | 0.8*IOVCC | - | IOVCC | 1,2,3 |
| Logic Low Level Output Voltage | VOL | V | IOL=1.0mA | VSSC | - | 0.2*IOVCC | 1,2,3 |
| Logic High Level Input Current | IIH | µA | - | - | - | 1 | 1,2,3 |
| Logic Low Level Input Current | IIL | µA | - | -1 | - | - | 1,2,3 |
| Logic Input Leakage Current | ILEA | µA | VIN=IOVCC or VSSC | -0.1 | - | +0.1 | 1,2,3 |
| Source Output Range | Vsout | V | - | VREG2 | - | VREG1 | 4 |

> Notes: (1) IOVCC=1.65–3.3V, VCI=2.5–3.3V, AGND=VSS=0V, Ta=-30 to 70°C (to +85 no damage).
> (2) Supply digital IOVCC ≤ analog VCI. (3) Applies to CSX, RDX, WRX, D[17:0], D/CX, RESX, TE,
> DOTCLK, VSYNC, HSYNC, DE, SDA, SCL, IM[3:0] and test pins. (4) Measured with LCD module.
> (5) VCI=2.6V. (6) VCI=3.3V. (7) Max value is between the Note-4 measure point and gamma setting.

### 7.3. AC Characteristics

#### 7.3.1. Parallel 18/16/9/8-bit Interface Timing (8080-I) — Table 45

Ta = -30 to 70°C, IOVCC=1.65–3.3V, VCI=2.5–3.3V, VSS=0V. Logic high/low = 70%/30% of IOVCC.

| Signal | Symbol | Parameter | Min | Max | Unit |
|--------|--------|-----------|-----|-----|------|
| DCX | tast | Address setup time | 0 | - | ns |
| DCX | taht | Address hold time (Write/Read) | 0 | - | ns |
| CSX | tchw | CSX "H" pulse width | 0 | - | ns |
| CSX | tcs | Chip select setup time (Write) | 15 | - | ns |
| CSX | trcs | Chip select setup time (Read ID) | 45 | - | ns |
| CSX | trcsfm | Chip select setup time (Read FM) | 355 | - | ns |
| CSX | tcsf | Chip select wait time (Write/Read) | 10 | - | ns |
| WRX | twc | Write cycle | 66 | - | ns |
| WRX | twrh | Write control pulse H duration | 15 | - | ns |
| WRX | twrl | Write control pulse L duration | 15 | - | ns |
| RDX(FM) | trcfm | Read cycle (FM) | 380 | - | ns |
| RDX(FM) | trdhfm | Read control H duration (FM) | 180 | - | ns |
| RDX(FM) | trdlfm | Read control L duration (FM) | 200 | - | ns |
| RDX(ID) | trc | Read cycle (ID) | 160 | - | ns |
| RDX(ID) | trdh | Read control H pulse duration | 90 | - | ns |
| RDX(ID) | trdl | Read control L pulse duration | 70 | - | ns |
| D[17:0] | tdst | Write data setup time | 10 | - | ns |
| D[17:0] | tdht | Write data hold time | 10 | - | ns |
| D[17:0] | trat | Read access time | - | 40 | ns |
| D[17:0] | tratfm | Read access time (FM) | - | 340 | ns |
| D[17:0] | trod | Read output disable time | 20 | 80 | ns |

> Write data setup/hold for maximum CL=30pF; read access for minimum CL=8pF. tr/tf ≤ 15ns.

#### 7.3.2. Parallel 18/16/9/8-bit Interface Timing (8080-II) — Table 46

Same conditions as 8080-I. Values are identical to Table 45:

| Signal | Symbol | Parameter | Min | Max | Unit |
|--------|--------|-----------|-----|-----|------|
| DCX | tast | Address setup time | 0 | - | ns |
| DCX | taht | Address hold time (Write/Read) | 0 | - | ns |
| CSX | tchw | CSX "H" pulse width | 0 | - | ns |
| CSX | tcs | Chip select setup time (Write) | 15 | - | ns |
| CSX | trcs | Chip select setup time (Read ID) | 45 | - | ns |
| CSX | trcsfm | Chip select setup time (Read FM) | 355 | - | ns |
| CSX | tcsf | Chip select wait time (Write/Read) | 10 | - | ns |
| WRX | twc | Write cycle | 66 | - | ns |
| WRX | twrh | Write control pulse H duration | 15 | - | ns |
| WRX | twrl | Write control pulse L duration | 15 | - | ns |
| RDX(FM) | trcfm | Read cycle (FM) | 380 | - | ns |
| RDX(FM) | trdhfm | Read control H duration (FM) | 180 | - | ns |
| RDX(FM) | trdlfm | Read control L duration (FM) | 200 | - | ns |
| RDX(ID) | trc | Read cycle (ID) | 160 | - | ns |
| RDX(ID) | trdh | Read control H pulse duration | 90 | - | ns |
| RDX(ID) | trdl | Read control L pulse duration | 70 | - | ns |
| D[17:0] | tdst | Write data setup time | 10 | - | ns |
| D[17:0] | tdht | Write data hold time | 10 | - | ns |
| D[17:0] | trat | Read access time | - | 40 | ns |
| D[17:0] | tratfm | Read access time (FM) | - | 340 | ns |
| D[17:0] | trod | Read output disable time | 20 | 80 | ns |

#### 7.3.3. Serial Interface Timing (3-line SPI) — Table 47

Ta = 25°C, IOVCC=1.65–3.3V, VCI=2.5–3.3V, VSSA=VSSC=0V.

| Signal | Symbol | Parameter | Min | Max | Unit |
|--------|--------|-----------|-----|-----|------|
| SCL | tscycw | Serial clock cycle (Write) | 10 | - | ns |
| SCL | tshw | SCL "H" pulse width (Write) | 5 | - | ns |
| SCL | tslw | SCL "L" pulse width (Write) | 5 | - | ns |
| SCL | tscycr | Serial clock cycle (Read) | 150 | - | ns |
| SCL | tshr | SCL "H" pulse width (Read) | 60 | - | ns |
| SCL | tslr | SCL "L" pulse width (Read) | 60 | - | ns |
| SDA/SDI (Input) | tsds | Data setup time (Write) | 5 | - | ns |
| SDA/SDI (Input) | tsdh | Data hold time (Write) | 5 | - | ns |
| SDA/SDO (Output) | tacc | Access time (Read) | 10 | - | ns |
| CSX | tscc | SCL-CSX | 10 | - | ns |
| CSX | tchw | CSX "H" pulse width | 10 | - | ns |
| CSX | tcss | CSX setup time | 20 | - | ns |
| CSX | tcsh | CSX-SCL time | 40 | - | ns |

> tr/tf ≤ 15ns (between 30% and 70% of IOVCC).

#### 7.3.4. Serial Interface Timing (4-line SPI) — Table 48

Ta = 25°C, IOVCC=1.65–3.3V, VCI=2.5–3.3V, AGND=VSS=0V.

| Signal | Symbol | Parameter | Min | Max | Unit |
|--------|--------|-----------|-----|-----|------|
| CSX | tcss | Chip select time (Write) | 20 | - | ns |
| CSX | tcsh | Chip select hold time (Read) | 40 | - | ns |
| SCL | twc | Serial clock cycle (Write) | 10 | - | ns |
| SCL | twrh | SCL "H" pulse width (Write) | 5 | - | ns |
| SCL | twrl | SCL "L" pulse width (Write) | 5 | - | ns |
| SCL | trc | Serial clock cycle (Read) | 150 | - | ns |
| SCL | trdh | SCL "H" pulse width (Read) | 60 | - | ns |
| SCL | trdl | SCL "L" pulse width (Read) | 60 | - | ns |
| D/CX | tas | D/CX setup time | 10 | - | ns |
| D/CX | tah | D/CX hold time (Write/Read) | 10 | - | ns |
| SDA/SDI (Input) | tds | Data setup time (Write) | 5 | - | ns |
| SDA/SDI (Input) | tdh | Data hold time (Write) | 5 | - | ns |
| SDA/SDO (Output) | tacc | Access time (Read) | 10 | - | ns |

> tr/tf ≤ 15ns. Note: write SCL cycle min 10ns ⇒ max SPI write clock ≈ 100 MHz; read cycle min
> 150ns ⇒ max read clock ≈ 6.67 MHz.

#### 7.3.5. Parallel 18/16/6-bit RGB Interface Timing — Table 49

Ta = -30 to 70°C, IOVCC=1.65–3.3V, VCI=2.5–3.3V, AGND=VSS=0V.

| Signal | Symbol | Parameter | Min | Max | Unit |
|--------|--------|-----------|-----|-----|------|
| VSYNC/HSYNC | tsyncs | VSYNC/HSYNC setup time | 15 | - | ns |
| VSYNC/HSYNC | tsynch | VSYNC/HSYNC hold time | 15 | - | ns |
| DE | tens | DE setup time | 15 | - | ns |
| DE | tenh | DE hold time | 15 | - | ns |
| D[17:0] | tpds | Data setup time | 15 | - | ns |
| D[17:0] | tpdh | Data hold time | 15 | - | ns |
| DOTCLK | PWDH | DOTCLK high-level period | 15 | - | ns |
| DOTCLK | PWDL | DOTCLK low-level period | 15 | - | ns |
| DOTCLK | tcycd | DOTCLK cycle time | 100 | - | ns |
| DOTCLK/HSYNC/VSYNC | trgbr, trgbf | Rise/fall time | - | 15 | ns |

> The same values apply to both 18/16-bit and 6-bit bus RGB interface modes.

---

*End of GC9A01A datasheet (192 pages). Graphical figures (block diagram, waveform/timing diagrams,
PAD bump views, gamma DAC schematics) are summarized in text; all tabular data has been reproduced.*

