# CRC

This module contains implementations for CRC-16(`crc.h`/`crc.c`) and CRC-8(`crc8.h`/`crc8.c`)

## Usage

You have two options when using the CRC-module:

### 1. Bulk calculation on a complete buffer

```c
uint8_t buffer[200];//some buffer containing data
crc_t   crc_ctx;//The structure containing the configuration and state of the crc-module

crc_init_handler(&crc_ctx, 0x1021, 0xFFFF, 0x0000);
uint16_t crc = crc_calc(&crc_ctx, buffer, 200);
```

### 2. Updating the calculation for every byte

```c
uint8_t buffer[200];//some buffer containing data
crc_t   crc_ctx;//The structure containing the configuration and state of the crc-module

crc_init_handler(&crc_ctx, 0x1021, 0xFFFF, 0x0000);
uint16_t crc = crc_ctx.initial;

for(uint8_t i = 0; i < 200; i++)
{
    crc = crc_calc_byte(&crc_ctx, crc, buffer[i]);
}

crc ^= crc_ctx.final_xor;
```

The corresponding functions for CRC-8 are called `crc8_init`, `crc8_calc` and `crc8_calc_byte`