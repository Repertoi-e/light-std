#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace file {

string path::file_name() const {
    if (UnifiedPath.Length == 0) return "";

    size_t last = UnifiedPath.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return UnifiedPath.substring(last + 1, UnifiedPath.Length + (is_pointing_to_content() ? -1 : 0));
}

string path::base_name() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return fileName;
    return fileName.substring(0, last);
}

string path::extension() const {
    auto fileName = file_name();
    size_t last = fileName.find_reverse('.');
    if (last == npos) return "";
    return fileName.substring(last, fileName.Length);
}

string path::directory() const {
    if (UnifiedPath.Length == 0) return "";

    size_t last = UnifiedPath.find_reverse('/', is_pointing_to_content() ? -2 : 0);
    if (last == npos) return "";
    return UnifiedPath.substring(0, last + 1);
}

string path::drive_letter() const {
    if (UnifiedPath.Length < 2) return "";
    if (UnifiedPath[1] == ':') return UnifiedPath.substring(0, 2);
    return "";
}

void path::combine_with(string str) {
    if (!UnifiedPath.is_owner()) {
        UnifiedPath.reserve(0);  // Trigger a reserve
    }

    auto other = path(str);
    if (!is_pointing_to_content()) {
        UnifiedPath.append("/");
    }
    if (other.is_absolute()) {
        UnifiedPath.ByteLength = UnifiedPath.Length = 0;
    }
    UnifiedPath.append(str);
    resolve();
}

void path::resolve() {
    size_t dots, beginning = 0;
    while ((dots = UnifiedPath.find("../", beginning)) != npos) {
        if (dots != beginning) break;
        beginning += 3;
        if (beginning == UnifiedPath.Length) return;
    }
    if (beginning == UnifiedPath.Length) return;

    size_t progress = beginning;
    while ((dots = UnifiedPath.find("..", progress)) != npos) {
        size_t previousSlash = UnifiedPath.find_reverse('/', dots - 2);
        UnifiedPath.remove(previousSlash, dots + 2);
        progress = previousSlash + 1;
        if (progress >= UnifiedPath.Length) break;
    }

    progress = beginning;
    while ((dots = UnifiedPath.find("./", progress)) != npos) {
        UnifiedPath.remove(dots, dots + 2);
        progress = dots + 1;
        if (progress >= UnifiedPath.Length) break;
    }
}
}  // namespace file

file::path *clone(file::path *dest, file::path src) {
    dest->UnifiedPath.release();
    clone(&dest->UnifiedPath, src.UnifiedPath);
    dest->unify();
    return dest;
}

file::path *move(file::path *dest, file::path *src) {
    dest->UnifiedPath.release();
    move(&dest->UnifiedPath, &src->UnifiedPath);
    return dest;
}

LSTD_END_NAMESPACE