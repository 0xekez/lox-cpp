// loxc values
#ifndef val_h
#define val_h

#include <variant>
#include <string>
#include <iostream>

using Val = std::variant<
    std::monostate,
    double,
    std::string,
    bool>;

std::ostream &operator<<(std::ostream &o, const Val& v);

bool is_truthy(const Val& v);

inline bool same_type(const Val& v1, const Val& v2)
{
    return v1.index() == v2.index();
}
template <typename... rest>
inline bool same_type(const Val& v1, const Val& v2, rest... r)
{
    return v1.index() == v2.index() && same_type(v2, r...);
}

#endif