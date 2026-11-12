#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>

namespace fuzzing {
namespace util {

class BinaryExecutor {
    protected:
        const std::string program;

        virtual bool preExecHook(void) {
            return true;
        }

        virtual bool postExecHook(const int systemRet) {
            (void)systemRet;

            return true;
        }

    public:
        BinaryExecutor(const std::string program) :
            program(program)
        { }

        bool Run(void) {
            if ( preExecHook() == false ) {
                return false;
            }

            const auto systemRet = system(program.c_str());
            const auto hookRet = postExecHook(systemRet);
            
            if ( systemRet != 0 || hookRet == false ) {
                return false;
            }

            return true;
        }
};

} /* namespace util */
} /* namespace fuzzing */
