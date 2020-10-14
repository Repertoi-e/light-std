#include "format_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
void format_struct_helper::finish() {
    write_no_specs(F, Name);
    write_no_specs(F, " {");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        write_no_specs(F, " ");
        write_field(begin);
        ++begin;
        while (begin != Fields.end()) {
            write_no_specs(F, ", ");
            write_field(begin);
            ++begin;
        }
    }
    write_no_specs(F, " }");
}

void format_struct_helper::write_field(field_entry *entry) {
    write_no_specs(F, entry->Name);
    write_no_specs(F, ": ");
    visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), entry->Arg);
}

void format_tuple_helper::finish() {
    write_no_specs(F, Name);
    write_no_specs(F, "(");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
        ++begin;
        while (begin != Fields.end()) {
            write_no_specs(F, ", ");
            visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
            ++begin;
        }
    }
    write_no_specs(F, ")");
}

void fmt::format_list_helper::finish() {
    write_no_specs(F, "[");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
        ++begin;
        while (begin != Fields.end()) {
            write_no_specs(F, ", ");
            visit_fmt_arg(internal::format_context_visitor(F, NoSpecs), *begin);
            ++begin;
        }
    }
    write_no_specs(F, "]");
}
}  // namespace fmt

LSTD_END_NAMESPACE
