#pragma once

#include <fuzzing/mutator/mutator.h>
#include <string.h>

namespace fuzzing {
namespace datasource {

class Mutator : public ::fuzzing::mutator::Base {
    public:
        Mutator(void) : Base() { }
        size_t Mutate(uint8_t* data, size_t size, const size_t maxSize) override;
};
        
#ifndef FUZZING_HEADERS_NO_IMPL
size_t Mutator::Mutate(uint8_t* data, size_t size, const size_t maxSize) {
    uint32_t s;
    if ( size < sizeof(s) ) {
        goto end;
    }

    if ( rand.RandBool() ) {
        for (size_t i = 0; i < (size-sizeof(s)); ) {
            //if ( rand.RandBool() ) {
            if ( false ) {
                //i += size % 1073741824;
            } else {
                /* Correct size prefix */
                memcpy(&s, data + i, sizeof(s));
                s %= size;
                memcpy(data + i, &s, sizeof(s));
                i += sizeof(s);

                if ( !dictionaries.empty() && rand.RandBool() ) {
                    const std::string entry = dictionaries[rand.Get(dictionaries.size())]->GetRandom();

                    if ( s >= entry.size() && i + s < size ) {
                        memcpy(data + i, entry.data(), entry.size());
                    }
                }

                i += s;
            }
        }
    }

end:
    return size;
}
#endif

} /* namespace datasource */
} /* namespace fuzzing */
