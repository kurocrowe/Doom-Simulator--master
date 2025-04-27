#ifndef _UTIL_H
#define _UTIL_H

#include <ctype.h>
#include <stddef.h>

static inline int strcmp_nocase(const char *str1, const char *str2) {
  int ret_code = 0;
  while (*str1 || *str2) {
    ret_code = tolower((int)(*str1)) - tolower((int)(*str2));
    if (ret_code != 0) { break; }
    str1++, str2++;
  }

  return ret_code;
}

static inline int strncmp_nocase(const char *str1, const char *str2, size_t n) {
  int    ret_code = 0;
  size_t i        = 0;
  while (i < n && (*str1 || *str2)) {
    ret_code = tolower((int)(*str1)) - tolower((int)(*str2));
    if (ret_code != 0) { break; }
    str1++, str2++, i++;
  }

  return ret_code;
}

#endif // !_UTIL_H
