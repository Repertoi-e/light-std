#include "path.hpp"

LSTD_BEGIN_NAMESPACE

namespace file {

path &path::operator=(const memory_view &other) {
    _Path = other;
    return *this;
}

string path::get_native() const {
#if OS == WINDOWS
    return _Path.replaced_all("/", "\\");
#else
    return Path;
#endif
}

string_view path::file_name() const {
    if (_Path.Length == 0) return "";

    size_t last = _Path.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return _Path(last + 1, _Path.Length + (is_pointing_to_content() ? -1 : 0));
}

string_view path::base_name() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return fileName;
    return fileName(0, last);
}

string_view path::extension() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return "";
    return fileName(last, fileName.Length);
}

string_view path::directory() const {
    if (_Path.Length == 0) return "";

    size_t last = _Path.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return _Path(0, last + 1);
}

string_view path::drive_letter() const {
    if (_Path.Length < 2) return "";
    if (_Path[1] == ':') return _Path(0, 2);
    return "";
}

path path::combined_with(const memory_view &memory) const {
    if (_Path.Length == 0) return memory;
    if (memory.ByteLength == 0) return *this;
    if (memory[0] == '/') return memory;
#if OS == WINDOWS
    if (memory.ByteLength > 1 && memory[1] == (byte) ':') return memory;
#endif
    return _Path + (is_pointing_to_content() ? "" : "/") + memory;
}

void path::resolve() {
    size_t dots, beginning = 0;
    while ((dots = _Path.find("../", beginning)) != npos) {
        if (dots != beginning) break;
        beginning += 3;
    }

    size_t progress = beginning;
    while ((dots = _Path.find("..", progress)) != npos) {
        size_t previousSlash = _Path.find_reverse('/', dots - 2);
        _Path.remove(previousSlash, dots + 2);
        progress = previousSlash + 1;
        if (progress >= _Path.Length) break;
    }

    progress = beginning;
    while ((dots = _Path.find("./", progress)) != npos) {
        _Path.remove(dots, dots + 2);
        progress = dots + 1;
        if (progress >= _Path.Length) break;
    }
}
}  // namespace file

LSTD_END_NAMESPACE