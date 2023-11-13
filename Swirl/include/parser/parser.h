#include <array>
#include <list>
#include <memory>
#include <utility>
#include <stack>

#include <tokenizer/Tokenizer.h>
#include <llvm/IR/Value.h>

#ifndef SWIRL_PARSER_H
#define SWIRL_PARSER_H

enum NodeType {
    ND_INVALID,
    ND_EXPR,
    ND_INT,
    ND_FLOAT,
    ND_OP,
    ND_VAR,
    ND_STR,
    ND_CALL,
    ND_IDENT,
    ND_FUNC
};

// A common base class for all the nodes
struct Node {
    struct Param {
        std::string var_ident;
        std::string var_type;

        bool initialized = false;
        bool is_const    = false;
    };

    std::string value;

    const virtual std::vector<std::unique_ptr<Node>>& getExprValue() { throw std::runtime_error("getExprValue called on Node instance"); }
    virtual Param getParamInstance() { return Param{}; }
    virtual std::string getValue() const { throw std::runtime_error("getValue called on base node"); };
    virtual NodeType getType() const { throw std::runtime_error("getType called on base node"); };
    virtual std::vector<Param> getParams() const { throw std::runtime_error("getParams called on base getParams"); };
    virtual llvm::Value* codegen() { throw std::runtime_error("unimplemented Node::codegen"); }
};


struct Op: Node {
    std::string value;

    // the value will be 3 bytes at max so no need of a reference

    Op() = default;
    explicit Op(std::string val): value(std::move(val)) {}

    std::string getValue() const override {
        return value;
    }

    [[nodiscard]] NodeType getType() const override {
        return ND_OP;
    }
};


struct Expression: Node {
    std::vector<std::unique_ptr<Node>> expr{};

    Expression() = default;
    Expression(Expression&& other) noexcept {
        expr.reserve(other.expr.size());

        std::move(
                std::make_move_iterator(other.expr.begin()),
                std::make_move_iterator(other.expr.end()),
                std::back_inserter(expr)
                );

        for (const auto& nd : expr) {
//            std::cout << "moving : " << nd->getValue() << std::endl;
        }
    }

    const std::vector<std::unique_ptr<Node>>& getExprValue() override { return expr; }

    std::string getValue() const override {
        throw std::runtime_error("getValue called on expression");
    }

    NodeType getType() const override {
        return ND_EXPR;
    }

    llvm::Value* codegen() override;
};


struct IntLit: Node {
    std::string value;

    explicit IntLit(std::string val): value(std::move(val)) {}

    std::string getValue() const override {
        return value;
    }

    NodeType getType() const override {
        return ND_INT;
    }

    llvm::Value *codegen() override;
};


struct FloatLit: Node {
    std::string value = 0;

    explicit FloatLit(std::string val): value(std::move(val)) {}

    std::string getValue() const override {
        return value;
    }

    NodeType getType() const override {
        return ND_FLOAT;
    }

    llvm::Value *codegen() override;
};


struct StrLit: Node {
    std::string value;

    explicit StrLit(std::string  val): value(std::move(val)) {}

    std::string getValue() const override {
        return value;
    }

    NodeType getType() const override {
        return ND_STR;
    }

    llvm::Value *codegen() override;
};

struct Ident: Node {
    std::string value;

    explicit Ident(std::string  val): value(std::move(val)) {}

    std::string getValue() const override {
        return value;
    }

    NodeType getType() const override {
        return ND_IDENT;
    }
};

struct Var: Node {
    std::string var_ident;
    std::string var_type;
    Expression value;

    bool initialized = false;
    bool is_const    = false;

    Var() {};
    std::string getValue() const override {
        return var_ident;
    }

    
    NodeType getType() const override {
        return ND_VAR;
    }
};

struct Function: Node {
    std::string ident;
    std::vector<Param> params{};

    NodeType getType() const override {
        return ND_FUNC;
    }

    std::string getValue() const override {
        return ident;
    }

    std::vector<Param> getParams() const override {
        return params;
    }
};

struct FuncCall: Node {
    std::vector<Expression> args;
    std::string ident;

    std::string getValue() const override { return ident; }

    NodeType getType() const override {
        return ND_CALL;
    }

    llvm::Value* codegen() override;
};

class Parser {
    Token cur_rd_tok{};
public:
    TokenStream m_Stream;
//    AbstractSyntaxTree* m_AST;
    bool m_AppendToScope = false;
    std::vector<std::string> registered_symbols{};

    explicit Parser(TokenStream&);

    void parseFunction();
    void parseCondition(TokenType);
    std::unique_ptr<Node> parseCall();
    void dispatch();
    void parseVar();
    void parseExpr(std::vector<Expression>*, bool isCall = false);
    void parseExpr(Expression&, bool isCall = false);
    void parseLoop(TokenType);
//    void appendAST(Node&);
    inline void next(bool swsFlg = false, bool snsFlg = false);

    ~Parser();
};

#endif
