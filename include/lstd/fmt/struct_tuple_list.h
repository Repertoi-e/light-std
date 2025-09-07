#pragma once

#include "context.h"

LSTD_BEGIN_NAMESPACE

//
// Global helper function for context-aware spec forwarding
// Can be used by any custom formatter to check if specs should be forwarded to a specific type
//
template <typename T>
bool should_forward_specs_to_type(const fmt_dynamic_specs &specs, const T &value) {
  // If no type specifier, forward based on value type (context-aware)
  if (!specs.Type) {
    return true;  // Default: forward precision/sign/etc to all types
  }
  
  // Type-specific forwarding - only forward if the type specifier matches the value type
  if constexpr (is_floating_point<remove_cvref_t<T>>) {
    // Float types: only forward float-specific specifiers
    return specs.Type == 'f' || specs.Type == 'F' || 
           specs.Type == 'g' || specs.Type == 'G' ||
           specs.Type == 'e' || specs.Type == 'E' || 
           specs.Type == '%';
  } else if constexpr (is_integral<remove_cvref_t<T>>) {
    // Integer types: only forward integer-specific specifiers
    return specs.Type == 'd' || specs.Type == 'x' || specs.Type == 'X' ||
           specs.Type == 'o' || specs.Type == 'b' || specs.Type == 'B' ||
           specs.Type == 'c' || specs.Type == 'n';
  } else if constexpr (is_same<remove_cvref_t<T>, string> || is_same<remove_cvref_t<T>, const char*>) {
    // String types: only forward string-specific specifiers
    return specs.Type == 's' || specs.Type == 'q' || specs.Type == 'p';
  } else if constexpr (is_pointer<remove_cvref_t<T>>) {
    // Pointer types: only forward pointer specifiers
    return specs.Type == 'p';
  }
  
  // For other types, don't forward type-specific specs
  return false;
}

//
// Helper function to create safe specs without incompatible type specifiers
//
inline fmt_dynamic_specs create_safe_specs(const fmt_dynamic_specs &original) {
  auto safe_specs = original;
  safe_specs.Type = 0;        // Clear incompatible type specifier
  safe_specs.Precision = -1;  // Clear precision that was meant for specific type
  safe_specs.Fill = ' ';      // Reset fill to default
  safe_specs.Width = 0;       // Clear width
  safe_specs.UserData = 0;    // Clear indentation data
  return safe_specs;
}

//
// Global helper function to create forwarded specs with container-specific formatting cleared
//
inline fmt_dynamic_specs create_forwarded_specs(const fmt_dynamic_specs &original) {
  auto forwarded = original;
  // Clear container-specific formatting but keep value-specific specs
  forwarded.Fill = ' ';      // Reset fill to default
  forwarded.Width = 0;       // Clear width (no pretty-printing for values)
  forwarded.UserData = 0;    // Clear indentation data
  return forwarded;
}

//
// Templated version that considers value type for safe forwarding
//
template <typename T>
fmt_dynamic_specs create_forwarded_specs(const fmt_dynamic_specs &original, const T &value) {
  if (should_forward_specs_to_type(original, value)) {
    // Compatible type specifier - forward everything
    return create_forwarded_specs(original);  // Use the non-templated version
  } else {
    // Incompatible type specifier - use safe specs
    return create_safe_specs(original);
  }
}

// Compute forwarded specs for a dynamic argument (fmt_arg) by visiting its concrete type
inline fmt_dynamic_specs forwarded_specs_for_arg(const fmt_dynamic_specs &original, fmt_arg ar) {
  return fmt_visit_arg([&](auto v) -> fmt_dynamic_specs {
    using VT = decltype(v);
    if constexpr (is_same<VT, fmt_custom_value>) {
      // Preserve type-specific selectors for custom values (e.g., variant)
      // so the nested formatter can perform the correct type-based forwarding.
      return create_forwarded_specs(original);
    } else {
      return create_forwarded_specs(original, v);
    }
  }, ar);
}

//
// Pretty-printing function for hash tables (moved from fmt.h)
//
template <typename T>
void format_dict_pretty(fmt_context *f, s32 indent_size, s32 current_level, const T &table) {
  // Need to cast away const to iterate since hash table iterators expect non-const
  auto &mutable_table = const_cast<T &>(table);
  
  if (table.Count == 0) {
    write_no_specs(f, "{}");
    return;
  }
  
  write_no_specs(f, "{\n");
  
  // Store original specs to forward to values
  auto original_specs = f->Specs;
  
  bool first = true;
  for (auto [key, value] : mutable_table) {
    if (!first) {
      write_no_specs(f, ",\n");
    }
    first = false;
    
    // Write indentation for this level
    for (s32 i = 0; i < (current_level + 1) * indent_size; ++i) {
      write_no_specs(f, " ");
    }
    
    // Format key: forward specs if they're type-appropriate
    if (original_specs) {
      auto key_specs = create_forwarded_specs(*original_specs, *key);
      f->Specs = &key_specs;
      format_value(*key, f);
    } else {
      f->Specs = nullptr;
      format_value(*key, f);
    }
    
    write_no_specs(f, ": ");
    
    // Format value: forward specs appropriately
    if constexpr (any_hash_table<remove_cvref_t<decltype(*value)>>) {
      // For nested hash tables, keep pretty-printing but increment level
      fmt_dynamic_specs nested_specs;
      if (original_specs) {
        nested_specs = *original_specs;
        nested_specs.UserData = current_level + 1; // Use UserData for nesting level
        f->Specs = &nested_specs;
      } else {
        f->Specs = nullptr;
      }
      format_value(*value, f);
    } else {
      // For non-containers, forward specs if type-appropriate
      if (original_specs) {
        auto value_specs = create_forwarded_specs(*original_specs, *value);
        f->Specs = &value_specs;
        format_value(*value, f);
      } else {
        f->Specs = nullptr;
        format_value(*value, f);
      }
    }
  }
  
  // Restore original specs
  f->Specs = original_specs;
  
  write_no_specs(f, "\n");
  
  // Write indentation for closing brace
  for (s32 i = 0; i < current_level * indent_size; ++i) {
    write_no_specs(f, " ");
  }
  write_no_specs(f, "}");
}

//
// The following three classes are used to quickly collect elements and then
// output them in a pretty way.
//
// e.g. usage for a custom quaternion formatter:
//     ...
//     fmt_tuple(f,
//     "quat").field(src.s)->field(src.i)->field(src.j)->field(src.k)->finish();
// Outputs: "quat(1.00, 2.00, 3.00, 4.00)"
//
// These are inspired by Rust's API
//

// Outputs in the following format: *name* { field1: value, field2: value, ... }
// e.g.     vector3(x: 1.00, y: 4.00, z: 9.00)
struct format_struct
{
  struct field_entry
  {
    string Name;
    fmt_arg Arg;
  };

  fmt_context *F;
  string Name;
  array<field_entry> Fields;
  bool NoSpecs; // Write the result without taking into account specs for
                // individual arguments

  format_struct(fmt_context *f, string name, bool noSpecs = false)
      : F(f), Name(name), NoSpecs(noSpecs) {}

  // I know we are against hidden freeing but having this destructor is fine
  // because it helps with code conciseness.
  ~format_struct() { free(Fields); }

  template <typename T>
  format_struct *field(string name, const T &value)
  {
    add(Fields, {name, fmt_make_arg(value)});
    return this;
  }

  void finish();
};

// Outputs in the following format: *name*(element1, element2, ...)
// e.g.     read_file_result("Hello world!", true)
struct format_tuple
{
  fmt_context *F;
  string Name;
  array<fmt_arg> Fields;
  bool NoSpecs; // Write the result without taking into account specs for
                // individual arguments

  format_tuple(fmt_context *f, string name, bool noSpecs = false)
      : F(f), Name(name), NoSpecs(noSpecs) {}

  // I know we are against hidden freeing but having this destructor is fine
  // because it helps with code conciseness.
  ~format_tuple() { free(Fields); }

  template <typename T>
  format_tuple *field(const T &value)
  {
    add(Fields, fmt_make_arg(value));
    return this;
  }

  void finish();
};

// Outputs in the following format: [element1, element2, ...]
// e.g.     ["This", "is", "an", "array", "of", "strings"]
struct format_list
{
  fmt_context *F;
  array<fmt_arg> Fields;
  bool NoSpecs; // Write the result without taking into account specs for
                // individual arguments

  format_list(fmt_context *f, bool noSpecs = false) : F(f), NoSpecs(noSpecs) {}

  // I know we are against hidden freeing but having this destructor is fine
  // because it helps with code conciseness.
  ~format_list() { free(Fields); }

  template <typename T>
  format_list *entries(array<T> values)
  {
    For(values) add(Fields, fmt_make_arg(it));
    return this;
  }

  template <typename T>
  format_list *entries(T *begin, T *end)
  {
    return entries(array<T>(begin, end - begin));
  }

  template <typename T>
  format_list *entries(T *begin, s64 count)
  {
    return entries(array<T>(begin, count));
  }

  void finish();
};

// Outputs in the following format: { key1: value1, key2: value2, ... }
// e.g.     { "name": "John", "age": 30, "city": "New York" }
struct format_dict
{
  struct key_value_entry
  {
    fmt_arg Key;
    fmt_arg Value;
  };

  fmt_context *F;
  array<key_value_entry> Fields;
  bool NoSpecs; // Write the result without taking into account specs for
                // individual arguments
  // Pretty-printing options
  bool Pretty = false;
  s32 IndentSize = 0;
  s32 CurrentLevel = 0;

  format_dict(fmt_context *f, bool noSpecs = false) : F(f), NoSpecs(noSpecs) {}

  // I know we are against hidden freeing but having this destructor is fine
  // because it helps with code conciseness.
  ~format_dict() { free(Fields); }

  template <typename K, typename V>
  format_dict *entry(const K &key, const V &value)
  {
    add(Fields, {fmt_make_arg(key), fmt_make_arg(value)});
    return this;
  }

  // Enable pretty printing with a given indent size and optional starting level
  inline format_dict *pretty(s32 indentSize, s32 currentLevel = 0) {
    Pretty = true;
    IndentSize = indentSize;
    CurrentLevel = currentLevel;
    return this;
  }

  void finish();
};

inline void format_struct::finish()
{
  auto write_field = [&](field_entry *entry)
  {
    write_no_specs(F, entry->Name);
    write_no_specs(F, ": ");
    
    // Store original specs for type-aware forwarding
    fmt_dynamic_specs *original_specs = F->Specs;
    if (original_specs) {
      auto field_specs = forwarded_specs_for_arg(*original_specs, entry->Arg);
      F->Specs = &field_specs;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), entry->Arg);
    } else {
      F->Specs = nullptr;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), entry->Arg);
    }
    
    // Restore original specs
    F->Specs = original_specs;
  };

  write_no_specs(F, Name);
  write_no_specs(F, " {");

  auto *p = Fields.Data;
  auto *end = Fields.Data + Fields.Count;

  if (p != end)
  {
    write_no_specs(F, " ");
    write_field(p);
    ++p;
    while (p != end)
    {
      write_no_specs(F, ", ");
      write_field(p);
      ++p;
    }
  }
  write_no_specs(F, " }");
}

inline void format_tuple::finish()
{
  write_no_specs(F, Name);
  write_no_specs(F, "(");

  auto *p = Fields.Data;
  auto *end = Fields.Data + Fields.Count;

  // Store original specs for type-aware forwarding
  fmt_dynamic_specs *original_specs = F->Specs;

  if (p != end)
  {
    if (original_specs) {
      auto element_specs = forwarded_specs_for_arg(*original_specs, *p);
      F->Specs = &element_specs;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
    } else {
      F->Specs = nullptr;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
    }
    ++p;
    
    while (p != end)
    {
      write_no_specs(F, ", ");
      if (original_specs) {
        auto element_specs = forwarded_specs_for_arg(*original_specs, *p);
        F->Specs = &element_specs;
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
      } else {
        F->Specs = nullptr;
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
      }
      ++p;
    }
  }
  
  // Restore original specs
  F->Specs = original_specs;
  
  write_no_specs(F, ")");
}

inline void format_list::finish()
{
  write_no_specs(F, "[");

  auto *p = Fields.Data;
  auto *end = Fields.Data + Fields.Count;

  // Store original specs for type-aware forwarding
  fmt_dynamic_specs *original_specs = F->Specs;

  if (p != end)
  {
    if (original_specs) {
      auto element_specs = forwarded_specs_for_arg(*original_specs, *p);
      F->Specs = &element_specs;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
    } else {
      F->Specs = nullptr;
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
    }
    ++p;
    
    while (p != end)
    {
      write_no_specs(F, ", ");
      if (original_specs) {
        auto element_specs = forwarded_specs_for_arg(*original_specs, *p);
        F->Specs = &element_specs;
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
      } else {
        F->Specs = nullptr;
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), *p);
      }
      ++p;
    }
  }
  
  // Restore original specs
  F->Specs = original_specs;
  
  write_no_specs(F, "]");
}

inline void format_dict::finish()
{
  // Store original specs for type-aware forwarding
  fmt_dynamic_specs *original_specs = F->Specs;

  if (!Pretty) {
    // Compact inline formatting
    write_no_specs(F, "{");

    auto *p = Fields.Data;
    auto *end = Fields.Data + Fields.Count;

    if (p != end)
    {
      write_no_specs(F, " ");

      // Key specs: compute based on key's concrete type
      fmt_dynamic_specs key_specs;
      if (original_specs) {
        key_specs = forwarded_specs_for_arg(*original_specs, p->Key);
        F->Specs = &key_specs;
      } else {
        F->Specs = nullptr;
      }
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Key);
      write_no_specs(F, ": ");

      // Value specs
      fmt_dynamic_specs value_specs;
      if (original_specs) {
        value_specs = forwarded_specs_for_arg(*original_specs, p->Value);
        F->Specs = &value_specs;
      } else {
        F->Specs = nullptr;
      }
      fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Value);
      ++p;

      while (p != end)
      {
        write_no_specs(F, ", ");

        // Key
        if (original_specs) {
          key_specs = forwarded_specs_for_arg(*original_specs, p->Key);
          F->Specs = &key_specs;
        } else {
          F->Specs = nullptr;
        }
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Key);
        write_no_specs(F, ": ");

        // Value
        if (original_specs) {
          value_specs = forwarded_specs_for_arg(*original_specs, p->Value);
          F->Specs = &value_specs;
        } else {
          F->Specs = nullptr;
        }
        fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Value);
        ++p;
      }

      // Restore original specs
      F->Specs = original_specs;
      write_no_specs(F, " ");
    }
    write_no_specs(F, "}");
    return;
  }

  // Pretty-printing with indentation
  if (Fields.Count == 0) {
    write_no_specs(F, "{}");
    return;
  }

  write_no_specs(F, "{\n");

  auto *p = Fields.Data;
  auto *end = Fields.Data + Fields.Count;
  bool first = true;
  while (p != end) {
    if (!first) write_no_specs(F, ",\n");
    first = false;
    for (s32 i = 0; i < (CurrentLevel + 1) * IndentSize; ++i) write_no_specs(F, " ");

    // Key specs
    fmt_dynamic_specs key_specs;
    if (original_specs) {
      key_specs = forwarded_specs_for_arg(*original_specs, p->Key);
      F->Specs = &key_specs;
    } else {
      F->Specs = nullptr;
    }
    fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Key);
    write_no_specs(F, ": ");

    // Value specs, propagate pretty indent to nested containers
    fmt_dynamic_specs value_specs;
    if (original_specs) {
      value_specs = forwarded_specs_for_arg(*original_specs, p->Value);
      value_specs.UserData = CurrentLevel + 1;
      value_specs.Width = IndentSize; // pass indent size down
      F->Specs = &value_specs;
    } else {
      F->Specs = nullptr;
    }
    fmt_visit_arg(fmt_context_visitor(F, NoSpecs), p->Value);
    ++p;
  }

  // Restore original specs
  F->Specs = original_specs;

  write_no_specs(F, "\n");
  for (s32 i = 0; i < CurrentLevel * IndentSize; ++i) write_no_specs(F, " ");
  write_no_specs(F, "}");
}

LSTD_END_NAMESPACE
