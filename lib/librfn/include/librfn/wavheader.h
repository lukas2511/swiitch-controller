/*
 * wavheader.h
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

#ifndef RF_WAVHEADER_H_
#define RF_WAVHEADER_H_

#include <stdint.h>
#include <stdlib.h>

/*!
 * \defgroup librfn_wavheader WAV file header
 *
 * \brief Routines to pack/unpack WAV file headers
 *
 * @{
 */

typedef struct rf_wavheader {
	uint8_t chunk_id[4];
	uint32_t chunk_size;
	uint8_t format[4];

	uint8_t fmt_chunk_id[4];
	uint32_t fmt_chunk_size;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint16_t cb_size; // not present in PCM
	uint16_t valid_bits_per_sample; // not present in PCM
	uint32_t channel_mask; // not present in PCM
	uint8_t sub_format[16]; // not present in PCM

	uint8_t fact_chunk_id[4]; // not present in PCM
	uint32_t fact_chunk_size; // not present in PCM
	uint32_t sample_length; // not present in PCM

	uint8_t data_chunk_id[4];
	uint32_t data_chunk_size;
} rf_wavheader_t;

typedef enum {
	RF_WAVHEADER_UNKNOWN = -1,
	RF_WAVHEADER_S16LE,
	RF_WAVHEADER_S32LE,
	RF_WAVHEADER_FLOAT
} rf_wavheader_format_t;

#define RF_WAVHEADER_MIN_SIZE 44

int rf_wavheader_decode(const uint8_t *p, unsigned int sz, rf_wavheader_t *wh);
int rf_wavheader_encode(rf_wavheader_t *wh, uint8_t *p, unsigned int sz);
rf_wavheader_format_t rf_wavheader_get_format(rf_wavheader_t *wh);
void rf_wavheader_init(rf_wavheader_t *wh, int sfreq, int num_channels,
		rf_wavheader_format_t format);
void rf_wavheader_set_num_frames(rf_wavheader_t *wh, unsigned int num_frames);
char *rf_wavheader_tostring(rf_wavheader_t *wh);
int rf_wavheader_validate(rf_wavheader_t *wh);

/*! @} */
#endif // RF_WAVHEADER_H_
