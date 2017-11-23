/*
 * ringbuftest.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#undef NDEBUG

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <librfn.h>

uint8_t smallbuf[4];
ringbuf_t smallring = RINGBUF_VAR_INIT(smallbuf, sizeof(smallbuf));

uint8_t largebuf[4096];
ringbuf_t largering = RINGBUF_VAR_INIT(largebuf, sizeof(largebuf));

void *producer(void *p)
{
	ringbuf_t *rb = p;
	uint32_t seed = RAND31_VAR_INIT;

	uint32_t endtime = time_now() + 2000000; // 2s
	while (time_now() < endtime) {
		uint32_t r = rand31_r(&seed);
		if ((r & 0xffff) == 0x1111)
			usleep(1000);
		while (!ringbuf_put(rb, r))
			;
	}
	
	return NULL;
}

void *consumer(void *p)
{
	ringbuf_t *rb = p;
	uint32_t seed = RAND31_VAR_INIT;

	for (long long int c=0; ; c++) {
		uint32_t r = rand31_r(&seed);
		int d;

		if ((r & 0xffff) == 0x2222)
			usleep(1000);

		uint32_t endtime = time_now() + 100000; // 100ms
		while (-1 == (d = ringbuf_get(rb))) {
			if (cyclecmp32(time_now(), endtime) > 0) {
				fprintf(stderr, "Existing after %lld bytes\n",
					c);
				return NULL; // timeout
			}
		}

		d &= 0xff;
		r &= 0xff;
		if (d != r) {
			fprintf(stderr, "MISMATCH after %lld bytes "
					"(exp %02x got %02x)\n", c, r, d);
			assert(0);
		}
	}
}

int main()
{
	ringbuf_t myring;

	/* prove the equivalence of the initializer and the init fn */
	ringbuf_init(&myring, smallbuf, sizeof(smallbuf));
	verify(0 == memcmp(&smallring, &myring, sizeof(smallring)));

	/* initially empty */
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));

	/* becomes full after three puts */
	verify(ringbuf_put(&smallring, 0));
	verify(ringbuf_put(&smallring, 1));
	verify(ringbuf_put(&smallring, 2));
	verify(!ringbuf_put(&smallring, 3));

	/* get/put/still full (for all possible read/write indices */
	verify(0 == ringbuf_get(&smallring));
	verify(ringbuf_put(&smallring, 3));
	verify(!ringbuf_put(&smallring, 4));
	verify(1 == ringbuf_get(&smallring));
	verify(ringbuf_put(&smallring, 4));
	verify(!ringbuf_put(&smallring, 5));
	verify(2 == ringbuf_get(&smallring));
	verify(ringbuf_put(&smallring, 5));
	verify(!ringbuf_put(&smallring, 6));
	verify(3 == ringbuf_get(&smallring));
	verify(ringbuf_put(&smallring, 6));
	verify(!ringbuf_put(&smallring, 7));

	/* empty the ring */
	verify(4 == ringbuf_get(&smallring));
	verify(5 == ringbuf_get(&smallring));
	verify(6 == ringbuf_get(&smallring));
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));

	/* put/get/still empty (for all possible read/write indices */
	verify(ringbuf_put(&smallring, 7));
	verify(7 == ringbuf_get(&smallring));
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));
	verify(ringbuf_put(&smallring, 8));
	verify(8 == ringbuf_get(&smallring));
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));
	verify(ringbuf_put(&smallring, 9));
	verify(9 == ringbuf_get(&smallring));
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));
	verify(ringbuf_put(&smallring, 10));
	verify(10 == ringbuf_get(&smallring));
	verify(-1 == ringbuf_get(&smallring) && ringbuf_empty(&smallring));

	/* correct sign handling for all data */
	for (int i=0; i<0x100; i++) {
		verify(ringbuf_put(&smallring, i));
		verify(i == ringbuf_get(&smallring));
	}

	/* producer/consumer soak test (pretty pointless on strongly ordered
	 * x86 but even on ARM/MIPS the runtime is pretty short)
	 */
	pthread_t pt, ct;
	verify(0 == pthread_create(&pt, NULL, producer, &largering));
	verify(0 == pthread_create(&ct, NULL, consumer, &largering));
	verify(0 == pthread_join(pt, NULL));
	verify(0 == pthread_join(ct, NULL));
	
	return 0;
}
