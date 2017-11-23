/*
 * pack.h
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012-2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef RF_PACK_H_
#define RF_PACK_H_

#include <stdint.h>

/*!
 * \defgroup librfn_pack Bitstream pack/unpack
 *
 * \brief Insert and extract data from a bitstream.
 *
 * \todo At present the smallest unit of data that can be packed/unpacked is a
 *       byte. This makes this code insufficient for data compression.
 *
 * @{
 */

typedef struct rf_pack {
	uint8_t *basep;
	uint8_t *endp;
	uint8_t *p;
} rf_pack_t;

void rf_pack_init(rf_pack_t *pack, void *p, unsigned int sz);
int rf_pack_remaining(rf_pack_t *pack);


void rf_pack_bytes(rf_pack_t *pack, void *p, unsigned int sz);
void rf_pack_char(rf_pack_t *pack, char c);
void rf_pack_s8(rf_pack_t *pack, int8_t s8);
void rf_pack_u8(rf_pack_t *pack, int16_t u8);
void rf_pack_s16be(rf_pack_t *pack, int16_t s16);
void rf_pack_s16le(rf_pack_t *pack, int16_t s16);
void rf_pack_u16be(rf_pack_t *pack, uint16_t u16);
void rf_pack_u16le(rf_pack_t *pack, uint16_t u16);
void rf_pack_s32be(rf_pack_t *pack, int32_t s32);
void rf_pack_s32le(rf_pack_t *pack, int32_t s32);
void rf_pack_u32be(rf_pack_t *pack, uint32_t u32);
void rf_pack_u32le(rf_pack_t *pack, uint32_t u32);

void rf_unpack_bytes(rf_pack_t *pack, void *p, unsigned int sz);
char rf_unpack_char(rf_pack_t *pack);
int8_t rf_unpack_s8(rf_pack_t *pack);
uint8_t rf_unpack_u8(rf_pack_t *pack);
int16_t rf_unpack_s16be(rf_pack_t *pack);
int16_t rf_unpack_s16le(rf_pack_t *pack);
uint16_t rf_unpack_u16be(rf_pack_t *pack);
uint16_t rf_unpack_u16le(rf_pack_t *pack);
int32_t rf_unpack_s32be(rf_pack_t *pack);
int32_t rf_unpack_s32le(rf_pack_t *pack);
uint32_t rf_unpack_u32be(rf_pack_t *pack);
uint32_t rf_unpack_u32le(rf_pack_t *pack);

/*! @} */
#endif // RF_PACK_H_
