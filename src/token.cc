#include <iostream>
#include <unordered_map>

#include "token_type.h"

std::ostream &operator<<(std::ostream &o, loxc::token_type n)
{
  switch (n)
  {
  case loxc::LEFT_PAREN:
    return o << "LEFT_PAREN";
  case loxc::RIGHT_PAREN:
    return o << "RIGHT_PAREN";
  case loxc::LEFT_BRACE:
    return o << "LEFT_BRACE";
  case loxc::RIGHT_BRACE:
    return o << "RIGHT_BRACE";
  case loxc::COMMA:
    return o << "COMMA";
  case loxc::DOT:
    return o << "DOT";
  case loxc::MINUS:
    return o << "MINUS";
  case loxc::PLUS:
    return o << "PLUS";
  case loxc::SEMICOLON:
    return o << "SEMICOLON";
  case loxc::SLASH:
    return o << "SLASH";
  case loxc::STAR:
    return o << "STAR";
  case loxc::BANG_EQUAL:
    return o << "BANG_EQUAL";
  case loxc::EQUAL:
    return o << "EQUAL";
  case loxc::EQUAL_EQUAL:
    return o << "EQUAL_EQUAL";
  case loxc::GREATER:
    return o << "GREATER";
  case loxc::GREATER_EQUAL:
    return o << "GREATER_EQUAL";
  case loxc::LESS:
    return o << "LESS";
  case loxc::LESS_EQUAL:
    return o << "LESS_EQUAL";
  case loxc::CLASS:
    return o << "CLASS";
  case loxc::ELSE:
    return o << "ELSE";
  case loxc::FALSE:
    return o << "FALSE";
  case loxc::FUN:
    return o << "FUN";
  case loxc::FOR:
    return o << "FOR";
  case loxc::IF:
    return o << "IF";
  case loxc::NIL:
    return o << "NIL";
  case loxc::OR:
    return o << "OR";
  case loxc::PRINT:
    return o << "PRINT";
  case loxc::ABORT:
    return o << "ABORT";
  case loxc::RETURN:
    return o << "RETURN";
  case loxc::SUPER:
    return o << "SUPER";
  case loxc::THIS:
    return o << "THIS";
  case loxc::TRUE:
    return o << "TRUE";
  case loxc::VAR:
    return o << "VAR";
  case loxc::WHILE:
    return o << "WHILE";
  case loxc::ID:
    return o << "ID";
  case loxc::STRING:
    return o << "STRING";
  case loxc::NUMBER:
    return o << "NUMBER";
  case loxc::END:
    return o << "END";
  default:
    return o << "(invalid value)";
  }
}
