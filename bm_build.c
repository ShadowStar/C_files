/*
 * lib/ts_bm.c		Boyer-Moore text search implementation
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Pablo Neira Ayuso <pablo@eurodev.net>
 *
 * ==========================================================================
 *
 *   Implements Boyer-Moore string matching algorithm:
 *
 *   [1] A Fast String Searching Algorithm, R.S. Boyer and Moore.
 *       Communications of the Association for Computing Machinery,
 *       20(10), 1977, pp. 762-772.
 *       http://www.cs.utexas.edu/users/moore/publications/fstrpos.pdf
 *
 *   [2] Handbook of Exact String Matching Algorithms, Thierry Lecroq, 2004
 *       http://www-igm.univ-mlv.fr/~lecroq/string/string.pdf
 *
 *   Note: Since Boyer-Moore (BM) performs searches for matchings from right
 *   to left, it's still possible that a matching could be spread over
 *   multiple blocks, in that case this algorithm won't find any coincidence.
 *
 *   If you're willing to ensure that such thing won't ever happen, use the
 *   Knuth-Pratt-Morris (KMP) implementation instead. In conclusion, choose
 *   the proper string search algorithm depending on your setting.
 *
 *   Say you're using the textsearch infrastructure for filtering, NIDS or
 *   any similar security focused purpose, then go KMP. Otherwise, if you
 *   really care about performance, say you're classifying packets to apply
 *   Quality of Service (QoS) policies, and you don't mind about possible
 *   matchings spread over multiple fragments, then go BM.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Alphabet size, use ASCII */
#define ASIZE 256

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1 : __max2; })

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

struct ts_bm
{
	uint8_t *pattern;
	uint32_t patlen;
	uint32_t bad_shift[ASIZE];
	uint32_t good_shift[0];
};

static char *pattern;
static uint32_t patlen;
static char *app_name;
static int ignorecase;

static inline uint8_t *bm_find(struct ts_bm *bm, const uint8_t *text, uint32_t text_len)
{
	unsigned int i;
	int shift = bm->patlen - 1, bs;

	while (shift < text_len) {
		for (i = 0; i < bm->patlen; i++)
			if (text[shift-i] != bm->pattern[bm->patlen-1-i])
				goto next;

		/* London calling... */
		return (uint8_t *)text + (shift-(bm->patlen-1));

next:
		bs = bm->bad_shift[text[shift-i]];

		/* Now jumping to... */
		shift = max_t(int, shift-i+bs, shift+bm->good_shift[i]);
	}

	return NULL;
}

#define PATTERN_STR ({ \
	int i; \
	for (i = 0; i < patlen; i++) \
		if (isalnum(pattern[i]) || pattern[i] == '_') \
			printf("%c", pattern[i]); \
		else \
			printf("%02x", pattern[i]); })

static void build_file_pre(void)
{
	printf("#ifndef __BM_");
	PATTERN_STR;
	printf("_INC\n");
	printf("#define __BM_");
	PATTERN_STR;
	printf("_INC\n\n");
	printf("#include <stdio.h>\n");
	printf("#include <stdlib.h>\n");
	printf("#include <stdint.h>\n");
	printf("#include <string.h>\n\n");
}

static void build_file_post(void)
{
	printf("static inline uint8_t *bm_find_");
	PATTERN_STR;
	printf("(const uint8_t *text, uint32_t len)\n");
	printf("{\n");
	printf("\tint i, shift = %d - 1, bs, gs;\n", patlen);
	printf("\tconst uint8_t pattern[] = {");
	int i;
	for (i = 0; i < patlen; i++)
		printf(" 0x%X,", pattern[i]);
	printf("};\n");
	printf("\twhile (shift < len) {\n");
	printf("\t\tfor (i = 0; i < %d; i++)\n", patlen);
	if (ignorecase)
		printf("\t\t\tif (tolower(text[shift - i]) != pattern[%d - 1 - i])\n",
		       patlen);
	else
		printf("\t\t\tif (text[shift - i] != pattern[%d - 1 - i])\n",
		       patlen);
	printf("\t\t\t\tgoto next;\n");
	printf("\t\treturn (uint8_t *)text + shift - %d + 1;\n", patlen);
	printf("next:\n");
	printf("\t\tbs = shift - i + get_bs_");
	PATTERN_STR;
	printf("(text[shift - i]);\n");
	printf("\t\tgs = shift + get_gs_");
	PATTERN_STR;
	printf("(i);\n");
	printf("\t\tshift = bs > gs ? bs : gs;\n");
	printf("\t}\n");
	printf("\treturn NULL;\n");
	printf("}\n");
	printf("#endif\n");
}

static void build_gs_pre(void)
{
	printf("static inline int get_gs_");
	PATTERN_STR;
	printf("(int i)\n");
	printf("{\n");
	printf("\tswitch (i) {\n");
	printf("\tdefault: return %d;\n", patlen);
	printf("\tcase 0: return 1;\n");
}

static void build_gs_post(void)
{
	printf("\t}\n");
	printf("}\n\n");
}

static void build_bs_pre(void)
{
	printf("static inline int get_bs_");
	PATTERN_STR;
	printf("(int i)\n");
	printf("{\n");
	printf("\tswitch (i) {\n");
	printf("\tdefault: return %d;\n", patlen);
}

static void build_bs_post(void)
{
	printf("\t}\n");
	printf("}\n\n");
}

static void build_shift(int i, int r)
{
	printf("\tcase %d: return %d;\n", i, r);
}

static int subpattern(uint8_t *pattern, int i, int j, int g)
{
	int x = i+g-1, y = j+g-1, ret = 0;

	while(pattern[x--] == pattern[y--]) {
		if (y < 0) {
			ret = 1;
			break;
		}
		if (--g == 0) {
			ret = pattern[i-1] != pattern[j-1];
			break;
		}
	}

	return ret;
}

static void compute_prefix_tbl(struct ts_bm *bm)
{
	int i, j, g;

	for (i = 0; i < ASIZE; i++)
		bm->bad_shift[i] = bm->patlen;
	for (i = 0; i < bm->patlen - 1; i++) {
		bm->bad_shift[bm->pattern[i]] = bm->patlen - 1 - i;
		if (ignorecase)
			bm->bad_shift[toupper(bm->pattern[i])]
			    = bm->patlen - 1 - i;
	}

	/* Compute the good shift array, used to match reocurrences
	 * of a subpattern */
	bm->good_shift[0] = 1;
	for (i = 1; i < bm->patlen; i++)
		bm->good_shift[i] = bm->patlen;
        for (i = bm->patlen-1, g = 1; i > 0; g++, i--) {
		for (j = i-1; j >= 1-g ; j--)
			if (subpattern(bm->pattern, i, j, g)) {
				bm->good_shift[g] = bm->patlen-j-g;
				break;
			}
	}
}

static void build_file(struct ts_bm *bm)
{
	int i;

	build_file_pre();
	build_bs_pre();

	for (i = 0; i < ASIZE; i++) {
		if (bm->bad_shift[i] != bm->patlen)
			build_shift(i, bm->bad_shift[i]);
	}
	build_bs_post();

	/* Compute the good shift array, used to match reocurrences
	 * of a subpattern */
	build_gs_pre();
	for (i = 1; i < bm->patlen; i++) {
		if (bm->good_shift[i] != bm->patlen)
			build_shift(i, bm->good_shift[i]);
	}
	build_gs_post();
	build_file_post();
}

static struct ts_bm *bm_init(const void *pattern, unsigned int len)
{
	struct ts_bm *bm;
	int i;
	unsigned int prefix_tbl_len = len * sizeof(uint32_t);
	size_t priv_size = sizeof(*bm) + len + prefix_tbl_len;

	bm = calloc(1, priv_size);
	bm->patlen = len;
	bm->pattern = (uint8_t *) bm->good_shift + prefix_tbl_len;
	memcpy(bm->pattern, pattern, len);
	compute_prefix_tbl(bm);

	fprintf(stderr, "%u-%s[%zu]\n", bm->patlen, bm->pattern, priv_size);
	fprintf(stderr, "Bad Shift:\n");
	for (i = 0; i < ASIZE; i++) {
		if ((i % 16) == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "%d ", bm->bad_shift[i]);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "Good Shift:\n");
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			fprintf(stderr, "\n");
		fprintf(stderr, "%d ", bm->good_shift[i]);
	}
	fprintf(stderr, "\n");
	return bm;
}

static int parse_char(char *src, char *dst)
{
	int err, len = 0;
	while (isprint(*src)) {
		if (*src == '\\') {
			switch (*(++src)) {
			case 'a':
				*dst = 0x07;
				break;
			case 'b':
				*dst = 0x08;
				break;
			case 'f':
				*dst = 0x0C;
				break;
			case 'n':
				*dst = 0x0A;
				break;
			case 'r':
				*dst = 0x0D;
				break;
			case 't':
				*dst = 0x09;
				break;
			case 'v':
				*dst = 0x0B;
				break;
			case 'x':
				*dst = __hextou8((++src), &err);
				if (err)
					return 0;
				++src;
				break;
			default:
				return 0;
			}
		} else if (ignorecase)
			*dst = tolower(*src);
		else
			*dst = *src;

		src++;
		dst++;
		len++;
	}
	return len;
}

static void usage(void)
{
	fprintf(stderr, "Usage: bm_build [-i] \"Pattern String\"\n");
	fprintf(stderr, "       -i  -- Ignore Case in Pattern String\n");
}

int main(int argc, char *argv[])
{
	char *pat;
	app_name = argv[0];
	switch (argc) {
	default:
		usage();
		exit(EXIT_FAILURE);
	case 3:
		if (memcmp("-i", argv[1], strlen(argv[1])) == 0)
			ignorecase = 1;
		else {
			usage();
			exit(EXIT_FAILURE);
		}
		pat = argv[2];
		break;
	case 2:
		pat = argv[1];
		break;
	}
	patlen = strlen(pat);
	pattern = calloc(1, patlen + 1);
	patlen = parse_char(pat, pattern);
	if (patlen == 0) {
		fprintf(stderr, "Pattern Error.\n");
		return -1;
	}
	struct ts_bm *bm = bm_init(pattern, patlen);
	build_file(bm);
	return EXIT_SUCCESS;
}	/* ----------  end of function main  ---------- */

