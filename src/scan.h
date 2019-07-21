// collection of functions involved in scanning a source file

#ifndef scan_h
#define scan_h

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "token.h"
#include "val.h"

class Scanner
{
public:
  Scanner() = default;
  /**
   * Turns an input string into a list of tokens.
   * 
   * @param src the string to be parsed.
   * @return a vector of tokens on success, std::nullopt on
   * an error.
   */
  std::optional<std::vector<loxc::token>> run(std::string src);

private:
  bool have_next() const { return current < stop; }
  char get_next() { return *current++; }
  char peek() { return *current; }
  char peek_peek() { return *(current + 1); }

  void add_tok(loxc::token_type t);
  void add_tok(loxc::token_type, Val data);
  bool next_is(char what);

  void read_string();
  void read_number();
  void read_id();
  void read_block_comment();

  bool is_digit(char c) { return c >= '0' && c <= '9'; }
  bool is_alpha(char c);
  bool is_alpha_numeric(char c);

  std::vector<loxc::token> tokens;

  std::string source;
  std::string::iterator start, current, stop;
  size_t line;
  bool had_error;
};

#endif
