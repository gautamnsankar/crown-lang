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

class StringLiteral : public Expression {
    public:
        std::string value;
        StringLiteral(std::string value) : value(value) {}
};

class BinaryExpression : public Expression {
    public:
        std::unique_ptr<Expression> right;
        std::unique_ptr<Expression> left;
        Token op;

        BinaryExpression(std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right) : 
            left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

class VariableDeclaration : public Statement {
    public:
        std::unique_ptr<Expression> value;
        std::string name;
        Type type;

        VariableDeclaration(std::string name, std::unique_ptr<Expression> value, Type type) :
            name(std::move(name)), value(std::move(value)), type(type) {}
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
            name(std::move(name)), type(std::move(type)) {}
};

class FunctionDeclaration : public Declaration {
    public:
        std::vector<FunctionParameter> parameters;
        std::unique_ptr<BlockStatement> body;
        std::string name;
        Type return_type;

        FunctionDeclaration(std::string name, std::vector<FunctionParameter> params, Type return_type, std::unique_ptr<BlockStatement> body) :
            name(std::move(name)), parameters(std::move(params)), return_type(std::move(return_type)), body(std::move(body)) {}
};

class FunctionCall : public Expression {
    public:
        std::vector<std::unique_ptr<Expression>> arguments;
        std::string callee;

        FunctionCall(std::string callee, std::vector<std::unique_ptr<Expression>> arguments) :
            callee(std::move(callee)), arguments(std::move(arguments)) {}
};

class ExpressionStatement : public Statement {
    public:
        std::unique_ptr<Expression> expression;
        ExpressionStatement(std::unique_ptr<Expression> expression) : expression(std::move(expression)) {}
};

class WhileStatement : public Statement {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<BlockStatement> body;

        WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> body) :
            condition(std::move(condition)), body(std::move(body)) {}
};

class AssignmentStatement : public Statement {
    public:
        std::string name;
        std::unique_ptr<Expression> value;

        AssignmentStatement(std::string name, std::unique_ptr<Expression> value) :
            name(std::move(name)), value(std::move(value)) {}
};

class IfStatement : public Statement {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<BlockStatement> then_body;
        std::unique_ptr<Statement> else_branch;

        IfStatement(std::unique_ptr<Expression> condition, std::unique_ptr<BlockStatement> then, std::unique_ptr<Statement> else_block) :
            condition(std::move(condition)), then_body(std::move(then)), else_branch(std::move(else_block)) {}
};

class BreakStatement : public Statement {
    public:
        BreakStatement() = default;
};

class ContinueStatement : public Statement {
    public:
        ContinueStatement() = default;
};

class UnaryExpression : public Expression {
    public:
        std::unique_ptr<Expression> right;
        Token op;

        UnaryExpression(Token op, std::unique_ptr<Expression> right) :
            op(std::move(op)), right(std::move(right)) {}
};

class ImportDeclaration : public Declaration {
    public:
        std::string path;
        ImportDeclaration(std::string path) : path(std::move(path)) {}
};

class ExternFunctionDeclaration : public Declaration {
    public:
        std::vector<FunctionParameter> parameters;
        std::string name;
        Type return_type;

        ExternFunctionDeclaration(std::string name, std::vector<FunctionParameter> parameters, Type return_type) :
            name(std::move(name)), parameters(std::move(parameters)), return_type(std::move(return_type)) {}
};