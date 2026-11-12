#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.hpp"

static uint8_t Counters[kNumPCs];

static void exit_hook(void) {
    char* filename = getenv("FUZZER_COUNTER_DUMP_FILE");
    printf("exit hook: filename is %s\n", filename);
    if ( filename != nullptr ) {
        FILE* fp = fopen(filename, "wb");
        if ( fp != nullptr ) {
            fwrite(Counters, kNumPCs, 1, fp);
            fclose(fp);
        }
    }
}

class Init {
    public:
        Init(void) {
            if ( atexit(exit_hook) != 0 ) {
                abort();
            }
        }
};

static Init init;

extern "C" void __sanitizer_cov_trace_pc_guard_init(uint32_t *start, uint32_t *stop) {
    if (start == stop || *start) return;
    size_t NumGuards = 0;
    for (uint32_t *x = start; x < stop; x++) {
        NumGuards++;
        *x = NumGuards % kNumPCs;
    }
}

extern "C" void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
    if (!*guard) return;
    uint32_t Idx = *guard;

    Counters[Idx] = 1;
}

/*
 * testing
int main(void)
{
    return 0;
}
*/
