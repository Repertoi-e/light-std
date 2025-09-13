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
        add(parser.arguments, clap_arg_flag("tokenSink", .short_name = "t", .long_name="token-sink", .help_text = "Print all tokens to stdout"));

        clap_parse_result result = clap_parse(parser, argc, argv);
        if (!result.success)
        {
            if (result.error.Count > 0)
            {
                ERR("", to_c_string_temp(result.error));
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
                [&](string fileContents)
                {
                    s64 invalid = utf8_find_invalid(fileContents.Data, fileContents.Count);
                    if (invalid >= 0)
                    {
                        // Provide pointer span: start = fileContents.Data + invalid, end = start + 1
                        ERR_ANNOTATED(fileContents.Data, "invalid UTF-8 sequence", fileContents.Data + invalid, fileContents.Data + invalid + 1, "invalid UTF-8 sequence");

                        s64 start = max(0, invalid - 10);
                        s64 end = min(fileContents.Count, invalid + 10);
                        print("{}\n", string(fileContents.Data + start, end - start));
                        print("{:>{}}\n", "^", invalid - start + 1);

                        return;
                    }

                    string_builder sb;
                    if (!utf8_normalize_nfd_to_string_builder(fileContents.Data, fileContents.Count, sb)) {
                        ERR("", "Failed to normalize UTF-8 string");
                        return;
                    }
                    
                    // Print 16 bytes of 0 after normalization, to make sure it's null terminated even for vectorized code.
                    For(range(16)) {
                        add(sb, '\0');
                    }

                    free(fileContents);
                    fileContents = builder_to_string_and_free_builder(sb);

                    if (clap_has_arg(result, "tokenSink")) {
                        token_array tokens = tokenizer_tokenize(fileContents, {.FileName = to_c_string_temp(file_path)});
                        print("Tokenized into {} tokens\n", tokens.Count);
                        for (s64 i = 0; i < tokens.Count; i++) {
                            print("{}: {} {}\n", tokens[i].Location, token_type_to_string(tokens[i].Type), token_to_string(tokens[i]));
                        }
                    }
                },
                [file_path](auto)
                {
                    ERR("", mprint("Could not read file '{}'", file_path));
                }});
        }
        else
        {
            ERR("", "No input\n");
        }
        return 0;
    }
}
