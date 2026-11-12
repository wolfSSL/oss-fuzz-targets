#pragma once

#include <fuzzing/datasource/datasource.hpp>
#include <fuzzing/datasource/id.hpp>
#include <fuzzing/exception.hpp>
#include <fuzzing/memory.hpp>
#include <fuzzing/testers/serialize/serialize.hpp>
#include <fuzzing/test.hpp>
#include <fuzzing/truth.hpp>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <set>

#include <unistd.h>

namespace fuzzing {
namespace testers {
namespace serialize {

template <class ObjectType>
class JsonManipulator {
    public:
        JsonManipulator(void) = default;
        virtual ~JsonManipulator() = default;

        /* Conversion */
        virtual std::optional<ObjectType> StringToObject(const std::string& input) {
            (void)input;

            return std::nullopt;
        }

        virtual std::optional<std::string> ObjectToString(const ObjectType& input) {
            (void)input;

            return {};
        }

        /* Introspection */
        virtual std::optional<bool> IsEqual(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }

        virtual std::optional<bool> IsNotEqual(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }

        virtual std::optional<bool> IsGreaterThan(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }

        virtual std::optional<bool> IsLessThan(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }

        virtual std::optional<bool> IsEqualOrGreaterThan(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }

        virtual std::optional<bool> IsEqualOrLessThan(const ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return {};
        }
        virtual std::optional<bool> IsObject(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<bool> IsArray(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<bool> IsString(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<bool> IsNumber(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<bool> IsBoolean(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<std::vector<std::string>> GetMemberNames(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<uint64_t> GetArraySize(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<double> GetDouble(ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<int32_t> GetInt32(ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<int64_t> GetInt64(ObjectType& input) {
            (void)input;

            return {};
        }

        virtual std::optional<bool> HasMember(const ObjectType& input, const std::string name) {
            (void)input;
            (void)name;

            return {};
        }

        virtual ObjectType& GetMemberReference(ObjectType& input, const std::string name) = 0;

        virtual ObjectType& GetMemberReference(ObjectType& input, const uint64_t index) = 0;

        /* CRUD */
        virtual std::optional<ObjectType> Copy(const ObjectType& input) {
            (void)input;

            return {};
        }

        virtual bool SetKey(ObjectType& dest, const std::string key) {
            (void)dest;
            (void)key;

            return false;
        }

        virtual bool SetDouble(ObjectType& dest, const double val) {
            (void)dest;
            (void)val;

            return false;
        }

        virtual bool SetInt32(ObjectType& dest, const int32_t val) {
            (void)dest;
            (void)val;

            return false;
        }

        virtual bool SetInt64(ObjectType& dest, const int64_t val) {
            (void)dest;
            (void)val;

            return false;
        }

        virtual bool SetString(ObjectType& input, const std::string string) {
            (void)input;
            (void)string;

            return false;
        }

        virtual bool RemoveMember(ObjectType& input, const std::string& memberName) {
            (void)input;
            (void)memberName;

            return false;
        }

        virtual bool RemoveIndex(ObjectType& input, const uint64_t index) {
            (void)input;
            (void)index;

            return false;
        }

        virtual bool Swap(ObjectType& input1, ObjectType& input2) {
            (void)input1;
            (void)input2;

            return false;
        }

        virtual bool Clear(ObjectType& input) = 0;

        virtual bool Set(ObjectType& input1, const ObjectType& input2) {
            (void)input1;
            (void)input2;

            return false;
        }
};

template <class ObjectType, bool WithConversions = true>
class JsonTester : public SerializeTester<ObjectType, std::string> {
    public:
        using global_TargetException = exception::TargetException;
        class TargetException : public global_TargetException {
            public:
                TargetException(const std::string reason) : global_TargetException(reason) { }
        };

    private:
        ObjectType slots[2];
        const std::unique_ptr<Multitest> mt;
        const std::unique_ptr<JsonManipulator<ObjectType>> jsonManipulator;

        template <class InType, typename Convert2XFn>
        void testConversion(const InType& input, Convert2XFn convert2XFn) const {
            const auto res = convert2XFn(input);

            if ( !res ) {
                return;
            }

            if ( res->first != res->second ) {
                throw TargetException("Double conversion mismatch");
            }
        }

        std::optional<std::string> objectToStringCStr(const ObjectType& input) {
            const auto res = jsonManipulator->ObjectToString(input);
            if ( !res ) {
                return {};
            }

            return std::string(res->data(), res->data() + strlen(res->c_str()));
        }

        template<bool withConversions = WithConversions> typename std::enable_if<withConversions, void>::type
        testStringConversion(const std::string& input) const {
            const auto binaryToObject2XWrapper = [this](const std::string& input) -> std::optional<std::pair<ObjectType, ObjectType>> {
                return this->binaryToObject2X(
                        input,
                        std::bind(&JsonManipulator<ObjectType>::StringToObject, jsonManipulator.get(), std::placeholders::_1),
                        std::bind(&JsonManipulator<ObjectType>::ObjectToString, jsonManipulator.get(), std::placeholders::_1));
            };

            testConversion<std::string, decltype(binaryToObject2XWrapper)>(input, binaryToObject2XWrapper);
        }

        template<bool withConversions = WithConversions> typename std::enable_if<!withConversions, void>::type
        testStringConversion(const std::string& input) const { }

        template<bool withConversions = WithConversions> typename std::enable_if<withConversions, void>::type
        testStringConversionCStr(const std::string& input) {
            const auto binaryToObject2XWrapper = [this](const std::string& input) -> std::optional<std::pair<ObjectType, ObjectType>> {
                return this->binaryToObject2X(
                        input,
                        std::bind(&JsonManipulator<ObjectType>::StringToObject, jsonManipulator.get(), std::placeholders::_1),
                        std::bind(&JsonTester<ObjectType>::objectToStringCStr, this, std::placeholders::_1));
            };

            testConversion<std::string, decltype(binaryToObject2XWrapper)>(input, binaryToObject2XWrapper);
        }

        template<bool withConversions = WithConversions> typename std::enable_if<!withConversions, void>::type
        testStringConversionCStr(const std::string& input) { }

        template<bool withConversions = WithConversions> typename std::enable_if<withConversions, void>::type
        testObjectConversion(const ObjectType& input) const {
            const auto ObjectToBinary2XWrapper = [this](const ObjectType& input) -> std::optional<std::pair<std::string, std::string>> {
                return this->objectToBinary2X(
                        input,
                        std::bind(&JsonManipulator<ObjectType>::ObjectToString, jsonManipulator.get(), std::placeholders::_1),
                        std::bind(&JsonManipulator<ObjectType>::StringToObject, jsonManipulator.get(), std::placeholders::_1));
            };

            testConversion<ObjectType, decltype(ObjectToBinary2XWrapper)>(input, ObjectToBinary2XWrapper);
        }

        template<bool withConversions = WithConversions> typename std::enable_if<!withConversions, void>::type
        testObjectConversion(const ObjectType& input) const { }

        template<bool withConversions = WithConversions> typename std::enable_if<withConversions, void>::type
        testObjectConversionCStr(const ObjectType& input) {
            const auto ObjectToBinary2XWrapper = [this](const ObjectType& input) -> std::optional<std::pair<std::string, std::string>> {
                return this->objectToBinary2X(
                        input,
                        std::bind(&JsonTester<ObjectType>::objectToStringCStr, this, std::placeholders::_1),
                        std::bind(&JsonManipulator<ObjectType>::StringToObject, jsonManipulator.get(), std::placeholders::_1));
            };

            testConversion<ObjectType, decltype(ObjectToBinary2XWrapper)>(input, ObjectToBinary2XWrapper);
        }

        template<bool withConversions = WithConversions> typename std::enable_if<!withConversions, void>::type
        testObjectConversionCStr(const ObjectType& input) { }

        ObjectType& getReference(datasource::Datasource& ds) {
            const auto slotIdx = ds.GetChoice( datasource::ID("JsonTester.getReference.GetChoice (slot selection)") ) % 2;
            ObjectType& startRef = slots[slotIdx];

            auto ret = std::ref(startRef);

            while ( true ) {
                if ( ds.Get<bool>( datasource::ID("JsonTester.getReference.Get<bool> (decide to halt)") ) == true ) {
                    break;
                }
                const auto isObject = jsonManipulator->IsObject(ret.get());
                if ( isObject && *isObject ) {
                    const auto memberNames = jsonManipulator->GetMemberNames(ret.get());
                    const size_t objectSize = memberNames->size();

                    if ( objectSize == 0 ) {
                        break;
                    }

                    const uint64_t whichMember = ds.Get<uint64_t>( datasource::ID("JsonTester.getReference.Get<uint64_t> (get member index)") ) % objectSize;
                    const auto memberName = (*memberNames)[whichMember];

                    const auto hasMember = jsonManipulator->HasMember(ret.get(), memberName);
                    if ( hasMember && *hasMember == false ) {
                        throw exception::LogicException("Member expected");
                    }

                    ret = jsonManipulator->GetMemberReference(std::ref(ret.get()), memberName);
                } else {
                    const auto isArray = jsonManipulator->IsArray(ret.get());
                    if ( isArray && *isArray ) {
                        const auto arraySize = jsonManipulator->GetArraySize(ret.get());
                        if ( !arraySize || *arraySize == 0 ) {
                            break;
                        }
                        const uint64_t index = ds.Get<uint64_t>( datasource::ID("JsonTester.getReference.Get<uint64_t> (get array index)") ) % *arraySize;

                        ret = jsonManipulator->GetMemberReference(std::ref(ret.get()), index);
                    } else {
                        break;
                    }
                }
            }
            return ret.get();
        }

        /* Start tests */
        void op_StringConversion(datasource::Datasource& ds) {
            const auto input = ds.Get<std::string>( datasource::ID("content-type:json") );
            if ( ds.Get<bool>( datasource::ID("JsonTester.op_StringConversion.Get<bool> (method choice)") ) == true ) {
                testStringConversion(input);
            } else {
                testStringConversionCStr(input);
            }
        }

        void op_Comparison(datasource::Datasource& ds) {
            const auto& input1 = getReference(ds);
            const auto& input2 = getReference(ds);

            const auto EQ = jsonManipulator->IsEqual(input1, input2);
            const auto NEQ = jsonManipulator->IsNotEqual(input1, input2);
            const auto GT = jsonManipulator->IsGreaterThan(input1, input2);
            const auto LT = jsonManipulator->IsLessThan(input1, input2);
            const auto EQGT = jsonManipulator->IsEqualOrGreaterThan(input1, input2);
            const auto EQLT = jsonManipulator->IsEqualOrLessThan(input1, input2);


            if ( fuzzing::truth::isValid( {EQ, NEQ, GT, LT, EQGT, EQLT} ) == false ) {
                throw TargetException("Incongruent truth values");
            }
        }

        void op_Clear(datasource::Datasource& ds) {
            auto& input = getReference(ds);

            jsonManipulator->Clear(input);
        }

        void op_Copy(datasource::Datasource& ds) {
            const auto& input = getReference(ds);

            const auto copy = jsonManipulator->Copy(input);
            if ( !copy ) {
                return;
            }

            if ( input != *copy ) {
                throw TargetException("Copy mismatch (1)");
            }

            const auto isEQ = jsonManipulator->IsEqual(input, *copy);
            if ( isEQ ) {
                if ( !(*isEQ) ) {
                    throw TargetException("Copy mismatch (2)");
                }
            }
        }

        void op_ConvertInto(datasource::Datasource& ds) {
            const auto input = ds.Get<std::string>( datasource::ID("content-type:json") );
            const auto obj = jsonManipulator->StringToObject(input);
            if ( !obj ) {
                return;
            }

            auto& dest = getReference(ds);
            jsonManipulator->Set(dest, *obj);
        }

        void op_SetKey(datasource::Datasource& ds) {
            auto& dest = getReference(ds);

            /* Only attempt to set a key in an object */
            if ( jsonManipulator->IsObject(dest) == false ) {
                return;
            }

            const auto key = ds.Get<std::string>( datasource::ID("JsonTester.op_SetKey.Get<std::string> (input)") );

            if ( jsonManipulator->SetKey(dest, key) == false ) {
                return;
            }

            const auto hasMember = jsonManipulator->HasMember(dest, key);
            if ( hasMember && *hasMember == false ) {
                throw TargetException("Expected key");
            }
        }

        void op_AssignRefToRef(datasource::Datasource& ds) {
            const auto& src = getReference(ds);
            auto& dest = getReference(ds);

            jsonManipulator->Set(dest, src);
        }

        void op_SetDouble(datasource::Datasource& ds) {
            auto& dest = getReference(ds);
            const auto val = ds.Get<double>( datasource::ID("JsonTester.op_SetDouble.Get<double> (input)") );

            if ( std::isnan(val) == true ) {
                return;
            }

            if ( jsonManipulator->SetDouble(dest, val) == false ) {
                return;
            }

            const auto isNumber = jsonManipulator->IsNumber(dest);

            if ( isNumber && *isNumber == false ) {
                throw TargetException("Expected type to be number");
            }

            const auto res = jsonManipulator->GetDouble(dest);
            if ( res && *res != val ) {
                throw TargetException("SetDouble mismatch");
            }

            testObjectConversion(dest);
        }


        void op_SetInt32(datasource::Datasource& ds) {
            auto& dest = getReference(ds);
            const auto val = ds.Get<int32_t>( datasource::ID("JsonTester.op_SetDouble.Get<int32_t> (input)") );

            if ( jsonManipulator->SetInt32(dest, val) == false ) {
                return;
            }

            const auto isNumber = jsonManipulator->IsNumber(dest);

            if ( isNumber && *isNumber == false ) {
                throw TargetException("Expected type to be number");
            }

            const auto res = jsonManipulator->GetInt32(dest);
            if ( res && *res != val ) {
                throw TargetException("SetInt32 mismatch");
            }

            testObjectConversion(dest);
        }

        void op_SetInt64(datasource::Datasource& ds) {
            auto& dest = getReference(ds);
            const auto val = ds.Get<int64_t>( datasource::ID("JsonTester.op_SetDouble.Get<int64_t> (input)") );

            if ( jsonManipulator->SetInt64(dest, val) == false ) {
                return;
            }

            const auto isNumber = jsonManipulator->IsNumber(dest);

            if ( isNumber && *isNumber == false ) {
                throw TargetException("Expected type to be number");
            }

            const auto res = jsonManipulator->GetInt64(dest);
            if ( res && *res != val ) {
                throw TargetException("SetInt64 mismatch");
            }
            testObjectConversion(dest);
        }

        void op_ObjectConversion(datasource::Datasource& ds) {
            const auto& input = getReference(ds);
            if ( ds.Get<bool>( datasource::ID("JsonTester.op_ObjectConversion.Get<bool> (method choice)") ) == true ) {
                testObjectConversion(input);
            } else {
                testObjectConversionCStr(input);
            }
        }

        void op_Swap(datasource::Datasource& ds) {
            auto& input1 = getReference(ds);
            auto& input2 = getReference(ds);

            const auto input1Copy = jsonManipulator->Copy(input1);
            const auto input2Copy = jsonManipulator->Copy(input2);

            if ( jsonManipulator->Swap(input1, input2) == false ) {
                return;
            }

            bool badSwap = false;

            if ( input1Copy ) {
                const auto EQ = jsonManipulator->IsEqual(input2, *input1Copy);

                if ( EQ && *EQ == false ) {
                    badSwap = true;
                }
            }

            if ( badSwap == false && input2Copy ) {
                const auto EQ = jsonManipulator->IsEqual(input1, *input2Copy);

                if ( EQ && *EQ == false ) {
                    badSwap = true;
                }
            }

            if ( badSwap == true ) {
                throw TargetException("Unexpected post-swap values");
            }
        }

        ObjectType& set(ObjectType& input, datasource::Datasource& ds) {
            ObjectType& ret = input;

            if ( jsonManipulator->IsNull(input) == true ) {
            } else if ( jsonManipulator->IsObject(input) == true ) {
                const auto key = ds.Get<std::string>();
                /* TODO non-recursive */
                const auto val = construct(ds);
                /* jsonManipulator->ObjectInsert(val); */
            } else if ( jsonManipulator->IsArray(input) == true ) {
                /* TODO non-recursive */
                const auto val = construct(ds);
                /* jsonManipulator->ArrayInsert(val); */
            } else if ( jsonManipulator->IsString(input) == true ) {
                const auto val = ds.Get<std::string>();
                /* jsonManipulator->SetString(val); */
            } else if ( jsonManipulator->IsNumber(input) == true ) {
                /* setNumber(input, ds); */
            } else if ( jsonManipulator->IsBoolean(input) == true ) {
                const auto val = ds.Get<bool>( datasource::ID("JsonTester.set.Get<bool> (input)") );
                /* jsonManipulator->SetBoolean(val); */
            }

            return ret;
        }

        void construct(datasource::Datasource& ds) {
            ObjectType root;
            ObjectType& rootRef = root;
            std::set<ObjectType&> nodes{rootRef};
            while ( ds.Get<bool>( datasource::ID("JsonTester.construct.Get<bool> (decide to halt)") ) == true ) {
                ObjectType& curRef = nodes[ds.Get<uint16_t>( datasource::ID("JsonTester.construct.Get<uint16_t> (node selection)") ) % nodes.size()];

                const auto op = ds.GetChoice( datasource::ID("JsonTester.construct.Get<bool> (method choice)") );
                switch ( op ) {
                    case    0:
                        {
                            /* jsonManipulator->SetNull */
                        }
                        break;
                    case    1:
                        {
                            /* jsonManipulator->SetObject */
                        }
                        break;
                    case    2:
                        {
                            /* jsonManipulator->SetArray */
                        }
                        break;
                    case    3:
                        {
                            /* jsonManipulator->SetString */
                        }
                        break;
                    case    4:
                        {
                            /* jsonManipulator->SetNumber */
                        }
                        break;
                    case    5:
                        {
                            /* jsonManipulator->SetBoolean */
                        }
                        break;
                    case    6:
                        {
                            nodes.insert( set(curRef, ds) );
                        }
                        break;
                }
            }
        }

    public:
        JsonTester(std::unique_ptr<JsonManipulator<ObjectType>> jsonManipulator) :
            SerializeTester<ObjectType, std::string>(),
            mt(
                new Multitest(
                    {
                      SingleTest(std::bind(&JsonTester::op_StringConversion, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_Comparison, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_Clear, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_Copy, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_ConvertInto, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_SetKey, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_AssignRefToRef, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_SetDouble, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_SetInt32, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_ObjectConversion, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_SetInt64, this, std::placeholders::_1)),
                      SingleTest(std::bind(&JsonTester::op_Swap, this, std::placeholders::_1)),
                    },
                    datasource::ID("JsonTester.Multitest")
                )
            ),
            jsonManipulator(std::move(jsonManipulator))
            {}

        void Test(datasource::Datasource& ds, const size_t numLoops = 5) {
            /* Reset state */
            if ( jsonManipulator->Clear(slots[0]) == false ) {
                throw exception::LogicException("Failed to clear JSON object slot");
            }
            if ( jsonManipulator->Clear(slots[1]) == false ) {
                throw exception::LogicException("Failed to clear JSON object slot");
            }

            for (size_t i = 0; i < numLoops; i++) {
                mt->Test(ds);
            }
        }
};

} /* namespace serialize */
} /* namespace testers */
} /* namespace fuzzing */
