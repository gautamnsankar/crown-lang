#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "classes/parser.hpp"

class LLVMCodeGenerator {
    private:
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::Module> module;

        std::unordered_map<std::string, llvm::AllocaInst*> variables;
        std::unordered_map<std::string, llvm::AllocaInst*> functions;

    public:
        llvm::Type* generate_type(const Type& type) {
            if (type.kind == TypeKind::Int) {
                return llvm::Type::getInt64Ty(*context);
            }

            if (type.kind == TypeKind::Double) {
                return llvm::Type::getDoubleTy(*context);
            }

            if (type.kind == TypeKind::Boolean) {
                return llvm::Type::getInt1Ty(*context);
            }

            if (type.kind == TypeKind::Void) {
                return llvm::Type::getVoidTy(*context);
            }

            throw std::runtime_error("Type not supported by LLVM.");
        }

        void generate(const Program& ast) {
            for (const auto& declaration : ast.declarations) {
                generate_declaration(*declaration);
            }
        }

        void generate_declaration(const Declaration& declaration) {
            if (const auto* function = dynamic_cast<const FunctionDeclaration*>(&declaration)) {
                return generate_function(*function);
            }

            throw std::runtime_error("Unknown declaration (LLVM)");
        }

        void generate_function(const FunctionDeclaration& function) {
            std::vector<llvm::Type*> parameter_types;

            for (const auto& paramater : function.parameters) {
                parameter_types.push_back(generate_type(paramater.type));
            }

            llvm::Type *return_type = generate_type(function.return_type);
            llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, parameter_types, false);

            llvm::Function* created_function = llvm::Function::Create(
                function_type,
                llvm::Function::ExternalLinkage,
                function.name,
                module.get()
            );

            llvm::BasicBlock *entry_block = llvm::BasicBlock::Create(*context, "entry", created_function);
            builder->SetInsertPoint(entry_block);

            variables.clear();

            std::size_t index = 0;
            for (auto& llvm_arg : created_function->args()) {
                const auto &parameter = function.parameters[index];
                llvm_arg.setName(parameter.name);

                llvm::AllocaInst *allocation = builder->CreateAlloca(llvm_arg.getType(), nullptr, parameter.name);
                builder->CreateStore(&llvm_arg, allocation);

                variables[parameter.name] = allocation;
                ++index;
            }

            generate_block(*function.body);

            if (!entry_block->getTerminator()) {
                if (function.return_type.kind == TypeKind::Void) {
                    builder->CreateRetVoid();
                } else {
                    throw std::runtime_error("non void function does not have a return statement.");
                }
            }
        }

        void generate_block(const BlockStatement& block) {
            for (const auto& statement : block.statements) {
                generate_statement(*statement);
            }
        }

        void generate_statement(const Statement& statement) {
            if (const auto* ret = dynamic_cast<const ReturnStatement*>(&statement)) {
                if (!ret->value) {
                    builder->CreateRetVoid();
                    return;
                }

                auto return_value = generate_expression(*ret->value);
                builder->CreateRet(return_value);

                return;
            }

            if (const auto* variable = dynamic_cast<const VariableDeclaration*>(&statement)) {
                llvm::Value *initializer = generate_expression(*variable->value);
                llvm::AllocaInst *allocation = builder->CreateAlloca(initializer->getType(), nullptr, variable->name);

                builder->CreateStore(initializer, allocation);
                variables[variable->name] = allocation;

                return;
            }

            if (const auto* expr_statement = dynamic_cast<const ExpressionStatement*>(&statement)) {
                generate_expression(*expr_statement->expression);
                return;
            }

            if (const auto* loop = dynamic_cast<const WhileStatement*>(&statement)) {
                llvm::Function* current_function = builder->GetInsertBlock()->getParent();
                llvm::BasicBlock* condition_block = llvm::BasicBlock::Create(*context, "while.condition", current_function);
                llvm::BasicBlock *body_block = llvm::BasicBlock::Create(*context, "while.body", current_function);
                llvm::BasicBlock *after_block = llvm::BasicBlock::Create(*context, "while.after", current_function);

                builder->CreateBr(condition_block);
                builder->SetInsertPoint(condition_block);

                llvm::Value *condition_value = generate_expression(*loop->condition);

                builder->CreateCondBr(condition_value, body_block, after_block);
                builder->SetInsertPoint(body_block);

                generate_block(*loop->body);

                if (!builder->GetInsertBlock()->getTerminator()) {
                    builder->CreateBr(condition_block);
                }

                builder->SetInsertPoint(after_block);
                return;
            }

            throw std::runtime_error("Unknown statement (LLVM)");
        }

        llvm::Value* generate_expression(const Expression& expression) {
            if (const auto* number = dynamic_cast<const NumberLiteral*>(&expression)) {
                if (number->number_type == NumberType::Integer) {
                    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), std::stoll(number->value), true);
                }

                if (number->number_type == NumberType::Double) {
                    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), std::stod(number->value));
                }
            }

            if (const auto* boolean = dynamic_cast<const BooleanLiteral*>(&expression)) {
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), boolean->value ? 1 : 0, true);
            }

            if (const auto* variable = dynamic_cast<const VariableReference*>(&expression)) {
                auto iterator = variables.find(variable->name);

                if (iterator == variables.end()) {
                    throw std::runtime_error("Variable not defined (LLVM)");
                }

                llvm::AllocaInst *allocation = iterator->second;
                return builder->CreateLoad(allocation->getAllocatedType(), allocation, variable->name);
            }

            if (const auto* call = dynamic_cast<const FunctionCall*>(&expression)) {
                llvm::Function *function = module->getFunction(call->callee);

                if (!function) {
                    throw std::runtime_error("Unknown function (LLVM)");
                }

                std::vector<llvm::Value*> argument_values;
                for (const auto& argument : call->arguments) {
                    argument_values.push_back(generate_expression(*argument));
                }

                if (function->getReturnType()->isVoidTy()) {
                    return builder->CreateCall(function, argument_values);
                }

                return builder->CreateCall(function, argument_values, call->callee + "_call");
            }

            if (const auto* binary = dynamic_cast<const BinaryExpression*>(&expression)) {
                auto left_expression = generate_expression(*binary->left);
                auto right_expression = generate_expression(*binary->right);

                bool is_double = left_expression->getType()->isDoubleTy() || right_expression->getType()->isDoubleTy();

                if (binary->op.value == "+") {
                    if (is_double) {
                        return builder->CreateFAdd(left_expression, right_expression);
                    }
                    return builder->CreateAdd(left_expression, right_expression);
                } else if (binary->op.value == "-") {
                    if (is_double) {
                        return builder->CreateFSub(left_expression, right_expression);
                    }
                    return builder->CreateSub(left_expression, right_expression);
                } else if (binary->op.value == "*") {
                    if (is_double) {
                        return builder->CreateFMul(left_expression, right_expression);
                    }
                    return builder->CreateMul(left_expression, right_expression);
                } else if (binary->op.value == "/") {
                    if (is_double) {
                        return builder->CreateFDiv(left_expression, right_expression);
                    }
                    return builder->CreateSDiv(left_expression, right_expression);
                }

                throw std::runtime_error("Unknown binary operation (LLVM)");
            }

            throw std::runtime_error("Unknown expression (LLVM)");
        }

        void visualize_ir() const {
            module->print(llvm::outs(), nullptr);
        }

        LLVMCodeGenerator() {
            context = std::make_unique<llvm::LLVMContext>();
            builder = std::make_unique<llvm::IRBuilder<>>(*context);
            module = std::make_unique<llvm::Module>("main", *context);
        }
};