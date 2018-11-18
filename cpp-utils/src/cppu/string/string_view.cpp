#include "string_view.h"

#include "string.h"

string_view::string_view(const string &str) : Data(str.Data), BytesUsed(str.BytesUsed), Length(str.Length) {}
