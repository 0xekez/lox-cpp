#ifndef time_h
#define time_h

#include <memory>
#include <vector>

#include "callable.h"
#include "val.h"

class Enviroment;

namespace builtins
{
    auto time = std::make_shared<loxc::callable>("<time builtin>", 
    [](std::shared_ptr<Enviroment> env, std::vector<Val> args)-> Val{
        // gross I know.
        return static_cast<double>(::time(0));
    });
}

#endif