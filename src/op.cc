#include <string>
#include <sstream>
#include <memory>
#include <initializer_list>

#include "expr.h"
#include "op.h"
#include "stmt.h"

/**
 * INTERPRETER
 */

Enviroment op::interpreter::enviroment = Enviroment();

Val op::interpreter::operator()(std::shared_ptr<BinaryExpr> e)
{
    // note that we are evaluating from left to right.
    Val left = std::visit(op::interpreter(), e->left);
    Val right = std::visit(op::interpreter(), e->right);
    
    switch (e->op.type)
    {
        // MATH
        case loxc::MINUS:
            assert_numeric(e->op, left, right);
            return std::get<double>(left) - std::get<double>(right);
        case loxc::PLUS:
            if (std::holds_alternative<double>(left) &&
                std::holds_alternative<double>(right))
                    return std::get<double>(left) + std::get<double>(right);
            if (std::holds_alternative<std::string>(left) &&
                std::holds_alternative<std::string>(right))
                    return std::get<std::string>(left) + std::get<std::string>(right);
            throw op::runtime_error(e->op, "Operands must be numbers or strings.");
        case loxc::SLASH:
            assert_numeric(e->op, left, right);
            return std::get<double>(left) / std::get<double>(right);
        case loxc::STAR:
            assert_numeric(e->op, left, right);
            return std::get<double>(left) * std::get<double>(right);

        // COMPARSON
        case loxc::GREATER:
            assert_numeric(e->op, left, right);
            return left > right;
        case loxc::GREATER_EQUAL:
            assert_numeric(e->op, left, right);
            return left >= right;
        case loxc::LESS_EQUAL:
            assert_numeric(e->op, left, right);
            return left <= right;
        case loxc::LESS:
            assert_numeric(e->op, left, right);
            return left < right;

        // EQUALITY
        case loxc::BANG_EQUAL:
            return left != right;
        case loxc::EQUAL_EQUAL:
            return left == right;

        default:
            throw op::runtime_error(e->op, "Invalid operator.");
    }
}

Val op::interpreter::operator()(std::shared_ptr<GroupingExpr> e)
{
    return std::visit(op::interpreter(), e->expression);
}

Val op::interpreter::operator()(std::shared_ptr<LiteralExpr> e)
{
    return e->value;
}

Val op::interpreter::operator()(std::shared_ptr<UnaryExpr> e)
{
    Val right = std::visit(op::interpreter(), e->right);
    switch (e->op.type)
    {
        case loxc::MINUS:
            return -std::get<double>(right);
        case loxc::BANG:
            return !is_truthy(right);
    }
    // we should never reach this.
    throw op::runtime_error(e->op, "Invalid operator in unary expression.");
}

Val op::interpreter::operator()(std::shared_ptr<VarExpr> e)
{
    return enviroment.get(e->name);
}

Val op::interpreter::operator()(std::shared_ptr<PrintStmt> s)
{
    Val value = std::visit(interpreter(), s->expression);
    std::cout << value << "\n";
    return std::monostate{};
}

Val op::interpreter::operator()(std::shared_ptr<ExprStmt> s)
{
    return std::visit(interpreter(), s->expression);
}

Val op::interpreter::operator()(std::shared_ptr<VarStmt> s)
{
    Val value = std::monostate{};
    if ( ! std::holds_alternative<std::monostate>(s->initializer) )
        value = std::visit(interpreter(), s->initializer);

    enviroment.define(s->name.lexme, value);
    return value;
}

Val op::interpreter::operator()(std::monostate m)
{
    return m;
}

/**
 * AST PRINTER
 */
std::string op::ast_printer::operator()(std::shared_ptr<BinaryExpr> e)
{
    return parenthesize(e->op.lexme, {e->left, e->right});
}

std::string op::ast_printer::operator()(std::shared_ptr<GroupingExpr> e)
{
    return parenthesize("grouping", {e->expression});
}

std::string op::ast_printer::operator()(std::shared_ptr<LiteralExpr> e)
{
    std::stringstream ss;
    ss << e->value;
    return parenthesize(ss.str(), {});
}

std::string op::ast_printer::operator()(std::shared_ptr<UnaryExpr> e)
{
    return parenthesize(e->op.lexme, {e->right});
}

std::string op::ast_printer::operator()(std::shared_ptr<VarExpr> e)
{
    return parenthesize(e->name.lexme, {});
}

std::string op::ast_printer::operator()(std::monostate)
{
    return "<uninitialized>";
}

std::string op::ast_printer::parenthesize(std::string name, std::initializer_list<Expr> in)
{
    std::stringstream builder;
    builder << "(" << name;
    for (const auto &e : in)
        builder << " " << std::visit(ast_printer(), e);
    builder << ")";
    return builder.str();
};

/**
 * RST PRINTER
 */
std::string op::rps_printer::operator()(std::shared_ptr<BinaryExpr> e)
{
    return reverse_polish(e->op.lexme, {e->left, e->right});
}

std::string op::rps_printer::operator()(std::shared_ptr<GroupingExpr> e)
{
    return reverse_polish("", {e->expression});
}

std::string op::rps_printer::operator()(std::shared_ptr<LiteralExpr> e)
{
    std::stringstream ss;
    ss << e->value;
    return reverse_polish(ss.str(), {});
}

std::string op::rps_printer::operator()(std::shared_ptr<UnaryExpr> e)
{
    return reverse_polish(e->op.lexme, {e->right});
}

std::string op::rps_printer::operator()(std::shared_ptr<VarExpr> e)
{
    return reverse_polish(e->name.lexme, {});
}

std::string op::rps_printer::operator()(std::monostate)
{
    return "<uninitialized>";
}

std::string op::rps_printer::reverse_polish(std::string name, std::initializer_list<Expr> in)
{
    std::stringstream builder;
    for (const auto& e : in)
    {
        builder << std::visit(rps_printer(), e) << " ";
    }
    builder << name;
    return builder.str();
}

