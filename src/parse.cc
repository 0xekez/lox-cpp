#include <string>
#include <memory>
#include <optional>
#include <initializer_list>
#include <vector>

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
    if (match(loxc::RETURN)) return returnStatement();
    if (match(loxc::FUN)) return funcStatement();
    if (match(loxc::PRINT)) return printStatement();
    if (match(loxc::LEFT_BRACE)) return blockStatement();
    if (match(loxc::IF)) return ifStatement();
    if (match(loxc::WHILE)) return whileStatement();
    if (match(loxc::FOR)) return forStatement();

    return expressionStatement();
}

Stmt Parser::printStatement()
{
    Expr value = expression();
    consume(loxc::SEMICOLON, "Expected ; after print statment.");
    return std::make_shared<PrintStmt>(std::move(value));
}

Stmt Parser::funcStatement()
{
    loxc::token name = consume(loxc::ID, "Expected a function name.");
    consume(loxc::LEFT_PAREN, "Expected opening '(' after function definition.");

    std::vector<loxc::token> params;
    if ( ! check(loxc::RIGHT_PAREN) )
        do {
            params.push_back(consume(loxc::ID, "Expected a parameter name."));
        } while(match(loxc::COMMA));

    consume(loxc::RIGHT_PAREN, "Expected closing ')' after function parameters.");

    Stmt body = statement();
    return std::make_shared<FuncStmt>(std::move(name), std::move(params), std::move(body));
}

Stmt Parser::returnStatement()
{
    loxc::token keyword = previous();
    Expr val;
    // only read an expression if there is one.
    if (! check(loxc::SEMICOLON) )
        val = expression();
    consume(loxc::SEMICOLON, "Expexted ';' after return statement.");
    return std::make_shared<ReturnStmt>(std::move(keyword), std::move(val));
}

Stmt Parser::blockStatement()
{
    std::vector<Stmt> stmt_list;

    while ( ! (check(loxc::RIGHT_BRACE) || isAtEnd()) )
        stmt_list.push_back(declaration());
    
    consume(loxc::RIGHT_BRACE, "Expected a closing bracket.");
    return std::make_shared<BlockStmt>(std::move(stmt_list));
}

Stmt Parser::ifStatement()
{
    consume(loxc::LEFT_PAREN, "Expected '(' after if.");
    Expr conditional = expression();
    consume(loxc::RIGHT_PAREN, "Expected closing ')' after if.");
    Stmt then = statement();

    Stmt otherwise; // std::monostate
    if (match(loxc::ELSE))
        otherwise = statement();

    return std::make_shared<IfStmt>(conditional, std::move(then), std::move(otherwise));
}

Stmt Parser::whileStatement()
{
    consume(loxc::LEFT_PAREN, "Expected '(' after while.");
    Expr condition = expression();
    consume(loxc::RIGHT_PAREN, "Expected closing ')' after while.");

    Stmt body = statement();

    return std::make_shared<WhileStmt>(condition, body);

}

// for (initializer ; condition ; increment) body
Stmt Parser::forStatement()
{
    consume(loxc::LEFT_PAREN, "Expected '(' after for.");

    Stmt initializer;
    // Initializer is already monostate. I think this is more
    // clear / doccuments intent better.
    if (match(loxc::SEMICOLON))
        initializer = std::monostate{};
    else if (match(loxc::VAR))
        initializer = variableDeclaration();
    else
        initializer = expressionStatement();

    Expr condition(std::monostate{});
    if ( ! check(loxc::SEMICOLON) )
        condition = expression();

    consume(loxc::SEMICOLON, "Expected ';' after for loop condition.");

    Expr increment(std::monostate{});
    if ( ! check(loxc::RIGHT_PAREN) )
        increment = expression();

    consume(loxc::RIGHT_PAREN, "Expected ')' after for.");
    Stmt body = statement();

    // A for loop is just sugar for a while loop. Here we build the
    // while loop syntax tree.
    if ( ! std::holds_alternative<std::monostate>(increment) )
        body = std::make_shared<BlockStmt>(
            std::vector<Stmt>({body, std::make_shared<ExprStmt>(increment)})
            );
    
    // A null condition is always true
    if ( std::holds_alternative<std::monostate>(condition) )
        condition = std::make_shared<LiteralExpr>(true);

    body = std::make_shared<WhileStmt>(condition, body);

    if ( ! std::holds_alternative<std::monostate>(initializer) )
        body = std::make_shared<BlockStmt>(
            std::vector<Stmt>({initializer, body})
            );

    // done :)
    return body;
}

Stmt Parser::expressionStatement()
{
    Expr expr = expression();
    consume(loxc::SEMICOLON, "Expected ; after expression statment.");
    return std::make_shared<ExprStmt>(std::move(expr));
}

Expr Parser::expression()
{
    return assignment();
}

Expr Parser::assignment()
    {
    // or -> and -> equality
    Expr expr = logical_or();

    if(match(loxc::EQUAL))
        {
        loxc::token equals = previous();
        Expr val = assignment();

        if (std::holds_alternative<std::shared_ptr<VarExpr>>(expr))
            {
            loxc::token name = std::get<std::shared_ptr<VarExpr>>(expr)->name;
            return std::make_shared<RedefExpr>(name, val);
            }
        error(equals, "Invalid assignment.");
        }

    return expr;
    }

Expr Parser::logical_or()
{
    Expr left = logical_and();

    while (match(loxc::OR))
    {
        loxc::token op = previous();
        Expr right = logical_and();
        left = std::make_shared<LogicExpr>
            (std::move(left), std::move(op), std::move(right));
    }

    return left;
}

Expr Parser::logical_and()
{
    Expr left = equality();

    while (match(loxc::AND))
    {
        loxc::token op = previous();
        Expr right = equality();
        left = std::make_shared<LogicExpr>
            (std::move(left), std::move(op), std::move(right));
    }

    return left;
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
    return call();
}

Expr Parser::call()
{
    Expr who = primary();

    while (true)
    {
        if ( match (loxc::LEFT_PAREN) )
            who = finishCall(std::move(who));
        else
            break;
    }

    return who;
}

Expr Parser::finishCall(Expr callee)
{
    std::vector<Expr> args;
    if ( ! check(loxc::RIGHT_PAREN) )
        do {
            args.push_back(expression());
        } while ( match(loxc::COMMA) );

    if (args.size() >= 255)
        error(peek(), "Functions can have a maximum of 255 arguments.");

    loxc::token paren = consume(loxc::RIGHT_PAREN, "Expected ')' after function call.");

    return std::make_shared<CallExpr>(callee, paren, args);
}

Expr Parser::primary()
{
    if (match(loxc::FALSE))
        return std::make_shared<LiteralExpr>(false);
    if (match(loxc::TRUE))
        return std::make_shared<LiteralExpr>(true);
    if (match(loxc::NIL))
        return std::make_shared<LiteralExpr>(std::monostate{});

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