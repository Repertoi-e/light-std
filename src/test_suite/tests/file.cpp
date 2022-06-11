#include "../test.h"

import lstd.path;

TEST(path_manipulation) {
    {
        string a = path_normalize("/home/data.txt");
        assert(path_is_absolute(a));

        assert_eq(path_base_name(a), string("data.txt"));
        assert_eq(path_split_extension(a).Root, path_normalize("/home/data"));
        assert_eq(path_split_extension(a).Extension, string(".txt"));
        assert_eq(path_directory(a), path_normalize("/home/"));
    }
    {
        string a = path_normalize("/home/data/bin");
        assert(path_is_absolute(a));

        assert_eq(path_base_name(a), string("bin"));
        assert_eq(path_split_extension(a).Root, path_normalize("/home/data/bin"));
        assert_eq(path_split_extension(a).Extension, string(""));
        assert_eq(path_directory(a), path_normalize("/home/data"));

        auto b = path_join(a, "lstd");
        assert_eq(b, path_normalize("/home/data/bin/lstd"));

        b = path_join(a, path_normalize("C:/User"));
        assert_eq(b, path_normalize("C:/User"));
    }

    {
        string a = path_normalize("../../data/bin/release-x64/../debug-x64/../debug/lstd.exe");
        assert(!path_is_absolute(a));

        assert_eq(a, path_normalize("../../data/bin/debug/lstd.exe"));

        assert_eq(path_base_name(a), string("lstd.exe"));
        assert_eq(path_split_extension(a).Root, path_normalize("../../data/bin/debug/lstd"));
        assert_eq(path_split_extension(a).Extension, string(".exe"));
        assert_eq(path_directory(a), path_normalize("../../data/bin/debug"));
    }
}

TEST(file_size) {
    auto thisFile     = string(__FILE__);
    string dataFolder = path_join(path_directory(thisFile), "data");
    defer(free(dataFolder.Data));

    string fiveBytes = path_join(dataFolder, "five_bytes");
    defer(free(fiveBytes.Data));

    string text = path_join(dataFolder, "text");
    defer(free(text.Data));

    assert_eq(path_file_size(fiveBytes), 5);
    assert_eq(path_file_size(text), 277);
}

/* Just wearing out the SSD :*
TEST(writing_hello_250_times) {
    auto thisFile = string(__FILE__);

    string filePath = path_join(path_directory(thisFile), "data/write_test");
    defer(free(filePath));

    auto file = file::handle(filePath);
    assert(!file.exists());

    auto contents = string("Hello ");
    repeat(contents, 250);

    defer(free(contents));

    assert(file.write_to_file(contents));
    assert_eq(250 * 6, file.file_size());

    auto [read, success] = file.read_entire_file();

    assert(success);
    assert_eq(contents, read);

    assert(file.delete_file());
}
 */

#define DO_READ_EVERY_FILE 0

#if DO_READ_EVERY_FILE
TE - ST(read_every_file_in_project) {
    string rootFolder = path_normalize(path_join(path_directory(string(__FILE__)), "../../../"));

    hash_table<string, s64> fileMap;

    auto files = path_walk(rootFolder, true);
    For(files) {
        string p = path_join(rootFolder, it);
        defer(free(p));

        assert(path_exists(p));

        auto *counter = find(fileMap, p).Value;
        if (!counter) counter = add(fileMap, p, 0).Value;
        *counter += 1;
    }

    for (auto [file, count] : fileMap) {
        assert_eq(*count, 1);
    }
}
#endif