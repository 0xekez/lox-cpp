// class for reporting errors

#ifndef reporter_h
#define reporter_h

#include <iostream>
#include <string>

#include "token.h"
#include "op.h"

class Reporter
{
public:
  static void runtime_error(const op::runtime_error& e)
  {
    std::cout << "[" << e.where << "] " << e.what() << " [line] " << e.where.line << "\n";
  }

  static void error(std::string what)
  {
    std::cout << "[Error] " << what << std::endl;
  }
  static void error(std::string what, size_t line)
  {
    std::cout << "[Error] " << what << " [line] " << line << "\n";
  }
  static void error(std::string what, std::string where, size_t line)
  {
    std::cout << "[Error] " << what << " '" << where << "' [line] " << line << "\n";
  }
    static void error(std::string what, char where, size_t line)
  {
    std::cout << "[Error] " << what << " '" << where << "' [line] " << line << "\n";
  }
  static void error(loxc::token tok, std::string what)
  {
    if (tok.type == loxc::END)
      Reporter::error("at EOF, line " + std::to_string(tok.line) + ":  " + what);
    else
      Reporter::error(what, tok.lexme, tok.line);
  }

  static void info(std::string what)
  {
    std::cout << "[INFO] " << what << "\n";
  }
};

#endif
