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
return_type operator()(std::shared_ptr<RedefExpr> e);       \
return_type operator()(std::shared_ptr<LogicExpr> e);       \
return_type operator()(std::monostate);                     \

namespace op
{

struct interpreter
{
    std::shared_ptr<Enviroment> env;

    interpreter(std::shared_ptr<Enviroment> parent_in)
    : env(parent_in)
    {}

    DECLARE_EXPR_VISITOR(Val)

    // TODO(zeke): Evaluating a statement in lox currently returns the last
    // value that was evaluated in the statement. At present this is not
    // accessible from the sripting layer, but I keep doing it as I think 
    // that the idea that an if/while/print/... statement could return a val
    // could prove to be quite powerful.
    Val operator()(std::shared_ptr<PrintStmt> s);
    Val operator()(std::shared_ptr<ExprStmt> s);
    Val operator()(std::shared_ptr<VarStmt> s);
    Val operator()(std::shared_ptr<BlockStmt> s);
    Val operator()(std::shared_ptr<IfStmt> s);
    Val operator()(std::shared_ptr<WhileStmt> s);
        
    inline void assert_numeric(loxc::token op, const Val& v1, const Val& v2)
    {
        if (!std::holds_alternative<double>(v1) || 
            !std::holds_alternative<double>(v2))
                throw runtime_error(op, "Operands must be numbers.");
    }
};

} // namespace op

#undef DECLARE_EXPR_VISITOR
#endif