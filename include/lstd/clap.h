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
/// ```cpp
/// clap_parser parser = {};
/// parser.program_name = "myapp";
/// parser.about_text = "A simple example application";
/// parser.version_text = "1.0.0";
/// 
/// clap_arg input_arg = clap_arg_positional("input", "INPUT");
/// input_arg.help_text = "Input file to process";
/// input_arg.is_required = true;
/// clap_add_arg(&parser, input_arg);
/// 
/// clap_arg output_arg = clap_arg_option("output", "o", "output");
/// output_arg.help_text = "Output file";
/// output_arg.default_val = "out.txt";
/// clap_add_arg(&parser, output_arg);
/// 
/// clap_arg verbose_arg = clap_arg_flag("verbose", "v", "verbose");
/// verbose_arg.help_text = "Enable verbose output";
/// clap_add_arg(&parser, verbose_arg);
/// 
/// clap_parse_result result = clap_parse(parser, argc, argv);
/// if (!result.success) {
///     print("{}\n", result.error);
///     return 1;
/// }
/// 
/// string input = clap_get_string(result, "input");
/// string output = clap_get_string(result, "output");
/// bool verbose = clap_get_bool(result, "verbose");
/// ```

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
    bool auto_help;
    bool auto_version;
};

// Create different types of arguments
clap_arg clap_arg_positional(string name, string value_name = "");
clap_arg clap_arg_option(string name, string short_opt, string long_opt);
clap_arg clap_arg_flag(string name, string short_opt, string long_opt);

// Parser functions
void clap_add_arg(clap_parser ref parser, clap_arg arg);
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
