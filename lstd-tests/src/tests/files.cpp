#include <lstd/file.hpp>

#include "../test.hpp"

TEST(path) {
    file::Path a = "/home/data.txt";
    assert(!a.is_pointing_to_content());
    assert(a.is_absolute());

    assert_eq(a.file_name(), "data.txt");
    assert_eq(a.base_name(), "data");
    assert_eq(a.extension(), ".txt");
    assert_eq(a.directory(), "/home/");

    a = "/home/data/bin/";
    assert(a.is_pointing_to_content());
    assert(a.is_absolute());

    assert_eq(a.file_name(), "bin");
    assert_eq(a.base_name(), "bin");
    assert_eq(a.extension(), "");
    assert_eq(a.directory(), "/home/data/");

    assert_eq(a / "lstd/", "/home/data/bin/lstd/");
    assert_eq(a / "C:/User", "C:/User");
    assert_eq(a / "", a);
    assert_eq("" / a, a);

    a = "../../data/bin/release-x64/../debug-x64/../debug/lstd.exe";
    assert(!a.is_pointing_to_content());
    assert(!a.is_absolute());

    a.resolve();
    assert_eq(a.get(), "../../data/bin/debug/lstd.exe");

    assert_eq(a.file_name(), "lstd.exe");
    assert_eq(a.base_name(), "lstd");
    assert_eq(a.extension(), ".exe");
    assert_eq(a.directory(), "../../data/bin/debug/");
}

TEST(size) {
    assert_eq(file::Handle("data/five_bytes").file_size(), 5);
}