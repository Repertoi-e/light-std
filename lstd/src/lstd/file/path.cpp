#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace file {

string path::file_name() const {
    if (Str.Length == 0) return "";

    s64 last = Str.find_reverse_any_of(OS_PATH_SEPARATORS, is_pointing_to_content() ? -2 : 0);
    if (last == -1) return "";
    return Str.substring(last + 1, Str.Length + (is_pointing_to_content() ? -1 : 0));
}

string path::base_name() const {
    auto fileName = file_name();
    s64 last = fileName.find_reverse('.');
    if (last == -1) return fileName;
    return fileName.substring(0, last);
}

string path::extension() const {
    auto fileName = file_name();
    s64 last = fileName.find_reverse('.');
    if (last == -1) return "";
    return fileName.substring(last, fileName.Length);
}

string path::directory() const {
    if (Str.Length == 0) return "";

    s64 last = Str.find_reverse_any_of(OS_PATH_SEPARATORS, is_pointing_to_content() ? -2 : 0);
    if (last == -1) return "";
    return Str.substring(0, last + 1);
}

string path::drive_letter() const {
    if (Str.Length < 2) return "";
    if (Str[1] == ':') return Str.substring(0, 2);
    return "";
}

void path::combine_with(const string &str) {
    if (!Reserved) {
        string old = Str;
        Str = "";
        Str.append(old);
        Reserved = true;
    }

    auto other = path(str);
    if (!is_pointing_to_content()) {
        Str.append("/");
    }
    if (other.is_absolute()) {
        Str.ByteLength = Str.Length = 0;
    }
    Str.append(str);
}

string path::resolved() const {
    string result;
    clone(&result, Str);

    s64 dots, beginning = 0;
    while ((dots = result.find("..", beginning)) != -1) {
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
    while ((dots = result.find("..", progress)) != -1) {
        s64 previousSlash = result.find_reverse_any_of(OS_PATH_SEPARATORS, dots - 2);
        result.remove(previousSlash, dots + 2);
        progress = previousSlash + 1;
        if (progress >= result.Length) break;
    }

    progress = beginning;
    while ((dots = result.find(".", progress)) != -1) {
        if (dots + 1 >= result.Length) break;  // Invalid path

        auto next = result[dots + 1];
        if (next != '/' && next != '\\') {
            progress = dots + 1;
            continue;
        }

        result.remove(dots, dots + 2);
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

// file::path *move(file::path *dest, file::path *src) {
//     dest->UnifiedPath.release();
//     move(&dest->UnifiedPath, &src->UnifiedPath);
//     return dest;
// }

LSTD_END_NAMESPACE