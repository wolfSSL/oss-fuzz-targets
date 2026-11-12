#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <fuzzing/util/binaryexecutor.hpp>
#include "shared.hpp"

extern "C" {
    __attribute__((section("__libfuzzer_extra_counters")))
    static uint8_t Counters[kNumPCs];
}

namespace fuzzing {
namespace harness {
namespace binaryexecutorcoverage {

class BinaryExecutorCoverage : public util::BinaryExecutor {
    private:
        const std::string pidStr;
        size_t counter = 0;
        std::string lastDumpfile;

        std::string getDumpfile(void) {
            counter++;
            const std::string ret = "COUNTER_DUMPFILE_" + pidStr + "_" + std::to_string(counter);
            lastDumpfile = ret;
            return ret;
        }

    public:
        BinaryExecutorCoverage(const std::string program) :
            util::BinaryExecutor(program), pidStr( std::to_string(getpid()) )
        { }

        bool preExecHook(void) override {
            const auto dumpfile = getDumpfile();
            /* TODO check ret */ unlink(dumpfile.c_str());

            if ( setenv("FUZZER_COUNTER_DUMP_FILE", dumpfile.c_str(), 1) != 0 ) {
                abort();
            }

            return true;
        }

        bool postExecHook(const int systemRet) override {
            /* TODO remove dumpfile */

            if ( systemRet != 0 ) {
                return false;
            }

            const auto dumpfile = lastDumpfile;
            FILE* fp = fopen(dumpfile.c_str(), "rb");
            if ( fp == nullptr ) {
                return false;
            }

            bool ret = false;
            uint8_t _Counters[kNumPCs];
            if ( fread(_Counters, kNumPCs, 1, fp) != 1 ) {
                goto end;
            }

            memcpy(Counters, _Counters, kNumPCs);

            ret = true;
end:
            fclose(fp);

            return ret;
        }

};

} /* namespace binaryexecutorcoverage */
} /* namespace harness */
} /* namespace fuzzing */

/*
 * testing
int main(void)
{
    ExternalBinaryExecutor ex("./program");
    ex.Run();
    printf("%d\n", ex.LoadCounters());
}
*/
