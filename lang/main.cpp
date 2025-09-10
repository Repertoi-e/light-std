#include "lstd/lstd.h"

#include "lang.h"
#include "snipffi.h"
#include "src/token/token.cpp"

#include <stdarg.h>

#define VERSION "0.0.1"

arena_allocator_data ARENA_GLOBAL_DATA;
#define ARENA_GLOBAL (allocator{arena_allocator, &ARENA_GLOBAL_DATA})

int main(int argc, char **argv)
{
    platform_state_init();

    auto newContext = Context;
    newContext.Alloc = ARENA_GLOBAL;
    PUSH_CONTEXT(newContext)
    {
        clap_parser parser = {
            .program_name = "lang",
            .about_text = "A language processor",
            .version_text = VERSION,
            .auto_help = true,
            .auto_version = true
        };
        add(parser.arguments, clap_arg_positional("file", .value_name = "FILE", .help_text = "Input file to process", .is_required = false));
        add(parser.arguments, clap_arg_option("output", .short_name = "o", .long_name = "output", .help_text = "Output file", .default_val = "out.txt"));
        add(parser.arguments, clap_arg_flag("verbose", .short_name = "V", .long_name = "verbose", .help_text = "Enable verbose output"));

        clap_parse_result result = clap_parse(parser, argc, argv);
        if (!result.success)
        {
            if (result.error.Count > 0)
            {
                error("{}", result.error);
                return 1;
            }
            return 0;
        }

        string output = clap_get_string(result, "output");
        if (clap_has_arg(result, "file"))
        {
            string file_path = clap_get_string(result, "file");
            optional<string> fileContent = os_read_entire_file(file_path);
            fileContent.visit(match{
                [&](string contents)
                {
                    s64 invalid = utf8_find_invalid(contents.Data, contents.Count);
                    if (invalid >= 0)
                    {
                        error("Invalid UTF-8 sequence at byte offset {}:", invalid);

                        s64 start = max(0, invalid - 10);
                        s64 end = min(contents.Count, invalid + 10);
                        print("{}\n", string(contents.Data + start, end - start));
                        print("{:>{}}\n", "^", invalid - start + 1);

                        return;
                    }

                    string normalized = contents; // make_string_normalized_nfc(contents);
                    if (!normalized.Data) {
                        error("Failed to normalize UTF-8 string");
                        return;
                    }

                    token_array tokens = tokenizer_tokenize(normalized);
                    print("Tokenized into {} tokens\n", tokens.Count);
                    for (s64 i = 0; i < tokens.Count; i++) {
                        print("  Token: {} Location: {}\n", token_to_string(tokens[i].Type), tokens[i].Location);
                    }

                    #if 0
                    print("{!CYAN;B}=== Diagnostic Examples with annotate-snippets ==={!}\n\n");

                    // Example 1: Simple error with primary annotation
                    {
                        print("{!YELLOW}Example 1: Syntax error{!}\n");
                        
                        SnippetHandle snippet = snippet_new(normalized.Data, 1);
                        snippet_set_path(snippet, file_path.Data);
                        
                        AnnotationHandle primary = annotation_new_primary(20, 30, "expected `;` here");
                        snippet_add_annotation(snippet, primary);
                        
                        char* output = render_error("expected `;` after statement", snippet);
                        if (output) {
                            print("{}\n", output);
                            free_string(output);
                        }
                        
                        annotation_free(primary);
                        snippet_free(snippet);
                    }

                    // Example 2: Warning with context
                    {
                        print("{!YELLOW}Example 2: Unused variable warning{!}\n");
                        
                        SnippetHandle snippet = snippet_new(normalized.Data, 1);
                        snippet_set_path(snippet, file_path.Data);
                        
                        AnnotationHandle primary = annotation_new_primary(50, 70, "this variable is never used");
                        AnnotationHandle context = annotation_new_context(45, 75, "consider using `_` if intentionally unused");
                        
                        snippet_add_annotation(snippet, primary);
                        snippet_add_annotation(snippet, context);
                        
                        char* output = render_warning("unused variable", snippet);
                        if (output) {
                            print("{}\n", output);
                            free_string(output);
                        }
                        
                        annotation_free(primary);
                        annotation_free(context);
                        snippet_free(snippet);
                    }

                    // Example 3: Type mismatch error
                    {
                        print("{!YELLOW}Example 3: Type mismatch{!}\n");
                        
                        const char* type_error_source = R"(fn calculate(x: i32, y: i32) -> i32 {
    let result = x + y;
    result as f64  // Error: expected i32, found f64
})";
                        
                        SnippetHandle snippet = snippet_new(type_error_source, 1);
                        snippet_set_path(snippet, "example.rs");
                        
                        AnnotationHandle primary = annotation_new_primary(73, 83, "expected `i32`, found `f64`");
                        AnnotationHandle context = annotation_new_context(31, 34, "expected `i32` because of return type");
                        
                        snippet_add_annotation(snippet, primary);
                        snippet_add_annotation(snippet, context);
                        
                        char* output = render_error("mismatched types", snippet);
                        if (output) {
                            print("{}\n", output);
                            free_string(output);
                        }
                        
                        annotation_free(primary);
                        annotation_free(context);
                        snippet_free(snippet);
                    }

                    // Example 4: Multiple annotations showing different spans
                    {
                        print("{!YELLOW}Example 4: Multiple related errors{!}\n");
                        
                        const char* multi_error_source = R"(let x = 42;
let y = "hello";
let z = x + y;  // Cannot add integer and string
println!("{}", z);)";
                        
                        SnippetHandle snippet = snippet_new(multi_error_source, 1);
                        snippet_set_path(snippet, "arithmetic.rs");
                        
                        AnnotationHandle primary = annotation_new_primary(37, 42, "cannot add `i32` and `&str`");
                        AnnotationHandle context1 = annotation_new_context(8, 10, "`i32`");
                        AnnotationHandle context2 = annotation_new_context(21, 28, "`&str`");
                        
                        snippet_add_annotation(snippet, primary);
                        snippet_add_annotation(snippet, context1);
                        snippet_add_annotation(snippet, context2);
                        
                        char* output = render_error("cannot add `i32` and `&str`", snippet);
                        if (output) {
                            print("{}\n", output);
                            free_string(output);
                        }
                        
                        annotation_free(primary);
                        annotation_free(context1);
                        annotation_free(context2);
                        snippet_free(snippet);
                    }

                    // Example 5: Warning with suggestion
                    {
                        print("{!YELLOW}Example 5: Performance suggestion{!}\n");
                        
                        const char* perf_source = R"(fn process_data(data: Vec<String>) {
    for item in data.iter() {
        println!("{}", item.clone());  // Unnecessary clone
    }
})";
                        
                        SnippetHandle snippet = snippet_new(perf_source, 1);
                        snippet_set_path(snippet, "performance.rs");
                        
                        AnnotationHandle primary = annotation_new_primary(78, 90, "unnecessary `.clone()` call");
                        AnnotationHandle visible = annotation_new_visible(60, 78, "consider removing this");
                        
                        snippet_add_annotation(snippet, primary);
                        snippet_add_annotation(snippet, visible);
                        
                        char* output = render_warning("unnecessary clone", snippet);
                        if (output) {
                            print("{}\n", output);
                            free_string(output);
                        }
                        
                        annotation_free(primary);
                        annotation_free(visible);
                        snippet_free(snippet);
                    }
                    #endif
                },
                [file_path](auto)
                {
                    error("Could not read file '{}'", file_path);
                }});
        }
        else
        {
            error("No input\n");
        }
        return 0;
    }
}
