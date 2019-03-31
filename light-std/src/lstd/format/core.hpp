#pragma once

#include "../common.hpp"
#include "../context.hpp"

#include "../../vendor/stb/stb_sprintf.hpp"

#include "format_integer.hpp"

#include "../io/writer.hpp"
#include "../memory/memory_buffer.hpp"

#include "specs.hpp"
#include "value.hpp"

#include "../intrinsics/intrin.hpp"

#include <type_traits>

LSTD_BEGIN_NAMESPACE

namespace fmt {

inline byte *float_callback_for_sprintf(byte *sprintfBuffer, void *user, int len) {
    auto *buffer = (memory_buffer<500> *) user;
    if (!buffer->has_space_for(len)) {
        buffer->grow(STB_SPRINTF_MIN);
    }
    buffer->append_pointer_and_size_unsafe(sprintfBuffer, len);
    return sprintfBuffer;
}

struct parse_context;
struct format_context;

template <typename T, typename Enable = void>
struct formatter {
    void format(const T &, format_context &) {
        assert(false);
        // static_assert(false, "Formatter<T> not specialized");
    }
};

struct argument {
    value Value;
    format_type Type = format_type::NONE;

    struct handle {
        custom_value Custom;

        explicit handle(custom_value custom) : Custom(custom) {}
        void format(format_context &f) const { Custom.Format(Custom.Data, f); }
    };

    explicit operator bool() const { return Type != format_type::NONE; }
};

template <bool IsPacked, typename T>
std::enable_if_t<IsPacked, value> make_argument(const T &value) {
    return make_value(value);
}

template <bool IsPacked, typename T>
std::enable_if_t<!IsPacked, argument> make_argument(const T &value) {
    return make_argument(value);
}

template <typename T>
constexpr argument make_argument(const T &value) {
    argument arg;
    arg.Type = get_type<T>::Value;
    arg.Value = make_value(value);
    return arg;
}

struct named_argument_base {
    string_view Name;
    mutable byte Data[sizeof(argument)]{};

    named_argument_base(const string_view &name) : Name(name) {}

    argument deserialize() const {
        argument result;
        copy_memory(&result, Data, sizeof(argument));
        return result;
    }
};

template <typename T>
struct named_argument : named_argument_base {
    const T &Value;

    named_argument(const string_view &name, const T &value) : named_argument_base(name), Value(value) {}
};

template <typename T>
init_value<const void *, format_type::NAMED_ARGUMENT> make_value(const named_argument<T> &value) {
    argument arg;
    arg.Type = get_type<decltype(value.Value)>::Value;
    arg.Value = make_value(value.Value);
    copy_memory(value.Data, &arg, sizeof(argument));
    return (const void *) (&value);
}

constexpr auto MAX_PACKED_ARGS = 15;

template <typename... Args>
struct arguments_array {
    static constexpr size_t NUM_ARGS = sizeof...(Args);
    static constexpr bool IS_PACKED = NUM_ARGS < MAX_PACKED_ARGS;

    using value_type = std::conditional_t<IS_PACKED, value, argument>;

    // If the arguments are not packed, add one more element to mark the end.
    static constexpr size_t DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);
    value_type Data[DATA_SIZE];

    arguments_array(const Args &... args) : Data{make_argument<IS_PACKED>(args)...} {}

   private:
    // This dummy template is required because
    // otherwise you can't call the non-templated
    // "get_types_impl" from the templated one.
    template <typename dummy>
    static constexpr u64 get_types_impl() {
        return 0ull;
    }

    template <typename dummy, typename Arg, typename... MyArgs>
    static constexpr u64 get_types_impl() {
        return ((u64) get_type<Arg>::Value) | (get_types_impl<dummy, MyArgs...>() << 4);
    }
    static constexpr s64 get_types() { return IS_PACKED ? (s64)(get_types_impl<s32, Args...>()) : -(s64)(NUM_ARGS); }

   public:
    static constexpr s64 TYPES = get_types();
};

struct arguments {
    // To reduce compiled code size per formatting function call, types of first
    // MAX_PACKED_ARGS arguments are passed in the _Types field.
    u64 Types = 0;
    union {
        const value *Values;
        const argument *Args;
        const void *DataPointer;
    };

    template <typename... Args>
    arguments(const arguments_array<Args...> &array) : Types((u64) array.TYPES), DataPointer(array.Data) {}

    arguments(const argument *args, u32 count) : Types(-((s64) count)) { Args = args; }

    u32 max_size() const {
        s64 signedTypes = (s64) Types;
        return (u32)(signedTypes < 0 ? -signedTypes : (s64) MAX_PACKED_ARGS);
    }

    format_type get_type_at(u32 index) const {
        u32 shift = index * 4;
        u64 mask = 0xf;
        return (format_type)((Types & (mask << shift)) >> shift);
    }

    argument get(u32 index) const {
        s64 signedTypes = (s64) Types;
        if (signedTypes < 0) {
            u64 numArgs = (u64)(-signedTypes);
            if (index < numArgs) {
                return Args[index];
            }
        }
        if (index > MAX_PACKED_ARGS) {
            return {};
        }

        argument result;
        result.Type = get_type_at(index);
        if (result.Type == format_type::NONE) {
            return result;
        }
        result.Value = Values[index];

        if (result.Type == format_type::NAMED_ARGUMENT) {
            auto &named = *(const named_argument_base *) (result.Value.Pointer_Value);
            result = named.deserialize();
        }
        return result;
    }
};

// A map from argument names to their values for named arguments.
struct argument_map {
    struct entry {
        string_view Name;
        argument Arg;
    };

    entry *Entries = null;
    u32 Size = 0;
    allocator_closure Allocator;

    argument_map() {}
    ~argument_map() {
        if (Entries) delete Entries;
    }

    void ensure_initted(const arguments &args) {
        if (Entries) return;

        Entries = new (&Allocator, ensure_allocator) entry[args.max_size()];

        if (args.get_type_at(MAX_PACKED_ARGS - 1) == format_type::NONE) {
            u32 i = 0;
            while (true) {
                auto type = args.get_type_at(i);
                if (type == format_type::NONE) {
                    return;
                } else if (type == format_type::NAMED_ARGUMENT) {
                    add(args.Values[i]);
                }
                ++i;
            }
        }
        u32 i = 0;
        while (true) {
            auto type = args.Args[i].Type;
            if (type == format_type::NONE) {
                return;
            } else if (type == format_type::NAMED_ARGUMENT) {
                add(args.Args[i].Value);
            }
            ++i;
        }
    }

    void add(value value) {
        auto &named = *(const named_argument_base *) (value.Pointer_Value);
        Entries[Size++] = entry{named.Name, named.deserialize()};
    }

    argument find(const string_view &name) const {
        // The list is unsorted, so just return the first matching name.
        for (auto *it = Entries, *end = Entries + Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return {};
    }

   private:
    argument_map(const argument_map &) = delete;
    void operator=(const argument_map &) = delete;
};

struct parse_context {
   private:
    s32 NextArgId = 0;

   public:
    string_view FormatString;
    const byte *It;
    dynamic_format_specs Specs;

    parse_context(const string_view &formatString) : FormatString(formatString) {
        It = (const byte *) FormatString.begin().to_pointer();
    }

    constexpr u32 next_arg_id() {
        if (NextArgId >= 0) return (u32) NextArgId++;
        assert(false && "Cannot switch from manual to automatic argument indexing");
        return 0;
    }

    constexpr bool check_arg_id(u32) {
        if (NextArgId > 0) {
            assert(false && "Cannot switch from manual to automatic argument indexing");
            return false;
        }
        NextArgId = -1;
        return true;
    }

    void check_arg_id(const string_view &) const {}
};

void format_context_write(void *data, const memory_view &memory);
void format_context_flush(void *data);

struct format_context : io::writer {
   private:
    argument_map ArgMap;
    arguments Args;

   public:
    parse_context ParseContext;
    memory_buffer<500> Out;
    writer &FlushOutput;

    // If you want to use this Writer to just output formatted types (without a format string, etc.) you can use this
    // constructor. If you want to control the format specifiers, modify ParseContext.Specs
    format_context(writer &flushOutput) : Args(null, 0), ParseContext(""), FlushOutput(flushOutput) {
        WriteFunction = format_context_write;
        FlushFunction = format_context_flush;
    }

    format_context(writer &flushOutput, const string_view &formatString, arguments args)
        : Args(args), ParseContext(formatString), FlushOutput(flushOutput) {
        WriteFunction = format_context_write;
        FlushFunction = format_context_flush;
    }

    // Returns the argument with specified index.
    argument do_get_arg(u32 argId) const {
        auto result = Args.get(argId);
        if (!result) {
            assert(false && "Argument index out of range");
        }
        return result;
    }

    // Checks if manual indexing is used and returns the argument with specified index.
    argument get_arg(u32 argId) { return ParseContext.check_arg_id(argId) ? do_get_arg(argId) : argument{}; }

    // Checks if manual indexing is used and returns the argument with the specified name.
    argument get_arg(const string_view &name) {
        ArgMap.ensure_initted(Args);

        auto result = ArgMap.find(name);
        if (!result) {
            assert(false && "Argument with this name not found");
        }
        return result;
    }

    argument next_arg() { return do_get_arg(ParseContext.next_arg_id()); }

    using writer::write;

    // Format an integer according to the current argument's format specs.
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>> write_int(T value) {
        byte prefix[4] = {0};
        size_t prefixSize = 0;

        using unsigned_type = std::make_unsigned_t<T>;
        unsigned_type absValue = (unsigned_type) value;
        if (is_negative(value)) {
            prefix[0] = '-';
            ++prefixSize;
            absValue = 0 - absValue;
        } else if (ParseContext.Specs.has_flag(flag::SIGN)) {
            prefix[0] = sign_plus() ? '+' : ' ';
            ++prefixSize;
        }

        switch (type()) {
            case 0:
            case 'd': {
                u32 numDigits = internal::count_digits(absValue);
                format_int(numDigits, memory_view(prefix, prefixSize),
                           [&](format_context &f) { internal::format_uint(f.Out, absValue, numDigits); });
            } break;
            case 'x':
            case 'X': {
                if (alternate()) {
                    prefix[prefixSize++] = '0';
                    prefix[prefixSize++] = (byte) type();
                }
                u32 numDigits = internal::count_digits<4>(absValue);
                format_int(numDigits, memory_view(prefix, prefixSize), [&](format_context &f) {
                    internal::format_uint<4>(f.Out, absValue, numDigits, type() != 'x');
                });
            } break;
            case 'b':
            case 'B': {
                if (alternate()) {
                    prefix[prefixSize++] = '0';
                    prefix[prefixSize++] = (byte) type();
                }
                u32 numDigits = internal::count_digits<1>(absValue);
                format_int(numDigits, memory_view(prefix, prefixSize),
                           [&](format_context &f) { internal::format_uint<1>(f.Out, absValue, numDigits); });
            } break;
            case 'o': {
                u32 numDigits = internal::count_digits<3>(absValue);
                if (alternate() && precision() <= (s32) numDigits) {
                    // Octal prefix '0' is counted as a digit, so only add it if precision
                    // is not greater than the number of digits.
                    prefix[prefixSize++] = '0';
                }
                format_int(numDigits, memory_view(prefix, prefixSize),
                           [&](format_context &f) { internal::format_uint<3>(f.Out, absValue, numDigits); });
            } break;
            case 'n': {
                u32 numDigits = internal::count_digits(absValue);
                char32_t sep = internal::thousands_separator();
                byte sepEncoded[4];
                encode_code_point(sepEncoded, sep);

                u32 size = numDigits + 1 * ((numDigits - 1) / 3);
                format_int(size, memory_view(prefix, prefixSize), [&](format_context &f) {
                    internal::format_uint(
                        f.Out, absValue, size,
                        internal::add_thousands_separator{memory_view(sepEncoded, get_size_of_code_point(sep))});
                });
            } break;
            default:
                // Shouldn't ever get here, since the specs have been checked in the parse stage.
                assert(false);
        }
    }

    void write_float_sprintf(STBSP_SPRINTFCB callback, void *user, const byte *format, ...) const {
        va_list va;
        va_start(va, format);

        byte buffer[STB_SPRINTF_MIN];
        auto len = stbsp_vsprintfcb(callback, user, buffer, format, va);
        assert(len > 0);
        va_end(va);
    }

    // Format an float according to the current argument's format specs.
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>> write_float(T value) {
        bool upper = is_upper(type());

        byte sign = 0;
        // Check signbit instead of value < 0 because the latter is always false for NaN.
        if (sign_bit(value)) {
            sign = '-';
            value = -value;
        } else if (ParseContext.Specs.has_flag(flag::SIGN)) {
            sign = sign_plus() ? '+' : ' ';
        }

        // Format NaN and ininity ourselves because sprintf's output is not consistent across platforms.
        if (is_nan((f64) value)) {
            format_padded(
                [&](format_context &f) {
                    if (sign) f.Out.append(sign);
                    f.Out.append(upper ? "NAN" : "nan");
                },
                align(), 3 + (sign ? 1 : 0));
            return;
        }
        if (is_infinity((f64) value)) {
            format_padded(
                [&](format_context &f) {
                    if (sign) f.Out.append(sign);
                    f.Out.append(upper ? "INF" : "inf");
                },
                align(), 3 + (sign ? 1 : 0));
            return;
        }

        // Formatting floats is hard... we use stb_snprintf

        // Build format string.
        byte format[10];  // longest format: %#-*.*Lg
        byte *formatPtr = format;
        *formatPtr++ = '%';
        if (alternate()) *formatPtr++ = '#';
        if (precision() >= 0) {
            *formatPtr++ = '.';
            *formatPtr++ = '*';
        }
        byte t = (char) type();
        if (t == 0 || t == 'F') t = 'f';
        *formatPtr++ = t;
        *formatPtr = '\0';

        memory_buffer<500> buffer;

        if (precision() < 0) {
            write_float_sprintf(float_callback_for_sprintf, &buffer, format, (f64) value);
        } else {
            write_float_sprintf(float_callback_for_sprintf, &buffer, format, precision(), (f64) value);
        }

        size_t n = buffer.ByteLength;
        alignment alignSpec = align();
        if (alignSpec == alignment::NUMERIC) {
            if (sign) {
                auto old = ParseContext.Specs;
                ParseContext.Specs = {};
                write_codepoint(sign);
                ParseContext.Specs = old;

                sign = 0;
                if (width()) --ParseContext.Specs.Width;
            }
            alignSpec = alignment::RIGHT;
        } else {
            if (alignSpec == alignment::DEFAULT) alignSpec = alignment::RIGHT;
            if (sign) ++n;
        }

        format_padded(
            [&](format_context &f) {
                if (sign) f.Out.append(sign);
                f.Out.append(buffer);
            },
            alignSpec, n);
    }

#define int_helper(x)                                                           \
    if (type() != 'c') {                                                        \
        write_int(x);                                                           \
    } else {                                                                    \
        format_padded([&](format_context &f) { f.Out.append(x); }, align(), 1); \
    }

    void write_argument(const argument &arg) {
        switch (arg.Type) {
            case format_type::S32:
                int_helper(arg.Value.S32_Value);
                break;
            case format_type::U32:
                int_helper(arg.Value.U32_Value);
                break;
            case format_type::S64:
                write_int(arg.Value.S64_Value);
                break;
            case format_type::U64:
                write_int(arg.Value.U64_Value);
                break;
            case format_type::BOOL: {
                if (type()) {
                    write_int(arg.Value.S32_Value ? 1 : 0);
                } else {
                    write(arg.Value.S32_Value ? "true" : "false");
                }
            } break;
            case format_type::F64:
                write_float(arg.Value.F64_Value);
                break;
            case format_type::CSTRING:
                if (!type() || type() == 's') {
                    auto strValue = arg.Value.String_Value;
                    if (!strValue.Data) {
                        Out.append("{String pointer is null}");
                        return;
                    }
                    write(strValue);
                } else if (type() == 'p') {
                    auto oldFlags = ParseContext.Specs.Flags;
                    auto oldType = ParseContext.Specs.Type;

                    ParseContext.Specs.Flags = flag::HASH;
                    ParseContext.Specs.Type = 'x';
                    write_int((uptr_t) arg.Value.Pointer_Value);
                    ParseContext.Specs.Flags = oldFlags;
                    ParseContext.Specs.Type = oldType;
                }
                break;
            case format_type::STRING: {
                auto strValue = arg.Value.String_Value;
                if (!strValue.Data) {
                    Out.append("{String pointer is null}");
                    return;
                }
                write(strValue);
            } break;
            case format_type::POINTER: {
                auto oldFlags = ParseContext.Specs.Flags;
                auto oldType = ParseContext.Specs.Type;

                ParseContext.Specs.Flags = flag::HASH;
                ParseContext.Specs.Type = 'x';
                write_int((uptr_t) arg.Value.Pointer_Value);
                ParseContext.Specs.Flags = oldFlags;
                ParseContext.Specs.Type = oldType;
            } break;
            case format_type::CUSTOM: {
                auto handle = argument::handle(arg.Value.Custom_Value);
                handle.format(*this);
            } break;
            default:
                assert(false && "Invalid argument type");
        }
    }

#undef int_helper

    // Helper functions to acess format specs more directly
    u32 width() const { return ParseContext.Specs.Width; }
    char32_t fill() const { return ParseContext.Specs.Fill; }
    alignment align() const { return ParseContext.Specs.Align; }
    s32 precision() const { return ParseContext.Specs.Precision; }
    char32_t type() const { return ParseContext.Specs.Type; }

    bool sign_plus() const { return ParseContext.Specs.has_flag(flag::PLUS) != 0; }
    bool sign_minus() const { return ParseContext.Specs.has_flag(flag::MINUS) != 0; }
    bool alternate() const { return ParseContext.Specs.has_flag(flag::HASH) != 0; }
    bool sign_aware_zero_pad() const { return align() == alignment::NUMERIC && fill() == '0'; }

   private:
    // Pad according to _spec_.
    // This calls _func_ when it is time to print the padded content.
    // _length_ should be the expected length of the output from calling _func_
    template <typename F>
    void format_padded(F func, alignment align, size_t length) {
        if (width() <= length) {
            func(*this);
            return;
        }

        byte fillCpData[4];
        size_t fillCpSize = 0;
        if (fill()) {
            encode_code_point(fillCpData, fill());
            fillCpSize = get_size_of_code_point(fillCpData);
        }

        size_t padding = width() - length;
        if (align == alignment::RIGHT) {
            Out.grow(padding * fillCpSize);
            For(range(padding)) Out.append_pointer_and_size_unsafe(fillCpData, fillCpSize);
            func(*this);
        } else if (align == alignment::CENTER) {
            size_t leftPadding = padding / 2;

            Out.grow(leftPadding * fillCpSize);
            For(range(leftPadding)) Out.append_pointer_and_size_unsafe(fillCpData, fillCpSize);

            func(*this);

            size_t rightPadding = padding - leftPadding;
            Out.grow(rightPadding * fillCpSize);
            For(range(rightPadding)) Out.append_pointer_and_size_unsafe(fillCpData, fillCpSize);
        } else {
            func(*this);
            Out.grow(padding * fillCpSize);
            For(range(padding)) Out.append_pointer_and_size_unsafe(fillCpData, fillCpSize);
        }
    }

    // Writes an integer in the format
    //   <left-padding><prefix><numeric-padding><digits><right-padding>
    // where <digits> are written by func((Format_Context &) *this).
    template <typename F>
    void format_int(u32 numDigits, const memory_view &prefix, F func) {
        size_t size = prefix.ByteLength + numDigits;
        char32_t fillChar = fill();
        size_t padding = 0;
        if (align() == alignment::NUMERIC) {
            if (width() > size) {
                padding = width() - size;
                size = width();
            }
        } else if (precision() > (s32) numDigits) {
            size = prefix.ByteLength + (size_t) precision();
            padding = (size_t) precision() - numDigits;
            fillChar = '0';
        }
        align_spec as = ParseContext.Specs;
        if (align() == alignment::DEFAULT) as.Align = alignment::RIGHT;
        format_padded(
            [&](format_context &f) {
                if (prefix) {
                    f.Out.append(prefix);
                }
                For(range(padding)) f.Out.append(fillChar);
                func(f);
            },
            align() == alignment::DEFAULT ? alignment::RIGHT : align(), size);
    }

    format_context(const format_context &) = delete;
    void operator=(const format_context &) = delete;

    friend void format_context_write(void *, const memory_view &);
};

inline void format_context_write(void *data, const memory_view &memory) {
    auto context = (format_context *) data;

    string_view writeData = memory;

    size_t prec = (size_t) context->precision();
    if (context->precision() >= 0 && prec < writeData.Length) {
        writeData.remove_suffix(writeData.Length - prec);
    }
    context->format_padded([&](format_context &f) { f.Out.append(writeData); }, context->align(), writeData.Length);
}

inline void format_context_flush(void *data) {
    auto context = (format_context *) data;
    context->FlushOutput.write(context->Out.Data, context->Out.ByteLength);
}

// :struct Value in value.h
template <typename T>
void value::format_custom_arg(const void *arg, format_context &f) {
    formatter<T> formatter;
    formatter.format(*static_cast<const T *>(arg), f);
}

struct named_argument_helper {
    string_view Name;

    template <typename T>
    named_argument<T> operator=(T &&value) const {
        return {Name, std::forward<T>(value)};
    }
};
}  // namespace fmt

constexpr fmt::named_argument_helper operator"" _a(const byte *str, size_t size) { return {{str, size}}; }

LSTD_END_NAMESPACE
