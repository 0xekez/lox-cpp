// the parser generates tokens. a token has a type, data, a lexme, and a line.
// the type informs how the data should be interpreted later.

#ifndef token_h
#define token_h

#include <memory>   // std::unique_ptr
#include <optional> // std::optional
#include <string>
#include <unordered_map>

#include "token_type.h"
#include "val.h"

std::ostream &operator<<(std::ostream &o, loxc::token_type n);

namespace loxc
{

/**
 * A token in loxc.
 * 
 * TODO: the current representation of data does not work.
 */
struct token
{
  loxc::token_type type;
  std::optional<Val> data;
  std::string lexme;
  int line;

  token(loxc::token_type t, Val d, std::string lex, int l)
      : type(t), data(std::move(d)), lexme(std::move(lex)), line(l) {}

  friend std::ostream &operator<<(std::ostream &os, const loxc::token &tok)
  {
    return os << "'" << tok.lexme << "'";
  }
};

const std::unordered_map<std::string, loxc::token_type> keywords_map =
    {
        {"and", loxc::AND},
        {"abort", loxc::ABORT},
        {"or", loxc::OR},
        {"if", loxc::IF},
        {"else", loxc::ELSE},
        {"class", loxc::CLASS},
        {"true", loxc::TRUE},
        {"false", loxc::FALSE},
        {"fun", loxc::FUN},
        {"anon", loxc::ANON},
        {"for", loxc::FOR},
        {"nil", loxc::NIL},
        {"or", loxc::OR},
        {"print", loxc::PRINT},
        {"return", loxc::RETURN},
        {"super", loxc::SUPER},
        {"this", loxc::THIS},
        {"var", loxc::VAR},
        {"while", loxc::WHILE}};

} // namespace loxc

#endif
