#include "string_view.hpp"

#include "string.hpp"

string_view::string_view(const string &str) : Data(str.Data), ByteLength(str.ByteLength), Length(str.Length) {}
