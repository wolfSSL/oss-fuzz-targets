#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <fuzzing/testers/serialize/serialize.hpp>
#include <fuzzing/datasource/datasource.hpp>

static std::optional<int> stringToInt(const std::string& in) {
    try {
        return std::stoi(in, nullptr, 10);
    } catch ( ... ) {
        return {};
    }
}

static std::optional<std::string> intToString(const int& in) {
    return std::to_string(in);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static fuzzing::testers::serialize::DefaultSerializeTester<int, std::string> tester(
            std::bind(&intToString, std::placeholders::_1),
            std::bind(&stringToInt, std::placeholders::_1));

    fuzzing::datasource::Datasource ds(data, size);

    tester.Test( ds );

    return 0;
}
