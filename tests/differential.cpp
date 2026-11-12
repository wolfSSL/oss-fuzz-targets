#include <fuzzing/testers/differential/differential.hpp>
#include <string>

using fuzzing::datasource::Datasource;
using fuzzing::testers::differential::UniversalFromGeneric;
using fuzzing::testers::differential::DifferentialTesterSingle;
using fuzzing::testers::differential::DifferentialTargetSingle;
using fuzzing::testers::differential::DifferentialReturn;

using TestUniversalInput = UniversalFromGeneric<std::string>;
using TestUniversalOutput = UniversalFromGeneric<std::string>;

template <class... Targets>
using TestDifferentialTester = DifferentialTesterSingle<TestUniversalInput, TestUniversalOutput, Targets...>;

class TestDifferentialTargetOne : public DifferentialTargetSingle<TestUniversalInput, TestUniversalOutput> {
    public:
        DifferentialReturn<TestUniversalOutput, false> Run(const TestUniversalInput& input) override { return {std::string("One"), true}; }
};

class TestDifferentialTargetTwo : public DifferentialTargetSingle<TestUniversalInput, TestUniversalOutput> {
    public:
        DifferentialReturn<TestUniversalOutput, false> Run(const TestUniversalInput& input) override { return {std::string("Two"), true}; }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    Datasource ds(data, size);

    TestDifferentialTester<
        TestDifferentialTargetOne,
        TestDifferentialTargetTwo
    > diff;

    try {
        diff.Run(ds);
    } catch ( fuzzing::datasource::Datasource::OutOfData ) { }
    return 0;
}
