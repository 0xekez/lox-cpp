#ifndef enviroment_h
#define enviroment_h

#include <unordered_map>
#include <string>

#include "op.h"
#include "val.h"
#include "token.h"

namespace op
{
    struct runtime_error : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
        loxc::token where;
        runtime_error(loxc::token w, const char* what) 
            : std::runtime_error(what), where(std::move(w)) {}
        runtime_error(loxc::token w, std::string what) 
            : std::runtime_error(what.c_str()), where(std::move(w)) {}
    };
}

class Enviroment
{
private:
    std::unordered_map<std::string, Val> val_map;
public:
    void define (std::string name, Val value)
    {
        // We let variables be reassigned.
        // This is a design choice that I'm not actually sure I like.
        val_map.insert_or_assign(std::move(name), std::move(value));
    }

    void print (std::string starter = "")
        {
        std::cout << starter + "enviroment:\n";
        for (const auto& item : val_map)
            std::cout << "\t" + starter << item.first << " -> " << item.second << "\n";

        if (parent)
            {
            std::cout << starter + "\t" + "parent\n";
            parent->print("\t" + starter);
            }
        }

    void assign (const loxc::token& name, Val value)
    {
        auto where = val_map.find(name.lexme);
        if (where == val_map.end())
            throw op::runtime_error(name, "Undefined variable '" + name.lexme + "'.");
        where->second = value;
    }

    Val get (const loxc::token& name)
    {
        print();
        std::cout << "finding: " << name.lexme << "\n";
        auto where = val_map.find(name.lexme);
        if (where == val_map.end())
            {
            if ( ! parent )
                throw op::runtime_error(name, "Undefined variable '" + name.lexme + "'.");
            std::cout << "asking parent\n";
            return parent->get(name);
            }
        return where->second;
    }

    std::shared_ptr<Enviroment> parent;
};

#endif