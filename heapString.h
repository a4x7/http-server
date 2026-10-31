#ifndef HEAPSTRING_H
#define HEAPSTRING_H

#include <stddef.h>
typedef struct{
    char *str;
    size_t n, c;
} String;

void string_init(String *s);
const char* string_read(const String *s);
const size_t *string_size(const String *s);
void string_resize(String *s, size_t len);
void string_append(String *s, const char c);
void string_pop(String *s);
void string_substr(const String *s, size_t begin, size_t end, String *substr);
void string_raw_cat(String *s, const char *other, size_t n);
void string_copy(String *s, const String *other);
void string_raw_copy(String *s, const char *other, size_t n);
void string_move(String *s, String *newS);
void string_deinit(String *s);

#endif
