#pragma once

#include <fuzzing/datasource/datasource.hpp>
#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <utility>

namespace fuzzing {
namespace testers {
namespace differential {

struct UniversalBase {
    virtual void Load(datasource::Datasource& ds) = 0;
    virtual ~UniversalBase() = default;
};

template <class T>
struct UniversalFromGeneric : public UniversalBase {
    T v;
    UniversalFromGeneric(void) = default;
    UniversalFromGeneric(T v) : v(v) { }
    void Load(datasource::Datasource& ds) override {
        v = ds.Get<T>();
    }
    bool operator!=(const UniversalFromGeneric<T>& other) const {
        return v != other.v;
    }
};

template <typename UniversalOutput, bool Multi> struct DifferentialReturn;

template <typename UniversalOutput> struct DifferentialReturn<UniversalOutput, false> {
    UniversalOutput output;
    bool success;
};

template <typename UniversalOutput> struct DifferentialReturn<UniversalOutput, true> {
    UniversalOutput output;
    bool success;
    bool proceed;
};

template <typename UniversalInput, typename UniversalOutput, bool Multi>
class DifferentialTarget {
    using UniversalOutputExtra = DifferentialReturn<UniversalOutput, Multi>;
    static_assert(std::is_base_of<UniversalBase, UniversalInput>::value);
    static_assert(std::is_base_of<UniversalBase, UniversalOutput>::value);
    public:
        DifferentialTarget(void) = default;
        virtual ~DifferentialTarget(void) = default;
        virtual UniversalOutputExtra Run(const UniversalInput& input) = 0;
};

template <typename UniversalInput, typename UniversalOutput>
using DifferentialTargetSingle = DifferentialTarget<UniversalInput, UniversalOutput, false>;

template <typename UniversalInput, typename UniversalOutput>
using DifferentialTargetMulti = DifferentialTarget<UniversalInput, UniversalOutput, true>;

template <typename InternalInput, typename UniversalInput, typename UniversalOutput, bool Multi>
class DifferentialTargetDefault : public DifferentialTarget<UniversalInput, UniversalOutput, Multi> {
    using UniversalOutputExtra = DifferentialReturn<UniversalOutput, Multi>;
    static_assert(std::is_base_of<UniversalBase, UniversalInput>::value);
    static_assert(std::is_base_of<UniversalBase, UniversalOutput>::value);
    protected:
        InternalInput internalInput;
        virtual bool toInternal(const UniversalInput& universalInput) = 0;
        virtual UniversalOutputExtra run(void) = 0;
    public:
        DifferentialTargetDefault(void) = default;
        virtual ~DifferentialTargetDefault(void) = default;
        UniversalOutputExtra Run(const UniversalInput& universalInput) override {
            if ( toInternal(universalInput) == false ) {
                /* TODO set proceed */
                return {.success = false};
            }

            return run();
        }
};

template <typename InternalInput, typename UniversalInput, typename UniversalOutput>
using DifferentialTargetDefaultSingle = DifferentialTargetDefault<InternalInput, UniversalInput, UniversalOutput, false>;

template <typename InternalInput, typename UniversalInput, typename UniversalOutput>
using DifferentialTargetDefaultMulti = DifferentialTargetDefault<InternalInput, UniversalInput, UniversalOutput, true>;

template <typename UniversalInput, typename UniversalOutput, bool Multi, class... Targets>
class DifferentialTester {
    using UniversalOutputExtra = DifferentialReturn<UniversalOutput, Multi>;
    static_assert(std::is_base_of<UniversalBase, UniversalInput>::value);
    static_assert(std::is_base_of<UniversalBase, UniversalOutput>::value);
    protected:
        template<std::size_t I = 0, typename... Tp> inline typename std::enable_if<I == sizeof...(Tp), void>::type RunTarget(
                const UniversalInput& input,
                std::vector<UniversalOutputExtra>& results,
                std::vector<size_t>& successfulRuns,
                std::tuple<Tp...>& t) {
            (void)input;
            (void)results;
            (void)successfulRuns,
            (void)t;
        }

        template<std::size_t I = 0, typename... Tp> inline typename std::enable_if<I < sizeof...(Tp), void>::type RunTarget(
                const UniversalInput& input,
                std::vector<UniversalOutputExtra>& results,
                std::vector<size_t>& successfulRuns,
                std::tuple<Tp...>& t) {
            /* Assert that the current target is derived from DifferentialTarget */
            static_assert(std::is_base_of<DifferentialTarget<UniversalInput, UniversalOutput, Multi>, typename std::remove_reference<decltype(std::get<I>(t))>::type>::value);
            results[I] = std::get<I>(t).Run(input);
            if ( results[I].success == true ) {
                successfulRuns.push_back(I);
            }
            RunTarget<I + 1, Tp...>(input, results, successfulRuns, t);
        }

    public:
        DifferentialTester(void) = default;
        ~DifferentialTester(void) = default;

        bool Run(datasource::Datasource& ds) {
            /* Instantiate each target class */
            std::tuple<Targets...> targets;

            constexpr size_t numTargets = std::tuple_size<decltype(targets)>::value;

            UniversalInput input;
            input.Load(ds);

            do {
                std::vector<UniversalOutputExtra> results(numTargets);
                std::vector<size_t> successfulRuns;

                /* Run 'results[i] = target.Run(input)' for each target */
                RunTarget(input, results, successfulRuns, targets);

                if ( successfulRuns.size() >= 2 ) {
                    for (size_t i = 0; i < successfulRuns.size() - 1; i++) {
                        const size_t curIndex = successfulRuns[i];
                        const size_t nextIndex = successfulRuns[i+1];
                        if ( results[curIndex].output != results[nextIndex].output ) {
                            /* TODO call crash callback */
                            printf("(crash)\n");
                            return false;
                        }
                    }
                }
            } while ( Multi == true );

            return true;
        }
};

template <typename UniversalInput, typename UniversalOutput, class... Targets>
using DifferentialTesterSingle = DifferentialTester<UniversalInput, UniversalOutput, false, Targets...>;

template <typename UniversalInput, typename UniversalOutput, class... Targets>
using DifferentialTesterMulti = DifferentialTester<UniversalInput, UniversalOutput, true, Targets...>;

} /* namespace differential */
} /* namespace testers */
} /* namespace fuzzing */
