#include "path.hpp"

LSTD_BEGIN_NAMESPACE

namespace file {

string Path::get_native() const {
#if OS == WINDOWS
    return PathStr.replaced_all("/", "\\");
#else
    return PathStr;
#endif
}

string_view Path::file_name() const {
    if (PathStr.Length == 0) return "";

    size_t last = PathStr.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return PathStr(last + 1, PathStr.Length + (is_pointing_to_content() ? -1 : 0));
}

string_view Path::base_name() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return fileName;
    return fileName(0, last);
}

string_view Path::extension() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return "";
    return fileName(last, fileName.Length);
}

string_view Path::directory() const {
    if (PathStr.Length == 0) return "";

    size_t last = PathStr.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return PathStr(0, last + 1);
}

string_view Path::drive_letter() const {
    if (PathStr.Length < 2) return "";
    if (PathStr[1] == ':') return PathStr(0, 2);
    return "";
}

Path Path::combined_with(const string_view &other) const {
    if (PathStr.Length == 0) return other;
    if (other.Length == 0) return *this;
    if (other[0] == '/') return other;
#if OS == WINDOWS
    if (other.Length > 1 && other[1] == ':') return other;
#endif
    return PathStr + (is_pointing_to_content() ? "" : "/") + other;
}

void Path::resolve() {
    size_t dots, beginning = 0;
    while ((dots = PathStr.find("../", beginning)) != npos) {
        if (dots != beginning) break;
        beginning += 3;
    }

    size_t progress = beginning;
    while ((dots = PathStr.find("..", progress)) != npos) {
        size_t previousSlash = PathStr.find_reverse('/', dots - 2);
        PathStr.remove(previousSlash, dots + 2);
        progress = previousSlash + 1;
        if (progress >= PathStr.Length) break;
    }

    progress = beginning;
    while ((dots = PathStr.find("./", progress)) != npos) {
        PathStr.remove(dots, dots + 2);
        progress = dots + 1;
        if (progress >= PathStr.Length) break;
    }
}
}  // namespace file

LSTD_END_NAMESPACE