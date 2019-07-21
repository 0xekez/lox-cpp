#include <variant>
#include <iostream>
#include <string>

#include "val.h"

std::ostream &operator<<(std::ostream &o, const Val& v)
{
    switch(v.index())
    {
        case 0:
            o << "<nil>"; break;
        case 1:
            o << std::get<double>(v); break;
        case 2:
            o << std::get<std::string>(v); break;
        case 3:
            o << std::get<bool>(v); break;
    }

    return o;
}

bool is_truthy(const Val& v)
{
    if (std::holds_alternative<std::monostate>(v))
        return false;
    if (std::holds_alternative<bool>(v))
        return std::get<bool>(v);
    return true;
}