#include "test.h"

//
// This file was automatically generated by build_tests.py
//

void build_test_table() {
    {
        auto [_, array] = add(g_TestTable, string("bits.cpp"), {});
        extern void test_msb();
        add(*array, test{"msb", test_msb});
        extern void test_lsb();
        add(*array, test{"lsb", test_lsb});
    }
    {
        auto [_, array] = add(g_TestTable, string("file.cpp"), {});
        extern void test_path_manipulation();
        add(*array, test{"path_manipulation", test_path_manipulation});
        extern void test_file_size();
        add(*array, test{"file_size", test_file_size});
    }
    {
        auto [_, array] = add(g_TestTable, string("fmt.cpp"), {});
        extern void test_write_bool();
        add(*array, test{"write_bool", test_write_bool});
        extern void test_write_integer_16();
        add(*array, test{"write_integer_16", test_write_integer_16});
        extern void test_write_integer_32();
        add(*array, test{"write_integer_32", test_write_integer_32});
        extern void test_write_integer_64();
        add(*array, test{"write_integer_64", test_write_integer_64});
        extern void test_write_f64();
        add(*array, test{"write_f64", test_write_f64});
        extern void test_write_code_point();
        add(*array, test{"write_code_point", test_write_code_point});
        extern void test_format_int();
        add(*array, test{"format_int", test_format_int});
        extern void test_format_int_binary();
        add(*array, test{"format_int_binary", test_format_int_binary});
        extern void test_format_int_octal();
        add(*array, test{"format_int_octal", test_format_int_octal});
        extern void test_format_int_decimal();
        add(*array, test{"format_int_decimal", test_format_int_decimal});
        extern void test_format_int_hexadecimal();
        add(*array, test{"format_int_hexadecimal", test_format_int_hexadecimal});
        extern void test_format_int_localeish();
        add(*array, test{"format_int_localeish", test_format_int_localeish});
        extern void test_format_f32();
        add(*array, test{"format_f32", test_format_f32});
        extern void test_format_f64();
        add(*array, test{"format_f64", test_format_f64});
        extern void test_format_nan();
        add(*array, test{"format_nan", test_format_nan});
        extern void test_format_inf();
        add(*array, test{"format_inf", test_format_inf});
        extern void test_format_custom();
        add(*array, test{"format_custom", test_format_custom});
        extern void test_precision_rounding();
        add(*array, test{"precision_rounding", test_precision_rounding});
        extern void test_prettify_float();
        add(*array, test{"prettify_float", test_prettify_float});
        extern void test_escape_brackets();
        add(*array, test{"escape_brackets", test_escape_brackets});
        extern void test_args_in_different_positions();
        add(*array, test{"args_in_different_positions", test_args_in_different_positions});
        extern void test_args_errors();
        add(*array, test{"args_errors", test_args_errors});
        extern void test_many_args();
        add(*array, test{"many_args", test_many_args});
        extern void test_auto_arg_index();
        add(*array, test{"auto_arg_index", test_auto_arg_index});
        extern void test_empty_specs();
        add(*array, test{"empty_specs", test_empty_specs});
        extern void test_left_align();
        add(*array, test{"left_align", test_left_align});
        extern void test_right_align();
        add(*array, test{"right_align", test_right_align});
        extern void test_numeric_align();
        add(*array, test{"numeric_align", test_numeric_align});
        extern void test_center_align();
        add(*array, test{"center_align", test_center_align});
        extern void test_fill();
        add(*array, test{"fill", test_fill});
        extern void test_plus_sign();
        add(*array, test{"plus_sign", test_plus_sign});
        extern void test_minus_sign();
        add(*array, test{"minus_sign", test_minus_sign});
        extern void test_space_sign();
        add(*array, test{"space_sign", test_space_sign});
        extern void test_hash_flag();
        add(*array, test{"hash_flag", test_hash_flag});
        extern void test_zero_flag();
        add(*array, test{"zero_flag", test_zero_flag});
        extern void test_width();
        add(*array, test{"width", test_width});
        extern void test_dynamic_width();
        add(*array, test{"dynamic_width", test_dynamic_width});
        extern void test_precision();
        add(*array, test{"precision", test_precision});
        extern void test_benchmark_string();
        add(*array, test{"benchmark_string", test_benchmark_string});
        extern void test_dynamic_precision();
        add(*array, test{"dynamic_precision", test_dynamic_precision});
        extern void test_colors_and_emphasis();
        add(*array, test{"colors_and_emphasis", test_colors_and_emphasis});
    }
    {
        auto [_, array] = add(g_TestTable, string("parse.cpp"), {});
        extern void test_int();
        add(*array, test{"int", test_int});
        extern void test_bool();
        add(*array, test{"bool", test_bool});
        extern void test_guid();
        add(*array, test{"guid", test_guid});
    }
    {
        auto [_, array] = add(g_TestTable, string("range.cpp"), {});
        extern void test_basic();
        add(*array, test{"basic", test_basic});
        extern void test_variable_steps();
        add(*array, test{"variable_steps", test_variable_steps});
        extern void test_reverse();
        add(*array, test{"reverse", test_reverse});
    }
    {
        auto [_, array] = add(g_TestTable, string("signal.cpp"), {});
        extern void test_global_function_delegate();
        add(*array, test{"global_function_delegate", test_global_function_delegate});
        extern void test_member_function_delegate();
        add(*array, test{"member_function_delegate", test_member_function_delegate});
        extern void test_functor_delegate();
        add(*array, test{"functor_delegate", test_functor_delegate});
    }
    {
        auto [_, array] = add(g_TestTable, string("storage.cpp"), {});
        extern void test_stack_array();
        add(*array, test{"stack_array", test_stack_array});
        extern void test_array();
        add(*array, test{"array", test_array});
        extern void test_hash_table();
        add(*array, test{"hash_table", test_hash_table});
        extern void test_hash_table_clone();
        add(*array, test{"hash_table_clone", test_hash_table_clone});
        extern void test_hash_table_alignment();
        add(*array, test{"hash_table_alignment", test_hash_table_alignment});
    }
    {
        auto [_, array] = add(g_TestTable, string("string.cpp"), {});
        extern void test_code_point_size();
        add(*array, test{"code_point_size", test_code_point_size});
        extern void test_substring();
        add(*array, test{"substring", test_substring});
        extern void test_substring_mixed_sizes();
        add(*array, test{"substring_mixed_sizes", test_substring_mixed_sizes});
        extern void test_index();
        add(*array, test{"index", test_index});
        extern void test_insert();
        add(*array, test{"insert", test_insert});
        extern void test_remove();
        add(*array, test{"remove", test_remove});
        extern void test_trim();
        add(*array, test{"trim", test_trim});
        extern void test_match_beginning();
        add(*array, test{"match_beginning", test_match_beginning});
        extern void test_match_end();
        add(*array, test{"match_end", test_match_end});
        extern void test_set();
        add(*array, test{"set", test_set});
        extern void test_iterator();
        add(*array, test{"iterator", test_iterator});
        extern void test_append();
        add(*array, test{"append", test_append});
        extern void test_builder();
        add(*array, test{"builder", test_builder});
        extern void test_remove_all();
        add(*array, test{"remove_all", test_remove_all});
        extern void test_replace_all();
        add(*array, test{"replace_all", test_replace_all});
        extern void test_find();
        add(*array, test{"find", test_find});
    }
    {
        auto [_, array] = add(g_TestTable, string("thread.cpp"), {});
        extern void test_hardware_concurrency();
        add(*array, test{"hardware_concurrency", test_hardware_concurrency});
        extern void test_ids();
        add(*array, test{"ids", test_ids});
        extern void test_thread_local_storage();
        add(*array, test{"thread_local_storage", test_thread_local_storage});
        extern void test_lock_free();
        add(*array, test{"lock_free", test_lock_free});
        extern void test_mutex_lock();
        add(*array, test{"mutex_lock", test_mutex_lock});
        extern void test_fast_mutex_lock();
        add(*array, test{"fast_mutex_lock", test_fast_mutex_lock});
        extern void test_condition_variable();
        add(*array, test{"condition_variable", test_condition_variable});
        extern void test_context();
        add(*array, test{"context", test_context});
    }

}
