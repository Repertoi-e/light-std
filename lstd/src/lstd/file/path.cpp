#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace file {

string path::file_name() const {
    if (Str.Length == 0) return "";

    s64 last = find_reverse_any_of(Str, OS_PATH_SEPARATORS, is_pointing_to_content() ? -2 : 0);
    if (last == -1) return "";
    return substring(Str, last + 1, Str.Length + (is_pointing_to_content() ? -1 : 0));
}

string path::base_name() const {
    auto fileName = file_name();
    s64 last = find_cp_reverse(fileName , '.');
    if (last == -1) return fileName;
    return substring(fileName, 0, last);
}

string path::extension() const {
    auto fileName = file_name();
    s64 last = find_cp_reverse(fileName , '.');
    if (last == -1) return "";
    return substring(fileName, last, fileName.Length);
}

string path::directory() const {
    if (Str.Length == 0) return "";

    s64 last = find_reverse_any_of(Str, OS_PATH_SEPARATORS, is_pointing_to_content() ? -2 : 0);
    if (last == -1) return "";
    return substring(Str, 0, last + 1);
}

string path::drive_letter() const {
    if (Str.Length < 2) return "";
    if (Str[1] == ':') return substring(Str, 0, 2);
    return "";
}

void path::combine_with(const string &str) {
    if (!Reserved) {
        string old = Str;
        Str = "";
        append_string(Str, old);
        Reserved = true;
    }

    auto other = path(str);
    if (!is_pointing_to_content()) {
        append_cp(Str, '/');
    }
    if (other.is_absolute()) {
        Str.Count = Str.Length = 0;
    }
    append_string(Str, str);
}

string path::resolved() const {
    string result;
    clone(&result, Str);

    s64 dots, beginning = 0;
    while ((dots = find_substring(result, "..", beginning)) != -1) {
        if (dots + 2 >= result.Length) break;  // Invalid path

        auto slash = result[dots + 2];
        if (slash != '/' && slash != '\\') {
            beginning += 3;
            continue;
        }

        if (dots != beginning) break;
        beginning += 3;
        if (beginning == result.Length) return result;
    }
    if (beginning == result.Length) return result;

    s64 progress = beginning;
    while ((dots = find_substring(result, "..", progress)) != -1) {
        s64 previousSlash = find_reverse_any_of(result, OS_PATH_SEPARATORS, dots - 2);
        remove_range(result, previousSlash, dots + 2);
        progress = previousSlash + 1;
        if (progress >= result.Length) break;
    }

    progress = beginning;
    while ((dots = find_cp(result, '.', progress)) != -1) {
        if (dots + 1 >= result.Length) break;  // Invalid path

        auto next = result[dots + 1];
        if (next != '/' && next != '\\') {
            progress = dots + 1;
            continue;
        }

        remove_range(result, dots, dots + 2);
        progress = dots + 1;
        if (progress >= result.Length) break;
    }
    return result;
}
}  // namespace file

file::path *clone(file::path *dest, const file::path &src) {
    dest->release();
    clone(&dest->Str, src.Str);
    dest->Reserved = true;
    return dest;
}

LSTD_END_NAMESPACE