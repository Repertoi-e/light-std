#pragma once

#include "array.h"
#include "string.h"
#include "variant.h"
#include "hash_table.h"
#include "parse.h"

LSTD_BEGIN_NAMESPACE

/// A Clap-like command line argument parser inspired by Rust's clap crate
/// 
/// Example usage:
/* ```cpp
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
```*/

enum clap_arg_type {
    CLAP_ARG_STRING,
    CLAP_ARG_INT,
    CLAP_ARG_FLOAT,
    CLAP_ARG_BOOL
};

enum clap_action {
    CLAP_ACTION_STORE,      // Store the value (default)
    CLAP_ACTION_SET_TRUE,   // Set to true when present (for flags)
    CLAP_ACTION_SET_FALSE,  // Set to false when present
    CLAP_ACTION_COUNT       // Count the number of times the flag appears
};

struct clap_argument_value {
    variant<string, s64, f64, bool> value;
    clap_arg_type type;
};

struct clap_arg {
    string name;
    string value_name;
    string help_text;
    string short_name;
    string long_name;
    string default_val;
    clap_arg_type arg_type;
    clap_action action;
    bool is_required;
    bool is_positional;
};

struct clap_parse_result {
    hash_table<string, clap_argument_value> values;
    bool success;
    string error;
};

struct clap_parser {
    string program_name;
    string about_text;
    string version_text;

    array<clap_arg> arguments; 

    bool auto_help; // Automatically add -h/--help
    bool auto_version; // Automatically add -v/--version
};

// Create different types of arguments
clap_arg clap_arg_positional(string name, string value_name = "");

// ---- Options-struct + macro overloads (POD-friendly DSL) ----
struct clap_option_desc {
    string value_name;    // defaults to name if empty
    string help_text;
    string short_name;    // may be empty
    string long_name;     // may be empty
    string default_val;   // optional default

    clap_arg_type arg_type = CLAP_ARG_STRING;
    clap_action action = CLAP_ACTION_STORE;
    bool is_required = false;
};

clap_arg clap_arg_option_opt(string name, clap_option_desc opts);
#define clap_arg_option(name, ...) clap_arg_option_opt(name, {__VA_ARGS__})

struct clap_positional_desc {
    string value_name;    // defaults to name if empty
    string help_text;
    bool is_required = false;
    clap_arg_type arg_type = CLAP_ARG_STRING;
};

clap_arg clap_arg_positional_opt(string name, clap_positional_desc opts);
#define clap_arg_positional(name, ...) clap_arg_positional_opt(name, {__VA_ARGS__})

struct clap_flag_desc {
    string help_text;
    string short_name;  // optional
    string long_name;   // optional
    clap_action action = CLAP_ACTION_SET_TRUE; // or SET_FALSE / COUNT
};

clap_arg clap_arg_flag_opt(string name, clap_flag_desc opts);
#define clap_arg_flag(name, ...) clap_arg_flag_opt(name, {__VA_ARGS__})

// Parser functions
clap_parse_result clap_parse(clap_parser ref parser, int argc, char **argv);
clap_parse_result clap_parse_args(clap_parser ref parser, array<string> args);
void clap_print_help(clap_parser ref parser);
void clap_print_version(clap_parser ref parser);

// Result access functions
string clap_get_string(clap_parse_result ref result, string key);
s64 clap_get_int(clap_parse_result ref result, string key);
f64 clap_get_float(clap_parse_result ref result, string key);
bool clap_get_bool(clap_parse_result ref result, string key);
bool clap_has_arg(clap_parse_result ref result, string key);

// Helper functions
clap_arg *clap_find_arg_by_short(clap_parser ref parser, string short_name);
clap_arg *clap_find_arg_by_long(clap_parser ref parser, string long_name);
bool clap_parse_value(string value_str, clap_arg_type type, clap_argument_value *out_value);

LSTD_END_NAMESPACE
