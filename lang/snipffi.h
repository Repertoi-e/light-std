#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handles
typedef void* SnippetHandle;          // Annotation snippet
typedef void* PatchSnippetHandle;     // Patch snippet
typedef void* AnnotationHandle;
typedef void* PatchHandle;
typedef void* GroupHandle;
typedef void* ReportHandle;

// -------------------------------------------------------------------------------------------------
// Enums mirroring Rust side
// -------------------------------------------------------------------------------------------------

typedef enum {
    FFI_LEVEL_ERROR = 0,
    FFI_LEVEL_WARNING = 1,
    FFI_LEVEL_INFO = 2,
    FFI_LEVEL_NOTE = 3,
    FFI_LEVEL_HELP = 4
} FfiLevelKind;

typedef enum {
    FFI_TITLE_PRIMARY = 0,
    FFI_TITLE_SECONDARY = 1
} FfiTitleKind;

typedef enum {
    FFI_DECOR_UNICODE = 0,
    FFI_DECOR_ASCII = 1
} FfiDecorStyle;

// -------------------------------------------------------------------------------------------------
// Annotation Snippet (highlight source with annotations)
// -------------------------------------------------------------------------------------------------
SnippetHandle snippet_new(const char* source, int line_start); // line_start <=0 means default (1)
void snippet_set_path(SnippetHandle snippet, const char* path); // normalized internally
void snippet_set_fold(SnippetHandle snippet, int fold);         // non-zero => fold(true)
void snippet_add_annotation(SnippetHandle snippet, AnnotationHandle annotation);
void snippet_free(SnippetHandle snippet);

// -------------------------------------------------------------------------------------------------
// Patch Snippet (suggested edits)
// -------------------------------------------------------------------------------------------------
PatchSnippetHandle patch_snippet_new(const char* source, int line_start);
void patch_snippet_set_path(PatchSnippetHandle snippet, const char* path); // alias to snippet_set_path semantics (not yet implemented separately)
void patch_snippet_set_fold(PatchSnippetHandle snippet, int fold);         // alias to snippet_set_fold semantics (not yet implemented separately)
void patch_snippet_add_patch(PatchSnippetHandle snippet, PatchHandle patch);
void patch_snippet_free(PatchSnippetHandle snippet);

// -------------------------------------------------------------------------------------------------
// Annotations
// -------------------------------------------------------------------------------------------------
AnnotationHandle annotation_new_primary(int start, int end, const char* label);
AnnotationHandle annotation_new_context(int start, int end, const char* label);
AnnotationHandle annotation_new_visible(int start, int end, const char* label);
void annotation_free(AnnotationHandle annotation);

// -------------------------------------------------------------------------------------------------
// Patches
// -------------------------------------------------------------------------------------------------
PatchHandle patch_new(int start, int end, const char* replacement);
void patch_free(PatchHandle patch);

// -------------------------------------------------------------------------------------------------
// Groups
// -------------------------------------------------------------------------------------------------
GroupHandle group_new(FfiLevelKind level, FfiTitleKind title_kind, const char* title_text); // title_text may be NULL for title-less group
void group_set_id(GroupHandle group, const char* id);          // e.g. error code
void group_set_id_url(GroupHandle group, const char* url);     // reference URL for id
void group_add_snippet(GroupHandle group, SnippetHandle snippet);
void group_add_patch_snippet(GroupHandle group, PatchSnippetHandle snippet);
void group_add_message(GroupHandle group, FfiLevelKind level, const char* text, int no_name); // no_name: suppress level tag
void group_add_padding(GroupHandle group);                     // visual spacer
void group_free(GroupHandle group);

// -------------------------------------------------------------------------------------------------
// Report
// -------------------------------------------------------------------------------------------------
ReportHandle report_new();
void report_add_group(ReportHandle report, GroupHandle group);
void report_set_decor_style(ReportHandle report, FfiDecorStyle style);
void report_set_anonymized_line_numbers(ReportHandle report, int anonymized);
void report_set_short_message(ReportHandle report, int short_message);
char* report_render(ReportHandle report);  // Returns newly allocated C string; must call free_string
void report_free(ReportHandle report);

// -------------------------------------------------------------------------------------------------
// Convenience simple render (legacy-style single snippet helpers)
// -------------------------------------------------------------------------------------------------
char* render_error(const char* title, SnippetHandle snippet);
char* render_warning(const char* title, SnippetHandle snippet);

// -------------------------------------------------------------------------------------------------
// Memory helpers
// -------------------------------------------------------------------------------------------------
void free_string(char* str);

#ifdef __cplusplus
}
#endif
