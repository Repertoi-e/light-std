#include "lstd/clap.h"
#include "lstd/fmt.h"
#include <stdlib.h>

LSTD_BEGIN_NAMESPACE

// Options-struct builders (POD DSL)
clap_arg clap_arg_option_opt(string name, clap_option_desc opts) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = opts.value_name.Count ? opts.value_name : name;
    arg.help_text = opts.help_text;
    arg.short_name = opts.short_name;
    arg.long_name = opts.long_name;
    arg.default_val = opts.default_val;
    arg.arg_type = opts.arg_type;
    arg.action = opts.action;
    arg.is_required = opts.is_required;
    arg.is_positional = false;
    return arg;
}

clap_arg clap_arg_positional_opt(string name, clap_positional_desc opts) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = opts.value_name.Count ? opts.value_name : name;
    arg.help_text = opts.help_text;
    arg.arg_type = opts.arg_type;
    arg.action = CLAP_ACTION_STORE;
    arg.is_required = opts.is_required;
    arg.is_positional = true;
    return arg;
}

clap_arg clap_arg_flag_opt(string name, clap_flag_desc opts) {
    clap_arg arg = {};
    arg.name = name;
    arg.value_name = name;
    arg.help_text = opts.help_text;
    arg.short_name = opts.short_name;
    arg.long_name = opts.long_name;
    arg.arg_type = CLAP_ARG_BOOL;
    arg.action = opts.action;
    arg.is_required = false;
    arg.is_positional = false;
    return arg;
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
        
    // Check for version (-v short)
    if (parser.auto_version && (strings_match(arg, "--version") || strings_match(arg, "-v"))) {
            clap_print_version(parser);
            result.success = false;
            return result;
        }
        
        if (strings_match(arg, "--")) {
            // End of options, rest are positional
            for (s64 j = i + 1; j < args.Count; ++j) {
                string pos = args[j];
                clap_arg *positional_arg = null;
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
                    result.error = tprint("Unexpected positional argument: {}", pos);
                    return result;
                }
                clap_argument_value arg_value = {};
                if (!clap_parse_value(pos, positional_arg->arg_type, &arg_value)) {
                    result.success = false;
                    result.error = tprint("Invalid value '{}' for argument {}", pos, positional_arg->name);
                    return result;
                }
                set(result.values, positional_arg->name, arg_value);
                positional_index++;
            }
            break;
        }

        if (arg[0] == '-') {
            // This is an option
            clap_arg *found_arg = null;
            string value_str = "";
            
            if (arg.Count > 1 && arg[1] == '-') {
                // Long option (--option)
                string opt_name = slice(arg, 2, length(arg));
                
                // Check for --option=value format
                s64 equals_pos = -1;
                {
                    s64 len = length(opt_name);
                    For_as(k, range(len)) {
                        if (opt_name[k] == '=') { equals_pos = k; break; }
                    }
                }
                if (equals_pos != -1) {
                    value_str = slice(opt_name, equals_pos + 1, length(opt_name));
                    opt_name = slice(opt_name, 0, equals_pos);
                }
                
                // Support --no-<flag> to explicitly set false
                if (length(opt_name) > 3 && strings_match(slice(opt_name, 0, 3), "no-")) {
                    string base_name = slice(opt_name, 3, length(opt_name));
                    found_arg = clap_find_arg_by_long(parser, base_name);
                    if (found_arg) {
                        clap_argument_value arg_value = {};
                        arg_value.type = CLAP_ARG_BOOL;
                        arg_value.value = false;
                        set(result.values, found_arg->name, arg_value);
                        continue; // Done with this argument
                    }
                }
                
                found_arg = clap_find_arg_by_long(parser, opt_name);
            } else {
                if (arg.Count == 1) {
                    result.success = false;
                    result.error = "Invalid option: -";
                    return result;
                }

                // Short option(s): support clusters like -abc, and -ovalue
                string shorts = slice(arg, 1, length(arg));
                s64 shorts_len = length(shorts);
                bool consumed_cluster = false;
                for (s64 j = 0; j < shorts_len; ++j) {
                    string opt_name = slice(shorts, j, j + 1);
                    found_arg = clap_find_arg_by_short(parser, opt_name);
                    if (!found_arg) {
                        result.success = false;
                        result.error = tprint("Unknown option: -{}", opt_name);
                        return result;
                    }

                    clap_argument_value arg_value = {};

                    if (found_arg->action == CLAP_ACTION_SET_TRUE) {
                        arg_value.type = CLAP_ARG_BOOL;
                        arg_value.value = true;
                        set(result.values, found_arg->name, arg_value);
                        // keep scanning next in cluster
                        continue;
                    } else if (found_arg->action == CLAP_ACTION_SET_FALSE) {
                        arg_value.type = CLAP_ARG_BOOL;
                        arg_value.value = false;
                        set(result.values, found_arg->name, arg_value);
                        continue;
                    } else if (found_arg->action == CLAP_ACTION_COUNT) {
                        // Increment a counter
                        s64 new_val = 1;
                        auto [kp, vp] = search(result.values, found_arg->name);
                        if (vp && vp->type == CLAP_ARG_INT) {
                            // extract existing
                            s64 existing = 0;
                            vp->value.visit(match {
                                [&existing](s64 v) { existing = v; },
                                [](auto) {}
                            });
                            new_val = existing + 1;
                        }
                        arg_value.type = CLAP_ARG_INT;
                        arg_value.value = new_val;
                        set(result.values, found_arg->name, arg_value);
                        continue;
                    }

                    // Needs a value (store)
                    if (j + 1 < shorts_len) {
                        // Rest of the cluster is the value, e.g., -oVALUE
                        value_str = slice(shorts, j + 1, length(shorts));
                        consumed_cluster = true;
                    } else {
                        // Next argv is the value
                        if (i + 1 < args.Count) {
                            value_str = args[++i];
                        } else {
                            result.success = false;
                            result.error = tprint("Option -{} requires a value", opt_name);
                            return result;
                        }
                    }
                    if (!clap_parse_value(value_str, found_arg->arg_type, &arg_value)) {
                        result.success = false;
                        result.error = tprint("Invalid value '{}' for option -{}", value_str, opt_name);
                        return result;
                    }
                    set(result.values, found_arg->name, arg_value);
                    // We've consumed the rest of the cluster as value; stop processing this token
                    consumed_cluster = true;
                    break;
                }
                // Cluster handled (only flags or consumed a value); proceed to next argv
                continue;
            }
            
            if (!found_arg) {
                // If we reached here, it means we had a single short option token (like "-x")
                // and it did not match any known option, or a long option not found.
                // For long forms, preserve their exact spelling (arg); for short, we handled in cluster.
                result.success = false;
                result.error = tprint("Unknown option: {}", arg);
                return result;
            }
            
            clap_argument_value arg_value = {};
            
            if (found_arg->action == CLAP_ACTION_SET_TRUE) {
                arg_value.type = CLAP_ARG_BOOL;
                if (value_str.Count) {
                    if (!clap_parse_value(value_str, CLAP_ARG_BOOL, &arg_value)) {
                        result.success = false;
                        result.error = tprint("Invalid boolean value '{}' for option {}", value_str, arg);
                        return result;
                    }
                } else {
                    arg_value.value = true;
                }
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
