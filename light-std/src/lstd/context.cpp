#include "context.hpp"

#include "io/writer.hpp"

LSTD_BEGIN_NAMESPACE

io::Writer *internal::ConsoleLog = &io::cout;

LSTD_END_NAMESPACE