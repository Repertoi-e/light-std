#include "../test.h"

void test_guid_case(guid id, char f) {
    string format;
    fmt::sprint(&format, "{{:{:c}}}", f);

    string buffer;
    fmt::sprint(&buffer, format, id);

    io::string_reader r(buffer);
    guid result;
    r.read(&result);
    assert(!r.LastFailed);

    size_t diff = compare_memory(id.Data, result.Data, 16);
    assert(diff == npos);
}

TEST(guid_write_read) {
    guid id = new_guid();

    array<char> cases = {'n', 'N', 'd', 'D', 'b', 'B', 'p', 'P', 'x', 'X'};
    For(cases) test_guid_case(id, it);
}