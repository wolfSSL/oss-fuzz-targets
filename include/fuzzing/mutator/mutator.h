#pragma once

#include <fuzzing/dictionary/dictionary.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t maxSize);

namespace fuzzing {
namespace mutator {

class Base {
    protected:
        ::fuzzing::util::Random rand;
        std::vector< std::shared_ptr<::fuzzing::dictionary::Dictionary> > dictionaries;
    public:
        Base(void) = default;
        virtual ~Base(void) = default;

        virtual size_t Mutate(uint8_t* data, size_t size, const size_t maxSize) = 0;

        void AddSource(std::shared_ptr<::fuzzing::dictionary::Dictionary> d);
};
        
#ifndef FUZZING_HEADERS_NO_IMPL
void Base::AddSource(std::shared_ptr<::fuzzing::dictionary::Dictionary> d) {
    dictionaries.push_back( d );
}
#endif

#ifndef FUZZING_HEADERS_NO_IMPL
::fuzzing::util::Random rand;
std::vector< std::unique_ptr<Base> > mutators;
#endif

} /* namespace mutator */
} /* namespace fuzzing */

#ifndef FUZZING_HEADERS_NO_IMPL
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* data, size_t size, size_t maxSize, unsigned int seed) {
    const bool runCustomMutator = seed & 1;
    seed >>= 1;

    size = LLVMFuzzerMutate(data, size, maxSize);
    if ( !fuzzing::mutator::mutators.empty() && runCustomMutator) {
        const size_t mutatorIndex = seed % fuzzing::mutator::mutators.size();
        size = fuzzing::mutator::mutators[mutatorIndex]->Mutate(data, size, maxSize);
    }

    return size;

}
#endif

