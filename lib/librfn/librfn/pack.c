/*
 * pack.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012 Daniel Thompson <daniel@redfelineninja.org.uk> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "librfn.h"

void rf_pack_init(rf_pack_t *pack, void *p, unsigned int sz)
{
	pack->basep = p;
	pack->p = p;
	pack->endp = pack->basep + sz;
}

int rf_pack_remaining(rf_pack_t *pack)
{
	return pack->endp - pack->p;
}

#define PACK(pack, decl, sz) \
	uint8_t *decl = pack->p; \
	pack->p += sz; \
	if (pack->p <= pack->endp)

void rf_pack_bytes(rf_pack_t *pack, void *p, unsigned int sz)
{
	PACK(pack, q, sz) {
		if (p)
			memcpy(q, p, sz);
		else
			memset(q, 0, sz);
	}
}

void rf_pack_char(rf_pack_t *pack, char c);
void rf_pack_s8(rf_pack_t *pack, int8_t s8);
void rf_pack_u8(rf_pack_t *pack, int16_t u8);
void rf_pack_s16be(rf_pack_t *pack, int16_t s16);
void rf_pack_s16le(rf_pack_t *pack, int16_t s16)
{
	PACK(pack, p, 2) {
		p[0] = s16 & 0xff;
		p[1] = (s16 >> 8) & 0xff;
	}
}

void rf_pack_u16be(rf_pack_t *pack, uint16_t u16)
{
	PACK(pack, p, 2) {
		p[0] = (u16 >> 8) & 0xff;
		p[1] = u16 & 0xff;
	}
}

void rf_pack_u16le(rf_pack_t *pack, uint16_t u16)
{
	PACK(pack, p, 2) {
		p[0] = u16 & 0xff;
		p[1] = (u16 >> 8) & 0xff;
	}
}

void rf_pack_s32be(rf_pack_t *pack, int32_t s32);

void rf_pack_s32le(rf_pack_t *pack, int32_t s32)
{
	PACK(pack, p, 4) {
		p[0] = s32 & 0xff;
		p[1] = (s32 >> 8) & 0xff;
		p[2] = (s32 >> 16) & 0xff;
		p[3] = (s32 >> 24) & 0xff;
	}
}

void rf_pack_u32be(rf_pack_t *pack, uint32_t u32);
void rf_pack_u32le(rf_pack_t *pack, uint32_t u32)
{
	PACK(pack, p, 4) {
		p[0] = u32 & 0xff;
		p[1] = (u32 >> 8) & 0xff;
		p[2] = (u32 >> 16) & 0xff;
		p[3] = (u32 >> 24) & 0xff;
	}
}

#define UNPACK(pack, decl, sz) \
	uint8_t *decl = pack->p; \
	pack->p += sz; \
	if (pack->p > pack->endp) \
		return 0; \
	else

void rf_unpack_bytes(rf_pack_t *pack, void *p, unsigned int sz)
{
	// this is an unpack operation but we use the pack macro for
	// this special case of an unpack macro that does not
	// return a value.
	PACK(pack, q, sz) {
		if (p)
			memcpy(p, q, sz);
	} else {
		if (p)
			memset(p, 0, sz);
	}
}

char rf_unpack_char(rf_pack_t *pack)
{
	UNPACK(pack, p, 1) {
		char c = p[0];
		return c;
	}
}

int8_t rf_unpack_s8(rf_pack_t *pack)
{
	UNPACK(pack, p, 1) {
		int8_t s8 = p[0];
		return s8;
	}
}

uint8_t rf_unpack_u8(rf_pack_t *pack)
{
	UNPACK(pack, p, 1) {
		uint8_t u8 = p[0];
		return u8;
	}
}

int16_t rf_unpack_s16be(rf_pack_t *pack);
int16_t rf_unpack_s16le(rf_pack_t *pack);
uint16_t rf_unpack_u16be(rf_pack_t *pack);

uint16_t rf_unpack_u16le(rf_pack_t *pack)
{
	UNPACK(pack, p, 2) {
		uint16_t u16 = p[0] | p[1] << 8;
		return u16;
	}
}

int32_t rf_unpack_s32be(rf_pack_t *pack);
int32_t rf_unpack_s32le(rf_pack_t *pack);
uint32_t rf_unpack_u32be(rf_pack_t *pack);
uint32_t rf_unpack_u32le(rf_pack_t *pack)
{
	UNPACK(pack, p, 4) {
		uint32_t u32 = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
		return u32;
	}
}

