/*
 * =============================================================================
 *
 *       Filename:  atomic.h
 *
 *    Description:  Atomic Assemble Code
 *
 *        Version:  1.0
 *        Created:  11/25/13 13:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ShadowStar, <orphen.leiliu@gmail.com>
 *   Organization:  Gmail
 *
 * =============================================================================
 */
#ifndef __atomic_INC__
#define __atomic_INC__
#include "cvmx-asm.h"
#include "common.h"

#define ___atomic_add_nosync(D)		({				\
	__asm__ __volatile__ (						\
	"	saa"#D"	%[v],	%[p]		\n"			\
	: [p] "+m" (*__p) : [v] "r" (__v) : "memory"); })

#define __atomic_add_nosync(TYPE, p, v)	({				\
	TYPE##_t *__p = p, __v = v;					\
	BUILD_EXPR_64or32(*__p, ___atomic_add_nosync(d),		\
			    ___atomic_add_nosync()); })

#define __atomic_add(TYPE, p, v)	({				\
	CVMX_SYNCWS; __atomic_add_nosync(TYPE, p, v); CVMX_SYNCWS; })

#define atomic_add32_nosync(p, v)	__atomic_add_nosync(uint32, p, v)
#define atomic_add64_nosync(p, v)	__atomic_add_nosync(uint64, p, v)
#define atomic_add32(p, v)		__atomic_add(uint32, p, v)
#define atomic_add64(p, v)		__atomic_add(uint64, p, v)

#define ___cmpadd(A, D)		({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tmp],	%[ptr]		\n"			\
	"	b"#A"	%[tmp],	%[old],	2f	\n"			\
	"	move	%[ex],	%[tmp]		\n"			\
	"\t"#D"addu	%[tmp],	%[v]		\n"			\
	"	sc"#D"	%[tmp],	%[ptr]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [ptr] "+m" (*(__p)), [ex] "=&r" (__e), [tmp] "=&r" (__t)	\
	: [old] "r" (__o), [v] "r" (__v) : "memory"); __e; })

#define __cmpadd(p, o, v, A)	({					\
	typeof(p) __p = p; typeof(*(p)) __o = o; typeof(*(p)) __v = v;	\
	typeof(*(p)) __e, __t;						\
	BUILD_EXPR_64or32(*__p, ___cmpadd(A, d), ___cmpadd(A,)); })

#define cmpadd_eq(p, o, v)		__cmpadd(p, o, v, ne)
#define cmpadd_ne(p, o, v)		__cmpadd(p, o, v, eq)
#define cmpadd_ge(p, o, v)		__cmpadd(p, o, v, lt)
#define cmpadd_geu(p, o, v)		__cmpadd(p, o, v, ltu)
#define cmpadd_gt(p, o, v)		__cmpadd(p, o, v, le)
#define cmpadd_gtu(p, o, v)		__cmpadd(p, o, v, leu)
#define cmpadd_le(p, o, v)		__cmpadd(p, o, v, gt)
#define cmpadd_leu(p, o, v)		__cmpadd(p, o, v, gtu)
#define cmpadd_lt(p, o, v)		__cmpadd(p, o, v, ge)
#define cmpadd_ltu(p, o, v)		__cmpadd(p, o, v, geu)

#define __atomic_set(TYPE, p, v)	({				\
	TYPE##_t *__p = p, __v = v;					\
	CVMX_SYNCWS; *__p = __v; CVMX_SYNCWS; })

#define atomic_set32(p, v)		__atomic_set(uint32, p, v)
#define atomic_set64(p, v)		__atomic_set(uint64, p, v)

#define __atomic_get(TYPE, p)		*(volatile TYPE##_t *)(p)

#define atomic_get32(p)			__atomic_get(uint32, p)
#define atomic_get64(p)			__atomic_get(uint64, p)

#ifdef CVMX_CAVIUM_OCTEON2
#define ___atomic_load_add_nosync(D)	({				\
	if (BUILD_CONST(__v, 1)) {					\
		__asm__ __volatile__ (					\
		"	lai"#D"	%[old],	(%[p])		\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p) : "memory");				\
	} else if (BUILD_CONST(__v, (typeof(__v))-1)) {			\
		__asm__ __volatile__ (					\
		"	lad"#D"	%[old],	(%[p])		\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p) : "memory");				\
	} else {							\
		__asm__ __volatile__ (					\
		"	laa"#D"	%[old],	(%[p]),	%[v]	\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p), [v] "r" (__v) : "memory");		\
	} })

#define __atomic_load_add_nosync(TYPE, p, v)	({			\
	TYPE##_t *__p = p, __v = v;					\
	CVMX_PUSH_OCTEON2;						\
	BUILD_EXPR_64or32(*__p, ___atomic_load_add_nosync(d),		\
			  ___atomic_load_add_nosync());			\
	CVMX_POP_OCTEON2; })
#else
#define ___atomic_load_add_nosync(D)		({			\
	__asm__ __volatile__ (						\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[tmp],	%[p]			\n"		\
	"	move	%[old],	%[tmp]			\n"		\
	"\t"#D"addu	%[tmp],	%[v]			\n"		\
	"	sc"#D"	%[tmp],	%[p]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	: [p] "+m" (*__p), [tmp] "=&r" (__t), [old] "=&r" (old)		\
	: [v] "r" (__v) : "memory"); })

#define __atomic_load_add_nosync(TYPE, p, v)	({			\
	TYPE##_t *__p = p, __v = v, __t;				\
	BUILD_EXPR_64or32(*__p, ___atomic_load_add_nosync(d),		\
			  ___atomic_load_add_nosync()); })
#endif

#define __atomic_load_add(TYPE, p, v)		({			\
	TYPE##_t old;							\
	CVMX_SYNCWS; __atomic_load_add_nosync(TYPE, p, v); CVMX_SYNCWS;	old; })

#define atomic_load_add32_nosync(p, v)		({			\
	uint32_t old;							\
	__atomic_load_add_nosync(uint32, p, v); old; })
#define atomic_load_add64_nosync(p, v)		({			\
	uint64_t old;							\
	__atomic_load_add_nosync(uint64, p, v); old; })
#define atomic_load_add32(p, v)		__atomic_load_add(uint32, p, v)
#define atomic_load_add64(p, v)		__atomic_load_add(uint64, p, v)

#define ___atomic_load_bset_nosync(D)		({			\
	__asm__ __volatile__ (						\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[tmp],	%[p]			\n"		\
	"	move	%[old],	%[tmp]			\n"		\
	"	or	%[tmp],	%[m]			\n"		\
	"	sc"#D"	%[tmp],	%[p]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	: [p] "+m" (*__p), [old] "=&r" (old), [tmp] "=&r" (__t)		\
	: [m] "r" (__m) : "memory"); })

#define __atomic_load_bset_nosync(TYPE, p, m)	({			\
	TYPE##_t *__p = p, __m = m, __t;				\
	BUILD_EXPR_64or32(*__p, ___atomic_load_bset_nosync(d),		\
			  ___atomic_load_bset_nosync()); })

#define __atomic_load_bset(TYPE, p, m)		({			\
	TYPE##_t old;							\
	CVMX_SYNCWS; __atomic_load_bset_nosync(TYPE, p, m); CVMX_SYNCWS; old; })

#define atomic_load_bset32_nosync(p, m)		({			\
	uint32_t old;							\
	__atomic_load_bset_nosync(uint32, p, m); old; })
#define atomic_load_bset64_nosync(p, m)		({			\
	uint64_t old;							\
	__atomic_load_bset_nosync(uint64, p, m); old; })
#define atomic_load_bset32(p, m)	__atomic_load_bset(uint32, p, m)
#define atomic_load_bset64(p, m)	__atomic_load_bset(uint64, p, m)

#define ___atomic_load_bclr_nosync(D)		({			\
	__asm__ __volatile__ (						\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[tmp],	%[p]			\n"		\
	"	move	%[old],	%[tmp]			\n"		\
	"	and	%[tmp],	%[m]			\n"		\
	"	sc"#D"	%[tmp],	%[p]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	: [p] "+m" (*__p), [old] "=&r" (old), [tmp] "=&r" (__t)		\
	: [m] "r" (__m) : "memory"); })

#define __atomic_load_bclr_nosync(TYPE, p, m)	({			\
	TYPE##_t *__p = p, __m = ~(m), __t;				\
	BUILD_EXPR_64or32(*__p, ___atomic_load_bclr_nosync(d),		\
			  ___atomic_load_bclr_nosync()); })

#define __atomic_load_bclr(TYPE, p, m)		({			\
	TYPE##_t old;							\
	CVMX_SYNCWS; __atomic_load_bclr_nosync(TYPE, p, m); CVMX_SYNCWS; old; })

#define atomic_load_bclr32_nosync(p, m)		({			\
	uint32_t old;							\
	__atomic_load_bclr_nosync(uint32, p, m); old; })
#define atomic_load_bclr64_nosync(p, m)		({			\
	uint64_t old;							\
	__atomic_load_bclr_nosync(uint64, p, m); old; })
#define atomic_load_bclr32(p, m)	__atomic_load_bclr(uint32, p, m)
#define atomic_load_bclr64(p, m)	__atomic_load_bclr(uint64, p, m)

#ifdef CVMX_CAVIUM_OCTEON2
#define ___atomic_xchg_nosync(D)		({			\
	if (BUILD_CONST(__v, 0)) {					\
		__asm__ __volatile__ (					\
		"	lac"#D"	%[old],	(%[p])		\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p) : "memory");				\
	} else if (BUILD_CONST(__v, (typeof(__v))~0)) {			\
		__asm__ __volatile__ (					\
		"	las"#D"	%[old],	(%[p])		\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p) : "memory");				\
	} else {							\
		__asm__ __volatile__ (					\
		"	law"#D"	%[old],	(%[p]),	%[v]	\n"		\
		: [old] "=&r" (old), "+m" (*__p)			\
		: [p] "r" (__p), [v] "r" (__v) : "memory");		\
	} })

#define __atomic_xchg_nosync(TYPE, p, v)	({			\
	TYPE##_t *__p = p, __v = v;					\
	CVMX_PUSH_OCTEON2;						\
	BUILD_EXPR_64or32(*__p, ___atomic_xchg_nosync(d),		\
			  ___atomic_xchg_nosync());			\
	CVMX_POP_OCTEON2; })
#else
#define ___atomic_xchg_nosync(D)		({			\
	__asm__ __volatile__ (						\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[old],	%[p]			\n"		\
	"	move	%[tmp],	%[v]			\n"		\
	"	sc"#D"	%[tmp],	%[p]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	: [p] "+m" (*__p), [tmp] "=&r" (__t), [old] "=&r" (old)		\
	: [v] "r" (__v) : "memory"); })

#define __atomic_xchg_nosync(TYPE, p, v)	({			\
	TYPE##_t *__p = p, __v = v, __t;				\
	BUILD_EXPR_64or32(*__p, ___atomic_xchg_nosync(d),		\
			  ___atomic_xchg_nosync()); })
#endif

#define __atomic_xchg(TYPE, p, v)		({			\
	TYPE##_t old;							\
	CVMX_SYNCWS; __atomic_xchg_nosync(TYPE, p, v); CVMX_SYNCWS; old; })

#define atomic_xchg32_nosync(p, v)		({			\
	uint32_t old;							\
	__atomic_xchg_nosync(uint32, p, v); old; })
#define atomic_xchg64_nosync(p, v)		({			\
	uint64_t old;							\
	__atomic_xchg_nosync(uint64, p, v); old; })
#define atomic_xchg32(p, v)		__atomic_xchg(uint32, p, v)
#define atomic_xchg64(p, v)		__atomic_xchg(uint64, p, v)

#define ___cmpxchg(A, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[ex],	%[ptr]		\n"			\
	"	b"#A"	%[ex],	%[old],	2f	\n"			\
	"	move	%[tmp],	%[new]		\n"			\
	"	sc"#D"	%[tmp],	%[ptr]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [ptr] "+m" (*(__p)), [ex] "=&r" (__e), [tmp] "=&r" (__t)	\
	: [old] "r" (__o), [new] "r" (__n) : "memory"); __e; })

#define __cmpxchg(p, o, n, A)	({					\
	typeof(p) __p = p; typeof(*(p)) __o = o; typeof(*(p)) __n = n;	\
	typeof(*(p)) __e, __t;						\
	BUILD_EXPR_64or32(*__p, ___cmpxchg(A, d), ___cmpxchg(A,)); })

#define cmpxchg_eq(p, o, n)		__cmpxchg(p, o, n, ne)
#define cmpxchg_ne(p, o, n)		__cmpxchg(p, o, n, eq)
#define cmpxchg_ge(p, o, n)		__cmpxchg(p, o, n, lt)
#define cmpxchg_geu(p, o, n)		__cmpxchg(p, o, n, ltu)
#define cmpxchg_gt(p, o, n)		__cmpxchg(p, o, n, le)
#define cmpxchg_gtu(p, o, n)		__cmpxchg(p, o, n, leu)
#define cmpxchg_le(p, o, n)		__cmpxchg(p, o, n, gt)
#define cmpxchg_leu(p, o, n)		__cmpxchg(p, o, n, gtu)
#define cmpxchg_lt(p, o, n)		__cmpxchg(p, o, n, ge)
#define cmpxchg_ltu(p, o, n)		__cmpxchg(p, o, n, geu)

#define __stack_push(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[ex],	%[ptr]		\n"			\
	"	s"#W"	%[ex],	(%[new])	\n"			\
	"	move	%[ex],	%[new]		\n"			\
	"	sc"#D"	%[ex],	%[ptr]		\n"			\
	"	beqz	%[ex],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	CVMX_SYNCWS_STR							\
	: [ptr] "+m" (*(__p)), [ex] "=&r" (__e)				\
	: [new] "r" (__n) : "memory"); })

#define stack_push(p, n)	({					\
	typeof(p) __p = p; typeof(n) __n = n; typeof(*(p)) __e;		\
	(void) (&__n == __p);						\
	BUILD_EXPR_64or32(*__p, __stack_push(d, d), __stack_push(w,)); })

#define __stack_pop(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[ex],	%[ptr]		\n"			\
	"	beqz	%[ex],	2f		\n"			\
	"	nop				\n"			\
	"	l"#W"	%[tmp],	(%[ex])		\n"			\
	"	sc"#D"	%[tmp],	%[ptr]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [ptr] "+m" (*(__p)), [ex] "=&r" (__e), [tmp] "=&r" (__t)	\
	: : "memory"); __e; })

#define stack_pop(p)	({						\
	typeof(p) __p = p; typeof(*(p)) __e, __t;			\
	BUILD_EXPR_64or32(*__p, __stack_pop(d, d), __stack_pop(wu,)); })

#define __stack_lock_push(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tmp],	%[lock]		\n"			\
	"	li	%[ret],	0		\n"			\
	"	bbit0	%[tmp],	0,	2f	\n"			\
	"	s"#W"	%[tmp],	(%[new])	\n"			\
	"	li	%[ret],	1		\n"			\
	"	move	%[tmp],	%[new]		\n"			\
	"2:	ori	%[tmp],	1		\n"			\
	"	sc"#D"	%[tmp],	%[lock]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	CVMX_SYNCWS_STR							\
	: [lock] "+m" (*(__l)), [ret] "=&r" (__r), [tmp] "=&r" (__t)	\
	: [new] "r" (__n) : "memory"); __r; })

#define stack_lock_push(l, n)	({					\
	typeof(l) __l = l; typeof(n) __n = n; typeof(*(l)) __r, __t;	\
	(void) (&__n == __l);						\
	BUILD_EXPR_64or32(*__l, __stack_lock_push(d, d),		\
			    __stack_lock_push(w,)); })

#define __stack_lock_try(D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tmp],	%[lock]		\n"			\
	"	bbit1	%[tmp],	0,	2f	\n"			\
	"	nop				\n"			\
	"	ori	%[tmp],	1		\n"			\
	"	sc"#D"	%[tmp],	%[lock]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	li	%[tmp],	0		\n"			\
	"2:					\n"			\
	".set reorder				\n"			\
	CVMX_SYNCWS_STR							\
	: [lock] "+m" (*(__l)), [tmp] "=&r" (__t)			\
	: : "memory"); (!!__t); })

#define stack_lock_try(l)	({					\
	typeof(l) __l = l; typeof(*(l)) __t;				\
	BUILD_EXPR_64or32(*__l, __stack_lock_try(d),			\
			    __stack_lock_try()); })

#define __stack_lock(D)	({						\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tmp],	%[lock]		\n"			\
	"	bbit1	%[tmp],	0,	1b	\n"			\
	"	nop				\n"			\
	"	ori	%[tmp],	1		\n"			\
	"	sc"#D"	%[tmp],	%[lock]		\n"			\
	"	beqz	%[tmp],	1b		\n"			\
	"	nop				\n"			\
	".set reorder				\n"			\
	CVMX_SYNCWS_STR							\
	: [lock] "+m" (*(__l)), [tmp] "=&r" (__t)			\
	: : "memory"); })

#define stack_lock(l)	({						\
	typeof(l) __l = l; typeof(*(l)) __t;				\
	BUILD_EXPR_64or32(*__l, __stack_lock(d), __stack_lock()); })

#define __stack_unlock_pop(W, D)	({				\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[ret],	%[lock]			\n"		\
	"	li	%[tmp],	1			\n"		\
	"	beq	%[tmp],	%[ret],	2f		\n"		\
	"	li	%[tmp],	0			\n"		\
	"\t"#D"ins	%[ret],	$zero,	0,	1	\n"		\
	"	l"#W"	%[tmp],	(%[ret])		\n"		\
	"2:	sc"#D"	%[tmp],	%[lock]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	"2:						\n"		\
	CVMX_SYNCWS_STR							\
	: [lock] "+m" (*(__l)), [ret] "=&r" (__r), [tmp] "=&r" (__t)	\
	: : "memory"); (typeof(__r))((uint64_t)__r & ~0x1UL); })

#define stack_unlock_pop(l)	({					\
	typeof(l) __l = l; typeof(*(l)) __r, __t;			\
	BUILD_EXPR_64or32(*__l, __stack_unlock_pop(d, d),		\
			    __stack_unlock_pop(wu,)); })

#define __stack_unlock(D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder					\n"		\
	"1:	ll"#D"	%[tmp],	%[lock]			\n"		\
	"\t"#D"ins	%[tmp],	$zero,	0,	1	\n"		\
	"	sc"#D"	%[tmp],	%[lock]			\n"		\
	"	beqz	%[tmp],	1b			\n"		\
	"	nop					\n"		\
	".set reorder					\n"		\
	"2:						\n"		\
	CVMX_SYNCWS_STR							\
	: [lock] "+m" (*(__l)), [tmp] "=&r" (__t)			\
	: : "memory"); })

#define stack_unlock(l)	({						\
	typeof(l) __l = l; typeof(*(l)) __t;				\
	BUILD_EXPR_64or32(*__l, __stack_unlock(d), __stack_unlock()); })

#define ___ring_fai_tail(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tt],	%[tail]		\n"			\
	"	l"#W"	%[th],	%[head]		\n"			\
	"\t"#D"li	%[r],	-1		\n"			\
	"\t"#D"addu	%[th],	%[mask]		\n"			\
	"\t"#D"subu	%[th],	%[tt]		\n"			\
	"	bltz	%[th],	2f		\n"			\
	"\t"#D"addiu	%[tt],	1		\n"			\
	"\t"#D"addiu	%[r],	%[tt],	-1	\n"			\
	"	sc"#D"	%[tt],	%[tail]		\n"			\
	"	beqz	%[tt],	1b		\n"			\
	"	nop				\n"			\
	"	and	%[r],	%[mask]		\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [tail] "+m" (*__t), [th] "=&r" (__th), [tt] "=&r" (__tt),	\
	  [r] "=&r" (__r)						\
	: [head] "m" (*__h), [mask] "r" (__m) : "memory"); })

#define ___ring_faa_tail(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[tt],	%[tail]		\n"			\
	"	l"#W"	%[th],	%[head]		\n"			\
	"\t"#D"li	%[r],	-1		\n"			\
	"\t"#D"addu	%[th],	%[mask]		\n"			\
	"\t"#D"addiu	%[th],	1		\n"			\
	"\t"#D"subu	%[th],	%[tt]		\n"			\
	"\t"#D"subu	%[th],	%[v]		\n"			\
	"	bltz	%[th],	2f		\n"			\
	"	move	%[r],	%[tt]		\n"			\
	"\t"#D"addu	%[tt],	%[v]		\n"			\
	"	sc"#D"	%[tt],	%[tail]		\n"			\
	"	beqz	%[tt],	1b		\n"			\
	"	nop				\n"			\
	"	and	%[r],	%[mask]		\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [tail] "+m" (*__t), [th] "=&r" (__th), [tt] "=&r" (__tt),	\
	  [r] "=&r" (__r)						\
	: [head] "m" (*__h), [v] "r" (__v), [mask] "r" (__m) : "memory"); })

#define __ring_faa_tail(W, D)	({					\
	if (__builtin_constant_p(__v) && __v == 1)			\
		___ring_fai_tail(W, D);					\
	else								\
		___ring_faa_tail(W, D);					\
	__r; })

#define ring_fetch_and_add_tail(h, t, v, m)	({			\
	typeof(h) __h = h; typeof(t) __t = t;				\
	typeof(*(t)) __v = v, __m = m;					\
	typeof(*(t)) __th, __tt, __r;					\
	(void) (&__h == &__t);						\
	BUILD_BUG_NOT_POWER_OF_2(__m + 1);				\
	BUILD_EXPR_64or32(*__t, __ring_faa_tail(d, d),		\
			    __ring_faa_tail(wu,)); })

#define ___ring_fai_head(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[th],	%[head]		\n"			\
	"	l"#W"	%[tt],	%[tail]		\n"			\
	"\t"#D"li	%[r],	-1		\n"			\
	"\t"#D"subu	%[tt],	%[th]		\n"			\
	"	beqz	%[tt],	2f		\n"			\
	"\t"#D"addiu	%[th],	1		\n"			\
	"\t"#D"addiu	%[r],	%[th],	-1	\n"			\
	"	sc"#D"	%[th],	%[head]		\n"			\
	"	beqz	%[th],	1b		\n"			\
	"	nop				\n"			\
	"	and	%[r],	%[mask]		\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [head] "+m" (*__h), [th] "=&r" (__th), [tt] "=&r" (__tt),	\
	  [r] "=&r" (__r)						\
	: [tail] "m" (*__t), [mask] "r" (__m) : "memory"); })

#define ___ring_faa_head(W, D)	({					\
	__asm__ __volatile__ (						\
	CVMX_SYNCWS_STR							\
	".set noreorder				\n"			\
	"1:	ll"#D"	%[th],	%[head]		\n"			\
	"	l"#W"	%[tt],	%[tail]		\n"			\
	"\t"#D"li	%[r],	-1		\n"			\
	"\t"#D"subu	%[tt],	%[th]		\n"			\
	"\t"#D"subu	%[tt],	%[v]		\n"			\
	"	bltz	%[tt],	2f		\n"			\
	"	nop				\n"			\
	"	move	%[r],	%[th]		\n"			\
	"\t"#D"addu	%[th],	%[v]		\n"			\
	"	sc"#D"	%[th],	%[head]		\n"			\
	"	beqz	%[th],	1b		\n"			\
	"	nop				\n"			\
	"	and	%[r],	%[mask]		\n"			\
	".set reorder				\n"			\
	"2:					\n"			\
	CVMX_SYNCWS_STR							\
	: [head] "+m" (*__h), [th] "=&r" (__th), [tt] "=&r" (__tt),	\
	  [r] "=&r" (__r)						\
	: [tail] "m" (*__t), [v] "r" (__v), [mask] "r" (__m) : "memory"); })

#define __ring_faa_head(W, D)	({					\
	if (__builtin_constant_p(__v) && __v == 1)			\
		___ring_fai_head(W, D);					\
	else								\
		___ring_faa_head(W, D);					\
	__r; })

#define ring_fetch_and_add_head(h, t, v, m)	({			\
	typeof(h) __h = h; typeof(t) __t = t;				\
	typeof(*(h)) __v = v, __m = m;					\
	typeof(*(h)) __th, __tt, __r;					\
	(void) (&__h == &__t);						\
	BUILD_BUG_NOT_POWER_OF_2(__m + 1);				\
	BUILD_EXPR_64or32(*__h, __ring_faa_head(d, d),		\
			    __ring_faa_head(wu,)); })

#endif	/* __atomic_INC__ */

