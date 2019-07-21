// operations that can be applied to expressions
#ifndef op_h
#define op_h

#include <memory>
#include <string>
#include <initializer_list>

#include "expr.h"
#include "val.h"
#include "stmt.h"
#include "enviroment.h"

#define DECLARE_EXPR_VISITOR(return_type)                   \
return_type operator()(std::shared_ptr<BinaryExpr> e);      \
return_type operator()(std::shared_ptr<GroupingExpr> e);    \
return_type operator()(std::shared_ptr<LiteralExpr> e);     \
return_type operator()(std::shared_ptr<UnaryExpr> e);       \
return_type operator()(std::shared_ptr<VarExpr> e);         \
return_type operator()(std::monostate);                     \

#define DECLARE_STMT_VISITOR(return_type)                   \
return_type operator()(std::shared_ptr<PrintStmt> s);       \
return_type operator()(std::shared_ptr<ExprStmt> s);        \
return_type operator()(std::shared_ptr<VarStmt> s);         \

namespace op
{

struct interpreter
{
    static Enviroment enviroment;

    DECLARE_EXPR_VISITOR(Val)

    Val operator()(std::shared_ptr<PrintStmt> s);
    Val operator()(std::shared_ptr<ExprStmt> s);
    Val operator()(std::shared_ptr<VarStmt> s);
        
    inline void assert_numeric(loxc::token op, const Val& v1, const Val& v2)
    {
        if (!std::holds_alternative<double>(v1) || 
            !std::holds_alternative<double>(v2))
                throw runtime_error(op, "Operands must be numbers.");
    }
};

/**
 * Walks the AST and prints a lisp-ish interpretation.
 * 
 * ex: (* (- (1.2)) (grouping (42.5)))
 */
struct ast_printer
{
    DECLARE_EXPR_VISITOR(std::string)
    std::string parenthesize(std::string name, std::initializer_list<Expr> in);
};

/**
 * Walks the AST and prints it in Reverse Polish Notation.
 * 
 * ex: (1 + 2) * (4 - 3) -> 1 2 + 4 3 - *
 */
struct rps_printer
{
    DECLARE_EXPR_VISITOR(std::string)
    std::string reverse_polish(std::string name, std::initializer_list<Expr> in);
};

} // namespace op

#undef DECLARE_EXPR_VISITOR
#endif