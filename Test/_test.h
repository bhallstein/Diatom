#ifdef __cplusplus
#include <cstdio>
#include <cassert>
#else
#include <stdio.h>
#include <assert.h>
#endif

#define p_assert(x) do {    \
  printf("%60s", #x);       \
  assert(x);                \
  printf(" - PASS :)\n");   \
} while (0)

#define p_header(s) do {    \
  printf("\n");             \
  printf("- %s\n", s);      \
} while (0)

#define p_file_header(FILE) do {                  \
  printf("\n" FILE ":\n");                        \
  printf("--------------------------------\n");   \
} while (0)

