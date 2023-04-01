#include "lstd/common.h"

#if OS == WINDOWS && COMPILER == MSVC

#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

struct aaa {
  aaa() {
    int a = 42;
    a;
  }
};

aaa a;

LSTD_END_NAMESPACE

struct ttt {
  ttt() {
    int a = 42;
    // hello?
  }
};

ttt t;

struct Initializer {
  Initializer() { platform_state_init(); }
};

Initializer g_Initializer = {};

#else
#error Implement.
#endif
