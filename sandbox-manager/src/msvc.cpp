//
// Compile this file only on MSVC.
// There are definitions of functions normally found in the CRT,
// but we aren't linking with it.
//
// :CRT

#include <SDL_stdinc.h>

extern "C" {
    int _fltused = 1;

    void* __cdecl memset(void *dst, int c, size_t num) {
        return SDL_memset(dst, c, num);
    }

    void* __cdecl memcpy(void *dst, const void *src, size_t len) {
        return SDL_memcpy(dst, src, len);
    }

    int __cdecl memcmp(const void *s1, const void *s2, size_t len) {
        return SDL_memcmp(s1, s2, len);
    }

    void* __cdecl memmove(void *dst, const void *src, size_t len) {
        return SDL_memmove(dst, src, len);
    }

    char* __cdecl strchr(const char *str, int c) {
        return SDL_strchr(str, c);
    }

    char* __cdecl strstr(const char *haystack, const char *needle) {
        return SDL_strstr(haystack, needle);
    }

    int __cdecl strcmp(const char *str1, const char *str2) {
        return SDL_strcmp(str1, str2);
    }

    int __cdecl strncmp(const char *str1, const char *str2, size_t maxlen) {
        return SDL_strncmp(str1, str2, maxlen);
    }

    size_t __cdecl strlen(const char *str) {
        return SDL_strlen(str);
    }

    void __cdecl strcpy(char *dst, const char *src) {
        for (int i = 0; dst[i] = src[i]; i++);
    }

    char* __cdecl strncpy(char *dst, const char *src, size_t num) {
        for (int i = 0; i < num; i++)
            dst[i] = src[i];
        return dst;
    }

    int __cdecl isdigit(int x) { return SDL_isdigit(x); }
    int __cdecl isspace(int x) { return SDL_isspace(x); }

    int __cdecl toupper_(int x) { return SDL_toupper(x); }
    int __cdecl tolower_(int x) { return SDL_tolower(x); }

    int __cdecl isprint_(int x) {
        return x > 31 && x != 127;
    }

    void __cdecl qsort(void *base, size_t num, size_t size, int(*compare)(const void*, const void*)) {
        return SDL_qsort(base, num, size, compare);
    }
}