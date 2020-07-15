#include "format_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
void debug_struct_helper::finish() {
    F->write_no_specs(Name);
    F->write_no_specs(" {");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        F->write_no_specs(" ");
        write_field(begin);
        ++begin;
        while (begin != Fields.end()) {
            F->write_no_specs(", ");
            write_field(begin);
            ++begin;
        }
    }
    F->write_no_specs(" }");
}

void debug_struct_helper::write_field(field_entry *entry) {
    F->write_no_specs(entry->Name);
    F->write_no_specs(": ");
    visit_fmt_arg(format_context_visitor(F, true), entry->Arg);
}

void debug_tuple_helper::finish() {
    F->write_no_specs(Name);
    F->write_no_specs("(");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(format_context_visitor(F, true), *begin);
        ++begin;
        while (begin != Fields.end()) {
            F->write_no_specs(", ");
            visit_fmt_arg(format_context_visitor(F, true), *begin);
            ++begin;
        }
    }
    F->write_no_specs(")");
}

void fmt::debug_list_helper::finish() {
    F->write_no_specs("[");

    auto *begin = Fields.begin();
    if (begin != Fields.end()) {
        visit_fmt_arg(format_context_visitor(F, true), *begin);
        ++begin;
        while (begin != Fields.end()) {
            F->write_no_specs(", ");
            visit_fmt_arg(format_context_visitor(F, true), *begin);
            ++begin;
        }
    }
    F->write_no_specs("]");
}
}  // namespace fmt

LSTD_END_NAMESPACE
