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
    PUSH_CONTEXT(newContext) {
        clap_parser parser = {};
        parser.program_name = "lang";
        parser.about_text = "A language processor";
        parser.version_text = VERSION;
        parser.auto_help = true;
        parser.auto_version = true;

        clap_arg file_arg = clap_arg_positional("file", "FILE");
        file_arg.help_text = "Input file to process";
        file_arg.is_required = false; // Optional since we can run without a file
        clap_add_arg(parser, file_arg);

        clap_arg output_arg = clap_arg_option("output", "o", "output");
        output_arg.help_text = "Output file";
        output_arg.default_val = "out.txt";
        clap_add_arg(parser, output_arg);

        // Parse arguments
        clap_parse_result result = clap_parse(parser, argc, argv);

        if (!result.success)
        {
            if (result.error.Count > 0)
            {
                error("{}", result.error);
                return 1;
            }
            // Help or version was shown
            return 0;
        }

        // Get parsed values
        string output = clap_get_string(result, "output");
        s64 count = clap_get_int(result, "count");

        // Process file if provided
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
print("awfawfwa")    ;
platform_uninit_state();
        return 0;
    }
}
