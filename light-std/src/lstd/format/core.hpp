#pragma once

#include "../common.hpp"
#include "../context.hpp"

#include "format_float.hpp"
#include "format_integer.hpp"

#include "../io/writer.hpp"
#include "../memory/memory_buffer.hpp"

#include "specs.hpp"
#include "value.hpp"

#include "../intrinsics/intrin.h"

#include <type_traits>

LSTD_BEGIN_NAMESPACE

namespace fmt {
struct Parse_Context;
struct Format_Context;

template <typename T, typename Enable = void>
struct Formatter {
    void format(const T &, Format_Context &) { 
        assert(false);
        // static_assert(false, "Formatter<T> not specialized"); 
    }
};

struct Argument {
    Value Value;
    Format_Type Type = Format_Type::NONE;

    struct Handle {
        Custom_Value Custom;

        explicit Handle(Custom_Value custom) : Custom(custom) {}
        void format(Format_Context &f) const { Custom.Format(Custom.Data, f); }
    };

    explicit operator bool() const { return Type != Format_Type::NONE; }
};

template <bool IsPacked, typename T>
inline typename std::enable_if_t<IsPacked, Value> make_argument(const T &value) {
    return make_value(value);
}

template <bool IsPacked, typename T>
inline typename std::enable_if_t<!IsPacked, Argument> make_argument(const T &value) {
    return make_argument(value);
}

template <typename T>
constexpr Argument make_argument(const T &value) {
    Argument arg;
    arg.Type = Get_Type<T>::Value;
    arg.Value = make_value(value);
    return arg;
}

struct Named_Argument_Base {
    string_view Name;
    mutable char Data[sizeof(Argument)];

    Named_Argument_Base(const string_view &name) : Name(name) {}

    Argument deserialize() const {
        Argument result;
        copy_memory(&result, Data, sizeof(Argument));
        return result;
    }
};

template <typename T>
struct Named_Argument : Named_Argument_Base {
    const T &Value;

    Named_Argument(const string_view &name, const T &value) : Named_Argument_Base(name), Value(value) {}
};

template <typename T>
Init_Value<const void *, Format_Type::NAMED_ARGUMENT> make_value(const Named_Argument<T> &value) {
    Argument arg;
    arg.Type = Get_Type<decltype(value.Value)>::Value;
    arg.Value = make_value(value.Value);
    copy_memory(value.Data, &arg, sizeof(Argument));
    return (const void *) (&value);
}

constexpr auto MAX_PACKED_ARGS = 15;

template <typename... Args>
struct Arguments_Array {
    static constexpr size_t NUM_ARGS = sizeof...(Args);
    static constexpr bool IS_PACKED = NUM_ARGS < MAX_PACKED_ARGS;

    using value_type = typename std::conditional_t<IS_PACKED, Value, Argument>;

    // If the arguments are not packed, add one more element to mark the end.
    static constexpr size_t DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);
    value_type Data[DATA_SIZE];

    Arguments_Array(const Args &... args) : Data{make_argument<IS_PACKED>(args)...} {}

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
        return ((u64) Get_Type<Arg>::Value) | (get_types_impl<dummy, MyArgs...>() << 4);
    }
    static constexpr s64 get_types() { return IS_PACKED ? (s64)(get_types_impl<s32, Args...>()) : -(s64)(NUM_ARGS); }

   public:
    static constexpr s64 TYPES = get_types();
};

struct Arguments {
    // To reduce compiled code size per formatting function call, types of first
    // MAX_PACKED_ARGS arguments are passed in the _Types field.
    u64 Types = 0;
    union {
        const Value *Values;
        const Argument *Args;
        const void *DataPointer;
    };

    template <typename... Args>
    Arguments(const Arguments_Array<Args...> &array) : Types((u64) array.TYPES), DataPointer(array.Data) {}

    Arguments(const Argument *args, u32 count) : Types(-((s64) count)) { Args = args; }

    u32 max_size() const {
        s64 signedTypes = (s64) Types;
        return (u32)(signedTypes < 0 ? -signedTypes : (s64) MAX_PACKED_ARGS);
    }

    Format_Type get_type_at(u32 index) const {
        u32 shift = index * 4;
        u64 mask = 0xf;
        return (Format_Type)((Types & (mask << shift)) >> shift);
    }

    Argument get(u32 index) const {
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

        Argument result;
        result.Type = get_type_at(index);
        if (result.Type == Format_Type::NONE) {
            return result;
        }
        result.Value = Values[index];

        if (result.Type == Format_Type::NAMED_ARGUMENT) {
            auto &named = *(const Named_Argument_Base *)(result.Value.Pointer_Value);
            result = named.deserialize();
        }
        return result;
    }
};

// A map from argument names to their values for named arguments.
struct Argument_Map {
    struct Entry {
        string_view Name;
        Argument Arg;
    };

    Entry *Entries = null;
    u32 Size = 0;
    Allocator_Closure Allocator;

    Argument_Map() {}
    ~Argument_Map() { Delete(Entries, Allocator); }

    void ensure_initted(const Arguments &args) {
        if (Entries) return;

        Entries = New_and_ensure_allocator<Entry>(args.max_size(), Allocator);

        if (args.get_type_at(MAX_PACKED_ARGS - 1) == Format_Type::NONE) {
            u32 i = 0;
            while (true) {
                auto type = args.get_type_at(i);
                if (type == Format_Type::NONE) {
                    return;
                } else if (type == Format_Type::NAMED_ARGUMENT) {
                    add(args.Values[i]);
                }
                ++i;
            }
        }
        u32 i = 0;
        while (true) {
            auto type = args.Args[i].Type;
            if (type == Format_Type::NONE) {
                return;
            } else if (type == Format_Type::NAMED_ARGUMENT) {
                add(args.Args[i].Value);
            }
            ++i;
        }
    }

    void add(Value value) {
        auto &named = *(const Named_Argument_Base *)(value.Pointer_Value);
        Entries[Size++] = Entry{named.Name, named.deserialize()};
    }

    Argument find(const string_view &name) const {
        // The list is unsorted, so just return the first matching name.
        for (auto *it = Entries, *end = Entries + Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return {};
    }

   private:
    Argument_Map(const Argument_Map &) = delete;
    void operator=(const Argument_Map &) = delete;
};

struct Parse_Context {
   private:
    s32 NextArgId = 0;

   public:
    string_view FormatString;
    const byte *It;
    Dynamic_Format_Specs Specs;

    Parse_Context(const string_view &formatString) : FormatString(formatString) {
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

    void check_arg_id(const string_view &) {}
};

struct Format_Context : io::Writer {
   private:
    Argument_Map ArgMap;
    Arguments Args;

   public:
    Parse_Context ParseContext;
    Memory_Buffer<500> Out;
    io::Writer &FlushOutput;

    // If you want to use this Writer to just output formatted types (without a format string, etc.) you can use this
    // constructor. If you want to control the format specifiers, modify ParseContext.Specs
    Format_Context(io::Writer &flushOutput) : FlushOutput(flushOutput), Args(null, 0), ParseContext("") {}

    Format_Context(io::Writer &flushOutput, const string_view &formatString, Arguments args)
        : FlushOutput(flushOutput), ParseContext(formatString), Args(args) {}

    // Returns the argument with specified index.
    Argument do_get_arg(u32 argId) {
        auto result = Args.get(argId);
        if (!result) {
            assert(false && "Argument index out of range");
        }
        return result;
    }

    // Checks if manual indexing is used and returns the argument with specified index.
    Argument get_arg(u32 argId) { return ParseContext.check_arg_id(argId) ? do_get_arg(argId) : Argument{}; }

    // Checks if manual indexing is used and returns the argument with the specified name.
    Argument get_arg(const string_view &name) {
        ArgMap.ensure_initted(Args);

        auto result = ArgMap.find(name);
        if (!result) {
            assert(false && "Argument with this name not found");
        }
        return result;
    }

    Argument next_arg() { return do_get_arg(ParseContext.next_arg_id()); }

    void flush() override {
        FlushOutput.write(Out.Data, Out.ByteLength);
        FlushOutput.flush();
    }

    using io::Writer::write;

    // Write a string and pad it according to the current argument's format specs.
    void write(const Memory_View &view) override {
        string_view toWrite = view;

        size_t prec = (size_t) precision();
        if (precision() >= 0 && prec < toWrite.Length) {
            toWrite.remove_suffix(toWrite.Length - prec);
        }
        format_padded([&](Format_Context &f) { f.Out.append(toWrite); }, align(), toWrite.Length);
    }

    // Format an integer according to the current argument's format specs.
    template <typename T>
    std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>> write_int(T value) {
        byte prefix[4] = {0};
        size_t prefixSize = 0;

        using unsigned_type = typename std::make_unsigned_t<T>;
        unsigned_type absValue = (unsigned_type) value;
        if (is_negative(value)) {
            prefix[0] = '-';
            ++prefixSize;
            absValue = 0 - absValue;
        } else if (ParseContext.Specs.has_flag(Flag::SIGN)) {
            prefix[0] = sign_plus() ? '+' : ' ';
            ++prefixSize;
        }

        switch (type()) {
            case 0:
            case 'd': {
                u32 numDigits = internal::count_digits(absValue);
                format_int(numDigits, Memory_View(prefix, prefixSize),
                           [&](Format_Context &f) { internal::format_uint(f.Out, absValue, numDigits); });
            } break;
            case 'x':
            case 'X': {
                if (alternate()) {
                    prefix[prefixSize++] = '0';
                    prefix[prefixSize++] = (byte) type();
                }
                u32 numDigits = internal::count_digits<4>(absValue);
                format_int(numDigits, Memory_View(prefix, prefixSize), [&](Format_Context &f) {
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
                format_int(numDigits, Memory_View(prefix, prefixSize),
                           [&](Format_Context &f) { internal::format_uint<1>(f.Out, absValue, numDigits); });
            } break;
            case 'o': {
                u32 numDigits = internal::count_digits<3>(absValue);
                if (alternate() && precision() <= (s32) numDigits) {
                    // Octal prefix '0' is counted as a digit, so only add it if precision
                    // is not greater than the number of digits.
                    prefix[prefixSize++] = '0';
                }
                format_int(numDigits, Memory_View(prefix, prefixSize),
                           [&](Format_Context &f) { internal::format_uint<3>(f.Out, absValue, numDigits); });
            } break;
            case 'n': {
                u32 numDigits = internal::count_digits(absValue);
                char32_t sep = internal::thousands_separator();
                byte sepEncoded[4];
                encode_code_point(sepEncoded, sep);

                u32 size = numDigits + 1 * ((numDigits - 1) / 3);
                format_int(size, Memory_View(prefix, prefixSize), [&](Format_Context &f) {
                    internal::format_uint(
                        f.Out, absValue, size,
                        internal::Add_Thousands_Separator{Memory_View(sepEncoded, get_size_of_code_point(sep))});
                });
            } break;
            default:
                // Shouldn't ever get here, since the specs have been checked in the parse stage.
                assert(false);
        }
    }

    // Format an float according to the current argument's format specs.
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>> write_float(T value) {
        char t = (char) type();
        bool upper = t == 'F' || t == 'G' || t == 'E' || t == 'A';
        if (t == 0 || t == 'G') t = 'f';

        byte sign = 0;
        // Check signbit instead of value < 0 because the latter is always false for NaN.
        if (sign_bit(value)) {
            sign = '-';
            value = -value;
        } else if (ParseContext.Specs.has_flag(Flag::SIGN)) {
            sign = sign_plus() ? '+' : ' ';
        }

        // Format NaN and ininity ourselves because sprintf's output is not consistent across platforms.
        if (is_nan((f64) value)) {
            format_padded(
                [&](Format_Context &f) {
                    if (sign) f.Out.append(sign);
                    f.Out.append_cstring(upper ? "NAN" : "nan");
                },
                align(), 3 + (sign ? 1 : 0));
            return;
        }
        if (is_infinity((f64) value)) {
            format_padded(
                [&](Format_Context &f) {
                    if (sign) f.Out.append(sign);
                    f.Out.append_cstring(upper ? "INF" : "inf");
                },
                align(), 3 + (sign ? 1 : 0));
            return;
        }

        Memory_Buffer<30> buffer;
        if (!(sizeof(T) <= sizeof(f64) && t != 'a' && t != 'A' &&
              internal::grisu2_format((f64) value, buffer, ParseContext.Specs))) {
            Format_Specs normalizedSpecs = ParseContext.Specs;
            normalizedSpecs.Type = t;
            internal::sprintf_format(value, buffer, normalizedSpecs);
        }

        size_t n = buffer.ByteLength;
        Alignment alignSpec = align();
        if (alignSpec == Alignment::NUMERIC) {
            if (sign) {
                auto old = ParseContext.Specs;
                ParseContext.Specs = {};
                write_codepoint(sign);
                ParseContext.Specs = old;

                sign = 0;
                if (width()) --ParseContext.Specs.Width;
            }
            alignSpec = Alignment::RIGHT;
        } else {
            if (alignSpec == Alignment::DEFAULT) alignSpec = Alignment::RIGHT;
            if (sign) ++n;
        }

        format_padded(
            [&](Format_Context &f) {
                if (sign) f.Out.append(sign);
                f.Out.append(buffer);
            },
            alignSpec, n);
    }

#define int_helper(x)                                                           \
    if (type() != 'c') {                                                        \
        write_int(x);                                                           \
    } else {                                                                    \
        format_padded([&](Format_Context &f) { f.Out.append(x); }, align(), 1); \
    }

    void write_argument(const Argument &arg) {
        switch (arg.Type) {
            case Format_Type::S32:
                int_helper(arg.Value.S32_Value);
                break;
            case Format_Type::U32:
                int_helper(arg.Value.U32_Value);
                break;
            case Format_Type::S64:
                write_int(arg.Value.S64_Value);
                break;
            case Format_Type::U64:
                write_int(arg.Value.U64_Value);
                break;
            case Format_Type::BOOL: {
                if (type()) {
                    write_int(arg.Value.S32_Value ? 1 : 0);
                } else {
                    write(arg.Value.S32_Value ? "true" : "false");
                }
            } break;
            case Format_Type::F64:
                write_float(arg.Value.F64_Value);
                break;
            case Format_Type::CSTRING:
                if (!type() || type() == 's') {
                    auto strValue = arg.Value.String_Value;
                    if (!strValue.Data) {
                        Out.append_cstring("{String pointer is null}");
                        return;
                    }
                    write(strValue);
                } else if (type() == 'p') {
                    auto oldFlags = ParseContext.Specs.Flags;
                    auto oldType = ParseContext.Specs.Type;

                    ParseContext.Specs.Flags = Flag::HASH;
                    ParseContext.Specs.Type = 'x';
                    write_int((uptr_t) arg.Value.Pointer_Value);
                    ParseContext.Specs.Flags = oldFlags;
                    ParseContext.Specs.Type = oldType;
                }
                break;
            case Format_Type::STRING: {
                auto strValue = arg.Value.String_Value;
                if (!strValue.Data) {
                    Out.append_cstring("{String pointer is null}");
                    return;
                }
                write(strValue);
            } break;
            case Format_Type::POINTER: {
                auto oldFlags = ParseContext.Specs.Flags;
                auto oldType = ParseContext.Specs.Type;

                ParseContext.Specs.Flags = Flag::HASH;
                ParseContext.Specs.Type = 'x';
                write_int((uptr_t) arg.Value.Pointer_Value);
                ParseContext.Specs.Flags = oldFlags;
                ParseContext.Specs.Type = oldType;
            } break;
            case Format_Type::CUSTOM: {
                auto handle = typename Argument::Handle(arg.Value.Custom_Value);
                handle.format(*this);
            } break;
            default:
                assert(false && "Invalid argument type");
        }
    }

#undef int_helper

    // Helper functions to acess format specs more directly
    inline u32 width() { return ParseContext.Specs.Width; }
    inline char32_t fill() { return ParseContext.Specs.Fill; }
    inline Alignment align() { return ParseContext.Specs.Align; }
    inline s32 precision() { return ParseContext.Specs.Precision; }
    inline char32_t type() { return ParseContext.Specs.Type; }

    inline bool sign_plus() { return ParseContext.Specs.has_flag(Flag::PLUS) != 0; }
    inline bool sign_minus() { return ParseContext.Specs.has_flag(Flag::MINUS) != 0; }
    inline bool alternate() { return ParseContext.Specs.has_flag(Flag::HASH) != 0; }
    inline bool sign_aware_zero_pad() { return align() == Alignment::NUMERIC && fill() == '0'; }

   private:
    // Pad according to _spec_.
    // This calls _func_ when it is time to print the padded content.
    // _length_ should be the expected length of the output from calling _func_
    template <typename F>
    void format_padded(F func, Alignment align, size_t length) {
        if (width() <= length) {
            func(*this);
            return;
        }

        size_t padding = width() - length;
        if (align == Alignment::RIGHT) {
            For(range(padding)) Out.append_codepoint(fill());
            func(*this);
        } else if (align == Alignment::CENTER) {
            size_t leftPadding = padding / 2;
            For(range(leftPadding)) Out.append_codepoint(fill());
            func(*this);
            For(range(padding - leftPadding)) Out.append_codepoint(fill());
        } else {
            func(*this);
            For(range(padding)) Out.append_codepoint(fill());
        }
    }

    // Writes an integer in the format
    //   <left-padding><prefix><numeric-padding><digits><right-padding>
    // where <digits> are written by func((Format_Context &) *this).
    template <typename F>
    void format_int(u32 numDigits, const Memory_View &prefix, F func) {
        size_t size = prefix.ByteLength + numDigits;
        char32_t fillChar = fill();
        size_t padding = 0;
        if (align() == Alignment::NUMERIC) {
            if (width() > size) {
                padding = width() - size;
                size = width();
            }
        } else if (precision() > (s32) numDigits) {
            size = prefix.ByteLength + (size_t) precision();
            padding = (size_t) precision() - numDigits;
            fillChar = '0';
        }
        Align_Spec as = ParseContext.Specs;
        if (align() == Alignment::DEFAULT) as.Align = Alignment::RIGHT;
        format_padded(
            [&](Format_Context &f) {
                if (prefix) {
                    f.Out.append(prefix);
                }
                For(range(padding)) f.Out.append_codepoint(fillChar);
                func(f);
            },
            align() == Alignment::DEFAULT ? Alignment::RIGHT : align(), size);
    }

    Format_Context(const Format_Context &) = delete;
    void operator=(const Format_Context &) = delete;
};

// :struct Value in value.h
template <typename T>
void Value::format_custom_arg(const void *arg, Format_Context &f) {
    Formatter<T> formatter;
    formatter.format(*static_cast<const T *>(arg), f);
}

struct Named_Argument_Helper {
    string_view Name;

    template <typename T>
    Named_Argument<T> operator=(T &&value) const {
        return {Name, std::forward<T>(value)};
    }
};
}  // namespace fmt

inline constexpr fmt::Named_Argument_Helper operator"" _a(const char *str, size_t size) { return {{str, size}}; }

LSTD_END_NAMESPACE
