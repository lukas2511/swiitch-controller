/*
 * wavheader.c
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

const uint8_t null_id[] = { 0, 0, 0, 0 };
const uint8_t riff[] = { 'R', 'I', 'F', 'F' };
const uint8_t wave[] = { 'W', 'A', 'V', 'E' };
const uint8_t fmt[] = { 'f' , 'm', 't', ' ' };
const uint8_t fact[] = { 'f', 'a', 'c', 't' };
const uint8_t data[] = { 'd', 'a', 't', 'a' };


int rf_wavheader_decode(const uint8_t *p, unsigned int sz, rf_wavheader_t *wh)
{
	rf_pack_t pack;
	rf_pack_init(&pack, (void *) p, sz);

	memset(wh, 0, sizeof(*wh));

	rf_unpack_bytes(&pack, wh->chunk_id, 4);
	wh->chunk_size = rf_unpack_u32le(&pack);
	rf_unpack_bytes(&pack, wh->format, 4);

	rf_unpack_bytes(&pack, wh->fmt_chunk_id, 4);
	wh->fmt_chunk_size = rf_unpack_u32le(&pack);
	wh->audio_format = rf_unpack_u16le(&pack);
	wh->num_channels = rf_unpack_u16le(&pack);
	wh->sample_rate = rf_unpack_u32le(&pack);
	wh->byte_rate = rf_unpack_u32le(&pack);
	wh->block_align = rf_unpack_u16le(&pack);
	wh->bits_per_sample = rf_unpack_u16le(&pack);
	if (wh->fmt_chunk_size >= 18) {
		wh->cb_size = rf_unpack_u16le(&pack);

		if (22 == wh->cb_size) {
			wh->valid_bits_per_sample = rf_unpack_u16le(&pack);
			wh->channel_mask = rf_unpack_u32le(&pack);
			rf_unpack_bytes(&pack, wh->sub_format, 16);
		} else {
			rf_unpack_bytes(&pack, NULL, (wh->fmt_chunk_size - 18));
		}
	}

	rf_unpack_bytes(&pack, wh->data_chunk_id, 4);

	/* this might actually be a fact chunk rather than a data chunk,
	 * if so we need to grab that (in order to handle IEEE f.p. samples)
	 */
	if (0 == memcmp(fact, wh->data_chunk_id, 4)) {
		memcpy(wh->fact_chunk_id, wh->data_chunk_id, 4);
		wh->fact_chunk_size = rf_unpack_u32le(&pack);
		wh->sample_length = rf_unpack_u32le(&pack);

		rf_unpack_bytes(&pack, wh->data_chunk_id, 4);
	}

	wh->data_chunk_size = rf_unpack_u32le(&pack);

	/* do some basic validation */
	if (0 != memcmp(riff, wh->chunk_id, 4))
		return -EINVAL;
	if (wh->chunk_size < (12 + wh->fmt_chunk_size + wh->fact_chunk_size))
		return -EINVAL;
	if (0 != memcmp(wave, wh->format, 4))
		return -EINVAL;

	/* if we have tried to read past the end of the buffer then
	 * rf_pack_remaining() will return a -ve number and therefore
	 * the return value will be larger than the value supplied.
	 */
	return sz - rf_pack_remaining(&pack);
}

int rf_wavheader_encode(rf_wavheader_t *wh, uint8_t *p, unsigned int sz)
{
	rf_pack_t pack;
	rf_pack_init(&pack, (void *) p, sz);

	rf_pack_bytes(&pack, wh->chunk_id, 4);
	rf_pack_u32le(&pack, wh->chunk_size);
	rf_pack_bytes(&pack, wh->format, 4);

	rf_pack_bytes(&pack, wh->fmt_chunk_id, 4);
	rf_pack_u32le(&pack, wh->fmt_chunk_size);
	rf_pack_u16le(&pack, wh->audio_format);
	rf_pack_u16le(&pack, wh->num_channels);
	rf_pack_u32le(&pack, wh->sample_rate);
	rf_pack_u32le(&pack, wh->byte_rate);
	rf_pack_u16le(&pack, wh->block_align);
	rf_pack_u16le(&pack, wh->bits_per_sample);
	if (wh->fmt_chunk_size >= 18) {
		rf_pack_u16le(&pack, wh->cb_size);

		if (22 == wh->cb_size) {
			rf_pack_u16le(&pack, wh->valid_bits_per_sample);
			rf_pack_u32le(&pack, wh->channel_mask);
			rf_pack_bytes(&pack, wh->sub_format, 16);
		} else {
			rf_pack_bytes(&pack, NULL, (wh->fmt_chunk_size - 18));
		}
	}

	if (0 == memcmp(fact, wh->fact_chunk_id, 4)) {
		rf_pack_bytes(&pack, wh->fact_chunk_id, 4);
		rf_pack_u32le(&pack, wh->fact_chunk_size);
		rf_pack_u32le(&pack, wh->sample_length);
	}

	rf_pack_bytes(&pack, wh->data_chunk_id, 4);
	rf_pack_u32le(&pack, wh->data_chunk_size);

	/* if we have tried to write past the end of the buffer then
	 * rf_pack_remaining() will return a -ve number and therefore
	 * the return value will be larger than the value supplied.
	 */
	return sz - rf_pack_remaining(&pack);
}

rf_wavheader_format_t rf_wavheader_get_format(rf_wavheader_t *wh)
{
	switch (wh->audio_format) {
	case 1:
		return (wh->bits_per_sample == 16 ? RF_WAVHEADER_S16LE :
			wh->bits_per_sample == 32 ? RF_WAVHEADER_S32LE :
					            RF_WAVHEADER_UNKNOWN);
	case 3:
		return (wh->bits_per_sample == 32 ? RF_WAVHEADER_FLOAT :
						    RF_WAVHEADER_UNKNOWN);
	case 0xfffe:
		// TODO: add support for f.p. sampes in extended format
		return (wh->bits_per_sample == 16 ? RF_WAVHEADER_S16LE :
					            RF_WAVHEADER_S32LE);
	default:
		return RF_WAVHEADER_UNKNOWN;
	}
}

void rf_wavheader_init(rf_wavheader_t *wh, int sfreq, int num_channels,
		rf_wavheader_format_t format)
{
	memcpy(wh->chunk_id, riff, 4);
	wh->chunk_size = 12 + 18 + 12 + 8; // chunks: riff, fmt, fact, data
	memcpy(wh->format, wave, 4);

	memcpy(wh->fmt_chunk_id, fmt, 4);
	wh->fmt_chunk_size = (format == RF_WAVHEADER_FLOAT ? 18 : 16);
	wh->audio_format = (format == RF_WAVHEADER_FLOAT ? 3 : 1);
	wh->num_channels = num_channels;
	wh->sample_rate = sfreq;

	uint8_t bytes_per_sample = (format == RF_WAVHEADER_S16LE ? 2 : 4);
	wh->byte_rate = sfreq * bytes_per_sample * num_channels;
	wh->block_align = bytes_per_sample * num_channels;
	wh->bits_per_sample = bytes_per_sample * 8;
	wh->cb_size = 0;
	// when cb_size is zero the rest of the fmt chunk is absent

	if (format == RF_WAVHEADER_FLOAT) {
		memcpy(wh->fact_chunk_id, fact, 4);
		wh->fact_chunk_size = 12;
		wh->sample_length = 0; // must be filled in after writing the samples
	}

	memcpy(wh->data_chunk_id, data, 4);
	wh->data_chunk_size = 0;
}

void rf_wavheader_set_num_frames(rf_wavheader_t *wh, unsigned int num_frames)
{
	// return chunk_size to merely the size of the header
	wh->chunk_size -= wh->data_chunk_size;

	wh->data_chunk_size = num_frames * wh->block_align;
	// doesn't matter if there is no fact chunk, we'll not emit this if this
	// chunk is absent
	wh->sample_length = num_frames * wh->num_channels;
	wh->chunk_size += num_frames * wh->block_align;
}

static const char *format_tostring(rf_wavheader_format_t format)
{
	switch(format) {
#define C(x) case RF_WAVHEADER_##x: return #x
	C(S16LE);
	C(S32LE);
	C(FLOAT);
#undef C
	default:
		return "UNKNOWN";
	}
}



char *rf_wavheader_tostring(rf_wavheader_t *wh)
{
	return strdup_printf("WAVE file: %d samples in %s %dch %dHz",
			wh->data_chunk_size / (wh->block_align),
			format_tostring(rf_wavheader_get_format(wh)),
			wh->num_channels, wh->sample_rate);

}

int rf_wavheader_validate(rf_wavheader_t *wh)
{
	if (0 != memcmp(riff, wh->chunk_id, 4))
		return -EINVAL;
	if (wh->chunk_size < (12 + wh->fmt_chunk_size + wh->fact_chunk_size))
		return -EINVAL;
	if (0 != memcmp(wave, wh->format, 4))
		return -EINVAL;

	if (0 != memcmp(fmt, wh->fmt_chunk_id, 4))
		return -EINVAL;

	if (0 != memcmp(data, wh->data_chunk_id, 4))
		return -EINVAL;

	return 0;
}
