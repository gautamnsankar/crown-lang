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
        NumberType number_type;
        std::string value;
        NumberLiteral(std::string value, NumberType type) : value(std::move(value)), number_type(type) {}
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

class VariableDeclaration : public Statement {
    public:
        std::unique_ptr<Expression> value;
        std::string name;
        Type type;

        VariableDeclaration(std::string name, std::unique_ptr<Expression> value, Type type) :
            name(std::move(name)),
            value(std::move(value)),
            type(type) {}
};

class VariableReference : public Expression {
    public:
        std::string name;
        VariableReference(std::string name) : name(std::move(name)) {}
};

class FunctionParameter {
    public:
        std::string name;
        Type type;

        FunctionParameter(std::string name, Type type) :
            name(std::move(name)),
            type(std::move(type)) {}
};

class FunctionDeclaration : public Declaration {
    public:
        std::unique_ptr<BlockStatement> body;
        std::vector<FunctionParameter> args;
        std::string name;
        Type return_type;

        FunctionDeclaration(std::string name, std::vector<FunctionParameter> args, Type return_type, std::unique_ptr<BlockStatement> body) :
            name(std::move(name)),
            args(std::move(args)),
            return_type(std::move(return_type)),
            body(std::move(body)) {}
};
