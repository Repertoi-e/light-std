#include <lstd/file.h>

import path;

#include "../test.h"

TEST(path_manipulation) {
    {
        string a = path_normalize("/home/data.txt");
        assert(path_is_absolute(a));

        assert_eq(path_base_name(a), "data.txt");
        assert_eq(path_split_extension(a).Root, path_normalize("/home/data"));
        assert_eq(path_split_extension(a).Extension, ".txt");
        assert_eq(path_directory(a), path_normalize("/home/"));
    }
    {
        string a = path_normalize("/home/data/bin");
        assert(path_is_absolute(a));

        assert_eq(path_base_name(a), "bin");
        assert_eq(path_split_extension(a).Root, path_normalize("/home/data/bin"));
        assert_eq(path_split_extension(a).Extension, "");
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

        assert_eq(path_base_name(a), "lstd.exe");
        assert_eq(path_split_extension(a).Root, path_normalize("../../data/bin/debug/lstd"));
        assert_eq(path_split_extension(a).Extension, ".exe");
        assert_eq(path_directory(a), path_normalize("../../data/bin/debug"));
    }
}

TEST(file_size) {
    auto thisFile = string(__FILE__);
    string dataFolder = path_join(path_directory(thisFile), "data");
    defer(free(dataFolder));

    string fiveBytes = path_join(dataFolder, "five_bytes");
    defer(free(fiveBytes));

    string text = path_join(dataFolder, "text");
    defer(free(text));

    assert_eq(file::handle(fiveBytes).file_size(), 5);
    assert_eq(file::handle(text).file_size(), 277);
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

//
// This is just causing more trouble that I want to cope with.
// Not a good idea for a test at all honestly.
// It was working the last time I tested it though.
//                                                  - 3.04.2020
//

/*
TEST(test_introspection) {
    auto thisFile = string(__FILE__);
    string testsFolder = thisFile.directory();

    auto tests = file::handle(testsFolder);
    For(tests) {
        string testPath;
        clone(&testPath, testsFolder);
        testPath.combine_with(it);

        auto test = file::handle(testPath);
        if (!test.is_file()) continue;

        string contents;
        test.read_entire_file(&contents);
        assert_eq(contents.Count, test.file_size());

        string nativePath;
        clone(&nativePath, testPath.Str);
#if OS == WINDOWS
        nativePath.replace_all('/', '\\');
#endif
        // Prevent counting the literal in this file
        auto *testLiteral =
            "TE"
            "ST(";

        auto *testArray = g_TestTable.find(get_short_file_name(nativePath));
        if (testArray) {
            assert_eq(contents.count(testLiteral), testArray->Count);
        }
    }
}
*/

#define DO_READ_EVERY_FILE 1

#if DO_READ_EVERY_FILE
TEST(read_every_file_in_project) {
    string rootFolder = path_normalize(path_join(path_directory(string(__FILE__)), "../../../"));

    hash_table<string, s64> files;

    s32 fileCounter = 100;
    auto callback = [&](string it) {
        if (fileCounter) {
            string p = path_join(rootFolder, it);
            defer(free(p));

            auto *counter = find(files, p).Value;
            if (!counter) counter = add(files, p, 0).Value;
            ++*counter;
            --fileCounter;
        }
    };
    file::handle(rootFolder).traverse_recursively(&callback);

    for (auto [file, count] : files) {
        assert_eq(*count, 1);
    }
}
#endif
