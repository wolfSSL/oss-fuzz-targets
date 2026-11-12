#pragma once

#include <vector>
#include <fuzzing/util/random.h>

namespace fuzzing {
namespace dictionary {

class Dictionary {
    private:
        ::fuzzing::util::Random rand;
        std::vector<std::string> dictionary;
    public:
        Dictionary(void);
        Dictionary(std::vector<std::string>& dictionary);

        std::string GetRandom(void);
        void Add(const std::string& s);
};

#ifndef FUZZING_HEADERS_NO_IMPL
Dictionary::Dictionary(void) { };
Dictionary::Dictionary(std::vector<std::string>& dictionary) : dictionary(dictionary) { }

std::string Dictionary::GetRandom(void) {
    if ( dictionary.empty() ) {
        return std::string();
    }

    return dictionary[ rand.Get(dictionary.size()) ];
}
        
void Dictionary::Add(const std::string& s) {
    dictionary.push_back( s );
}
#endif


} /* namespace dictionary */
} /* namespace fuzzing */
