#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void* SnippetHandle;
typedef void* AnnotationHandle;

// Snippet functions
SnippetHandle snippet_new(const char* source, int line_start);
void snippet_set_path(SnippetHandle snippet, const char* path);
void snippet_add_annotation(SnippetHandle snippet, AnnotationHandle annotation);
void snippet_free(SnippetHandle snippet);

// Annotation functions
AnnotationHandle annotation_new_primary(int start, int end, const char* label);
AnnotationHandle annotation_new_context(int start, int end, const char* label);
AnnotationHandle annotation_new_visible(int start, int end, const char* label);
void annotation_free(AnnotationHandle annotation);

// Rendering functions
char* render_error(const char* title, SnippetHandle snippet);
char* render_warning(const char* title, SnippetHandle snippet);
void free_string(char* str);

#ifdef __cplusplus
}
#endif
