#pragma once

#include "lstd/common.h"

extern "C" {
const void *memchr(const void *s, int c, size_t n);
char *strcat(char *s1, const char *s2);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *s);

const char *strchr(const char *s, int c);
const char *strstr(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);
const char *strrchr(const char *s, int n);

int strtol(const char *nptr, char **endptr, int base);
double atof(const char *str);
double strtod(const char *str, char **endptr);

double fmod(double x, double y);

int sscanf(const char *str, const char *fmt, ...);
int sprintf(char *str, const char *format, ...);

void qsort(void *data, size_t items, size_t size, int (*compare)(const void *, const void *));

int toupper(int c);

float fmodf(float x, float y);
float powf(float x, float y);
float logf(float x);
float fabsf(float x);
float sqrtf(float x);
float cosf(float x);
float sinf(float x);
float acosf(float x);
float atan2f(float x, float y);
float ceilf(float x);
}
