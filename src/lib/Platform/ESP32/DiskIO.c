/*-----------------------------------------------------------------------/
/  Low level disk I/O module for ESP32 (FatFS diskio interface)          /
/  Implements FatFs disk_* using the ESP-IDF SPI master driver.          /
/  Init/command structure mirrors the RP2040 port (no-OS-FatFS based).   /
/-----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"
#include "HALConfig.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*-----------------------------------------------------------------------*/
/* SD Card State                                                         */
/*-----------------------------------------------------------------------*/

typedef struct {
    bool Initialized;
    bool HighCapacity;   /* true for SDHC/SDXC (block addressing) */
    uint32_t SectorCount;
    uint16_t SectorSize;
    uint32_t BlockSize;
} SdCardState;

static SdCardState sdState = {
    .Initialized = false,
    .HighCapacity = false,
    .SectorCount = 0,
    .SectorSize = 512,
    .BlockSize = 512
};

/*-----------------------------------------------------------------------*/
/* SD Card Commands (raw index; SPI_CMD() adds the 0x40 start bits)      */
/*-----------------------------------------------------------------------*/

#define CMD0    0   /* GO_IDLE_STATE */
#define CMD8    8   /* SEND_IF_COND */
#define CMD9    9   /* SEND_CSD */
#define CMD12   12  /* STOP_TRANSMISSION */
#define CMD17   17  /* READ_SINGLE_BLOCK */
#define CMD18   18  /* READ_MULTIPLE_BLOCK */
#define CMD24   24  /* WRITE_BLOCK */
#define CMD25   25  /* WRITE_MULTIPLE_BLOCK */
#define CMD55   55  /* APP_CMD */
#define CMD58   58  /* READ_OCR */
#define CMD59   59  /* CRC_ON_OFF */
#define ACMD41  41  /* SD_SEND_OP_COND */

#define SPI_CMD(x)       (0x40 | ((x) & 0x3F))

#define R1_NO_RESPONSE   0xFF
#define R1_RESPONSE_RECV 0x80
#define R1_IDLE_STATE    0x01

#define SPI_FILL_CHAR    0xFF
#define SD_START_BLOCK   0xFE   /* data token for read/single write */
#define SD_MULTI_TOKEN   0xFC   /* data token for multi-block write */
#define SD_STOP_TOKEN    0xFD

#define OCR_HCS_CCS      (0x1u << 30)

#define SD_COMMAND_TIMEOUT_MS  2000
#define SD_CMD_RETRIES         3
#define SD_CMD0_RETRIES        10

#define SD_INIT_BAUDRATE       (400 * 1000)

/*-----------------------------------------------------------------------*/
/* CRC7 (for command packets) — table from no-OS-FatFS crc.c             */
/*-----------------------------------------------------------------------*/

static const unsigned char crc7Table[256] = {
    0x00,0x09,0x12,0x1B,0x24,0x2D,0x36,0x3F,0x48,0x41,0x5A,0x53,0x6C,0x65,0x7E,0x77,
    0x19,0x10,0x0B,0x02,0x3D,0x34,0x2F,0x26,0x51,0x58,0x43,0x4A,0x75,0x7C,0x67,0x6E,
    0x32,0x3B,0x20,0x29,0x16,0x1F,0x04,0x0D,0x7A,0x73,0x68,0x61,0x5E,0x57,0x4C,0x45,
    0x2B,0x22,0x39,0x30,0x0F,0x06,0x1D,0x14,0x63,0x6A,0x71,0x78,0x47,0x4E,0x55,0x5C,
    0x64,0x6D,0x76,0x7F,0x40,0x49,0x52,0x5B,0x2C,0x25,0x3E,0x37,0x08,0x01,0x1A,0x13,
    0x7D,0x74,0x6F,0x66,0x59,0x50,0x4B,0x42,0x35,0x3C,0x27,0x2E,0x11,0x18,0x03,0x0A,
    0x56,0x5F,0x44,0x4D,0x72,0x7B,0x60,0x69,0x1E,0x17,0x0C,0x05,0x3A,0x33,0x28,0x21,
    0x4F,0x46,0x5D,0x54,0x6B,0x62,0x79,0x70,0x07,0x0E,0x15,0x1C,0x23,0x2A,0x31,0x38,
    0x41,0x48,0x53,0x5A,0x65,0x6C,0x77,0x7E,0x09,0x00,0x1B,0x12,0x2D,0x24,0x3F,0x36,
    0x58,0x51,0x4A,0x43,0x7C,0x75,0x6E,0x67,0x10,0x19,0x02,0x0B,0x34,0x3D,0x26,0x2F,
    0x73,0x7A,0x61,0x68,0x57,0x5E,0x45,0x4C,0x3B,0x32,0x29,0x20,0x1F,0x16,0x0D,0x04,
    0x6A,0x63,0x78,0x71,0x4E,0x47,0x5C,0x55,0x22,0x2B,0x30,0x39,0x06,0x0F,0x14,0x1D,
    0x25,0x2C,0x37,0x3E,0x01,0x08,0x13,0x1A,0x6D,0x64,0x7F,0x76,0x49,0x40,0x5B,0x52,
    0x3C,0x35,0x2E,0x27,0x18,0x11,0x0A,0x03,0x74,0x7D,0x66,0x6F,0x50,0x59,0x42,0x4B,
    0x17,0x1E,0x05,0x0C,0x33,0x3A,0x21,0x28,0x5F,0x56,0x4D,0x44,0x7B,0x72,0x69,0x60,
    0x0E,0x07,0x1C,0x15,0x2A,0x23,0x38,0x31,0x46,0x4F,0x54,0x5D,0x62,0x6B,0x70,0x79
};

static unsigned char Crc7(const unsigned char* data, int length) {
    unsigned char crc = 0;
    for (int i = 0; i < length; i++) {
        crc = crc7Table[(crc << 1) ^ data[i]];
    }
    return crc;
}

/*-----------------------------------------------------------------------*/
/* ESP-IDF SPI plumbing (manual CS control via GPIO)                      */
/*-----------------------------------------------------------------------*/

static spi_device_handle_t spiHandle = NULL;
static bool busInited = false;

static int SpiBusSetup(void) {
    if (busInited) return 0;
    spi_bus_config_t busCfg = {
        .mosi_io_num = SD_SPI_MOSI,
        .miso_io_num = SD_SPI_MISO,
        .sclk_io_num = SD_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 512 + 16
    };
    if (spi_bus_initialize(SD_SPI, &busCfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        return -1;
    }
    busInited = true;

    /* CS is driven manually as a plain GPIO so it can be held low across a
     * full command/response/data sequence. */
    gpio_set_direction((gpio_num_t)SD_SPI_CS, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)SD_SPI_CS, 1);
    return 0;
}

/* (Re)configure the SPI device clock. spics_io_num = -1 → manual CS. */
static int SpiSetClock(uint32_t hz) {
    if (spiHandle) {
        spi_bus_remove_device(spiHandle);
        spiHandle = NULL;
    }
    spi_device_interface_config_t devCfg = {
        .clock_speed_hz = (int)hz,
        .mode = 0,
        .spics_io_num = -1,   /* manual CS */
        .queue_size = 1
    };
    return (spi_bus_add_device(SD_SPI, &devCfg, &spiHandle) == ESP_OK) ? 0 : -1;
}

static uint8_t SdSpiWrite(uint8_t value) {
    uint8_t rx = SPI_FILL_CHAR;
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &value,
        .rx_buffer = &rx
    };
    spi_device_transmit(spiHandle, &t);
    return rx;
}

static void SpiRecvBytes(uint8_t* data, size_t len) {
    /* SD reads require 0xFF on MOSI while clocking in data. Pre-fill the
     * buffer with 0xFF and use it as both tx and rx (full-duplex). */
    memset(data, 0xFF, len);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .rx_buffer = data
    };
    spi_device_transmit(spiHandle, &t);
}

static void SpiSendBytes(const uint8_t* data, size_t len) {
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .rx_buffer = NULL
    };
    spi_device_transmit(spiHandle, &t);
}

/* Acquire: select card (CS low) + one fill byte to synchronise. */
static void SdAcquire(void) {
    gpio_set_level((gpio_num_t)SD_SPI_CS, 0);
    SdSpiWrite(SPI_FILL_CHAR);
}

/* Release: deselect card (CS high) + one fill byte so DO is released. */
static void SdRelease(void) {
    gpio_set_level((gpio_num_t)SD_SPI_CS, 1);
    SdSpiWrite(SPI_FILL_CHAR);
}

/* Send 0xFF until the card releases DO (returns non-zero), or timeout. */
static bool SdWaitReady(int timeoutMs) {
    /* ~ timeoutMs milliseconds: each byte at 400kHz is ~20us; use a generous
     * iteration cap derived from the timeout. */
    int iters = timeoutMs * 50;
    uint8_t resp;
    do {
        resp = SdSpiWrite(SPI_FILL_CHAR);
        if (resp != 0x00) return true;
    } while (--iters > 0);
    return false;
}

/*-----------------------------------------------------------------------*/
/* Raw command — sends the 6-byte packet (with real CRC7) and reads R1.   */
/*-----------------------------------------------------------------------*/

static uint8_t SdCmdSpi(uint8_t cmd, uint32_t arg) {
    unsigned char packet[6];
    uint8_t response;

    packet[0] = SPI_CMD(cmd);
    packet[1] = (unsigned char)(arg >> 24);
    packet[2] = (unsigned char)(arg >> 16);
    packet[3] = (unsigned char)(arg >> 8);
    packet[4] = (unsigned char)(arg);
    packet[5] = (Crc7(packet, 5) << 1) | 0x01;

    for (int i = 0; i < 6; i++) {
        SdSpiWrite(packet[i]);
    }

    if (cmd == CMD12) {
        SdSpiWrite(SPI_FILL_CHAR);  /* discard stuff byte */
    }

    for (int i = 0; i < 16; i++) {
        response = SdSpiWrite(SPI_FILL_CHAR);
        if (!(response & R1_RESPONSE_RECV)) break;
    }
    return response;
}

static uint8_t SdCmd(uint8_t cmd, uint32_t arg) {
    uint8_t response = R1_NO_RESPONSE;
    if (cmd != CMD12) {
        SdWaitReady(SD_COMMAND_TIMEOUT_MS);
    }
    for (int i = 0; i < SD_CMD_RETRIES; i++) {
        response = SdCmdSpi(cmd, arg);
        if (R1_NO_RESPONSE == response) continue;
        break;
    }
    return response;
}

static uint8_t SdAcmd(uint8_t cmd, uint32_t arg) {
    uint8_t response = R1_NO_RESPONSE;
    if (cmd != CMD12) {
        SdWaitReady(SD_COMMAND_TIMEOUT_MS);
    }
    for (int i = 0; i < SD_CMD_RETRIES; i++) {
        SdCmdSpi(CMD55, 0);
        SdWaitReady(SD_COMMAND_TIMEOUT_MS);
        response = SdCmdSpi(cmd, arg);
        if (R1_NO_RESPONSE == response) continue;
        break;
    }
    return response;
}

static int SdWaitToken(uint8_t token, int timeoutMs) {
    int iters = timeoutMs * 50;
    uint8_t resp;
    do {
        resp = SdSpiWrite(SPI_FILL_CHAR);
        if (resp == token) return 0;
        if (resp != 0xFF)  return -1;  /* error token */
    } while (--iters > 0);
    return -1;
}

/*-----------------------------------------------------------------------*/
/* CSD read (CS managed by caller — must be acquired)                     */
/*-----------------------------------------------------------------------*/

static int SdReadCsdNolock(void) {
    uint8_t csd[16];

    if (SdCmd(CMD9, 0) != 0x00) {
        return -1;
    }
    if (SdWaitToken(SD_START_BLOCK, SD_COMMAND_TIMEOUT_MS) != 0) {
        return -1;
    }
    SpiRecvBytes(csd, 16);
    SdSpiWrite(0xFF); /* CRC */
    SdSpiWrite(0xFF);

    uint8_t csdStructure = (csd[0] >> 6) & 0x03;
    if (csdStructure == 1) {
        uint32_t cSize = ((uint32_t)(csd[7] & 0x3F) << 16) |
                          ((uint32_t)csd[8] << 8) |
                          (uint32_t)csd[9];
        sdState.SectorCount = (cSize + 1) * 1024;
    } else {
        uint32_t cSize      = ((uint32_t)(csd[6] & 0x03) << 10) |
                               ((uint32_t)csd[7] << 2) |
                               ((uint32_t)(csd[8] >> 6) & 0x03);
        uint32_t cSizeMult = ((uint32_t)(csd[9] & 0x03) << 1) |
                               ((uint32_t)(csd[10] >> 7) & 0x01);
        uint32_t readBlLen = csd[5] & 0x0F;
        uint32_t blockNr    = (cSize + 1) * (1u << (cSizeMult + 2));
        uint32_t blockLen   = 1u << readBlLen;
        sdState.SectorCount = (blockNr * blockLen) / 512;
    }
    sdState.SectorSize = 512;
    sdState.BlockSize  = 512;
    return 0;
}

/*-----------------------------------------------------------------------*/
/* FatFS diskio interface                                                */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv) {
    uint8_t response;
    bool isV2 = false;

    if (pdrv != 0) return STA_NOINIT;

    if (SpiBusSetup() != 0) return STA_NOINIT;
    if (SpiSetClock(SD_INIT_BAUDRATE) != 0) return STA_NOINIT;

    sdState.HighCapacity = false;

    /* Initializing sequence: CS HIGH, send 0xFF (74+ clocks) */
    gpio_set_level((gpio_num_t)SD_SPI_CS, 1);
    for (int i = 0; i < 16; i++) SdSpiWrite(SPI_FILL_CHAR);

    /* Acquire — CS stays LOW for the whole init sequence */
    SdAcquire();

    /* CMD0: GO_IDLE_STATE, retried */
    response = R1_NO_RESPONSE;
    for (int i = 0; i < SD_CMD0_RETRIES; i++) {
        response = SdCmd(CMD0, 0);
        if (response == R1_IDLE_STATE) break;
        SdRelease();
        vTaskDelay(pdMS_TO_TICKS(100));
        SdAcquire();
    }
    if (response != R1_IDLE_STATE) {
        SdRelease();
        return STA_NOINIT;
    }

    /* CMD8: SEND_IF_COND — detect SDv2 */
    response = SdCmd(CMD8, 0x000001AA);
    if (response == R1_IDLE_STATE) {
        uint8_t r7[4];
        for (int i = 0; i < 4; i++) r7[i] = SdSpiWrite(SPI_FILL_CHAR);
        if (r7[2] != 0x01 || r7[3] != 0xAA) {
            SdRelease();
            return STA_NOINIT;
        }
        isV2 = true;
    }

    /* Enable CRC checking on the card */
    SdCmd(CMD59, 1);

    /* ACMD41: SD_SEND_OP_COND — loop until idle bit clears (or timeout) */
    uint32_t acmd41Arg = isV2 ? OCR_HCS_CCS : 0;
    int acmd41Iters = SD_COMMAND_TIMEOUT_MS;  /* coarse cap; SdAcmd already paces */
    do {
        response = SdAcmd(ACMD41, acmd41Arg);
    } while ((response & R1_IDLE_STATE) && --acmd41Iters > 0);

    if (response != 0x00) {
        SdRelease();
        return STA_NOINIT;
    }

    /* CMD58: READ_OCR — determine card capacity class (CCS bit) */
    if (isV2) {
        response = SdCmd(CMD58, 0);
        if (response == 0x00) {
            uint8_t ocr[4];
            for (int i = 0; i < 4; i++) ocr[i] = SdSpiWrite(SPI_FILL_CHAR);
            sdState.HighCapacity = (ocr[0] & 0x40) != 0;
        }
    }

    /* Read CSD for capacity (CS still held low) */
    if (SdReadCsdNolock() != 0) {
        SdRelease();
        return STA_NOINIT;
    }

    SdRelease();

    /* Switch to full speed for data transfer */
    if (SpiSetClock(SD_SPI_BAUDRATE) != 0) {
        return STA_NOINIT;
    }

    sdState.Initialized = true;
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv != 0) return STA_NOINIT;
    return sdState.Initialized ? 0 : STA_NOINIT;
}

/* SDSC uses byte addressing; SDHC/SDXC uses block addressing */
static uint32_t SdAddr(LBA_t sector) {
    return sdState.HighCapacity ? (uint32_t)sector
                                  : (uint32_t)sector * 512u;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0)              return RES_PARERR;
    if (!sdState.Initialized)   return RES_NOTRDY;
    if (count == 0)             return RES_PARERR;

    uint32_t addr = SdAddr(sector);

    SdAcquire();
    if (count == 1) {
        if (SdCmd(CMD17, addr) != 0x00) { SdRelease(); return RES_ERROR; }
        if (SdWaitToken(SD_START_BLOCK, SD_COMMAND_TIMEOUT_MS) != 0) { SdRelease(); return RES_ERROR; }
        SpiRecvBytes(buff, sdState.SectorSize);
        SdSpiWrite(0xFF); SdSpiWrite(0xFF); /* CRC */
    } else {
        if (SdCmd(CMD18, addr) != 0x00) { SdRelease(); return RES_ERROR; }
        for (UINT i = 0; i < count; i++) {
            if (SdWaitToken(SD_START_BLOCK, SD_COMMAND_TIMEOUT_MS) != 0) {
                SdCmd(CMD12, 0);
                SdRelease();
                return RES_ERROR;
            }
            SpiRecvBytes(buff + (i * sdState.SectorSize), sdState.SectorSize);
            SdSpiWrite(0xFF); SdSpiWrite(0xFF); /* CRC */
        }
        SdCmd(CMD12, 0);
    }
    SdRelease();
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv != 0)              return RES_PARERR;
    if (!sdState.Initialized)   return RES_NOTRDY;
    if (count == 0)             return RES_PARERR;

    uint32_t addr = SdAddr(sector);
    uint8_t response;

    SdAcquire();
    if (count == 1) {
        if (SdCmd(CMD24, addr) != 0x00) { SdRelease(); return RES_ERROR; }
        SdSpiWrite(SD_START_BLOCK);
        SpiSendBytes(buff, sdState.SectorSize);
        SdSpiWrite(0xFF); SdSpiWrite(0xFF); /* dummy CRC */
        response = SdSpiWrite(0xFF);
        if ((response & 0x1F) != 0x05) { SdRelease(); return RES_ERROR; }
        if (!SdWaitReady(SD_COMMAND_TIMEOUT_MS)) { SdRelease(); return RES_ERROR; }
    } else {
        if (SdCmd(CMD25, addr) != 0x00) { SdRelease(); return RES_ERROR; }
        for (UINT i = 0; i < count; i++) {
            SdSpiWrite(SD_MULTI_TOKEN);
            SpiSendBytes(buff + (i * sdState.SectorSize), sdState.SectorSize);
            SdSpiWrite(0xFF); SdSpiWrite(0xFF);
            response = SdSpiWrite(0xFF);
            if ((response & 0x1F) != 0x05) {
                SdSpiWrite(SD_STOP_TOKEN);
                SdRelease();
                return RES_ERROR;
            }
            if (!SdWaitReady(SD_COMMAND_TIMEOUT_MS)) {
                SdSpiWrite(SD_STOP_TOKEN);
                SdRelease();
                return RES_ERROR;
            }
        }
        SdSpiWrite(SD_STOP_TOKEN);
        SdWaitReady(SD_COMMAND_TIMEOUT_MS);
    }
    SdRelease();
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv != 0)            return RES_PARERR;
    if (!sdState.Initialized) return RES_NOTRDY;

    switch (cmd) {
    case CTRL_SYNC:        return RES_OK;
    case GET_SECTOR_COUNT: if (!buff) return RES_PARERR; *(LBA_t*)buff = sdState.SectorCount; return RES_OK;
    case GET_SECTOR_SIZE:  if (!buff) return RES_PARERR; *(WORD*)buff  = sdState.SectorSize;  return RES_OK;
    case GET_BLOCK_SIZE:   if (!buff) return RES_PARERR; *(DWORD*)buff = sdState.BlockSize;   return RES_OK;
    default:               return RES_PARERR;
    }
}

/*-----------------------------------------------------------------------*/
/* Card detect ISR                                                       */
/*-----------------------------------------------------------------------*/

static void IRAM_ATTR SdCardDetectIsr(void *arg) {
    (void)arg;
    sdState.Initialized = false;  /* force re-init on next mount */
}

/*-----------------------------------------------------------------------*/
/* Platform init                                                         */
/*-----------------------------------------------------------------------*/

bool SDCardInit(void) {
    /* Card detect pin: input with pull-up, ISR on both edges.
     * Switch is normally open; closes to GND when card is present. */
    gpio_config_t detectCfg = {
        .pin_bit_mask = (1ULL << SD_DETECT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&detectCfg);
    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)SD_DETECT_PIN, SdCardDetectIsr, NULL);

    if (gpio_get_level((gpio_num_t)SD_DETECT_PIN) != 0) {
        return false;  /* no card present */
    }

    DSTATUS status = disk_initialize(0);
    return (status == 0);
}
