#pragma once
// High-level diagnostic convenience API built on top of snipffi.h
// Replaces previous macro-based system with inline functions.
// Provides simple single-shot emission helpers plus the original ERR_* style names as functions.

#include "snipffi.h"
#include "lstd/fmt.h"
#include "lstd/string.h"

// Forward declare tokenizer accessor indirection (implemented in lang.h)
using diag_get_line_fn = s64(*)(const void*);
using diag_get_filename_fn = const char*(*)(const void*);

inline exponential_array<string>* g_diag_sink = nullptr; // If set, rendered diagnostics appended here instead of printed
inline const void* g_active_tokenizer = nullptr;         // Opaque pointer to tokenizer
inline diag_get_line_fn g_diag_get_line = nullptr;
inline diag_get_filename_fn g_diag_get_filename = nullptr;

inline void diag_set_sink(exponential_array<string>* sink) { g_diag_sink = sink; }
inline void diag_set_active_tokenizer(const void* tz, diag_get_line_fn lf, diag_get_filename_fn ff) {
    g_active_tokenizer = tz; g_diag_get_line = lf; g_diag_get_filename = ff; }

// Core low-level building helpers --------------------------------------------------------------

inline GroupHandle diag__make_group(FfiLevelKind lvl, FfiTitleKind tk, const char* title,
                                    const char* id = nullptr, const char* id_url = nullptr) {
    GroupHandle g = group_new(lvl, tk, title);
    if (id) group_set_id(g, id);
    if (id_url) group_set_id_url(g, id_url);
    return g;
}

inline char* diag__render_single_group(GroupHandle g) {
    ReportHandle r = report_new();
    report_add_group(r, g);
    char* out = report_render(r);
    report_free(r);
    return out; // caller prints + frees string, frees group separately
}

inline void diag__print_and_free(char* rendered) {
    if (!rendered) return;
    if (g_diag_sink) {
        // Copy into arena (diagnostics may live past temporary render buffer)
        add(*g_diag_sink, make_string(rendered));
    } else {
        fmt_to_writer(&cerr, "{}\n", rendered);
    }
    free_string(rendered);
}

inline void diag__free_snippet_if_nonnull(SnippetHandle s) {
    if (s) snippet_free(s);
}

inline void diag__free_annotation_if_nonnull(AnnotationHandle a) {
    if (a) annotation_free(a);
}

// Single snippet simple error / warning --------------------------------------------------------
inline void diag_error(const char* source, const char* title) {
    SnippetHandle sn = snippet_new(source ? source : "", 1);
    GroupHandle g = diag__make_group(FFI_LEVEL_ERROR, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

inline void diag_warning(const char* source, const char* title) {
    SnippetHandle sn = snippet_new(source ? source : "", 1);
    GroupHandle g = diag__make_group(FFI_LEVEL_WARNING, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

// Annotated single primary span ----------------------------------------------------------------
inline int diag__line_start_for(const char* source, const char* pos) {
    if (!g_active_tokenizer || !g_diag_get_line) return 1;
    s64 tokenizer_line = g_diag_get_line(g_active_tokenizer);
    s64 lines_before = 0; for (const char* p = source; p < pos && *p; ++p) if (*p == '\n') ++lines_before;
    s64 baseline = tokenizer_line - lines_before; if (baseline < 1) baseline = 1; return (int)baseline;
}

inline void diag_error_annotated(const char* source, const char* title,
                                 const char* start, const char* end, const char* label) {
    if (!source) source = "";
    int line_start = diag__line_start_for(source, start);
    SnippetHandle sn = snippet_new(source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle a = annotation_new_primary((int)(start - source), (int)(end - source), label);
    snippet_add_annotation(sn, a);
    annotation_free(a);
    GroupHandle g = diag__make_group(FFI_LEVEL_ERROR, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

inline void diag_warning_annotated(const char* source, const char* title,
                                   const char* start, const char* end, const char* label) {
    if (!source) source = "";
    int line_start = diag__line_start_for(source, start);
    SnippetHandle sn = snippet_new(source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle a = annotation_new_primary((int)(start - source), (int)(end - source), label);
    snippet_add_annotation(sn, a);
    annotation_free(a);
    GroupHandle g = diag__make_group(FFI_LEVEL_WARNING, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

// Primary + context span -----------------------------------------------------------------------
inline void diag_error_annotated_context(const char* source, const char* title,
                                         const char* estart, const char* eend, const char* emsg,
                                         const char* cstart, const char* cend, const char* cmsg) {
    if (!source) source = "";
    int line_start = diag__line_start_for(source, estart);
    SnippetHandle sn = snippet_new(source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle pa = annotation_new_primary((int)(estart - source), (int)(eend - source), emsg);
    AnnotationHandle ca = annotation_new_context((int)(cstart - source), (int)(cend - source), cmsg);
    snippet_add_annotation(sn, pa);
    snippet_add_annotation(sn, ca);
    annotation_free(pa); annotation_free(ca);
    GroupHandle g = diag__make_group(FFI_LEVEL_ERROR, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

inline void diag_warning_annotated_context(const char* source, const char* title,
                                           const char* estart, const char* eend, const char* emsg,
                                           const char* cstart, const char* cend, const char* cmsg) {
    if (!source) source = "";
    int line_start = diag__line_start_for(source, estart);
    SnippetHandle sn = snippet_new(source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle pa = annotation_new_primary((int)(estart - source), (int)(eend - source), emsg);
    AnnotationHandle ca = annotation_new_context((int)(cstart - source), (int)(cend - source), cmsg);
    snippet_add_annotation(sn, pa);
    snippet_add_annotation(sn, ca);
    annotation_free(pa); annotation_free(ca);
    GroupHandle g = diag__make_group(FFI_LEVEL_WARNING, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

// Legacy-style convenience names (functions instead of macros) ---------------------------------
inline void ERR(const char* source, const char* title) { diag_error(source, title); }
inline void WARN(const char* source, const char* title) { diag_warning(source, title); }
inline void ERR_ANNOTATED(const char* source, const char* title, const char* start, const char* end, const char* msg) { diag_error_annotated(source, title, start, end, msg); }
inline void WARN_ANNOTATED(const char* source, const char* title, const char* start, const char* end, const char* msg) { diag_warning_annotated(source, title, start, end, msg); }
inline void ERR_ANNOTATED_CONTEXT(const char* source, const char* title,
                                 const char* estart, const char* eend, const char* emsg,
                                 const char* cstart, const char* cend, const char* cmsg) {
    diag_error_annotated_context(source, title, estart, eend, emsg, cstart, cend, cmsg);
}
inline void WARN_ANNOTATED_CONTEXT(const char* source, const char* title,
                                   const char* estart, const char* eend, const char* emsg,
                                   const char* cstart, const char* cend, const char* cmsg) {
    diag_warning_annotated_context(source, title, estart, eend, emsg, cstart, cend, cmsg);
}

// Future extension hooks:
// - Aggregated report building (vector of groups) with arena lifetime
// - Patch suggestion helpers
// - ID/URL injection helpers
// - Anonymized line number / short message variants
