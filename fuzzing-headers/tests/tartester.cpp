#include <fuzzing/testers/serialize/filesystem.hpp>
#include <fuzzing/harness/binaryexecutorcoverage/server.hpp>
#include <stdlib.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    fuzzing::datasource::Datasource ds(data, size);
    try {
        fuzzing::testers::filesystem::TarTester<fuzzing::harness::binaryexecutorcoverage::BinaryExecutorCoverage> tester(ds, "fsroot", "./tar");
        tester.Run();
    } catch ( fuzzing::datasource::Datasource::OutOfData ) {
    } catch ( fuzzing::generators::filesystem::FlowException ) {
    }

    return 0;
}
