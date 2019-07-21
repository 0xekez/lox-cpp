
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <variant>

#include "token.h"
#include "token_type.h"
#include "scan.h"
#include "reporter.h"
#include "val.h"

std::optional<std::vector<loxc::token>> Scanner::run(std::string src)
{
  tokens.clear();
  had_error = false;
  source = std::move(src);
  current = start = source.begin();
  stop = source.end();

  line = 0;

  while (have_next())
  {
    char pivot = get_next();

    switch (pivot)
    {
    // single characters
    case '(':
      add_tok(loxc::LEFT_PAREN);
      break;
    case ')':
      add_tok(loxc::RIGHT_PAREN);
      break;
    case '{':
      add_tok(loxc::LEFT_BRACE);
      break;
    case '}':
      add_tok(loxc::RIGHT_BRACE);
      break;
    case ',':
      add_tok(loxc::COMMA);
      break;
    case '.':
      add_tok(loxc::DOT);
      break;
    case '-':
      add_tok(loxc::MINUS);
      break;
    case '+':
      add_tok(loxc::PLUS);
      break;
    case ';':
      add_tok(loxc::SEMICOLON);
      break;
    case '*':
      add_tok(loxc::STAR);
      break;

    // one or two characters
    case '!':
      add_tok(next_is('=') ? loxc::BANG_EQUAL : loxc::BANG);
      break;
    case '=':
      add_tok(next_is('=') ? loxc::EQUAL_EQUAL : loxc::EQUAL);
      break;
    case '>':
      add_tok(next_is('=') ? loxc::GREATER_EQUAL : loxc::GREATER);
      break;
    case '<':
      add_tok(next_is('=') ? loxc::LESS_EQUAL : loxc::LESS);
      break;

    case '/':
      if (next_is('/'))
        while (have_next() && peek() != '\n')
          ++current;
      else if (next_is('*'))
        read_block_comment();
      else
        add_tok(loxc::SLASH);
      break;

    // garbage
    case ' ':
    case '\t':
    case '\r':
      break;
    case '\n':
      ++line;
      break;

    case '"':
      read_string();
      break;

    default:
      if (is_digit(pivot))
        read_number();
      else if (is_alpha(pivot))
        read_id();
      else
      {
        Reporter::error("scan error", pivot, line);
        had_error = true;
      }
      break;
    }
    start = current;
  }

  add_tok(loxc::END);

  if (!had_error)
    return tokens;

  return std::nullopt;
}

void Scanner::add_tok(loxc::token_type t)
{
  add_tok(t, std::monostate{});
}

void Scanner::add_tok(loxc::token_type t, Val data)
{
  tokens.emplace_back(t, data, std::string(start, current), line);
}

bool Scanner::next_is(char what)
{
  if (!have_next())
    return false;
  if (what != *current)
    return false;
  ++current;
  return true;
}

void Scanner::read_string()
{
  while (peek() != '"' && have_next())
  {
    if (peek() == '\n')
      ++line;
    ++current;
  }

  if (current == stop)
  {
    Reporter::error("Unterminated string", *(start + 1), line);
    had_error = true;
  }

  // closing "
  ++current;

  add_tok(loxc::STRING, std::string(start + 1, current - 1));
}

void Scanner::read_number()
{
  while (have_next() && is_digit(peek()))
    ++current;

  if (have_next() && peek() == '.' && is_digit(peek_peek()))
    ++current;

  while (is_digit(peek()))
    ++current;

  add_tok(loxc::NUMBER, stod(std::string(start, current)));
}

void Scanner::read_id()
{
  while (is_alpha_numeric(peek()))
    ++current;

  std::string id(start, current);

  auto search = loxc::keywords_map.find(id);

  if (search == loxc::keywords_map.end())
    add_tok(loxc::ID);
  else
    add_tok(search->second);
}

void Scanner::read_block_comment()
{
  while (!(peek() == '*' && peek_peek() == '/'))
  {
    if (peek() == '/' && peek_peek() == '*')
    {
      ++current;
      read_block_comment();
    }
    ++current;
  }
  // closing comment
  current += 2;
}

bool Scanner::is_alpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::is_alpha_numeric(char c)
{
  return (is_digit(c) || is_alpha(c));
}
