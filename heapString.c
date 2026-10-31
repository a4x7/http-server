#include "heapString.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void string_init(String *s){
    s->str = NULL;
    s->n = 0;
    s->c = 0;
}

const size_t *string_size(const String *s){
    if(s == NULL)
        return NULL;
    return &s->n;
}

const char* string_read(const String *s){
    if(s == NULL)
        return NULL;
    return s->str;
}

void string_resize(String *s, size_t len){
    if(s == NULL || len <= s->n || len == 0)
        return;
    char *new_str = malloc(sizeof(char)*len+1);
    for(size_t i = 0; i < s->n; i++)
        new_str[i] = s->str[i];
    new_str[s->n] = '\0';
    free(s->str);
    s->str = new_str;
    s->c = len;
}

void string_append(String *s, const char c){
    if(s == NULL)
        return;
    if(s->str == NULL)
        string_resize(s, 1);
    if(s->n == s->c)
        string_resize(s, 2*s->c);
    s->str[s->n++] = c;
    s->str[s->n] = '\0';
}

void string_raw_cat(String *s, const char *other, size_t n){
    if(s == NULL || other == NULL)
        return;
    for(size_t i = 0; i < n && other[i] != '\0'; i++)
        string_append(s, other[i]);
}

void string_pop(String *s){
    if(s == NULL || s->n == 0)
        return;
    s->str[--s->n] = '\0';
}

void string_substr(const String *s, size_t start, size_t end, String *substr){
    if(s == NULL || substr == NULL || start < 0 || start >= s->n || end < 0 && end >= s->n || start > end)
        return;
    for(size_t i = start; i < end; i++)
        string_append(substr, s->str[i]);
}

void string_copy(String *s, const String *other){
    if(s == NULL || other == NULL)
        return;
    string_deinit(s);
    for(size_t i = 0; i < s->n; i++)
        string_append(s, other->str[i]);
}

void string_raw_copy(String *s, const char *other, size_t n){
    if(s == NULL || other == NULL)
        return;
    string_deinit(s);
    for(size_t i = 0; i < n && other[i] != '\0'; i++)
        string_append(s, other[i]);
}

void string_move(String *s, String *other){
    if(s == other)
        return;
    string_deinit(s);
    s->str = s->str;
    other->str = NULL;
    s->n = s->n;
    other->n = 0;
    s->c = s->c;
    other->c = 0;
}

void string_deinit(String *s){
    if(s == NULL)
        return;
    s->n = 0;
    s->c = 0;
    if(s->str != NULL)
        free(s->str);
}
