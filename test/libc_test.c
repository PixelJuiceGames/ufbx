
#define UFBXC_HAS_MALLOC
#define UFBXC_HAS_STDERR
#define UFBXC_HAS_EXIT
#include "../misc/ufbx_libc.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// -- OS API

size_t malloc_count = 0;
size_t free_count = 0;

void *ufbxc_os_allocate(size_t size, size_t *p_allocated_size)
{
	malloc_count++;

	size_t min_size = 16*1024*1024;
	if (size < min_size) {
		size = min_size;
	}

	*p_allocated_size = size;
	return malloc(size);
}

bool ufbxc_os_free(void *pointer, size_t allocated_size)
{
	free_count++;
	free(pointer);
	return true;
}

void ufbxc_os_print_error(const char *message, size_t length)
{
	fprintf(stderr, "%s", message);
}

void ufbxc_os_exit(int code)
{
	exit(code);
}

// -- Tests

char print_buf[1024];

static int test_sprintf(const char *expected, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = ufbxc_vsnprintf(print_buf, sizeof(print_buf), fmt, args);
	va_end(args);


	ufbxc_assert(strcmp(print_buf, expected) == 0);
	ufbxc_assert((int)strlen(print_buf) == result);
}

void test_printf()
{
	printf("test_printf()\n");

	test_sprintf("Hello", "Hello");
	test_sprintf("Hello", "%s", "Hello");
	test_sprintf("Hello world", "%s %s", "Hello", "world");
	test_sprintf("Hel", "%.3s", "Hello");
	test_sprintf("Hel", "%.3s", "Hello");
	test_sprintf("Hel", "%.*s", 3, "Hello");
	test_sprintf("00beef", "%06x", 0xbeef);
	test_sprintf("beef  ", "%-6x", 0xbeef);
	test_sprintf("0XBEEF", "%#X", 0xbeef);
}

void test_float(const char *str)
{
	double ref_d = strtod(str, NULL);
	double d = ufbxc_strtod(str, NULL);
	float ref_f = strtof(str, NULL);
	float f = ufbxc_strtof(str, NULL);

	if (isfinite(ref_d)) {
		if (d != ref_d) {
			fprintf(stderr, "strtod() mismatch: '%s': reference %.20g, ufbxc %.20g\n", str, ref_d, d);
			exit(1);
		}
	} else {
		ufbxc_assert(!isfinite(d));
	}
	if (isfinite(ref_f)) {
		if (f != ref_f) {
			fprintf(stderr, "strtod() mismatch: '%s': reference %.20g, ufbxc %.20g\n", str, ref_d, d);
			exit(1);
		}
	} else {
		ufbxc_assert(!isfinite(f));
	}
}

void test_float_parse_fmt(const char *fmt, int width, uint32_t bits)
{
	char buffer[128];
	printf("test_float_parse() %s %d %ubits\n", fmt, width, bits);

	uint32_t max_hi = 1 << bits;
	for (uint32_t hi = 0; hi < max_hi; hi++) {
		for (int32_t delta = -2; delta <= 2; delta++) {
			uint32_t bits_f = (hi << (32u - bits)) + (uint32_t)delta;
			uint64_t bits_d = ((uint64_t)hi << (64u - bits)) + (uint64_t)(int64_t)delta;

			float val_f;
			double val_d;
			memcpy(&val_f, &bits_f, sizeof(float));
			memcpy(&val_d, &bits_d, sizeof(double));

			if (isfinite(val_f)) {
				snprintf(buffer, sizeof(buffer), fmt, width, val_f);
				test_float(buffer);
			}
			if (isfinite(val_d)) {
				snprintf(buffer, sizeof(buffer), fmt, width, val_d);
				test_float(buffer);
			}
		}
	}
}

void test_float_parse(uint32_t bits)
{
	test_float("0");
	test_float("-0");
	test_float(".5");
	test_float("-.5");
	test_float("1e100");
	test_float("1e1000");
	test_float("1e10000");
	test_float("1e-10000");
	test_float("7.67844768714563e-239");

	for (int width = 4; width <= 20; width++) {
		test_float_parse_fmt("%.*f", width, bits);
	}
	for (int width = 4; width <= 20; width++) {
		test_float_parse_fmt("%.*e", width, bits);
	}
}

typedef struct {
    uint64_t a;
} xorshift64_state;

uint64_t xorshift64(xorshift64_state *state)
{
	uint64_t x = state->a;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return state->a = x;
}

#define TEST_MALLOC_SLOTS 16384

void test_malloc(uint32_t rounds)
{
	static void *memory[TEST_MALLOC_SLOTS] = { 0 };

	ufbxc_free(NULL);

	xorshift64_state rng = { 1 };
	double prev_prog = 0.0;
	for (size_t i = 0; i < rounds; i++) {

		if (i % 4096 == 0) {
			double prog = (double)i / (double)rounds * 100.0;
			if (prog >= prev_prog + 10.0) {
				printf("test_malloc() %.1f%%\n", prog);
				prev_prog = prog;
			}
		}

		size_t op = xorshift64(&rng) % 64;

		void **slot = &memory[xorshift64(&rng) % TEST_MALLOC_SLOTS];
		size_t mantissa = xorshift64(&rng) % 256;
		size_t exponent = xorshift64(&rng) % 8;
		size_t size = mantissa << exponent;

		if (op <= 2) {
			*slot = ufbxc_realloc(*slot, size);
			continue;
		}

		if (*slot) {
			ufbxc_free(*slot);
			*slot = NULL;
		}

		if (op <= 16) continue;

		void *pointer = ufbxc_malloc(size);
		ufbxc_assert(pointer);
		*slot = pointer;
	}

		printf("test_malloc() %.1f%%\n", 100.0);

	for (size_t i = 0; i < TEST_MALLOC_SLOTS; i++) {
		ufbxc_free(memory[i]);
	}

	ufbxc_assert(malloc_count == free_count);
}

int main(int argc, char **argv)
{
	test_printf();
#if 1
	test_float_parse(14);
	test_malloc(8*1024*1024);
#else
	test_float_parse(18);
	test_malloc(64*1024*1024);
#endif
}

#include "../misc/ufbx_libc.c"
