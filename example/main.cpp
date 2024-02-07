#include "lstd/lstd.h"

int main() { 
    platform_state_init();
    int a = sizeof(pthread_mutex_t);
    print("Hello, world!\n"); 
}
