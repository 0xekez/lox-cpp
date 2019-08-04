#ifndef callable_h
#define callable_h

#include <functional>
#include <vector>

#include "val.h"

class Enviroment;

namespace loxc
{
    struct callable
    {
        std::string str;
        std::function<Val(std::shared_ptr<Enviroment>, std::vector<Val>)> func;

        callable(std::string str, 
            std::function<Val(std::shared_ptr<Enviroment>, std::vector<Val>)> func):
        str(std::move(str)), func(std::move(func)) {}
    };
}

#endif