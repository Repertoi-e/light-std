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

    TemporaryAllocatorData = make_arena_with_os_allocate_block(1_GiB);
    ARENA_GLOBAL_DATA = make_arena_with_os_allocate_block(1_GiB);

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
                [](string contents)
                {
                    print("Processing file content ({} bytes):\n", contents.Count);
                    write(&cout, sprint("File content:\n{}\n", contents));
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
