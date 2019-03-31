#include "context.hpp"

#include "io/writer.hpp"

LSTD_BEGIN_NAMESPACE

io::writer *internal::g_ConsoleLog = &io::cout;

LSTD_END_NAMESPACE