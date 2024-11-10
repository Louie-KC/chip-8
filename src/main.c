#include <stdio.h>

// #define ENABLE_DEBUG_LOG

#ifdef ENABLE_DEBUG_LOG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif  // ENABLE_DEBUG_LOG

int init() {
    // TODO
}

int step() {
    // TODO
}

int main(int argc, char *argv[]) {
    printf("Hello world\n");
    
    init();

    for (;;) {
        step();
    }

    return 0;
}
