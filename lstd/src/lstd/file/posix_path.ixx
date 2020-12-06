module;

#include "../internal/common.h"
#include "../memory/string.h"

export module path.posix;

//
// Common pathname manipulations, POSIX version.
//
// We import this module when compiling for posix platforms.
// If you want to explicitly work with posix paths, import this module directly.
//

export constexpr char OS_PATH_SEPARATOR = '/';

export always_inline constexpr bool path_is_sep(char32_t ch) { return ch == '/'; }


// Returns whether a path is absolute.
// Trivial in Posix (starts with '/'), harder on Windows.
//
// e.g.
//    /home/user/me       -> true
//    ./data/myData       -> false
//    ../data/myData      -> false
//    data/myData         -> false
constexpr bool path_is_absolute(const string &path) { return is_path_sep(path[0]); }

// Joins two or more paths.
// Ignore the previous parts if a part is absolute.
// This is the de facto way to build paths. Takes care of slashes automatically.
[[nodiscard("Leak")]] string path_join(const array_view<string> &paths) {
    assert(false && "Not implemented");
    return "";
}

string path_normalize(const string &path) {
    assert(false && "Not implemented");
    return "";
}

