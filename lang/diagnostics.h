#pragma once

#include "snipffi.h"
#include "lstd/fmt.h"
#include "lstd/string.h"

// Forward declare tokenizer accessor indirection (implemented in lang.h)
using diag_get_line_fn = s64(*)(const void*);
using diag_get_filename_fn = const char*(*)(const void*);

inline array<string>* g_diag_sink = null;        // If set, rendered diagnostics appended here instead of printed
inline const void* g_active_tokenizer = null;    // Opaque pointer to tokenizer
inline const char* g_source = null;              // Current source code being tokenized (for snippets)
inline diag_get_line_fn g_diag_get_line = null;
inline diag_get_filename_fn g_diag_get_filename = null;

inline void diag_set_source(const char* source) { g_source = source; }
inline void diag_set_sink(array<string>* sink) { g_diag_sink = sink; }
inline void diag_set_active_tokenizer(const void* tz, diag_get_line_fn lf, diag_get_filename_fn ff) { 
    g_active_tokenizer = tz; g_diag_get_line = lf; g_diag_get_filename = ff; 
}

// Core low-level building helpers --------------------------------------------------------------

inline GroupHandle diag__make_group(FfiLevelKind lvl, FfiTitleKind tk, const char* title,
                                    const char* id = null, const char* id_url = null) {
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

inline void diag__print(char* rendered) {
    if (!rendered) return;
    if (g_diag_sink) {
        // Copy into arena (diagnostics may live past temporary render buffer)
        add(*g_diag_sink, make_string(rendered));
    } else {
        fmt_to_writer(&cerr, "\n{}\n", rendered);
    }
}

inline void diag__print_and_free(char* rendered) {
    if (!rendered) return;
    diag__print(rendered);
    free_string(rendered);
}

inline void diag__free_snippet_if_nonnull(SnippetHandle s) {
    if (s) snippet_free(s);
}

inline void diag__free_annotation_if_nonnull(AnnotationHandle a) {
    if (a) annotation_free(a);
}

// Single snippet simple error / warning --------------------------------------------------------
inline void diag_error(const char* title) {
    if (!g_source && !g_active_tokenizer)  {
        char *out = mprint("{!tBRIGHT_RED;B}error:{!WHITE} {}{!}\n", title);
        diag__print(out);
    } else {
        SnippetHandle sn = snippet_new(g_source ? g_source : "", 1);
        GroupHandle g = diag__make_group(FFI_LEVEL_ERROR, FFI_TITLE_PRIMARY, title);
        group_add_snippet(g, sn);
        char *out = diag__render_single_group(g);
        diag__print_and_free(out);
        group_free(g);
        snippet_free(sn);
    }
}

inline void diag_warning(const char* title) {
    if (!g_source && !g_active_tokenizer)  {
        char *out = mprint("{!YELLOW;B}warning:{!WHITE} {}{!}\n", title);
        diag__print(out);
    } else {
        SnippetHandle sn = snippet_new(g_source ? g_source : "", 1);
        GroupHandle g = diag__make_group(FFI_LEVEL_WARNING, FFI_TITLE_PRIMARY, title);
        group_add_snippet(g, sn);
        char* out = diag__render_single_group(g);
        diag__print_and_free(out);
        group_free(g);
        snippet_free(sn);
    }
}

// Annotated single primary span ----------------------------------------------------------------
inline int diag__line_start_for(const char* pos) {
    if (!g_active_tokenizer || !g_diag_get_line) return 1;
    s64 tokenizer_line = g_diag_get_line(g_active_tokenizer);
    s64 lines_before = 0; for (const char* p = g_source; p < pos && *p; ++p) if (*p == '\n') ++lines_before;
    s64 baseline = tokenizer_line - lines_before; if (baseline < 1) baseline = 1; return (int)baseline;
}

inline void diag_error_annotated(const char* title,
                                 const char* start, const char* end, const char* label) {
    if (!g_source) g_source = "";
    int line_start = diag__line_start_for(start);
    SnippetHandle sn = snippet_new(g_source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle a = annotation_new_primary((int)(start - g_source), (int)(end - g_source), label);
    snippet_add_annotation(sn, a);
    annotation_free(a);
    GroupHandle g = diag__make_group(FFI_LEVEL_ERROR, FFI_TITLE_PRIMARY, title);
    group_add_snippet(g, sn);
    char* out = diag__render_single_group(g);
    diag__print_and_free(out);
    group_free(g);
    snippet_free(sn);
}

inline void diag_warning_annotated(const char* title,
                                   const char* start, const char* end, const char* label) {
    if (!g_source) g_source = "";
    int line_start = diag__line_start_for(start);
    SnippetHandle sn = snippet_new(g_source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle a = annotation_new_primary((int)(start - g_source), (int)(end - g_source), label);
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
inline void diag_error_annotated_context(const char* title,
                                         const char* estart, const char* eend, const char* emsg,
                                         const char* cstart, const char* cend, const char* cmsg) {
    if (!g_source) g_source = "";
    int line_start = diag__line_start_for(estart);
    SnippetHandle sn = snippet_new(g_source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle pa = annotation_new_primary((int)(estart - g_source), (int)(eend - g_source), emsg);
    AnnotationHandle ca = annotation_new_context((int)(cstart - g_source), (int)(cend - g_source), cmsg);
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

inline void diag_warning_annotated_context(const char* title,
                                           const char* estart, const char* eend, const char* emsg,
                                           const char* cstart, const char* cend, const char* cmsg) {
    if (!g_source) g_source = "";
    int line_start = diag__line_start_for(estart);
    SnippetHandle sn = snippet_new(g_source, line_start);
    if (g_active_tokenizer && g_diag_get_filename) { const char* fn = g_diag_get_filename(g_active_tokenizer); if (fn) snippet_set_path(sn, fn); }
    AnnotationHandle pa = annotation_new_primary((int)(estart - g_source), (int)(eend - g_source), emsg);
    AnnotationHandle ca = annotation_new_context((int)(cstart - g_source), (int)(cend - g_source), cmsg);
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
inline void ERR(const char* title) { diag_error(title); }
inline void WARN(const char* title) { diag_warning(title); }
inline void ERR_ANNOTATED(const char* title, const char* start, const char* end, const char* msg) { diag_error_annotated(title, start, end, msg); }
inline void WARN_ANNOTATED(const char* title, const char* start, const char* end, const char* msg) { diag_warning_annotated(title, start, end, msg); }
inline void ERR_ANNOTATED_CONTEXT(const char* title,
                                 const char* estart, const char* eend, const char* emsg,
                                 const char* cstart, const char* cend, const char* cmsg) {
    diag_error_annotated_context(title, estart, eend, emsg, cstart, cend, cmsg);
}
inline void WARN_ANNOTATED_CONTEXT(const char* title,
                                   const char* estart, const char* eend, const char* emsg,
                                   const char* cstart, const char* cend, const char* cmsg) {
    diag_warning_annotated_context(title, estart, eend, emsg, cstart, cend, cmsg);
}

// Future extension hooks:
// - Patch suggestion helpers
// - ID/URL injection helpers
// - Anonymized line number / short message variants
