// entry point for all loxc programs.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <variant>
#include <algorithm>

#include <memory>

#include "op.h"
#include "scan.h"
#include "expr.h"
#include "token.h"
#include "parse.h"
#include "reporter.h"
#include "enviroment.h"

std::shared_ptr<Enviroment> global_env(new Enviroment());

enum return_status
{
  GOOD,
  ERROR,
  EXIT
};

int run_file(const char *c);
int run_prompt();
int run(std::string s);

int main(int argc, char **argv)
{
  if (argc > 2)
  {
    std::cout << "usage: jlox [script]\n";
    return -1;
  }
  else if (argc == 2)
    return run_file(argv[1]);
  else
    return run_prompt();
}

int run_file(const char *c)
{
  std::ifstream t(c);
  std::stringstream buffer;
  buffer << t.rdbuf();

  return run(buffer.str());
}

int run_prompt()
{
  int status = 0;

  while (status != EXIT)
  {
    std::cout << ">> ";
    std::string in;
    std::getline(std::cin, in);
    status = run(in);
  }

  return status;
}

int run(std::string in)
{
  Scanner scanner;
  auto tokens = scanner.run(in);

  if (!tokens.has_value())
    return ERROR;

  Parser parser;
  auto expr = parser.parse(tokens.value());

  if (!expr.has_value())
    return ERROR;

  try
  {
    std::for_each(expr.value().begin(), expr.value().end(), [](Stmt s) {
      // The parent of the top level interpreter is the global variables.
      Val val = std::visit(op::interpreter(global_env), s);
      });
  }
  catch(const op::runtime_error& e)
  {
    Reporter::runtime_error(e);
    return ERROR;
  }

  return GOOD;
}