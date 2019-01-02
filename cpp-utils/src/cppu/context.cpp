#include "context.hpp"

#include "io/writer.hpp"

CPPU_BEGIN_NAMESPACE

io::Writer *internal::ConsoleLog = &io::cout;

CPPU_END_NAMESPACE