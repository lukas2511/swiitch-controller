#ifndef __BITSET_H__
#define __BITSET_H__

#ifndef _UV
#define _UV(x) (1 << (x))
#endif

#ifndef _SETBIT
#define _SETBIT(x, y) ((x) |= _UV(y))
#endif

#ifndef _CLRBIT
#define _CLRBIT(x, y) ((x) &= ~_UV(y))
#endif

#ifndef _GETBITS
#define _GETBITS(c, start, len) (((c << (8-start-1)) & 0xFF) >> (8-len))
#endif

#ifndef _GETBIT
#define _GETBIT(c, n) _GETBITS(c, n, 1)
#endif

#endif
