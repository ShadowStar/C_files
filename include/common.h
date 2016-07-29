#ifndef __COMMON_H
#define __COMMON_H
#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/ctype.h>
#else
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN	4321
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN	1234
#endif

#ifndef __KERNEL__
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
#endif

#ifndef likely
#define likely(x)		__builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x)		__builtin_expect(!!(x), 0)
#endif

#define PRE_MASK(x)		({ unsigned _x = 64 - (x);	\
				 (_x >= 64 ? 0ULL : ~0ULL << _x); })
#define SUF_MASK(x)		({ unsigned _x = 64 - (x);	\
				 (_x >= 64 ? 0ULL : ~0ULL >> _x); })

#define PRE_MASK_R(x)		({ unsigned _x = (x);		\
				 (_x >= 64 ? 0ULL : ~0ULL >> _x); })
#define SUF_MASK_R(x)		({ unsigned _x = (x);		\
				 (_x >= 64 ? 0ULL : ~0ULL << _x); })

#define RANGE_MASK(x, y)	(PRE_MASK(64 - (x)) & SUF_MASK((y) + 1))
#define RANGE_MASK_R(x, y)	(PRE_MASK(63 - (y)) | SUF_MASK(x))

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x)		(*(volatile typeof(x) *)&(x))
#endif	/* end of ACCESS_ONCE */

#define VOLATILE(x)		(*(volatile typeof(x) *)&(x))

#define SUF_FUNC(NAME, SUFFIX, ...)	NAME##SUFFIX(__VA_ARGS__)
#define CALL_FUNC_VA(NAME, VA, ...)	SUF_FUNC(NAME, VA, ##__VA_ARGS__)

#ifndef ALIGN
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_MASK(x, (typeof(x))(a)-1)
#endif

#define ALIGN_INC_POW2(x)	(((x) & ((x) - 1)) == 0 ? \
	(x) + 2 * ((x) == 0) : (typeof(x))(1ULL << (64 - clz64(x))))

#define ALIGN_DEC_POW2(x)	(((x) & ((x) - 1)) == 0 ? \
	(x) + 2 * ((x) == 0) : (typeof(x))(1ULL << (63 - clz64(x))))

#define ALIGN_INC(x, a)		\
	(((x) + ((x) == 0) + (a) - 1) & ~((typeof(x))(a) - 1))
#define ALIGN_DEC(x, a)		((x) & ~((typeof(x))(a) - 1))

#ifndef PTR_ALIGN
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))
#endif

#ifndef IS_ALIGNED
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)
#endif

#ifndef bit_mask_to_strip
#define bit_mask_to_strip(x, y) ({			\
	int _ex = (y);					\
	typeof(x) _mask = (x);				\
	typeof(x) _stp = _mask & SUF_MASK(_ex);		\
	_stp |= (_mask & PRE_MASK(63 - _ex)) >> 1; })
#endif

#ifndef bit_strip_to_mask
#define bit_strip_to_mask(x, y) ({			\
	int _ex = (y);					\
	typeof(x) _stp = (x);				\
	typeof(x) _mask = _stp & SUF_MASK(_ex);		\
	_mask |= (_stp << 1) & PRE_MASK(63 - _ex); })
#endif

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#ifndef min
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

#ifndef max
#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

/**
 * clamp - return a value clamped to a given range with strict typechecking
 * @val: current value
 * @min: minimum allowable value
 * @max: maximum allowable value
 *
 * This macro does strict typechecking of min/max to make sure they are of the
 * same type as val.  See the unnecessary pointer comparisons.
 */
#ifndef clamp
#define clamp(val, min, max) ({			\
	typeof(val) __val = (val);		\
	typeof(min) __min = (min);		\
	typeof(max) __max = (max);		\
	(void) (&__val == &__min);		\
	(void) (&__val == &__max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })
#endif

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#ifndef min_t
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })
#endif

#ifndef max_t
#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })
#endif

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @min: minimum allowable value
 * @max: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * 'type' to make all the comparisons.
 */
#ifndef clamp_t
#define clamp_t(type, val, min, max) ({		\
	type __val = (val);			\
	type __min = (min);			\
	type __max = (max);			\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })
#endif

/**
 * clamp_val - return a value clamped to a given range using val's type
 * @val: current value
 * @min: minimum allowable value
 * @max: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of whatever
 * type the input argument 'val' is.  This is useful when val is an unsigned
 * type and min and max are literals that will otherwise be assigned a signed
 * integer type.
 */
#ifndef clamp_val
#define clamp_val(val, min, max) ({		\
	typeof(val) __val = (val);		\
	typeof(val) __min = (min);		\
	typeof(val) __max = (max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })
#endif

#ifndef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif /* offsetof */

#ifndef rangeof
#define rangeof(TYPE, FROM, TO)			\
	(offsetof(TYPE, TO) - offsetof(TYPE, FROM) + sizeof(((TYPE *)0)->TO))
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type, member) );})
#endif /* container_of */

/* Force a compilation error if condition is true, but also produce a
   result (of value 0 and type size_t), so the expression can be used
   e.g. in a structure initializer (or where-ever else comma expressions
   aren't permitted). */
#ifndef BUILD_BUG_NOT_ZERO
#define BUILD_BUG_NOT_ZERO(e)	(sizeof(struct { int:-!!(e); }))
#endif

/* Force a compilation error if condition is true */
#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(e)		((void)sizeof(char[1 - 2 * !!(e)]))
#endif

#ifndef BUILD_BUG_NOT_NULL
#define BUILD_BUG_NOT_NULL(e)	((void *)BUILD_BUG_NOT_ZERO(e))
#endif

/* Force a compilation error if a constant expression is not a power of 2 */
#define BUILD_BUG_NOT_POWER_OF_2(n)		\
	BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))

#define BUILD_EXPR(v, b0, b1)			\
	__builtin_choose_expr((v), b0, b1)

#define BUILD_EXPR32(v, b32)			\
	__builtin_choose_expr(sizeof(v) == 4, b32, (void)0)

#define BUILD_EXPR64(v, b64)			\
	__builtin_choose_expr(sizeof(v) == 8, b64, (void)0)

#define BUILD_EXPR_64or32(v, b64, b32)		\
	__builtin_choose_expr(sizeof(v) == 8, b64, BUILD_EXPR32(v, b32))

#define BUILD_EXPR_CONST(v, bconst, bdynamic)	\
	(__builtin_constant_p(v) ? bconst : bdynamic)

#define BUILD_CONST(v, V)	(__builtin_constant_p(v) && (v) == V)

/* Are two types/vars the same type (ignoring qualifiers)? */
#define SAME_TYPE(a, b)		\
	__builtin_types_compatible_p(typeof(a), typeof(b))

#define BUILD_SAME_TYPE(a, b)	\
	BUILD_BUG_ON(!SAME_TYPE(a, b))

/* &a[0] degrades to a pointer: a different type from an array */
#define BUILD_BE_ARRAY(a)	\
	BUILD_BUG_NOT_ZERO(SAME_TYPE((a), &(a)[0]))

#define ARRAY_SIZE(arr)		\
	(sizeof(arr) / sizeof((arr)[0]) + BUILD_BE_ARRAY(arr))

#ifndef NIPQUAD
#define NIPQUAD_FMT	"%u.%u.%u.%u"
#define NIPQUAD(value)	\
	(value & 0xFF000000) >> 24, \
	(value & 0x00FF0000) >> 16, \
	(value & 0x0000FF00) >> 8, \
	(value & 0x000000FF)
#endif

#define NIP6QUAD_FMT	"%x:%x:%x:%x:%x:%x:%x:%x"
#define NIP6QUAD(v)	\
	((uint16_t *)v)[0], \
	((uint16_t *)v)[1], \
	((uint16_t *)v)[2], \
	((uint16_t *)v)[3], \
	((uint16_t *)v)[4], \
	((uint16_t *)v)[5], \
	((uint16_t *)v)[6], \
	((uint16_t *)v)[7]

#define MACQUAD_FMT	"%02X:%02X:%02X:%02X:%02X:%02X"
#define MACQUAD(addr)	\
	((uint8_t *)addr)[0], \
	((uint8_t *)addr)[1], \
	((uint8_t *)addr)[2], \
	((uint8_t *)addr)[3], \
	((uint8_t *)addr)[4], \
	((uint8_t *)addr)[5]

#define Ki		1024ULL
#define Mi		(1024 * Ki)
#define Gi		(1024 * Mi)
#define Ti		(1024 * Gi)

#define MS_PER_SECOND	1000
#define US_PER_SECOND	1000000

#define U64_MAX		0xFFFFFFFFFFFFFFFF
#define U32_MAX		0xFFFFFFFF
#define U16_MAX		0xFFFF
#define U8_MAX		0xFF

#define TGMK_FMT "%luTi %luGi %luMi %luKi %lu"
#define TGMK(x) \
	((x) >> 40), \
	(((x) & (Ti - 1)) >> 30), \
	(((x) & (Gi - 1)) >> 20), \
	(((x) & (Mi - 1)) >> 10), \
	((x) & (Ki - 1))

static inline int ___constant_popcnt16(uint16_t x)
{
	x -= ((x >> 1) & 0x5555U);
	x = (x & 0x3333U) + ((x >> 2) & 0x3333U);
	x = (x + (x >> 4)) & 0x0F0FU;
	x += x >> 8;
	return x & 0x1F;
}

static inline int ___constant_popcnt32(uint32_t x)
{
	x -= ((x >> 1) & 0x55555555ULL);
	x = (x & 0x33333333ULL) + ((x >> 2) & 0x33333333ULL);
	x = (x + (x >> 4)) & 0x0F0F0F0FULL;
	x += x >> 8;
	x += x >> 16;
	return x & 0x3F;
}

static inline int ___constant_popcnt64(uint64_t x)
{
	x -= ((x >> 1) & 0x5555555555555555ULL);
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	x += x >> 8;
	x += x >> 16;
	x += x >> 32;
	return x & 0x7F;
}

static inline int ___constant_clz64(uint64_t x)
{
	int n;

	if (x == 0)
		return 64;
	n = 1;
	if ((x >> 32) == 0) {
		n += 32;
		x <<= 32;
	}
	if ((x >> 48) == 0) {
		n += 16;
		x <<= 16;
	}
	if ((x >> 56) == 0) {
		n += 8;
		x <<= 8;
	}
	if ((x >> 60) == 0) {
		n += 4;
		x <<= 4;
	}
	if ((x >> 62) == 0) {
		n += 2;
		x <<= 2;
	}
	n -= (x >> 63);
	return n;
}

static inline int ___constant_clz32(uint32_t x)
{
	int n;

	if (x == 0)
		return 32;
	n = 1;
	if ((x >> 16) == 0) {
		n += 16;
		x <<= 16;
	}
	if ((x >> 24) == 0) {
		n += 8;
		x <<= 8;
	}
	if ((x >> 28) == 0) {
		n += 4;
		x <<= 4;
	}
	if ((x >> 30) == 0) {
		n += 2;
		x <<= 2;
	}
	n -= (x >> 31);
	return n;
}

#define ___constant_clo64(x) \
	___constant_clz64(~(x))

#define ___constant_clo32(x) \
	___constant_clz32(~(x))

#define ___constant_swap16(x) ((uint16_t)(				\
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |			\
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define ___constant_swap32(x) ((uint32_t)(				\
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |		\
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |		\
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |		\
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

#define ___constant_swap64(x) ((uint64_t)(				\
	(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) |	\
	(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) |	\
	(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) |	\
	(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) |	\
	(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) |	\
	(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) |	\
	(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) |	\
	(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56)))

#if defined(__x86_64__)
#ifndef __BYTE_ORDER
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

static inline int arch_popcnt32(uint32_t x)
{
	__asm__(
	"popcntl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return x;
}

static inline int arch_popcnt64(uint64_t x)
{
	__asm__(
	"popcntq	%1, %0"
	: "=r" (x)
	: "0" (x));
	return x;
}

static inline int arch_clz32(uint32_t x)
{
	if (x == 0)
		return 32;
	__asm__(
	"bsrl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 31 - x;
}

static inline int arch_clo32(uint32_t x)
{
	if (x == -1U)
		return 32;
	__asm__(
	"notl	%0\n\t"
	"bsrl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 31 - x;
}

static inline int arch_clz64(uint64_t x)
{
	if (x == 0)
		return 64;
	__asm__(
	"bsrq	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 63 - x;
}

static inline int arch_clo64(uint64_t x)
{
	if (x == -1ULL)
		return 64;
	__asm__(
	"notq	%0\n\t"
	"bsrq	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 63 - x;
}

static inline uint16_t arch_swap16(uint16_t x)
{
	return ((x & 0x00FFU) << 8) | ((x & 0xFF00U) >> 8);
}

static inline uint32_t arch_swap32(uint32_t x)
{
	__asm__(
	"bswapl	%0"
	: "=r" (x)
	: "0" (x));
	return x;
}

static inline uint64_t arch_swap64(uint64_t x)
{
	__asm__(
	"bswapq	%0"
	: "=r" (x)
	: "0" (x));
	return x;
}
#elif defined(__i386__)
#ifndef __BYTE_ORDER
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

static inline int arch_popcnt32(uint32_t x)
{
	__asm__(
	"popcntl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return x;
}

static inline int arch_popcnt64(uint64_t x)
{
	union {
		struct {
			uint32_t a;
			uint32_t b;
		} s;
		uint64_t u;
	} v = { .u = x };

	__asm__(
	"popcntl	%2, %0\n\t"
	"popcntl	%3, %1\n\t"
	"addl		%1, %0"
	: "=r" (v.s.a), "=r" (v.s.b)
	: "0" (v.s.a), "1" (v.s.b));
	return v.s.a;
}

static inline int arch_clz32(uint32_t x)
{
	if (x == 0)
		return 32;
	__asm__(
	"bsrl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 31 - x;
}

static inline int arch_clo32(uint32_t x)
{
	if (x == -1U)
		return 32;
	__asm__(
	"notl	%0\n\t"
	"bsrl	%1, %0"
	: "=r" (x)
	: "0" (x));
	return 31 - x;
}

static inline int arch_clz64(uint64_t x)
{
	if (x == 0)
		return 64;
	union {
		struct {
			uint32_t b;
			uint32_t a;
		} s;
		uint64_t u;
	} v = { .u = x };
	__asm__(
	"cmpl	$0, %0\n\t"
	"je	1f\n\t"
	"bsrl	%1, %0\n\t"
	"addl	$32, %0\n\t"
	"jmp	2f\n\t"
	"1:"
	"bsrl	%2, %0\n\t"
	"2:"
	: "=r" (v.s.a)
	: "0" (v.s.a), "r" (v.s.b));
	return 63 - v.s.a;
}

static inline int arch_clo64(uint64_t x)
{
	if (x == -1ULL)
		return 64;
	union {
		struct {
			uint32_t b;
			uint32_t a;
		} s;
		uint64_t u;
	} v = { .u = x };
	__asm__(
	"notl	%0\n\t"
	"notl	%2\n\t"
	"cmpl	$0, %0\n\t"
	"je	1f\n\t"
	"bsrl	%1, %0\n\t"
	"addl	$32, %0\n\t"
	"jmp	2f\n\t"
	"1:"
	"bsrl	%2, %0\n\t"
	"2:"
	: "=r" (v.s.a)
	: "0" (v.s.a), "r" (v.s.b));
	return 63 - v.s.a;
}

static inline uint16_t arch_swap16(uint16_t x)
{
	return ((x & 0x00FFU) << 8) | ((x & 0xFF00U) >> 8);
}

static inline uint32_t arch_swap32(uint32_t x)
{
	__asm__(
	"bswapl	%0"
	: "=r" (x)
	: "0" (x));
	return x;
}

static inline uint64_t arch_swap64(uint64_t x)
{
	union {
		struct {
			uint32_t a;
			uint32_t b;
		} s;
		uint64_t u;
	} v = { .u = x };

	__asm__(
	"bswapl	%0\n\t"
	"bswapl	%1\n\t"
	"xchgl	%0, %1"
	: "=r" (v.s.a), "=r" (v.s.b)
	: "0" (v.s.a), "1"(v.s.b));
	return v.u;
}
#elif defined(__mips64)

#if defined(__MIPSEB) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER	__BIG_ENDIAN
#elif defined(__MIPSEL) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

static inline int arch_popcnt32(uint32_t x)
{
	__asm__ (
	"pop	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_popcnt64(uint64_t x)
{
	__asm__ (
	"dpop	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_clz32(uint32_t x)
{
	__asm__(
	"clz	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_clo32(uint32_t x)
{
	__asm__(
	"clo	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_clz64(uint64_t x)
{
	__asm__(
	"dclz	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_clo64(uint64_t x)
{
	__asm__(
	"dclo	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline uint16_t arch_swap16(uint16_t x)
{
	__asm__(
	"wsbh	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline uint32_t arch_swap32(uint32_t x)
{
	__asm__(
	"wsbh	%0, %1\n\t"
	"rotr	%0, %0, 16"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline uint64_t arch_swap64(uint64_t x)
{
	__asm__(
	"dsbh	%0, %1\n\t"
	"dshd	%0, %0"
	: "=r" (x)
	: "r" (x));

	return x;
}
#elif defined(__arm__)

#if defined(__ARMEB__) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER	__BIG_ENDIAN
#elif defined(__ARMEL__) && !defined(__BYTE_ORDER)
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

#if defined(__thumb2__)
static inline int arch_popcnt32(uint32_t x)
{
	uint32_t tmp;
	__asm__ (
	"and	%1, %0, #0xAAAAAAAA\n\t"
	"sub	%0, %0, %1, lsr #1\n\t"
	"and	%1, %0, #0xCCCCCCCC\n\t"
	"and	%0, %0, #0x33333333\n\t"
	"add	%0, %0, %1, lsr #2\n\t"
	"add	%0, %0, %0, lsr #4\n\t"
	"and	%0, %0, #0x0F0F0F0F\n\t"
	"add	%0, %0, %0, lsr #8\n\t"
	"add	%0, %0, %0, lsr #16\n\t"
	"and	%0, %0, #63"
	: "+r" (x), "=&r" (tmp));
	return x;
}
#elif defined(__thumb__)
#define arch_popcnt32(x)	___constant_popcnt32(x)
#else
static inline int arch_popcnt32(uint32_t x)
{
	uint32_t t0, t1, t2, t3, t4;
	__asm__ (
	"mov	%2, #0xFF\n\t"
	"orr	%2, %2, #0xFF<<16\n\t"
	"eor	%3, %2, %2, lsl #4\n\t"
	"eor	%4, %3, %3, lsl #2\n\t"
	"eor	%5, %4, %4, lsl #1\n\t"
	"and	%1, %5, %0, lsr #1\n\t"
	"sub	%0, %0, %1\n\t"
	"and	%1, %4, %0, lsr #2\n\t"
	"and	%0, %4, %0\n\t"
	"add	%0, %0, %1\n\t"
	"add	%0, %0, %0, lsr #4\n\t"
	"and	%0, %0, %3\n\t"
	"add	%0, %0, %0, lsr #8\n\t"
	"add	%0, %0, %0, lsr #16\n\t"
	"and	%0, %0, #63\n\t"
	: "+r" (x), "=&r" (t0), "=&r" (t1), "=&r" (t2), "=&r" (t3), "=&r" (t4));
	return x;
}
#endif

static inline int arch_popcnt64(uint64_t x)
{
	union {
		struct {
			uint32_t a;
			uint32_t b;
		} s;
		uint64_t u;
	} v = { .u = x };
	return arch_popcnt32(v.s.a) + arch_popcnt32(v.s.b);
}

static inline int arch_clz32(uint32_t x)
{
	__asm__ (
	"clz	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline int arch_clz64(uint64_t x)
{
	if (x == 0)
		return 64;
	union {
		struct {
			uint32_t b;
			uint32_t a;
		} s;
		uint64_t u;
	} v = { .u = x };
	__asm__ (
	"clz	%0, %1\n\t"
	"teq	%0, #32\n\t"
	"clz	%2, %2\n\t"
	"addeq	%0, %2, #32"
	: "=r" (v.s.a)
	: "0" (v.s.a), "r" (v.s.b));
	return v.s.a;
}

static inline int arch_clo32(uint32_t x)
{
	return arch_clz32(~x);
}

static inline int arch_clo64(uint64_t x)
{
	return arch_clz64(~x);
}

static inline uint16_t arch_swap16(uint32_t x)
{
	__asm__ (
	"rev16	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline uint32_t arch_swap32(uint32_t x)
{
	__asm__ (
	"rev	%0, %1"
	: "=r" (x)
	: "r" (x));
	return x;
}

static inline uint64_t arch_swap64(uint64_t x)
{
	uint32_t h = x >> 32;
	uint32_t l = x & (0xFFFFFFFFU);
	return ((uint64_t)arch_swap32(l) << 32) | ((uint64_t)arch_swap32(h));
}
#endif

/* Count Ones in 32-Bits */
#define popcnt32(x) (__builtin_constant_p(x) ?				\
	___constant_popcnt32(x) : arch_popcnt32(x))

/* Count Ones in 64-Bits */
#define popcnt64(x) (__builtin_constant_p(x) ?				\
	___constant_popcnt64(x) : arch_popcnt64(x))

/* Count Leading Ones in 64-Bits */
#define clz64(x) (__builtin_constant_p(x) ?				\
	___constant_clz64(x) : arch_clz64(x))

/* Count Leading Zero in 64-Bits */
#define clo64(x) (__builtin_constant_p(x) ?				\
	___constant_clo64(x) : arch_clo64(x))

/* Count Leading Ones in 32-Bits */
#define clz32(x) (__builtin_constant_p(x) ?				\
	___constant_clz32(x) : arch_clz32(x))

/* Count Leading Zero in 32-Bits */
#define clo32(x) (__builtin_constant_p(x) ?				\
	___constant_clo32(x) : arch_clo32(x))

/* Swap Byte in 16-Bits */
#define swap16(x) (__builtin_constant_p((uint16_t)(x)) ?		\
	___constant_swap16(x) : arch_swap16(x))

/* Swap Byte in 32-Bits */
#define swap32(x) (__builtin_constant_p((uint32_t)(x)) ?		\
	___constant_swap32(x) : arch_swap32(x))

/* Swap Byte in 64-Bits */
#define swap64(x) (__builtin_constant_p((uint64_t)(x)) ?		\
	___constant_swap64(x) : arch_swap64(x))

#define bitmap_foreach(p, m, t)					\
	for (t = m; t && ({ p = 63 - clz64(t); 1; }); t &= ~(1ULL << p))

#ifndef __KERNEL__
#if (__BYTE_ORDER == __BIG_ENDIAN)
#define cpu_to_be64(x)	(x)
#define cpu_to_be32(x)	(x)
#define cpu_to_be16(x)	(x)
#define cpu_to_le64(x)	swap64(x)
#define cpu_to_le32(x)	swap32(x)
#define cpu_to_le16(x)	swap16(x)
#else	/* __BYTE_ORDER == __LITTLE_ENDIAN */
#define cpu_to_be64(x)	swap64(x)
#define cpu_to_be32(x)	swap32(x)
#define cpu_to_be16(x)	swap16(x)
#define cpu_to_le64(x)	(x)
#define cpu_to_le32(x)	(x)
#define cpu_to_le16(x)	(x)
#endif
#define be64_to_cpu(x)	cpu_to_be64(x)
#define be32_to_cpu(x)	cpu_to_be32(x)
#define be16_to_cpu(x)	cpu_to_be16(x)
#define le64_to_cpu(x)	cpu_to_le64(x)
#define le32_to_cpu(x)	cpu_to_le32(x)
#define le16_to_cpu(x)	cpu_to_le16(x)
#endif

#define load_u8(x)	(*(uint8_t *)(x))
#define load_u16(x)	(*(uint16_t *)(x))
#define load_u32(x)	(*(uint32_t *)(x))
#define load_u64(x)	(*(uint64_t *)(x))

#if (__BYTE_ORDER == __BIG_ENDIAN)
#define load_be16(x)	load_u16(x)
#define load_be32(x)	load_u32(x)
#define load_be64(x)	load_u64(x)
#define load_le16(x)	le16_to_cpu(load_u16(x))
#define load_le32(x)	le32_to_cpu(load_u32(x))
#define load_le64(x)	le64_to_cpu(load_u64(x))
#define load_h16(x)	load_be16(x)
#define load_h32(x)	load_be32(x)
#define load_h64(x)	load_be64(x)
#define load_n16(x)	load_be16(x)
#define load_n32(x)	load_be32(x)
#define load_n64(x)	load_be64(x)
#else	/* __BYTE_ORDER == __LITTLE_ENDIAN */
#define load_be16(x)	be16_to_cpu(load_u16(x))
#define load_be32(x)	be32_to_cpu(load_u32(x))
#define load_be64(x)	be64_to_cpu(load_u64(x))
#define load_le16(x)	load_u16(x)
#define load_le32(x)	load_u32(x)
#define load_le64(x)	load_u64(x)
#define load_h16(x)	load_le16(x)
#define load_h32(x)	load_le32(x)
#define load_h64(x)	load_le64(x)
#define load_n16(x)	load_be16(x)
#define load_n32(x)	load_be32(x)
#define load_n64(x)	load_be64(x)
#endif

static inline void swap_dat(uint8_t *buf, int len)
{
	while (len >= 16) {
		*(uint64_t *)buf ^= *(uint64_t *)(buf + len - 8);
		*(uint64_t *)(buf + len - 8) ^= *(uint64_t *)buf;
		*(uint64_t *)buf ^= *(uint64_t *)(buf + len - 8);
		*(uint64_t *)buf = swap64(*(uint64_t *)buf);
		*(uint64_t *)(buf + len - 8) =
			swap64(*(uint64_t *)(buf + len - 8));
		buf += 8;
		len -= 16;
	}
	switch (len) {
	case 8 ... 15:
		*(uint32_t *)buf ^= *(uint32_t *)(buf + len - 4);
		*(uint32_t *)(buf + len - 4) ^= *(uint32_t *)buf;
		*(uint32_t *)buf ^= *(uint32_t *)(buf + len - 4);
		*(uint32_t *)buf = swap32(*(uint32_t *)buf);
		*(uint32_t *)(buf + len - 4) =
			swap32(*(uint32_t *)(buf + len - 4));
		buf += 4;
		len -= 8;
		break;
	}
	switch (len) {
	case 4 ... 7:
		*(uint16_t *)buf ^= *(uint16_t *)(buf + len - 2);
		*(uint16_t *)(buf + len - 2) ^= *(uint16_t *)buf;
		*(uint16_t *)buf ^= *(uint16_t *)(buf + len - 2);
		*(uint16_t *)buf = swap16(*(uint16_t *)buf);
		*(uint16_t *)(buf + len - 2) =
			swap16(*(uint16_t *)(buf + len - 2));
		buf += 2;
		len -= 4;
		break;
	}
	switch (len) {
	case 2 ... 3:
		buf[0] ^= buf[len - 1];
		buf[len - 1] ^= buf[0];
		buf[0] ^= buf[len - 1];
		break;
	}
}

static inline int strmatch(char *dst, const char *src)
{
	if (strlen(dst) < strlen(src))
		return 0;
	return memcmp(dst, src, strlen(src)) == 0;
}

static inline uint8_t __str2u8(char **str, int *err)
{
	uint16_t ret = 0;
	*err = 1;
	while (isdigit((int)**str)) {
		*err = 0;
		ret = ret * 10 + **str - '0';
		if (ret > U8_MAX) {
			*err = 1;
			return 0;
		}
		(*str)++;
	}
	return ret & U8_MAX;
}

static inline uint8_t str2u8(char *str, int *err)
{
	return __str2u8(&str, err);
}

static inline uint16_t __str2u16(char **str, int *err)
{
	uint32_t ret = 0;
	*err = 1;
	while (isdigit((int)**str)) {
		*err = 0;
		ret = ret * 10 + **str - '0';
		if (ret > U16_MAX) {
			*err = 1;
			return 0;
		}
		(*str)++;
	}
	return ret & U16_MAX;
}

static inline uint16_t str2u16(char *str, int *err)
{
	return __str2u16(&str, err);
}

static inline uint32_t __str2u32(char **str, int *err)
{
	uint64_t ret = 0;
	*err = 1;
	while (isdigit((int)**str)) {
		*err = 0;
		ret = ret * 10 + **str - '0';
		if (ret > U32_MAX) {
			*err = 1;
			return 0;
		}
		(*str)++;
	}
	return ret & U32_MAX;
}

static inline uint32_t str2u32(char *str, int *err)
{
	return __str2u32(&str, err);
}

static inline uint32_t __dotted2u32(char **str, int *err)
{
	uint64_t ret = 0;
	int i = 4;
	*err = 0;
	do {
		i--;
		ret = __str2u8(str, err) + ret * 256;
		if (*err || ret > U32_MAX)
			return 0;
		if (**str == '.')
			(*str)++;
	} while (i);
	if (i != 0)
		*err = 1;
	return ret & U32_MAX;
}

static inline uint32_t dotted2u32(char *str, int *err)
{
	return __dotted2u32(&str, err);
}

static inline uint32_t str2maskip(uint32_t *mask, char *str, int *err)
{
	uint32_t ip = __dotted2u32(&str, err);

	if (*err)
		return 0;
	if (*str != '/') {
		*err = 1;
		return 0;
	}
	++str;
	if (strlen(str) < 3) {
		uint8_t bit_width = str2u8(str, err);
		if (*err || bit_width > 32)
			return 0;
		*mask = ~0U << (32 - bit_width);
	} else {
		*mask = dotted2u32(str, err);
		if (*err)
			return 0;
	}
	return ip;
}

static inline uint8_t __hextou8(char *str, int *err)
{
	uint8_t ret = 0;
	*err = 0;
	switch (*str) {
	case '0' ... '9':
		ret = (*str - '0') << 4;
		break;
	case 'A' ... 'F':
		ret = (*str - 'A' + 10) << 4;
		break;
	case 'a' ... 'f':
		ret = (*str - 'a' + 10) << 4;
		break;
	default:
		*err = 1;
		return 0;
	}
	++str;
	switch (*str) {
	case '0' ... '9':
		ret |= (*str - '0');
		break;
	case 'A' ... 'F':
		ret |= (*str - 'A' + 10);
		break;
	case 'a' ... 'f':
		ret |= (*str - 'a' + 10);
		break;
	default:
		*err = 1;
		return 0;
	}
	return ret;
}

static inline void str2mac(uint8_t *dst, char *str, int *err)
{
	int i;

	*err = 0;
	for (i = 0; i < 6; i++) {
		switch (*str) {
		case ':':
		case '-':
			++str;
			break;
		case '0' ... '9':
		case 'A' ... 'F':
		case 'a' ... 'f':
			break;
		default:
			*err = 1;
			return;
		}
		dst[i] = __hextou8(str, err);
		if (*err)
			return;
		str += 2;
	}
}

static inline uint64_t get_gcd(uint64_t x, uint64_t y)
{
	if (x < y) {
		x ^= y;
		y ^= x;
		x ^= y;
	}
	while (y) {
		uint64_t r = x % y;
		x = y;
		y = r;
	}
	return x;
}

static inline uint64_t get_lcm(uint64_t x, uint64_t y)
{
	return (x * y) / get_gcd(x, y);
}

#endif /* __COMMON_H */

