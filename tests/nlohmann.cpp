#include <fuzzing/testers/serialize/json.hpp>
#include "json.hpp"

class NlohmannJsonManipulator : public fuzzing::testers::serialize::JsonManipulator<nlohmann::json> {
    public:
        NlohmannJsonManipulator(void) : fuzzing::testers::serialize::JsonManipulator<nlohmann::json>() { }
        ~NlohmannJsonManipulator() override = default;

        /* Conversion */
        std::optional<nlohmann::json> StringToObject(const std::string& input) override {
            return nlohmann::json::parse(input);
        }

        std::optional<std::string> ObjectToString(const nlohmann::json& input) override {
            return input.dump();
        }

        /* Introspection */
        std::optional<bool> IsEqual(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 == input2;
        }

        std::optional<bool> IsNotEqual(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 != input2;
        }

        std::optional<bool> IsGreaterThan(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 > input2;
        }

        std::optional<bool> IsLessThan(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 < input2;
        }

        std::optional<bool> IsEqualOrGreaterThan(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 >= input2;
        }

        std::optional<bool> IsEqualOrLessThan(const nlohmann::json& input1, const nlohmann::json& input2) override {
            return input1 <= input2;
        }

        std::optional<bool> IsObject(const nlohmann::json& input) override {
            return input.is_object();
        }

        std::optional<bool> IsArray(const nlohmann::json& input) override {
            return input.is_array();
        }

        std::optional<bool> IsString(const nlohmann::json& input) override {
            return input.is_string();
        }

        std::optional<bool> IsNumber(const nlohmann::json& input) override {
            return input.is_number();
        }

        std::optional<bool> IsBoolean(const nlohmann::json& input) override {
            return input.is_boolean();
        }

        std::optional<std::vector<std::string>> GetMemberNames(const nlohmann::json& input) override {
            std::vector<std::string> ret;

            for (auto it = input.begin(); it < input.end(); it++) {
                ret.push_back( it.key() );
            }

            return ret;
        }

        std::optional<uint64_t> GetArraySize(const nlohmann::json& input) override {
            return input.size();
        }

        std::optional<double> GetDouble(nlohmann::json& input) override {
            double ret = input;
            return ret;
        }

        std::optional<int32_t> GetInt32(nlohmann::json& input) override {
            int32_t ret = input;
            return ret;
        }

        std::optional<int64_t> GetInt64(nlohmann::json& input) override {
            int64_t ret = input;
            return ret;
        }

        std::optional<bool> HasMember(const nlohmann::json& input, const std::string name) override {
            return input.find(name) != input.end();
        }

        nlohmann::json& GetMemberReference(nlohmann::json& input, const std::string name) override {
            return input[name];
        }

        nlohmann::json& GetMemberReference(nlohmann::json& input, const uint64_t index) override {
            return input[index];
        }

        /* CRUD */
        std::optional<nlohmann::json> Copy(const nlohmann::json& input) override {
            return nlohmann::json(input);
        }

        bool SetKey(nlohmann::json& dest, const std::string key) override {
            dest[key] = {};

            return true;
        }

        bool SetDouble(nlohmann::json& dest, const double val) override {
            dest = val;

            return true;
        }

        bool SetInt32(nlohmann::json& dest, const int32_t val) override {
            dest = val;

            return true;
        }

        bool SetInt64(nlohmann::json& dest, const int64_t val) override {
            dest = val;

            return true;
        }

        bool Swap(nlohmann::json& input1, nlohmann::json& input2) override {
            input1.swap(input2);

            return true;
        }

        bool Clear(nlohmann::json& input) override {
            input.clear();

            return true;
        }

        bool Set(nlohmann::json& input1, const nlohmann::json& input2) override {
            input1 = input2;

            return true;
        }
};

std::unique_ptr<fuzzing::testers::serialize::JsonTester<nlohmann::json>> jsonTester;

extern "C" int LLVMFuzzerInitialize(int *_argc, char ***_argv) {
    (void)_argc;
    (void)_argv;

    jsonTester = std::make_unique<fuzzing::testers::serialize::JsonTester<nlohmann::json>>( std::make_unique<NlohmannJsonManipulator>() );

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    fuzzing::datasource::Datasource ds(data, size);

    try {
        jsonTester->Test(ds);
    } catch ( fuzzing::datasource::Datasource::OutOfData ) {
    } catch ( nlohmann::detail::exception ) {
    }


    return 0;
}
