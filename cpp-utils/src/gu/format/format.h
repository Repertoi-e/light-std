#pragma once

#include "../common.h"
#include "../context.h"
#include "../string/string.h"

#include <type_traits>

GU_BEGIN_NAMESPACE

namespace fmt {

struct Format_Context {};

template <typename T>
struct Formatter {
    static_assert(false, "There's no Formatter<T> specialization for this type.");

    // template <typename ParseContext>
    // typename ParseContext::iterator parse(ParseContext &);
    //
    // template <typename FormatContext>
    // auto format(const T &value, FormatContext &context) -> decltype(context.out());
};

// template <typename T, typename Char, typename Enable = void>
// struct Is_Convertible_To_Int : std::integral_constant<b32, !std::is_arithmetic_v<T> && std::is_convertible_v<T, s32>>
// {};

enum class Format_Type {
    NONE = 0,
    NAMED_ARGUMENT,

    // Integers
    S32,
    U32,
    S64,
    U64,
    BOOL,
    CHAR,
    LAST_INTEGER_TYPE = CHAR,

    // Floating-point
    DOUBLE,
    LAST_NUMERIC_TYPE = DOUBLE,

    CSTRING,
    STRING,
    POINTER,
    CUSTOM
};

constexpr b32 is_type_integral(Format_Type type) {
    assert(type != Format_Type::NAMED_ARGUMENT);
    return type > Format_Type::NONE && type <= Format_Type::LAST_INTEGER_TYPE;
}

constexpr b32 is_type_arithmetic(Format_Type type) {
    assert(type != Format_Type::NAMED_ARGUMENT);
    return type > Format_Type::NONE && type <= Format_Type::LAST_NUMERIC_TYPE;
}

struct String_Value {
    const char *Data;
    size_t Size;
};

struct Custom_Value {
    const void *Data;
    void (*Format)(const void *arg, Format_Context &context);
};

class Value {
   public:
    union {
        s32 S32_Value;
        u32 U32_Value;
        s64 S64_Value;
        u64 U64_Value;
        f64 F64_Value;
        const void *Pointer_Value;
        String_Value String_Value;
        Custom_Value Custom_Value;
    };

    constexpr Value(s32 value = 0) : S32_Value(value) {}
    Value(u32 value) { U32_Value = value; }
    Value(s64 value) { S64_Value = value; }
    Value(u64 value) { U64_Value = value; }
    Value(f64 value) { F64_Value = value; }
    Value(const char *value) {
        String_Value.Data = value;
        String_Value.Size = cstring_strlen(value);
    }

    Value(const string_view &value) {
        String_Value.Data = value.Data;
        String_Value.Size = value.BytesUsed;
    }
    Value(const void *value) { Pointer_Value = value; }

    template <typename T>
    explicit Value(const T &value) {
        Custom_Value.Data = &value;
        Custom_Value.Format = &format_custom_arg<T>;
    }

    const Named_Argument_Base &as_named_arg() { return *static_cast<const Named_Argument_Base *>(Pointer_Value); }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, Format_Context &context) {
        Formatter<T> f;
        auto &&parseContext = context.parse_context();
        parseContext.advance_to(f.parse(parseContext));
        context.advance_to(f.format(*static_cast<const T *>(arg), context));
    }
};

// Value initializer used to delay conversion to value and reduce memory churn.
template <typename T, Format_Type Type>
struct Init_Value {
    static const Format_Type type_tag = Type;

    T _Value;

    constexpr Init_Value(const T &value) : _Value(value) {}
    constexpr operator Value() const { return Value(_Value); }
};

#define MAKE_VALUE_HELPER(TAG, ArgType, ValueType) \
    constexpr Init_Value<ValueType, TAG> make_value(ArgType value) { return static_cast<ValueType>(value); }

#define MAKE_VALUE_SAME_HELPER(TAG, Type) \
    constexpr Init_Value<Type, TAG> make_value(Type value) { return value; }

MAKE_VALUE_HELPER(Format_Type::BOOL, bool, s32)
MAKE_VALUE_HELPER(Format_Type::S32, s16, s32)
MAKE_VALUE_HELPER(Format_Type::U32, u16, u32)
MAKE_VALUE_SAME_HELPER(Format_Type::S32, s32)
MAKE_VALUE_SAME_HELPER(Format_Type::U32, u32)

// To minimize the number of types we need to deal with, long is translated
// either to int or to long long depending on its size.
using long_type = std::conditional_t<sizeof(long) == sizeof(s32), s32, s64>;
MAKE_VALUE_HELPER((sizeof(long) == sizeof(s32) ? Format_Type::S32 : Format_Type::S64), long, long_type)
using ulong_type = std::conditional_t<sizeof(unsigned long) == sizeof(u32), u32, u64>;
MAKE_VALUE_HELPER((sizeof(unsigned long) == sizeof(u32) ? Format_Type::U32 : Format_Type::U64), unsigned long,
                  ulong_type)

MAKE_VALUE_SAME_HELPER(Format_Type::S64, s64)
MAKE_VALUE_SAME_HELPER(Format_Type::U64, u64)
MAKE_VALUE_HELPER(Format_Type::S32, s8, s32)
MAKE_VALUE_HELPER(Format_Type::U32, u8, u32)
MAKE_VALUE_HELPER(Format_Type::CHAR, char32_t, s32)

MAKE_VALUE_HELPER(Format_Type::DOUBLE, f32, f64)
MAKE_VALUE_SAME_HELPER(Format_Type::DOUBLE, f64)

///
/// CONTINUE HERE
///

MAKE_VALUE_HELPER(Format_Type::CSTRING, char *, const char *);
MAKE_VALUE_SAME_HELPER(Format_Type::CSTRING, const char *);
MAKE_VALUE_SAME_HELPER(Format_Type::STRING, string_view);
MAKE_VALUE_HELPER(Format_Type::STRING, const string &, string_view);

MAKE_VALUE_HELPER(Format_Type::POINTER, void *, const void *);
MAKE_VALUE_SAME_HELPER(Format_Type::POINTER, const void *)

MAKE_VALUE_HELPER(pointer_type, std::nullptr_t, const void *)

template <typename T>
typename std::enable_if_t<!std::is_same_v<T, char>> make_value(const T *) {
    static_assert(!sizeof(T), "Formatting of non-void pointers is not allowed");
}

template <typename T>
inline typename std::enable_if_t<std::is_enum_v<T> && !std::is_arithmetic_v<T> && std::is_convertible_v<T, s32>,
                                 Init_Value<s32, Format_Type::S32>>
make_value(const T &value) {
    return static_cast<int>(value);
}

template <typename T>
inline typename std::enable_if_t<std::is_constructible_v<string_view, T>, Init_Value<string_view, Format_Type::STRING>>
make_value(const T &value) {
    return string_view(value);
}

template <typename T>
inline typename std::enable_if_t<std::is_arithmetic_v<T> || !std::is_convertible_v<T, s32> &&
                                                                !std::is_convertible_v<T, string_view> &&
                                                                !std::is_constructible_v<string_view, T>,
                                 // Implicit conversion to string is not handled here
                                 Init_Value<const T &, Format_Type::CUSTOM>>
make_value(const T &value) {
    return value;
}

template <typename T>
Init_Value<const void *, Format_Type::NAMED_ARGUMENT> make_value(const Named_Argument<T> &value) {
    Basic_Format_Argument arg = make_argument(value.Value);
    CopyMemory(value.Data, &arg, sizeof(Basic_Format_Argument));
    return (const void *) (&value);
}

struct Format_Arguments;

struct Basic_Format_Argument {
    Value _Value;
    Format_Type _Type = Format_Type::NONE;

    struct Handle {
        Custom_Value _Custom;

        explicit Handle(Custom_Value custom) : _Custom(custom) {}
        void format(Format_Context &context) const { _Custom.Format(_Custom.Data, context); }
    };

    explicit operator bool() const { return _Type != Format_Type::NONE; }

    b32 is_integral() const { return is_type_integral(_Type); }
    b32 is_arithmetic() const { return is_type_arithmetic(_Type); }
};

template <typename T>
constexpr Basic_Format_Argument make_argument(const T &value) {
    Basic_Format_Argument arg;
    arg._Type = _Get_Type<T>::value;
    arg._Value = make_value(value);
    return arg;
}

template <bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if_t<IS_PACKED, Value> make_argument(const T &value) {
    return make_value(value);
}

template <bool IS_PACKED, typename Context, typename T>
inline typename std::enable_if_t<!IS_PACKED, Basic_Format_Argument> make_argument(const T &value) {
    return make_argument(value);
}

struct Named_Argument_Base;

// A map from argument names to their values for named arguments.
struct Argument_Map {
    Argument_Map(const Argument_Map &) = delete;
    void operator=(const Argument_Map &) = delete;

    struct Entry {
        string_view Name;
        Basic_Format_Argument Arg;
    };

    Entry *_Entries = null;
    u32 _Size = 0;

    ~Argument_Map() { delete[] _Entries; }

    void init(const Format_Arguments &args);

    void add(Value value) {
        const Named_Argument_Base &named = value.as_named_arg();
        _Entries[_Size] = Entry{named.Name, named.deserialize()};
        ++_Size;
    }

    Basic_Format_Argument find(const string_view &name) const {
        // The list is unsorted, so just return the first matching name.
        for (Entry *it = _Entries, *end = _Entries + _Size; it != end; ++it) {
            if (it->Name == name) return it->Arg;
        }
        return {};
    }
};

constexpr auto MAX_PACKED_ARGS = 15;

template <typename... Args>
struct Format_Arguments_Store {
    static const size_t NUM_ARGS = sizeof...(Args);
    static const b32 IS_PACKED = NUM_ARGS < MAX_PACKED_ARGS;

    using value_type = typename std::conditional_t<IS_PACKED, Value, Basic_Format_Argument>;

    // If the arguments are not packed, add one more element to mark the end.
    static const size_t DATA_SIZE = NUM_ARGS + (IS_PACKED && NUM_ARGS != 0 ? 0 : 1);
    value_type _Data[DATA_SIZE];

    template <typename T>
    struct _Get_Type {
        using value_type = decltype(make_value(std::declval<typename std::decay_t<T> &>()));
        static const Format_Type value = value_type::type_tag;
    };

    constexpr u64 _get_types() { return 0ull; }

    template <typename Arg, typename... Args>
    constexpr u64 _get_types() {
        return _Get_Type<Arg>::value | (_get_types<Args...>() << 4);
    }
    static constexpr s64 _get_types() { return IS_PACKED ? (s64)(_get_types<Args...>()) : -(s64)(NUM_ARGS); }

    static constexpr s64 TYPES = _get_types();

    Format_Arguments_Store(const Args &... args) : _Data{make_argument<IS_PACKED>(args)...} {}
};

struct Format_Arguments {
    // To reduce compiled code size per formatting function call, types of first
    // MAX_PACKED_ARGS arguments are passed in the _Types field.
    u64 _Types = 0;
    union {
        const Value *_Values;
        const Basic_Format_Argument *_Args;
    };

    Format_Type type(u32 index) const {
        u32 shift = index * 4;
        u64 mask = 0xf;
        return (Format_Type)((_Types & (mask << shift)) >> shift);
    }

    // void set_data(const Value *values) { _Values = values; }
    // void set_data(const Basic_Format_Argument *args) { _Args = args; }

    Basic_Format_Argument do_get(u32 index) const {
        s64 signedTypes = (s64) _Types;
        if (signedTypes < 0) {
            u64 numArgs = (u64)(-signedTypes);
            if (index < numArgs) {
                return _Args[index];
            }
        }
        if (index > MAX_PACKED_ARGS) {
            return {};
        }

        Basic_Format_Argument result;
        result._Type = type(index);
        if (result._Type == Format_Type::NONE) {
            return result;
        }
        Value &value = result._Value;
        value = _Values[index];
        return result;
    }

    template <typename... Args>
    Format_Arguments(const Format_Arguments_Store<Args...> &store)
        : _Types((u64) store.TYPES), _Args(store._Data) {}

    Format_Arguments(const Basic_Format_Argument *args, u32 count) : _Types(-((s64) count)) { _Args = args; }

    Basic_Format_Argument get(u32 index) const {
        Basic_Format_Argument arg = do_get(index);
        if (arg._Type == Format_Type::NAMED_ARGUMENT) {
            arg = arg._Value.as_named_arg().deserialize();
        }
        return arg;
    }

    u32 max_size() const {
        s64 signedTypes = (s64) _Types;
        return (u32)(signedTypes < 0 ? -signedTypes : (s64) MAX_PACKED_ARGS);
    }
};

struct Named_Argument_Base {
    string_view Name;
    mutable char Data[sizeof(Basic_Format_Argument)];

    Named_Argument_Base(const string_view &name) : Name(name) {}

    Basic_Format_Argument deserialize() const {
        Basic_Format_Argument result;
        CopyMemory(&result, Data, sizeof(Basic_Format_Argument));
        return result;
    }
};

template <typename T>
struct Named_Argument : Named_Argument_Base {
    const T &Value;

    Named_Argument(const string_view &name, const T &value) : Named_Argument_Base(name), Value(value) {}
};
}  // namespace fmt

GU_END_NAMESPACE
