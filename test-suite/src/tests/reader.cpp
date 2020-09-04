#include <lstd/parse.h>

#include "../test.h"

void test_guid_case(guid id, char f) {
    string format = fmt::sprint("{{:{:c}}}\n", f);
    defer(format.release());

    string formatted = fmt::sprint(format, id);
    defer(formatted.release());

    io::string_reader string_reader(formatted);

    // Weird scenario just to stress test
    io::chunked_reader<2> r(&string_reader);

    array<byte> buffer;
    defer(free(buffer));  // If we allocate, we free, this doesn't crash if there was no memory allocated!

    bool done = false;
    while (!done && !r.EOF) {
        r.request_next_buffer();

        auto [b, done] = r.read_bytes_until('\n');
        append_array_or_set_fields(buffer, b);
    }

    auto [parsed, parsedStatus, rest] = parse_guid(buffer);
    assert(parsed == id);
    assert(parsedStatus == PARSE_SUCCESS);
    assert(rest == (bytes)(string) "");
}

TEST(guid_write_read) {
    guid id = guid_new();

    auto formats = to_stack_array('n', 'N', 'd', 'D', 'b', 'B', 'p', 'P', 'x', 'X');
    For(formats) test_guid_case(id, it);
}
