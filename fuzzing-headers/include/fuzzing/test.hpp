#pragma once

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/test.hpp>
#include <functional>
#include <vector>

namespace fuzzing {

class SingleTest {
    private:
        std::function<void(datasource::Datasource& ds)> fn;
    public:
        SingleTest(std::function<void(datasource::Datasource& ds)> fn) : fn(fn) { }
        void Test(datasource::Datasource& ds) const {
            fn(ds);
        }
};

class Multitest {
    private:
        std::vector<SingleTest> tests;
        const size_t numTests;
        const uint64_t id;

    public:
        Multitest(std::initializer_list<SingleTest> tests, const uint64_t id = 0) : tests{std::move(tests)}, numTests(this->tests.size()), id(id) {}
        void Test(datasource::Datasource& ds) const {
            const auto which = ds.Get<uint16_t>(id);

            if ( numTests == 0 ) {
                /* Abort ? */
                return;
            }

            if ( which >= numTests ) {
                return;
            }

            tests[which].Test(ds);
        }
        
        void Loop(datasource::Datasource& ds, const size_t numLoops) const {
            for (size_t i = 0; i < numLoops; i++) {
                Test(ds);
            }
        }
};

} /* namespace fuzzing */
