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

    r.request_next_buffer();
    auto [buffer, success] = r.read_bytes_until('\n');

    array<char> result = buffer;
    defer(result.release());

    while (!success && !r.EOF) {
        r.request_next_buffer();

        tie(buffer, success) = r.read_bytes_until('\n');
        result.append_array(buffer);

        if (success) break;
    }

    auto [parsed, parsedStatus, rest] = parse_guid(result);
    assert(parsed == id);
    assert(parsedStatus == PARSE_SUCCESS);
    assert(rest == (array<char>) (string) "");
}

TEST(guid_write_read) {
    guid id = guid_new();

    auto formats = to_stack_array('n', 'N', 'd', 'D', 'b', 'B', 'p', 'P', 'x', 'X');
    For(formats) test_guid_case(id, it);
}
