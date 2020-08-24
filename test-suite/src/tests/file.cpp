#include <lstd/file.h>

#include "../test.h"

#define DO_READ_EVERY_FILE 0

TEST(path_manipulation) {
    {
        file::path a = "/home/data.txt";
        assert(!a.is_pointing_to_content());
        assert(a.is_absolute());

        assert_eq(a.file_name(), "data.txt");
        assert_eq(a.base_name(), "data");
        assert_eq(a.extension(), ".txt");
        assert_eq(a.directory(), "/home/");
    }
    {
        file::path a = "/home/data/bin/";
        assert(a.is_pointing_to_content());
        assert(a.is_absolute());

        assert_eq(a.file_name(), "bin");
        assert_eq(a.base_name(), "bin");
        assert_eq(a.extension(), "");
        assert_eq(a.directory(), "/home/data/");

        auto b = a;
        b.combine_with("lstd/");
        assert_eq(b, "/home/data/bin/lstd/");
        a.combine_with("C:/User");
        assert_eq(a.Str, "C:/User");
    }

    {
        file::path a = "../../data/bin/release-x64/../debug-x64/../debug/lstd.exe";
        assert(!a.is_pointing_to_content());
        assert(!a.is_absolute());

        a = file::path(a.resolved());
        assert_eq(a.Str, "../../data/bin/debug/lstd.exe");

        assert_eq(a.file_name(), "lstd.exe");
        assert_eq(a.base_name(), "lstd");
        assert_eq(a.extension(), ".exe");
        assert_eq(a.directory(), "../../data/bin/debug/");
    }
}

TEST(file_size) {
    auto thisFile = file::path(__FILE__);
    file::path dataFolder = thisFile.directory();
    dataFolder.combine_with("data");
    defer(dataFolder.release());

    file::path fiveBytes;
    clone(&fiveBytes, dataFolder);
    defer(fiveBytes.release());
    fiveBytes.combine_with("five_bytes");

    file::path text;
    clone(&text, dataFolder);
    defer(text.release());
    text.combine_with("text");

    assert_eq(file::handle(fiveBytes).file_size(), 5);
    assert_eq(file::handle(text).file_size(), 277);
}

/* Just wearing out the SSD :*
TEST(writing_hello_250_times) {
    auto thisFile = file::path(__FILE__);
    file::path filePath = thisFile.directory();
    filePath.combine_with("data/write_test");
    defer(filePath.release());

    auto file = file::handle(filePath);
    assert(!file.exists());

    auto contents = string("Hello ");
    contents.repeat(250);

    defer(contents.release());

    assert(file.write_to_file(contents));
    assert_eq(250 * 6, file.file_size());

    auto [read, sucess] = file.read_entire_file();
    defer(read.release());

    assert(sucess);
    assert_eq(contents, read);

    assert(file.delete_file());
}*/

//
// This is just causing more trouble that I want to cope with.
// Not a good idea for a test at all honestly.
// It was working the last time I tested it though.
//                                                  - 3.04.2020
//

/*
TEST(test_introspection) {
    auto thisFile = file::path(__FILE__);
    file::path testsFolder = thisFile.directory();

    auto tests = file::handle(testsFolder);
    For(tests) {
        file::path testPath;
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

#if DO_READ_EVERY_FILE
TEST(read_every_file_in_project) {
    file::path rootFolder = file::path(__FILE__).directory();
    rootFolder.combine_with("../../../");

    defer(rootFolder.release());

    hash_table<string, s64> files;

    s32 fileCounter = 100;
    auto callback = [&](file::path it) {
        if (fileCounter) {
            file::path p;
            clone(&p, rootFolder);
            p.combine_with(it);
            defer(p.release());

            auto *counter = files.find(p.Str).second;
            if (!counter) {
                counter = files.add(p.Str, 0).second;
            }
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
