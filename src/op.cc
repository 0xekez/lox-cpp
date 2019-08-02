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

Val op::interpreter::operator()(std::shared_ptr<BinaryExpr> e)
{
    // note that we are evaluating from left to right.
    Val left = std::visit(op::interpreter(env), e->left);
    Val right = std::visit(op::interpreter(env), e->right);
    
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
    return std::visit(op::interpreter(env), e->expression);
}

Val op::interpreter::operator()(std::shared_ptr<LiteralExpr> e)
{
    return e->value;
}

Val op::interpreter::operator()(std::shared_ptr<UnaryExpr> e)
{
    Val right = std::visit(op::interpreter(env), e->right);
    switch (e->op.type)
    {
        case loxc::MINUS:
            return -std::get<double>(right);
        case loxc::BANG:
            return !is_truthy(right);
    }
    // We should never reach this.
    throw op::runtime_error(e->op, "Invalid operator in unary expression.");
}

Val op::interpreter::operator()(std::shared_ptr<VarExpr> e)
{
    return env->get(e->name);
}

Val op::interpreter::operator()(std::shared_ptr<RedefExpr> e)
{
    Val value = std::visit(op::interpreter(env), e->value);
    env->assign(e->name, value);
    return value;
}

Val op::interpreter::operator()(std::shared_ptr<LogicExpr> e)
{
    Val left = std::visit(op::interpreter(env), e->left);
    if (e->op.type == loxc::OR)
        if (is_truthy(left))
            return left;
    if (e->op.type == loxc::AND)
        if ( ! is_truthy(left) )
            return left;
    return std::visit(op::interpreter(env), e->right);
}

Val op::interpreter::operator()(std::unique_ptr<PrintStmt> s)
{
    Val value = std::visit(interpreter(env), s->expression);
    std::cout << value << "\n";
    return std::monostate{};
}

Val op::interpreter::operator()(std::unique_ptr<ExprStmt> s)
{
    return std::visit(interpreter(env), s->expression);
}

Val op::interpreter::operator()(std::unique_ptr<VarStmt> s)
{
    Val value = std::monostate{};
    if ( ! std::holds_alternative<std::monostate>(s->initializer) )
        value = std::visit(interpreter(env), s->initializer);
    // Throw runtime error here if we want to require variables to have
    // initializers.

    env->define(s->name.lexme, value);

    return value;
}

Val op::interpreter::operator()(std::unique_ptr<BlockStmt> s)
{
    auto block_env = std::make_shared<Enviroment>();
    block_env->parent = env;
    
    Val last;

    for (Stmt& stmt : s->stmt_list)
        last = std::visit(interpreter(block_env), std::move(stmt));

    return last;
}

Val op::interpreter::operator()(std::unique_ptr<IfStmt> s)
{
    if ( is_truthy(std::visit(interpreter(env), std::move(s->condition))) )
        return std::visit(interpreter(env), std::move(s->t_branch));
    else if ( ! std::holds_alternative<std::monostate>(s->f_branch) )
        return std::visit(interpreter(env), std::move(s->f_branch));

    return std::monostate{};
}

Val op::interpreter::operator()(std::monostate m)
{
    return m;
}

