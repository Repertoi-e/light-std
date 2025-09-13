use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;
use std::collections::HashMap;
use std::sync::{Mutex, LazyLock};
use std::sync::atomic::{AtomicUsize, Ordering};

use annotate_snippets::{Level, Renderer, Snippet, AnnotationKind, Annotation, Group, Patch};
// Padding re-exported via pub use snippet::* in crate; direct import unnecessary
use annotate_snippets::renderer::DecorStyle;

// Global storage for handles (thread-safe)
static NEXT_HANDLE_ID: AtomicUsize = AtomicUsize::new(1);
static SNIPPETS: LazyLock<Mutex<HashMap<usize, SnippetBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new())); // annotation snippets
static PATCH_SNIPPETS: LazyLock<Mutex<HashMap<usize, PatchSnippetBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static ANNOTATIONS: LazyLock<Mutex<HashMap<usize, AnnotationBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static PATCHES: LazyLock<Mutex<HashMap<usize, PatchBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static GROUPS: LazyLock<Mutex<HashMap<usize, GroupBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static REPORTS: LazyLock<Mutex<HashMap<usize, ReportBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));

#[derive(Debug)]
struct SnippetBuilder {
    source: String,
    line_start: Option<usize>,
    path: Option<String>,
    fold: Option<bool>,
    annotations: Vec<AnnotationBuilder>,
    // patches: Vec<PatchBuilder>, // temporarily disabled
}

#[derive(Debug)]
struct PatchSnippetBuilder {
    source: String,
    line_start: Option<usize>,
    path: Option<String>,
    fold: Option<bool>,
    patches: Vec<PatchBuilder>,
}

#[derive(Debug, Clone)]
struct AnnotationBuilder {
    kind: AnnotationKind,
    start: usize,
    end: usize,
    label: Option<String>,
}

#[derive(Debug, Clone)]
struct PatchBuilder { start: usize, end: usize, replacement: String }

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub enum FfiLevelKind { ERROR = 0, WARNING = 1, INFO = 2, NOTE = 3, HELP = 4 }

fn ffi_level(kind: FfiLevelKind) -> Level<'static> {
    match kind {
        FfiLevelKind::ERROR => Level::ERROR,
        FfiLevelKind::WARNING => Level::WARNING,
        FfiLevelKind::INFO => Level::INFO,
        FfiLevelKind::NOTE => Level::NOTE,
        FfiLevelKind::HELP => Level::HELP,
    }
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub enum FfiDecorStyle { UNICODE = 0, ASCII = 1 }

fn ffi_decor(style: FfiDecorStyle) -> DecorStyle {
    match style { FfiDecorStyle::UNICODE => DecorStyle::Unicode, FfiDecorStyle::ASCII => DecorStyle::Ascii }
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub enum FfiTitleKind { PRIMARY = 0, SECONDARY = 1 }

#[derive(Debug, Clone)]
struct MessageElement { level: FfiLevelKind, text: String, no_name: bool }

#[derive(Debug, Clone)]
enum GroupElementKind {
    AnnotationSnippet(usize),
    PatchSnippet(usize),
    Message(MessageElement),
    Padding,
}

#[derive(Debug)]
struct GroupBuilder {
    level: FfiLevelKind,
    title_kind: FfiTitleKind,
    title_text: Option<String>,
    id: Option<String>,
    id_url: Option<String>,
    elements: Vec<GroupElementKind>,
}

#[derive(Debug)]
struct ReportBuilder {
    groups: Vec<usize>,
    decor_style: DecorStyle,
    anonymized_line_numbers: bool,
    short_message: bool,
}

// New extended API uses Group / Report builders

fn next_handle() -> usize { NEXT_HANDLE_ID.fetch_add(1, Ordering::Relaxed) }

unsafe fn cstr_to_string(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() { None } else { CStr::from_ptr(ptr).to_str().ok().map(|s| s.to_string()) }
}

// -------------------------------------------------------------------------------------------------
// Snippet API (existing + extended)
// -------------------------------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn snippet_new(source: *const c_char, line_start: c_int) -> *mut std::os::raw::c_void {
    unsafe {
        let source_str = match cstr_to_string(source) { Some(s) => s, None => return ptr::null_mut() };
        let handle = next_handle();
        let snippet = SnippetBuilder {
            source: source_str,
            line_start: if line_start > 0 { Some(line_start as usize) } else { None },
            path: None,
            fold: None,
            annotations: Vec::new(),
        };
        SNIPPETS.lock().unwrap().insert(handle, snippet);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn patch_snippet_new(source: *const c_char, line_start: c_int) -> *mut std::os::raw::c_void {
    unsafe {
        let source_str = match cstr_to_string(source) { Some(s) => s, None => return ptr::null_mut() };
        let handle = next_handle();
        let snippet = PatchSnippetBuilder {
            source: source_str,
            line_start: if line_start > 0 { Some(line_start as usize) } else { None },
            path: None,
            fold: None,
            patches: Vec::new(),
        };
        PATCH_SNIPPETS.lock().unwrap().insert(handle, snippet);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn snippet_set_path(handle: *mut std::os::raw::c_void, path: *const c_char) {
    let id = handle as usize;
    let path_str = unsafe { cstr_to_string(path) };
    
    if let Ok(mut snippets) = SNIPPETS.lock() {
        if let Some(snippet) = snippets.get_mut(&id) {
            snippet.path = path_str;
        }
    }
}

#[no_mangle]
pub extern "C" fn snippet_set_fold(handle: *mut std::os::raw::c_void, fold: c_int) {
    let id = handle as usize;
    if let Ok(mut snippets) = SNIPPETS.lock() {
        if let Some(snippet) = snippets.get_mut(&id) {
            snippet.fold = Some(fold != 0);
        }
    }
}

#[no_mangle]
pub extern "C" fn patch_snippet_set_path(handle: *mut std::os::raw::c_void, path: *const c_char) {
    let id = handle as usize;
    let path_str = unsafe { cstr_to_string(path) };
    if let Ok(mut snippets) = PATCH_SNIPPETS.lock() { if let Some(snippet) = snippets.get_mut(&id) { snippet.path = path_str; } }
}

#[no_mangle]
pub extern "C" fn patch_snippet_set_fold(handle: *mut std::os::raw::c_void, fold: c_int) {
    let id = handle as usize;
    if let Ok(mut snippets) = PATCH_SNIPPETS.lock() { if let Some(snippet) = snippets.get_mut(&id) { snippet.fold = Some(fold != 0); } }
}

// Patch API ------------------------------------------------------------------------------------------------
#[no_mangle]
pub extern "C" fn patch_new(start: c_int, end: c_int, replacement: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let repl = match cstr_to_string(replacement) { Some(s) => s, None => String::new() };
        let handle = next_handle();
        let patch = PatchBuilder { start: start as usize, end: end as usize, replacement: repl };
        PATCHES.lock().unwrap().insert(handle, patch);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn patch_snippet_add_patch(patch_snippet: *mut std::os::raw::c_void, patch_handle: *mut std::os::raw::c_void) {
    let sid = patch_snippet as usize; let pid = patch_handle as usize;
    if let (Ok(mut snippets), Ok(patches)) = (PATCH_SNIPPETS.lock(), PATCHES.lock()) {
        if let (Some(snippet), Some(patch)) = (snippets.get_mut(&sid), patches.get(&pid)) { snippet.patches.push(patch.clone()); }
    }
}

// Annotation API -------------------------------------------------------------------------------------------
#[no_mangle]
pub extern "C" fn annotation_new_primary(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder { kind: AnnotationKind::Primary, start: start as usize, end: end as usize, label: cstr_to_string(label) };
        ANNOTATIONS.lock().unwrap().insert(handle, annotation);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn annotation_new_visible(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder { kind: AnnotationKind::Visible, start: start as usize, end: end as usize, label: cstr_to_string(label) };
        ANNOTATIONS.lock().unwrap().insert(handle, annotation);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn annotation_new_context(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder { kind: AnnotationKind::Context, start: start as usize, end: end as usize, label: cstr_to_string(label) };
        ANNOTATIONS.lock().unwrap().insert(handle, annotation);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn snippet_add_annotation(snippet_handle: *mut std::os::raw::c_void, annotation_handle: *mut std::os::raw::c_void) {
    let snippet_id = snippet_handle as usize;
    let annotation_id = annotation_handle as usize;
    if let (Ok(mut snippets), Ok(annotations)) = (SNIPPETS.lock(), ANNOTATIONS.lock()) {
        if let (Some(snippet), Some(annotation)) = (snippets.get_mut(&snippet_id), annotations.get(&annotation_id)) {
            snippet.annotations.push(annotation.clone());
        }
    }
}

fn build_annotation_snippet(sb: &SnippetBuilder) -> Snippet<'static, Annotation<'static>> {
    // Own all data so lifetime issues across FFI are avoided (we render immediately anyway)
    let mut snip: Snippet<'static, Annotation<'static>> = Snippet::source(sb.source.clone());
    if let Some(ls) = sb.line_start { snip = snip.line_start(ls); }
    if let Some(ref p) = sb.path { snip = snip.path(p.clone()); }
    if let Some(f) = sb.fold { snip = snip.fold(f); }
    for ann in &sb.annotations {
        let mut a: Annotation<'static> = ann.kind.span(ann.start..ann.end);
        if let Some(ref lbl) = ann.label { a = a.label(lbl.clone()); }
        snip = snip.annotation(a);
    }
    snip
}

fn build_patch_snippet(sb: &PatchSnippetBuilder) -> Snippet<'static, Patch<'static>> {
    let mut snip: Snippet<'static, Patch<'static>> = Snippet::source(sb.source.clone());
    if let Some(ls) = sb.line_start { snip = snip.line_start(ls); }
    if let Some(ref p) = sb.path { snip = snip.path(p.clone()); }
    if let Some(f) = sb.fold { snip = snip.fold(f); }
    for patch in &sb.patches {
        let p = Patch::new(patch.start..patch.end, patch.replacement.clone());
        snip = snip.patch(p);
    }
    snip
}

// Group / Report API --------------------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn group_new(level: FfiLevelKind, title_kind: FfiTitleKind, title_text: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let text = cstr_to_string(title_text);
        let handle = next_handle();
        let g = GroupBuilder { level, title_kind, title_text: text, id: None, id_url: None, elements: Vec::new() };
        GROUPS.lock().unwrap().insert(handle, g);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn group_set_id(group: *mut std::os::raw::c_void, id: *const c_char) {
    unsafe { if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&(group as usize)) { g.id = cstr_to_string(id); } } }
}

#[no_mangle]
pub extern "C" fn group_set_id_url(group: *mut std::os::raw::c_void, url: *const c_char) {
    unsafe { if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&(group as usize)) { g.id_url = cstr_to_string(url); } } }
}

#[no_mangle]
pub extern "C" fn group_add_snippet(group: *mut std::os::raw::c_void, snippet: *mut std::os::raw::c_void) {
    let gid = group as usize; let sid = snippet as usize;
    if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&gid) { g.elements.push(GroupElementKind::AnnotationSnippet(sid)); } }
}

#[no_mangle]
pub extern "C" fn group_add_patch_snippet(group: *mut std::os::raw::c_void, snippet: *mut std::os::raw::c_void) {
    let gid = group as usize; let sid = snippet as usize;
    if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&gid) { g.elements.push(GroupElementKind::PatchSnippet(sid)); } }
}

#[no_mangle]
pub extern "C" fn group_add_message(group: *mut std::os::raw::c_void, level: FfiLevelKind, text: *const c_char, no_name: c_int) {
    unsafe {
        let msg = MessageElement { level, text: cstr_to_string(text).unwrap_or_default(), no_name: no_name != 0 };
        if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&(group as usize)) { g.elements.push(GroupElementKind::Message(msg)); } }
    }
}

#[no_mangle]
pub extern "C" fn group_add_padding(group: *mut std::os::raw::c_void) {
    if let Ok(mut groups) = GROUPS.lock() { if let Some(g) = groups.get_mut(&(group as usize)) { g.elements.push(GroupElementKind::Padding); } }
}

fn build_group(gb: &GroupBuilder, ann_snips: &HashMap<usize, SnippetBuilder>, patch_snips: &HashMap<usize, PatchSnippetBuilder>) -> Group<'static> {
    let level = ffi_level(gb.level);
    if let Some(ref text) = gb.title_text {
        let mut title = match gb.title_kind { FfiTitleKind::PRIMARY => level.primary_title(text.clone()), FfiTitleKind::SECONDARY => level.secondary_title(text.clone()) };
        if let Some(ref id) = gb.id { title = title.id(id.clone()); }
        if let Some(ref url) = gb.id_url { title = title.id_url(url.clone()); }
        let mut group = Group::with_title(title);
        for elem in &gb.elements {
            group = match elem {
                GroupElementKind::AnnotationSnippet(sid) => {
                    if let Some(sb) = ann_snips.get(sid) { group.element(build_annotation_snippet(sb)) } else { group }
                }
                GroupElementKind::PatchSnippet(sid) => {
                    if let Some(sb) = patch_snips.get(sid) { group.element(build_patch_snippet(sb)) } else { group }
                }
                GroupElementKind::Message(me) => {
                    let mut mlvl = ffi_level(me.level);
                    if me.no_name { mlvl = mlvl.no_name(); }
                    group.element(mlvl.message(me.text.clone()))
                }
                GroupElementKind::Padding => group.element(annotate_snippets::Padding),
            };
        }
        group
    } else {
        // title-less group: use level as primary_level
        let mut group = Group::with_level(level);
        for elem in &gb.elements {
            group = match elem {
                GroupElementKind::AnnotationSnippet(sid) => {
                    if let Some(sb) = ann_snips.get(sid) { group.element(build_annotation_snippet(sb)) } else { group }
                }
                GroupElementKind::PatchSnippet(sid) => {
                    if let Some(sb) = patch_snips.get(sid) { group.element(build_patch_snippet(sb)) } else { group }
                }
                GroupElementKind::Message(me) => {
                    let mut mlvl = ffi_level(me.level);
                    if me.no_name { mlvl = mlvl.no_name(); }
                    group.element(mlvl.message(me.text.clone()))
                }
                GroupElementKind::Padding => group.element(annotate_snippets::Padding),
            };
        }
        group
    }
}

#[no_mangle]
pub extern "C" fn report_new() -> *mut std::os::raw::c_void {
    let handle = next_handle();
    let rb = ReportBuilder { groups: Vec::new(), decor_style: DecorStyle::Unicode, anonymized_line_numbers: false, short_message: false };
    REPORTS.lock().unwrap().insert(handle, rb);
    handle as *mut std::os::raw::c_void
}

#[no_mangle]
pub extern "C" fn report_add_group(report: *mut std::os::raw::c_void, group: *mut std::os::raw::c_void) {
    if let Ok(mut reports) = REPORTS.lock() { if let Some(r) = reports.get_mut(&(report as usize)) { r.groups.push(group as usize); } }
}

#[no_mangle]
pub extern "C" fn report_set_decor_style(report: *mut std::os::raw::c_void, style: FfiDecorStyle) {
    if let Ok(mut reports) = REPORTS.lock() { if let Some(r) = reports.get_mut(&(report as usize)) { r.decor_style = ffi_decor(style); } }
}

#[no_mangle]
pub extern "C" fn report_set_anonymized_line_numbers(report: *mut std::os::raw::c_void, anonymized: c_int) {
    if let Ok(mut reports) = REPORTS.lock() { if let Some(r) = reports.get_mut(&(report as usize)) { r.anonymized_line_numbers = anonymized != 0; } }
}

#[no_mangle]
pub extern "C" fn report_set_short_message(report: *mut std::os::raw::c_void, short_msg: c_int) {
    if let Ok(mut reports) = REPORTS.lock() { if let Some(r) = reports.get_mut(&(report as usize)) { r.short_message = short_msg != 0; } }
}

#[no_mangle]
pub extern "C" fn report_render(report: *mut std::os::raw::c_void) -> *mut c_char {
    let reports = match REPORTS.lock() { Ok(r) => r, Err(_) => return ptr::null_mut() };
    let rb = match reports.get(&(report as usize)) { Some(r) => r, None => return ptr::null_mut() };
    let ann_snips = match SNIPPETS.lock() { Ok(s) => s, Err(_) => return ptr::null_mut() };
    let patch_snips = match PATCH_SNIPPETS.lock() { Ok(s) => s, Err(_) => return ptr::null_mut() };
    let groups_map = match GROUPS.lock() { Ok(g) => g, Err(_) => return ptr::null_mut() };
    let mut built: Vec<Group> = Vec::with_capacity(rb.groups.len());
    for gid in &rb.groups { if let Some(gb) = groups_map.get(gid) { built.push(build_group(gb, &ann_snips, &patch_snips)); } }
    let mut renderer = Renderer::styled().decor_style(rb.decor_style).anonymized_line_numbers(rb.anonymized_line_numbers);
    if rb.short_message { renderer = renderer.short_message(true); }
    let output = renderer.render(built.as_slice());
    match CString::new(output) { Ok(c) => c.into_raw(), Err(_) => ptr::null_mut() }
}

// -------------------------------------------------------------------------------------------------
// Backwards compatible helpers (render_error, render_warning)
// -------------------------------------------------------------------------------------------------

#[no_mangle]
pub extern "C" fn render_error(title: *const c_char, snippet_handle: *mut std::os::raw::c_void) -> *mut c_char {
    unsafe {
        let title_str = match cstr_to_string(title) { Some(s) => s, None => return ptr::null_mut() };
        let sid = snippet_handle as usize;
        let snippets = match SNIPPETS.lock() { Ok(s) => s, Err(_) => return ptr::null_mut() };
        let sb = match snippets.get(&sid) { Some(s) => s, None => return ptr::null_mut() };
    let group: Group = Level::ERROR.primary_title(title_str).element(build_annotation_snippet(sb));
        let report = &[group];
        let renderer = Renderer::styled().decor_style(DecorStyle::Unicode);
        match CString::new(renderer.render(report)) { Ok(c) => c.into_raw(), Err(_) => ptr::null_mut() }
    }
}

#[no_mangle]
pub extern "C" fn render_warning(title: *const c_char, snippet_handle: *mut std::os::raw::c_void) -> *mut c_char {
    unsafe {
        let title_str = match cstr_to_string(title) { Some(s) => s, None => return ptr::null_mut() };
        let sid = snippet_handle as usize;
        let snippets = match SNIPPETS.lock() { Ok(s) => s, Err(_) => return ptr::null_mut() };
        let sb = match snippets.get(&sid) { Some(s) => s, None => return ptr::null_mut() };
    let group: Group = Level::WARNING.primary_title(title_str).element(build_annotation_snippet(sb));
        let report = &[group];
        let renderer = Renderer::styled().decor_style(DecorStyle::Unicode);
        match CString::new(renderer.render(report)) { Ok(c) => c.into_raw(), Err(_) => ptr::null_mut() }
    }
}

#[no_mangle]
pub extern "C" fn free_string(s: *mut c_char) {
    unsafe { 
        if !s.is_null() {
            let _ = CString::from_raw(s); 
        }
    }
}

#[no_mangle]
pub extern "C" fn snippet_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut snippets) = SNIPPETS.lock() {
        snippets.remove(&id);
    }
}

#[no_mangle]
pub extern "C" fn patch_snippet_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut snippets) = PATCH_SNIPPETS.lock() { snippets.remove(&id); }
}

#[no_mangle]
pub extern "C" fn annotation_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut annotations) = ANNOTATIONS.lock() {
        annotations.remove(&id);
    }
}

#[no_mangle]
pub extern "C" fn patch_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut patches) = PATCHES.lock() { patches.remove(&id); }
}

#[no_mangle]
pub extern "C" fn group_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut groups) = GROUPS.lock() { groups.remove(&id); }
}

#[no_mangle]
pub extern "C" fn report_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut reports) = REPORTS.lock() { reports.remove(&id); }
}

// patch_free disabled

// level_free / report_free no longer needed (removed API)
