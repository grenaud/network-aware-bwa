/* The MIT License

   Copyright (c) 2008 Genome Research Ltd (GRL).

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/* Contact: Heng Li <lh3@sanger.ac.uk> */

#ifndef BWA_BWT_H
#define BWA_BWT_H

#include <stdint.h>
#include "bntseq.h"

// requirement: (OCC_INTERVAL%16 == 0)
#define OCC_INTERVAL 0x80

#ifndef BWA_UBYTE
#define BWA_UBYTE
typedef unsigned char ubyte_t;
#endif
typedef uint32_t bwtint_t;

typedef struct {
    bwtint_t primary; // S^{-1}(0), or the primary index of BWT
    bwtint_t L2[5]; // C(), cumulative count
    bwtint_t seq_len; // sequence length
    bwtint_t bwt_size; // size of bwt, about seq_len/4
    uint32_t *bwt; // BWT
    // occurance array, separated to two parts
    uint32_t cnt_table[256];
    // suffix array
    int sa_intv;
    bwtint_t n_sa;
    bwtint_t *sa;
#ifdef USE_MMAP
    char mmap_bwt ;
    char mmap_sa ;
#endif
} bwt_t;

#define bwt_bwt(b, k) ((b)->bwt[(k)/OCC_INTERVAL*12 + 4 + (k)%OCC_INTERVAL/16])
#define __occ_aux4(bwt, b)						\
    ((bwt)->cnt_table[(b)&0xff] + (bwt)->cnt_table[(b)>>8&0xff]		\
     + (bwt)->cnt_table[(b)>>16&0xff] + (bwt)->cnt_table[(b)>>24])

/* retrieve a character from the $-removed BWT string. Note that
 * bwt_t::bwt is not exactly the BWT string and therefore this macro is
 * called bwt_B0 instead of bwt_B */
#define bwt_B0(b, k) (bwt_bwt(b, k)>>((~(k)&0xf)<<1)&3)

#define bwt_occ_intv(b, k) ((b)->bwt + (k)/OCC_INTERVAL*12)

// inverse Psi function
#define bwt_invPsi(bwt, k)												\
    (((k) == (bwt)->primary)? 0 :					\
     ((k) < (bwt)->primary)?						\
     (bwt)->L2[bwt_B0(bwt, k)] + bwt_occ(bwt, k, bwt_B0(bwt, k))	\
     : (bwt)->L2[bwt_B0(bwt, (k)-1)] + bwt_occ(bwt, k, bwt_B0(bwt, (k)-1)))

#ifdef __cplusplus
extern "C" {
#endif

    void bwt_dump_bwt(const char *fn, const bwt_t *bwt);
    void bwt_dump_sa(const char *fn, const bwt_t *bwt);

    ubyte_t *bwt_restore_pac(const bntseq_t *bns, int);
    bwt_t *bwt_restore_bwt(const char *fn,int);
    void bwt_restore_sa(const char *fn, bwt_t *bwt, int);
    
    void bwt_destroy(bwt_t *bwt);
    void bwt_destroy_bwt(bwt_t *bwt, bwtint_t bwt_size);
    void bwt_destroy_sa(bwt_t *bwt);
    void bwt_destroy_pac( ubyte_t *pac, const bntseq_t *bns );

    void bwt_bwtgen(const char *fn_pac, const char *fn_bwt); // from BWT-SW
    void bwt_cal_sa(bwt_t *bwt, int intv);

    void bwt_bwtupdate_core(bwt_t *bwt);

    /* inline bwtint_t bwt_occ(const bwt_t *bwt, bwtint_t k, ubyte_t c); */
    /* inline void bwt_occ4(const bwt_t *bwt, bwtint_t k, bwtint_t cnt[4]); */
    bwtint_t bwt_sa(const bwt_t *bwt, bwtint_t k);

    // more efficient version of bwt_occ/bwt_occ4 for retrieving two close Occ values
    void bwt_gen_cnt_table(bwt_t *bwt);
    //inline void bwt_2occ(const bwt_t *bwt, bwtint_t k, bwtint_t l, ubyte_t c, bwtint_t *ok, bwtint_t *ol);
    //inline void bwt_2occ4(const bwt_t *bwt, bwtint_t k, bwtint_t l, bwtint_t cntk[4], bwtint_t cntl[4]);
    //static inline int __occ_aux(uint64_t y, int c);

    static inline int __occ_aux(uint64_t y, int c)    {
	// reduce nucleotide counting to bits counting
	y = ((c&2)? y : ~y) >> 1 & ((c&1)? y : ~y) & 0x5555555555555555ull;
	// count the number of 1s in y
	y = (y & 0x3333333333333333ull) + (y >> 2 & 0x3333333333333333ull);
	return ((y + (y >> 4)) & 0xf0f0f0f0f0f0f0full) * 0x101010101010101ull >> 56;
    }

    inline bwtint_t bwt_occ(const bwt_t *bwt, bwtint_t k, ubyte_t c){
	bwtint_t n, l, j;
	uint32_t *p;

	if (k == bwt->seq_len) return bwt->L2[c+1] - bwt->L2[c];
	if (k == (bwtint_t)(-1)) return 0;
	if (k >= bwt->primary) --k; // because $ is not in bwt

	// retrieve Occ at k/OCC_INTERVAL
	n = (p = bwt_occ_intv(bwt, k))[c];
	p += 4; // jump to the start of the first BWT cell

	// calculate Occ up to the last k/32
	j = k >> 5 << 5;
	for (l = k/OCC_INTERVAL*OCC_INTERVAL; l < j; l += 32, p += 2)
		n += __occ_aux((uint64_t)p[0]<<32 | p[1], c);

	// calculate Occ
	n += __occ_aux(((uint64_t)p[0]<<32 | p[1]) & ~((1ull<<((~k&31)<<1)) - 1), c);
	if (c == 0) n -= ~k&31; // corrected for the masked bits

	return n;
    }

    inline void bwt_occ4(const bwt_t *bwt, bwtint_t k, bwtint_t cnt[4])    {
	bwtint_t l, j, x;
	uint32_t *p;
	if (k == (bwtint_t)(-1)) {
	    memset(cnt, 0, 4 * sizeof(bwtint_t));
	    return;
	}
	if (k >= bwt->primary) --k; // because $ is not in bwt
	p = bwt_occ_intv(bwt, k);
	memcpy(cnt, p, 16);
	p += 4;
	j = k >> 4 << 4;
	for (l = k / OCC_INTERVAL * OCC_INTERVAL, x = 0; l < j; l += 16, ++p)
	    x += __occ_aux4(bwt, *p);
	x += __occ_aux4(bwt, *p & ~((1U<<((~k&15)<<1)) - 1)) - (~k&15);
	cnt[0] += x&0xff; cnt[1] += x>>8&0xff; cnt[2] += x>>16&0xff; cnt[3] += x>>24;
    }

    
    inline void bwt_2occ(const bwt_t *bwt, bwtint_t k, bwtint_t l, ubyte_t c, bwtint_t *ok, bwtint_t *ol)
    {
	bwtint_t _k, _l;
	if (k == l) {
	    *ok = *ol = bwt_occ(bwt, k, c);
	    return;
	}
	_k = (k >= bwt->primary)? k-1 : k;
	_l = (l >= bwt->primary)? l-1 : l;
	if (_l/OCC_INTERVAL != _k/OCC_INTERVAL || k == (bwtint_t)(-1) || l == (bwtint_t)(-1)) {
	    *ok = bwt_occ(bwt, k, c);
	    *ol = bwt_occ(bwt, l, c);
	} else {
	    bwtint_t m, n, i, j;
	    uint32_t *p;
	    if (k >= bwt->primary) --k;
	    if (l >= bwt->primary) --l;
	    n = (p = bwt_occ_intv(bwt, k))[c];
	    p += 4;
	    // calculate *ok
	    j = k >> 5 << 5;
	    for (i = k/OCC_INTERVAL*OCC_INTERVAL; i < j; i += 32, p += 2)
		n += __occ_aux((uint64_t)p[0]<<32 | p[1], c);
	    m = n;
	    n += __occ_aux(((uint64_t)p[0]<<32 | p[1]) & ~((1ull<<((~k&31)<<1)) - 1), c);
	    if (c == 0) n -= ~k&31; // corrected for the masked bits
	    *ok = n;
	    // calculate *ol
	    j = l >> 5 << 5;
	    for (; i < j; i += 32, p += 2)
		m += __occ_aux((uint64_t)p[0]<<32 | p[1], c);
	    m += __occ_aux(((uint64_t)p[0]<<32 | p[1]) & ~((1ull<<((~l&31)<<1)) - 1), c);
	    if (c == 0) m -= ~l&31; // corrected for the masked bits
	    *ol = m;
	}
    }

    // an analogy to bwt_occ4() but more efficient, requiring k <= l
    inline void bwt_2occ4(const bwt_t *bwt, bwtint_t k, bwtint_t l, bwtint_t cntk[4], bwtint_t cntl[4])
    {
	bwtint_t _k, _l;
	if (k == l) {
	    bwt_occ4(bwt, k, cntk);
	    memcpy(cntl, cntk, 4 * sizeof(bwtint_t));
	    return;
	}
	_k = (k >= bwt->primary)? k-1 : k;
	_l = (l >= bwt->primary)? l-1 : l;
	if (_l/OCC_INTERVAL != _k/OCC_INTERVAL || k == (bwtint_t)(-1) || l == (bwtint_t)(-1)) {
	    bwt_occ4(bwt, k, cntk);
	    bwt_occ4(bwt, l, cntl);
	} else {
	    bwtint_t i, j, x, y;
	    uint32_t *p;
	    int cl[4];
	    if (k >= bwt->primary) --k; // because $ is not in bwt
	    if (l >= bwt->primary) --l;
	    cl[0] = cl[1] = cl[2] = cl[3] = 0;
	    p = bwt_occ_intv(bwt, k);
	    memcpy(cntk, p, 4 * sizeof(bwtint_t));
	    p += 4;
	    // prepare cntk[]
	    j = k >> 4 << 4;
	    for (i = k / OCC_INTERVAL * OCC_INTERVAL, x = 0; i < j; i += 16, ++p)
		x += __occ_aux4(bwt, *p);
	    y = x;
	    x += __occ_aux4(bwt, *p & ~((1U<<((~k&15)<<1)) - 1)) - (~k&15);
	    // calculate cntl[] and finalize cntk[]
	    j = l >> 4 << 4;
	    for (; i < j; i += 16, ++p) y += __occ_aux4(bwt, *p);
	    y += __occ_aux4(bwt, *p & ~((1U<<((~l&15)<<1)) - 1)) - (~l&15);
	    memcpy(cntl, cntk, 16);
	    cntk[0] += x&0xff; cntk[1] += x>>8&0xff; cntk[2] += x>>16&0xff; cntk[3] += x>>24;
	    cntl[0] += y&0xff; cntl[1] += y>>8&0xff; cntl[2] += y>>16&0xff; cntl[3] += y>>24;
	}
    }







    int bwt_match_exact(const bwt_t *bwt, int len, const ubyte_t *str, bwtint_t *sa_begin, bwtint_t *sa_end);
    int bwt_match_exact_alt(const bwt_t *bwt, int len, const ubyte_t *str, bwtint_t *k0, bwtint_t *l0);

#ifdef __cplusplus
}
#endif

#endif
