#include "lstd/lstd.h"

#include "lang.h"
#include "snipffi.h"
#include "src/token.cpp"

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
        string page1 = R"(
    The last compiler aims to define the common IR for all programming languages,
    supporting dynamic IR for interpreted languages, and static IR for compiled languages.
    Ihe idea came from the observation that the current programming culture forces the choice of
    one tool for every job in a project, and languages are designed in a techno-feudalistic matter
    to vendor lock-in users into a specific ecosystem. For small projects this is not a problem,
    since technically you can assemble a cup-board with a screwdriver, or a knife, or a key,
    but you cannot build it from scratch with only any one of those tools.

    Languages are our tools to instruct the computer, and there's no reason different tools 
    can't be used together seemlessly. The last compiler allows the execution and compilation 
    of multiple languages in the same program, and the seem-less interop between them. 
    While at the syntax level languages are very different, at the IR level they are all the same. 
    They all tell the computer what to do in simplified assembly-like languages called 
    Intermediate Representations (IRs). Dynamic languages are relaxed in how they express the IR, 
    while static languages are strict - since typing is what makes static languages very fast,
    as they get closer to what the CPU does with bits.

    The last compiler aims to try building the infrastructure for a multi-language one-IR compiler,
    which would enable turning the current feudalistic language ecosystems into an open 
    market for tools, where new languages stop competing for mind-share, and instead
    compete for technical merit, based on actual problems they solve. There's no need for all 
    languages to look the same, to have the same features, to be good at everything, since that's
    a quick path to be mediocre at everything.
)";

        string page2 = R"(
    You will not see garbage collection in the last compiler, although completely possible
    to implement as a library. The last compiler encourages to abolish the tradition of
    using malloc/free, new/delete, and all the other fragmented- heap allocation functions,
    and instead use region-based memory management, with 1 practically infinite virtual
    allocation per arena, and a hierarchy of arenas for different lifetimes.
    
    Free-all instead of free-one reduces fragmentation, improves locality, simplifies reasoning,
    reduces the chance for leaks. Coupled with a powerful debugging memory layer, which
    can track all allocations, report double-free attempts, out-of-bounds accesses,
    and memory corruption by padding allocations with canaries. Instead of going the Rust-style
    of proving every alloc-free is safe at compile-time, at a great increase of complexity,
    friction and cognitive load, the last compiler goes a simpler way of just removing
    most of the need for them in the first place. malloc/free came from a place of necessity,
    when memory was scarce, and programs had to be small. Nowadays memory is abundant,
    and it makes sense to delay groups of individual frees to a single free-all.

    It is easy to argue that for most non-trivial programs that do non-trivial amounts of work,
    if reasonably written, can get away with not freeing 99% of the allocations done,
    and it will not even come close to the memory usage of a "modern" web browser tab.
)";

        string page3 = R"(
    C-- is a refinement of C, which is the only language that the author has felt is 
    aesthetically and topologically human. C-- aims to remove from C the rough edges which 
    historically accumulated to make C complex to learn, hard to parse, and hard to compile 
    on its own. C-- is not a better C, it is a simpler C. 
    
    C is organic, irregular, and alive. It grew out of the constraints of hardware, the 
    limitations of compilers, and the needs of programmers, who designed declarations to
    look like the things you'd scribble on paper. C is messy in the same way people are messy.
    For a small example of the programmer-facing interaction of C, consider function headers: 
    
        C_Function* compiler_emit_function(C_Module* module, C_FunctionType* type, C_Linkage linkage); 
        
    which can be copy-pasted in an editor into the calling site, and filled out as a template, verbatim: 
    
        C_Function* func = compiler_emit_function(module, type, linkage); 
        
    While in "modernist", pragmatic, system-programming languages, you have to introduce punctuation 
    and/or swap the return value back to the beginning of the line. 
    
    C-- removes parsing ambiguities, removes the preprocessor in place of a hygienic macro
    and metaprogramming system, removes the header/source file split in favor of a single file
    module system with ability to name-space at import, removes the need for forward declarations
    and manual clerical work by supporting non-linear compilation. C-- defines its own build process,
    so it can skip 100% of the boilerplate build systems which fail all the time, make programming painful.
    So we can go back to the joy of old computers when you could open a CLI/file and just hack code to see
    results immediately. 
    
    C-- supports interpreted execution, including during compile-time. It's simultaneously a scripting
    language, and a systems programming. In the beginning it gives results, and in the end it gives
    full control. C-- is never implicit about what it does, you can't have constructors, destructors,
    or any other "magic" happening that is not explicitly spelled out in the code as a potential side-effect.
)";


        string about_text = R"(        The Last Compiler project. The first front-end for C--.

The Last Compiler aims to define a common Intermediate Representation (IR) for all programming languages.
C-- is a simpler C.
    
Exit codes:
  0   Success, even with compiler errors or warnings
  1   File/input read error
  2   Invalid command line usage / missing input
  4   Invalid UTF-8 or UTF-8 normalization failure
)";
        clap_parser parser = {
            .program_name = "lang",
            .about_text = about_text,
            .version_text = VERSION,
            .auto_help = true,
            .auto_version = true
        };
        add(parser.arguments, clap_arg_positional("file", .value_name = "FILE", .help_text = "Input file to process", .is_required = false));
        add(parser.arguments, clap_arg_flag("tokenSink", .short_name = "t", .long_name="token-sink", .help_text = "Print all tokens to stdout"));

        add(parser.arguments, clap_arg_flag("page1", .long_name="last-compiler", .short_name="l", .help_text = "Print page 1 of the manifesto and exit"));
        add(parser.arguments, clap_arg_flag("page2", .long_name="memory-management", .short_name="m", .help_text = "Print page 2 of the manifesto and exit"));
        add(parser.arguments, clap_arg_flag("page3", .long_name="c--", .short_name="c", .help_text = "Print page 3 of the manifesto and exit"));

        clap_parse_result result = clap_parse(parser, argc, argv);
        if (!result.success)
        {
            if (result.error.Count > 0)
            {
                ERR(to_c_string_temp(result.error));
                return 2;
            }
            return 0;
        }

        if (clap_has_arg(result, "page1")) {
            print("{}\n", page1);
            return 0;
        }
        if (clap_has_arg(result, "page2")) {
            print("{}\n", page2);
            return 0;
        }
        if (clap_has_arg(result, "page3")) {
            print("{}\n", page3);
            return 0;
        }

        s32 status = 0;

        string output = clap_get_string(result, "output");
        if (clap_has_arg(result, "file"))
        {
            string file_path = clap_get_string(result, "file");
            
            // Dummy tokenizer so we can report file_name in case processing the file failed
            tokenizer tz = {null, null, to_c_string_temp(file_path), 1, null, null};
            diagnostics_set_active_tokenizer(&tz);

            optional<string> fileContent = os_read_entire_file(file_path);
            fileContent.visit(match{
                [&](string fileContents)
                {
                    s64 invalid = utf8_find_invalid(fileContents.Data, fileContents.Count);
                    if (invalid >= 0)
                    {
                        ERR_ANNOTATED("Invalid UTF-8 sequence", fileContents.Data + invalid, fileContents.Data + invalid + 1, "Invalid UTF-8 sequence");

                        s64 start = max(0, invalid - 10);
                        s64 end = min(fileContents.Count, invalid + 10);
                        print("{}\n", string(fileContents.Data + start, end - start));
                        print("{:>{}}\n", "^", invalid - start + 1);

                        status = 4;
                        return;
                    }

                    const char* sourceCode = tokenizer_prepare_source(fileContents);
                    if (!sourceCode) {
                        status = 4;
                        return;
                    }

                    if (clap_has_arg(result, "tokenSink")) {
                        token_array tokens = tokenizer_tokenize(sourceCode, to_c_string_temp(file_path));
                        print("{} tokens\n", tokens.Count);
                        for (s64 i = 0; i < tokens.Count; i++) {
                            print("{}: {} {}\n", tokens[i].Location, token_type_to_string(tokens[i].Type), token_to_string(tokens[i]));
                        }
                    }
                },
                [&](auto)
                {
                    ERR(mprint("Could not read file '{}'", file_path));
                    status = 1;
                }});
        }
        else
        {
            ERR("No input\n");
            return 2;
        }
        return status;
    }
}
