#ifndef TINYJS_EVAL
#define TINYJS_EVAL
#include "ast.h"
#include "env.h"

namespace Eval
{
    using namespace AST;

    template <typename T>
    class EvalImpl : public Env::EnvImpl<T>
    {
    private:
        // T => vector<unique_ptr<ExprAST>>
        T& Expression;

    public:
        EvalImpl() = delete;
        EvalImpl(T&& Expression) : Expression(Expression) {}
        ~EvalImpl() = default;
        
        EvalImpl(const EvalImpl&) = delete;
        const EvalImpl& operator =(const EvalImpl&) = delete;
        EvalImpl(EvalImpl&&) = delete;
        const EvalImpl& operator =(EvalImpl&&) = delete;

        void eval()
        {
            for (auto i : Expression)
                action(i);
        }

        void action(std::unique_ptr<ExprAST> expr)
        {
            // if (expr->get_type() == Type::binary_op_expr)
            //     action()
        }

        void eval_value_expr(std::unique_ptr<ExprAST> expr)
        {
            return;
        }
    };
}

#endif