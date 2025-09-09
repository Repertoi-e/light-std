#include "lstd/lstd.h"

#include <stdarg.h>

#define VERSION "0.0.1"

template <typename... Args>
void error(string message, Args no_copy... arguments)
{
    print("{!RED}error:{!} {}\n", tprint(message, arguments...));
}

template <typename... Args>
void warn(string message, Args no_copy... arguments)
{
    print("{!YELLOW}warning:{!} {}\n", tprint(message, arguments...));
}

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
                    print("... {} ({} bytes).\n", file_path, contents.Count);
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

                    string normalized = make_string_normalized_nfc(contents);
                    if (!normalized.Data) {
                        error("Failed to normalize UTF-8 string");
                        return;
                    }
                    print("Normalized size: {} bytes\n", normalized.Count);
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
