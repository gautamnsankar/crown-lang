#pragma once

#include <memory>
#include <string>
#include <vector>

#include "classes/token.hpp"

class ASTNode {
    public:
        ASTNode() = default;
        virtual ~ASTNode() = default;

        std::size_t column;
        std::size_t span;
        std::size_t row;
};

class Declaration : public ASTNode {
    public:
        virtual ~Declaration() = default;
};

class Expression : public ASTNode {
    public:
        virtual ~Expression() = default;
};

class Statement : public ASTNode {
    public:
        virtual ~Statement() = default;
};

class Program : public ASTNode {
    public:
        std::vector<std::unique_ptr<Declaration>> declarations;
};

class BlockStatement : public Statement {
    public:
        std::vector<std::unique_ptr<Statement>> statements;
};

class ReturnStatement : public Statement {
    public:
        std::unique_ptr<Expression> value;
        ReturnStatement(std::unique_ptr<Expression> expression) : value(std::move(expression)) {}
};

class NumberLiteral : public Expression {
    public:
        std::string value;
        NumberLiteral(std::string value) : value(std::move(value)) {}
};

class BooleanLiteral : public Expression {
    public:
        bool value;
        BooleanLiteral(bool value) : value(value) {}
};

class BinaryExpression : public Expression {
    public:
        std::unique_ptr<Expression> right;
        std::unique_ptr<Expression> left;
        Token op;

        BinaryExpression(std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right) : 
        left(std::move(left)), 
        op(std::move(op)), 
        right(std::move(right)) {}
};
