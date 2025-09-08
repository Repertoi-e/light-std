#include "lstd/clap.h"
#include "lstd/fmt.h"

LSTD_BEGIN_NAMESPACE

clap_arg clap_arg_positional(string name, string value_name) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = value_name.Count ? value_name : name;
    arg.arg_type = CLAP_ARG_STRING;
    arg.action = CLAP_ACTION_STORE;
    arg.is_required = false;
    arg.is_positional = true;
    return arg;
}

clap_arg clap_arg_option(string name, string short_opt, string long_opt) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = name;
    arg.short_name = short_opt;
    arg.long_name = long_opt;
    arg.arg_type = CLAP_ARG_STRING;
    arg.action = CLAP_ACTION_STORE;
    arg.is_required = false;
    arg.is_positional = false;
    return arg;
}

clap_arg clap_arg_flag(string name, string short_opt, string long_opt) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = name;
    arg.short_name = short_opt;
    arg.long_name = long_opt;
    arg.arg_type = CLAP_ARG_BOOL;
    arg.action = CLAP_ACTION_SET_TRUE;
    arg.is_required = false;
    arg.is_positional = false;
    return arg;
}

void clap_add_arg(clap_parser ref parser, clap_arg arg) {
    add(parser.arguments, arg);
}

clap_arg *clap_find_arg_by_short(clap_parser ref parser, string short_name) {
    For(parser.arguments) {
        if (strings_match(it.short_name, short_name)) {
            return &it;
        }
    }
    return null;
}

clap_arg *clap_find_arg_by_long(clap_parser ref parser, string long_name) {
    For(parser.arguments) {
        if (strings_match(it.long_name, long_name)) {
            return &it;
        }
    }
    return null;
}

bool clap_parse_value(string value_str, clap_arg_type type, clap_argument_value *out_value) {
    out_value->type = type;
    
    switch (type) {
        case CLAP_ARG_STRING:
            out_value->value = value_str;
            return true;
            
        case CLAP_ARG_INT: {
            auto [result, success, _] = parse_int<s32>(value_str);
            if (success == PARSE_SUCCESS) {
                out_value->value = (s64)result;
                return true;
            }
            return false;
        }
        
        case CLAP_ARG_FLOAT: {
            // For now, use a simple conversion - you might want to implement proper float parsing
            char *endptr;
            auto c = to_c_string_temp(value_str);
            f64 result = strtod(c, &endptr);
            if (endptr != c) {
                out_value->value = result;
                return true;
            }
            return false;
        }
        
        case CLAP_ARG_BOOL: {
            if (strings_match(value_str, "true") || strings_match(value_str, "1") || 
                strings_match(value_str, "yes") || strings_match(value_str, "on")) {
                out_value->value = true;
                return true;
            } else if (strings_match(value_str, "false") || strings_match(value_str, "0") || 
                       strings_match(value_str, "no") || strings_match(value_str, "off")) {
                out_value->value = false;
                return true;
            }
            return false;
        }
    }
    return false;
}

void clap_print_help(clap_parser ref parser) {
    print("Usage: {}", parser.program_name);
    
    // Print positional args
    For(parser.arguments) {
        if (it.is_positional) {
            if (it.is_required) {
                print(" <{}>", it.value_name);
            } else {
                print(" [{}]", it.value_name);
            }
        }
    }
    
    // Print options placeholder
    bool has_options = false;
    For(parser.arguments) {
        if (!it.is_positional) {
            has_options = true;
            break;
        }
    }
    
    if (has_options) {
        print(" {!GRAY}[options]{!}");
    }
    
    print("\n");
    
    if (parser.about_text.Count) {
        print("\n{}\n", parser.about_text);
    }
    
    // Print positional arguments
    bool has_positional = false;
    For(parser.arguments) {
        if (it.is_positional) {
            if (!has_positional) {
                print("\n{!CYAN}Arguments:{!}\n");
                has_positional = true;
            }
            print("  {!GREEN}{}{!}", it.value_name);
            if (it.help_text.Count) {
                print("  {}", it.help_text);
            }
            print("\n");
        }
    }
    
    // Print options
    if (has_options) {
        print("\n{!CYAN}Options:{!}\n");
        For(parser.arguments) {
            if (!it.is_positional) {
                print("  ");
                if (it.short_name.Count) {
                    print("-{}, ", it.short_name);
                }
                if (it.long_name.Count) {
                    print("--{}", it.long_name);
                }
                if (it.arg_type != CLAP_ARG_BOOL) {
                    print(" <{}>", it.value_name);
                }
                if (it.help_text.Count) {
                    print("  {}", it.help_text);
                }
                if (it.default_val.Count) {
                    print(" (default: {})", it.default_val);
                }
                print("\n");
            }
        }
        
        print("  -h, --help     Show this help message\n");
        if (parser.version_text.Count) {
            print("  -v, --version  Show version information\n");
        }
    }
}

void clap_print_version(clap_parser ref parser) {
    if (parser.version_text.Count) {
        print("{} {}\n", parser.program_name, parser.version_text);
    } else {
        print("{}\n", parser.program_name);
    }
}

clap_parse_result clap_parse(clap_parser ref parser, int argc, char **argv) {
    array<string> args = os_parse_arguments(argc, argv);
    return clap_parse_args(parser, args);
}

clap_parse_result clap_parse_args(clap_parser ref parser, array<string> args) {
    clap_parse_result result = {};
    result.success = true;
    
    s64 positional_index = 0;
    
    for (s64 i = 1; i < args.Count; ++i) {  // Skip program name
        string arg = args[i];
        
        if (arg.Count == 0) continue;
        
        // Check for help
        if (parser.auto_help && (strings_match(arg, "--help") || strings_match(arg, "-h"))) {
            clap_print_help(parser);
            result.success = false;
            return result;
        }
        
        // Check for version
        if (parser.auto_version && (strings_match(arg, "--version") || strings_match(arg, "-v"))) {
            clap_print_version(parser);
            result.success = false;
            return result;
        }
        
        if (arg[0] == '-') {
            // This is an option
            clap_arg *found_arg = null;
            string value_str = "";
            
            if (arg.Count > 1 && arg[1] == '-') {
                // Long option (--option)
                string opt_name = slice(arg, 2, -1);
                
                // Check for --option=value format
                s64 equals_pos = search(opt_name, '=');
                if (equals_pos != -1) {
                    value_str = slice(opt_name, equals_pos + 1, -1);
                    opt_name = slice(opt_name, 0, equals_pos);
                }
                
                found_arg = clap_find_arg_by_long(parser, opt_name);
            } else {
                if (arg.Count == 1) {
                    result.success = false;
                    result.error = "Invalid option: -";
                    return result;
                }

                // Short option (-o)
                string opt_name = slice(arg, 1, 2);  // Just one character
                found_arg = clap_find_arg_by_short(parser, opt_name);
                
                // Check if value is attached (-ovalue)
                if (arg.Count > 2 && found_arg && found_arg->arg_type != CLAP_ARG_BOOL) {
                    value_str = slice(arg, 2, -1);
                }
            }
            
            if (!found_arg) {
                result.success = false;
                result.error = tprint("Unknown option: {}", arg);
                return result;
            }
            
            clap_argument_value arg_value = {};
            
            if (found_arg->action == CLAP_ACTION_SET_TRUE) {
                arg_value.type = CLAP_ARG_BOOL;
                arg_value.value = true;
            } else if (found_arg->action == CLAP_ACTION_SET_FALSE) {
                arg_value.type = CLAP_ARG_BOOL;
                arg_value.value = false;
            } else {
                // Need a value
                if (value_str.Count == 0) {
                    if (i + 1 < args.Count) {
                        value_str = args[++i];
                    } else {
                        result.success = false;
                        result.error = tprint("Option {} requires a value", arg);
                        return result;
                    }
                }
                
                if (!clap_parse_value(value_str, found_arg->arg_type, &arg_value)) {
                    result.success = false;
                    result.error = tprint("Invalid value '{}' for option {}", value_str, arg);
                    return result;
                }
            }
            
            set(result.values, found_arg->name, arg_value);
            
        } else {
            // This is a positional argument
            clap_arg *positional_arg = null;
            
            // Find the positional_index-th positional argument
            s64 pos_count = 0;
            For(parser.arguments) {
                if (it.is_positional) {
                    if (pos_count == positional_index) {
                        positional_arg = &it;
                        break;
                    }
                    pos_count++;
                }
            }
            
            if (!positional_arg) {
                result.success = false;
                result.error = tprint("Unexpected positional argument: {}", arg);
                return result;
            }
            
            clap_argument_value arg_value = {};
            if (!clap_parse_value(arg, positional_arg->arg_type, &arg_value)) {
                result.success = false;
                result.error = tprint("Invalid value '{}' for argument {}", arg, positional_arg->name);
                return result;
            }
            
            set(result.values, positional_arg->name, arg_value);
            positional_index++;
        }
    }
    
    // Set default values and check required arguments
    For(parser.arguments) {
        auto [kp, val] = search(result.values, it.name);
        
        if (!val) {
            if (it.is_required) {
                result.success = false;
                if (it.is_positional) {
                    result.error = tprint("Missing required argument: {}", it.value_name);
                } else {
                    result.error = tprint("Missing required option: --{}", it.long_name.Count ? it.long_name : it.short_name);
                }
                return result;
            }
            
            // Set default value if available
            if (it.default_val.Count) {
                clap_argument_value default_value = {};
                if (clap_parse_value(it.default_val, it.arg_type, &default_value)) {
                    set(result.values, it.name, default_value);
                }
            }
        }
    }
    
    return result;
}

string clap_get_string(clap_parse_result ref result, string key) {
    auto [kp, val] = search(result.values, key);
    if (val && val->type == CLAP_ARG_STRING) {
        string ret_val = "";
        val->value.visit(match {
            [ref ret_val](string value) { ret_val = value; },
            [](auto) {}
        });
        return ret_val;
    }
    return "";
}

s64 clap_get_int(clap_parse_result ref result, string key) {
    auto [kp, val] = search(result.values, key);
    if (val && val->type == CLAP_ARG_INT) {
        s64 ret_val = 0;
        val->value.visit(match {
            [ref ret_val](s64 value) { ret_val = value; },
            [](auto) {}
        });
        return ret_val;
    }
    return 0;
}

f64 clap_get_float(clap_parse_result ref result, string key) {
    auto [kp, val] = search(result.values, key);
    if (val && val->type == CLAP_ARG_FLOAT) {
        f64 ret_val = 0.0;
        val->value.visit(match {
            [ref ret_val](f64 value) { ret_val = value; },
            [](auto) {}
        });
        return ret_val;
    }
    return 0.0;
}

bool clap_get_bool(clap_parse_result ref result, string key) {
    auto [kp, val] = search(result.values, key);
    if (val && val->type == CLAP_ARG_BOOL) {
        bool ret_val = false;
        val->value.visit(match {
            [ref ret_val](bool value) { ret_val = value; },
            [](auto) {}
        });
        return ret_val;
    }
    return false;
}

bool clap_has_arg(clap_parse_result ref result, string key) {
    return search(result.values, key).Value != null;
}

LSTD_END_NAMESPACE
