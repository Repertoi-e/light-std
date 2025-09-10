use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;
use std::collections::HashMap;
use std::sync::{Mutex, LazyLock};

use annotate_snippets::{Level, Renderer, Snippet, AnnotationKind};
use annotate_snippets::renderer::DecorStyle;

// Global storage for handles
static mut NEXT_HANDLE_ID: usize = 1;
static SNIPPETS: LazyLock<Mutex<HashMap<usize, SnippetBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));
static ANNOTATIONS: LazyLock<Mutex<HashMap<usize, AnnotationBuilder>>> = LazyLock::new(|| Mutex::new(HashMap::new()));

#[derive(Debug)]
struct SnippetBuilder {
    source: String,
    line_start: Option<usize>,
    path: Option<String>,
    annotations: Vec<AnnotationBuilder>,
}

#[derive(Debug, Clone)]
struct AnnotationBuilder {
    kind: AnnotationKind,
    start: usize,
    end: usize,
    label: Option<String>,
}

unsafe fn next_handle() -> usize {
    let id = NEXT_HANDLE_ID;
    NEXT_HANDLE_ID += 1;
    id
}

unsafe fn cstr_to_string(ptr: *const c_char) -> Option<String> {
    if ptr.is_null() {
        None
    } else {
        CStr::from_ptr(ptr).to_str().ok().map(|s| s.to_string())
    }
}

#[no_mangle]
pub extern "C" fn snippet_new(source: *const c_char, line_start: c_int) -> *mut std::os::raw::c_void {
    unsafe {
        let source_str = match cstr_to_string(source) {
            Some(s) => s,
            None => return ptr::null_mut(),
        };
        
        let handle = next_handle();
        let snippet = SnippetBuilder {
            source: source_str,
            line_start: if line_start > 0 { Some(line_start as usize) } else { None },
            path: None,
            annotations: Vec::new(),
        };
        
        SNIPPETS.lock().unwrap().insert(handle, snippet);
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
pub extern "C" fn annotation_new_primary(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder {
            kind: AnnotationKind::Primary,
            start: start as usize,
            end: end as usize,
            label: cstr_to_string(label),
        };
        
        ANNOTATIONS.lock().unwrap().insert(handle, annotation);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn annotation_new_visible(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder {
            kind: AnnotationKind::Visible,
            start: start as usize,
            end: end as usize,
            label: cstr_to_string(label),
        };
        
        ANNOTATIONS.lock().unwrap().insert(handle, annotation);
        handle as *mut std::os::raw::c_void
    }
}

#[no_mangle]
pub extern "C" fn annotation_new_context(start: c_int, end: c_int, label: *const c_char) -> *mut std::os::raw::c_void {
    unsafe {
        let handle = next_handle();
        let annotation = AnnotationBuilder {
            kind: AnnotationKind::Context,
            start: start as usize,
            end: end as usize,
            label: cstr_to_string(label),
        };
        
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

#[no_mangle]
pub extern "C" fn render_error(title: *const c_char, snippet_handle: *mut std::os::raw::c_void) -> *mut c_char {
    unsafe {
        let title_str = match cstr_to_string(title) {
            Some(s) => s,
            None => return ptr::null_mut(),
        };
        
        let snippet_id = snippet_handle as usize;
        
        let snippets = match SNIPPETS.lock() {
            Ok(s) => s,
            Err(_) => return ptr::null_mut(),
        };
        
        let snippet_builder = match snippets.get(&snippet_id) {
            Some(s) => s,
            None => return ptr::null_mut(),
        };
        
        // Build the snippet
        let mut snippet = Snippet::source(&snippet_builder.source);
        
        if let Some(line_start) = snippet_builder.line_start {
            snippet = snippet.line_start(line_start);
        }
        
        if let Some(ref path) = snippet_builder.path {
            snippet = snippet.path(path);
        }
        
        // Add annotations
        for ann in &snippet_builder.annotations {
            let annotation = ann.kind.span(ann.start..ann.end);
            let annotation = if let Some(ref label) = ann.label {
                annotation.label(label)
            } else {
                annotation
            };
            snippet = snippet.annotation(annotation);
        }
        
        // Create the report
        let report = &[Level::ERROR
            .primary_title(&title_str)
            .element(snippet)];
        
        // Render
        let renderer = Renderer::styled().decor_style(DecorStyle::Unicode);
        let output = renderer.render(report);
        
        // Convert to C string
        match CString::new(output.to_string()) {
            Ok(c_string) => c_string.into_raw(),
            Err(_) => ptr::null_mut(),
        }
    }
}

#[no_mangle]
pub extern "C" fn render_warning(title: *const c_char, snippet_handle: *mut std::os::raw::c_void) -> *mut c_char {
    unsafe {
        let title_str = match cstr_to_string(title) {
            Some(s) => s,
            None => return ptr::null_mut(),
        };
        
        let snippet_id = snippet_handle as usize;
        
        let snippets = match SNIPPETS.lock() {
            Ok(s) => s,
            Err(_) => return ptr::null_mut(),
        };
        
        let snippet_builder = match snippets.get(&snippet_id) {
            Some(s) => s,
            None => return ptr::null_mut(),
        };
        
        // Build the snippet
        let mut snippet = Snippet::source(&snippet_builder.source);
        
        if let Some(line_start) = snippet_builder.line_start {
            snippet = snippet.line_start(line_start);
        }
        
        if let Some(ref path) = snippet_builder.path {
            snippet = snippet.path(path);
        }
        
        // Add annotations
        for ann in &snippet_builder.annotations {
            let annotation = ann.kind.span(ann.start..ann.end);
            let annotation = if let Some(ref label) = ann.label {
                annotation.label(label)
            } else {
                annotation
            };
            snippet = snippet.annotation(annotation);
        }
        
        // Create the report
        let report = &[Level::WARNING
            .primary_title(&title_str)
            .element(snippet)];
        
        // Render
        let renderer = Renderer::styled().decor_style(DecorStyle::Unicode);
        let output = renderer.render(report);
        
        // Convert to C string
        match CString::new(output.to_string()) {
            Ok(c_string) => c_string.into_raw(),
            Err(_) => ptr::null_mut(),
        }
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
pub extern "C" fn annotation_free(handle: *mut std::os::raw::c_void) {
    let id = handle as usize;
    if let Ok(mut annotations) = ANNOTATIONS.lock() {
        annotations.remove(&id);
    }
}
