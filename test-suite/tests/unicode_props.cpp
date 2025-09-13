#include "../test.h"

TEST(unicode_basic_properties) {
    // A: U+0041
    assert_true(unicode_is_upper('A'));
    assert_true(unicode_is_alpha('A'));
    assert_false(unicode_is_lower('A'));

    // a: U+0061
    assert_true(unicode_is_lower('a'));
    assert_true(unicode_is_alpha('a'));
    assert_false(unicode_is_upper('a'));

    // SPACE U+0020 is White_Space but not Alphabetic
    assert_true(unicode_is_whitespace(' '));
    assert_false(unicode_is_alpha(' '));

    // IDEOGRAPHIC IDEOGRAPH FIRE (just pick a common CJK ideograph U+4E00)
    code_point han = 0x4E00;
    assert_true(unicode_has_property(han, unicode_property::Unified_Ideograph));
    assert_false(unicode_has_property(han, unicode_property::White_Space));

    // Combining acute accent U+0301 should be Grapheme_Extend
    code_point comb = 0x0301;
    assert_true(unicode_has_property(comb, unicode_property::Grapheme_Extend));
}
