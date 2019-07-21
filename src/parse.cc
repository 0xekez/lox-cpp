#include <string>
#include <memory>
#include <optional>

#include "parse.h"
#include "token_type.h"
#include "expr.h"
#include "reporter.h"
#include "stmt.h"

std::optional<std::vector<Stmt>> Parser::parse(std::vector<loxc::token> in)
{
    had_error = false;

    tokens = std::move(in);
    current = tokens.begin();
    std::vector<Stmt> stmt_list;

    while ( ! isAtEnd() )
        stmt_list.push_back( declaration() );
    
    return had_error ? std::nullopt : std::make_optional<std::vector<Stmt>>(std::move(stmt_list));
}

Stmt Parser::declaration()
{
    try
    {
        if (match(loxc::VAR))
            return variableDeclaration();

        return statement();
    }
    catch (const parse_error& e)
    {
        had_error = true;
        synchronize();
        return std::monostate{};
    }
}

Stmt Parser::variableDeclaration()
{
    loxc::token name = consume(loxc::ID, "Expected a variable name.");

    Expr init = std::monostate{};
    if (match(loxc::EQUAL))
        init = expression();

    consume(loxc::SEMICOLON, "Expected a semicolon after variable declaration");
    return std::make_shared<VarStmt>(std::move(name), std::move(init));
}

Stmt Parser::statement()
{
    if (match(loxc::PRINT)) return printStatement();

    return expressionStatement();
}

Stmt Parser::printStatement()
{
    Expr value = expression();
    consume(loxc::SEMICOLON, "Expected ; after print statment.");
    return std::make_shared<PrintStmt>(std::move(value));
}

Stmt Parser::expressionStatement()
{
    Expr expr = expression();
    consume(loxc::SEMICOLON, "Expected ; after expression statment.");
    return std::make_shared<PrintStmt>(std::move(expr));
}

Expr Parser::expression()
{
    return equality();
}

#define MAKE_RULE(name, next, matches)                                  \
    Expr Parser:: name ()                                               \
    {                                                                   \
        Expr expr = next ();                                            \
        while (match matches )                                          \
        {                                                               \
            loxc::token op = previous();                                \
            Expr right = next ();                                       \
            expr = std::make_shared<BinaryExpr>                         \
                (std::move(expr), std::move(op), std::move(right));     \
        }                                                               \
        return expr;                                                    \
    }                                                                   \

MAKE_RULE(equality, comparason, (loxc::BANG_EQUAL, loxc::EQUAL_EQUAL))
MAKE_RULE(comparason, addition, (loxc::LESS, loxc::LESS_EQUAL, loxc::GREATER_EQUAL, loxc::GREATER))
MAKE_RULE(addition, multiplication, (loxc::PLUS, loxc::MINUS))
MAKE_RULE(multiplication, unary, (loxc::STAR, loxc::SLASH))

#undef MAKE_RULE

Expr Parser::unary()
{
    while (match(loxc::BANG, loxc::MINUS))
    {
        loxc::token op = previous();
        Expr right = unary();
        return std::make_shared<UnaryExpr>(op, right);
    }
    return primary();
}

Expr Parser::primary()
{
    if (match(loxc::FALSE))
        return std::make_shared<LiteralExpr>(false);
    if (match(loxc::TRUE))
        return std::make_shared<LiteralExpr>(true);
    if (match(loxc::NIL))
        return std::make_shared<LiteralExpr>(nullptr);

    if (match(loxc::NUMBER, loxc::STRING))
    {
        auto tok = previous();
        if ( ! tok.data.has_value() )
            throw error(tok, "Expected value with token");
        return std::make_shared<LiteralExpr>(tok.data.value());
    }

    if (match(loxc::LEFT_PAREN))
    {
        Expr expr = expression();
        consume(loxc::RIGHT_PAREN, "Expected ')' after expression.");
        return std::make_shared<GroupingExpr>(expr);
    }

    if (match(loxc::ID))
        return std::make_shared<VarExpr>(previous());

    throw error(peek(), "Expected an expression.");
}

loxc::token Parser::consume(loxc::token_type in, std::string error_message)
{
    if (check(in))
        return advance();
    throw Parser::error(peek(), error_message);
}

Parser::parse_error Parser::error(loxc::token bad, std::string what)
{
    Reporter::error(bad, what);
    return parse_error(what);
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd())
    {
        if (previous().type == loxc::SEMICOLON)
            return;

        switch (peek().type)
        {
        case loxc::CLASS:
        case loxc::FUN:
        case loxc::VAR:
        case loxc::FOR:
        case loxc::IF:
        case loxc::WHILE:
        case loxc::PRINT:
        case loxc::RETURN:
            return;
        default:
            advance();
        }
    }
}