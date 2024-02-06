#pragma once

#include "common.h"
#include "string.h"

LSTD_BEGIN_NAMESPACE

//
// Provides a way to write types and bytes with a simple extension API.
// Subclasses of this stuct override the write/flush methods depending on the
// output (console, files, buffers, etc.) Types are written with the _write_
// overloads outside of this struct.
//
struct writer {
  virtual void write(const char *data, s64 count) = 0;
  virtual void flush() {}
};

// For printing and formatting more types use fmt.h.
inline void write(writer *w, string str) { w->write(str.Data, str.Count); }

inline void write(writer *w, const char *data, s64 size) {
  w->write(data, size);
}

inline void write(writer *w, code_point cp) {
  char data[4];
  utf8_encode_cp(data, cp);
  w->write(data, utf8_get_size_of_cp(data));
}

inline void flush(writer *w) { w->flush(); }

//
// Doesn't do anything but count how much bytes would have been written to it.
// E.g. used in fmt.h to calculate formatted length.
//
struct counting_writer : writer {
  s64 Count = 0;

  void write(const char *data, s64 count) override { Count += count; }
  void flush() override {}
};

//
// Output to the console (this might be OS-specific)
//

struct console : writer {
  // By default, we are thread-safe.
  // If you don't use seperate threads and aim for maximum console output
  // performance, set this to false.
  bool LockMutex = true;

  enum output_type { COUT, CERR };
  output_type OutputType;

  // Used to keep track where in the buffer we are
  char *Buffer = null, *Current = null;
  s64 Available = 0, BufferSize = 0;

  console(output_type type) : OutputType(type) {}

  void write(const char *data, s64 size) override;
  void flush() override;
};

inline auto cout = console(console::COUT);
inline auto cerr = console(console::CERR);

LSTD_END_NAMESPACE
