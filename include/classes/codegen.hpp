#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
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

        std::vector<llvm::BasicBlock*> continue_targets;
        std::vector<llvm::BasicBlock*> break_targets;

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

            if (type.kind == TypeKind::String) {
                return llvm::PointerType::getUnqual(*context);
            }

            throw std::runtime_error("Type not supported by LLVM.");
        }

        llvm::Function* declare_function(const FunctionDeclaration& function) {
            if (llvm::Function* existing = module->getFunction(function.name)) {
                return existing;
            }

            std::vector<llvm::Type*> parameter_types;

            for (const auto& parameter : function.parameters) {
                parameter_types.push_back(generate_type(parameter.type));
            }

            llvm::FunctionType* function_type = llvm::FunctionType::get(generate_type(function.return_type), parameter_types, false);
            llvm::Function *llvm_function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, function.name, module.get());

            std::size_t index = 0;
            for (auto& arg : llvm_function->args()) {
                arg.setName(function.parameters[index].name);
                ++index;
            }

            return llvm_function;
        }

        void generate(const Program& ast) {
            for (const auto& declaration : ast.declarations) {
                if (const auto* function = dynamic_cast<const ExternFunctionDeclaration*>(declaration.get())) {
                    generate_extern_function(*function);
                    continue;
                }

                if (const auto* function = dynamic_cast<const FunctionDeclaration*>(declaration.get())) {
                    declare_function(*function);
                    continue;
                }
            }

            for (const auto& declaration : ast.declarations) {
                if (const auto* function = dynamic_cast<const FunctionDeclaration*>(declaration.get())) {
                    generate_function_body(*function);
                }
            }
        }

        void generate_extern_function(const ExternFunctionDeclaration& function) {
            if (module->getFunction(function.name)) {
                return;
            }

            std::vector<llvm::Type*> parameter_types;

            for (const auto& parameter : function.parameters) {
                parameter_types.push_back(generate_type(parameter.type));
            }

            llvm::FunctionType *function_type = llvm::FunctionType::get(generate_type(function.return_type), parameter_types, false);
            llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, function.name, module.get());
        }

        void generate_function_body(const FunctionDeclaration& function) {
            llvm::Function *created_function = module->getFunction(function.name);

            if (!created_function) {
                throw std::runtime_error("Function was not declared before body generation.");
            }

            if (!created_function->empty()) {
                return;
            }

            llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(*context, "entry", created_function);
            builder->SetInsertPoint(entry_block);

            variables.clear();

            std::size_t index = 0;
            for (auto& llvm_arg : created_function->args()) {
                const auto& parameter = function.parameters[index];
                llvm_arg.setName(parameter.name);

                llvm::AllocaInst* allocation = builder->CreateAlloca(llvm_arg.getType(), nullptr, parameter.name);
                builder->CreateStore(&llvm_arg, allocation);

                variables[parameter.name] = allocation;
                ++index;
            }

            generate_block(*function.body);

            if (!builder->GetInsertBlock()->getTerminator()) {
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
            if (const auto* block = dynamic_cast<const BlockStatement*>(&statement)) {
                generate_block(*block);
                return;
            }
            
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
                llvm::Value* initializer = generate_expression(*variable->value);
                llvm::AllocaInst* allocation = builder->CreateAlloca(initializer->getType(), nullptr, variable->name);

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
                llvm::BasicBlock* body_block = llvm::BasicBlock::Create(*context, "while.body", current_function);
                llvm::BasicBlock* after_block = llvm::BasicBlock::Create(*context, "while.after", current_function);

                builder->CreateBr(condition_block);
                builder->SetInsertPoint(condition_block);

                llvm::Value* condition_value = generate_expression(*loop->condition);

                builder->CreateCondBr(condition_value, body_block, after_block);
                builder->SetInsertPoint(body_block);

                continue_targets.push_back(condition_block);
                break_targets.push_back(after_block);

                generate_block(*loop->body);

                continue_targets.pop_back();
                break_targets.pop_back();

                if (!builder->GetInsertBlock()->getTerminator()) {
                    builder->CreateBr(condition_block);
                }

                builder->SetInsertPoint(after_block);
                return;
            }

            if (const auto* variable = dynamic_cast<const AssignmentStatement*>(&statement)) {
                auto iterator = variables.find(variable->name);

                if (iterator == variables.end()) {
                    throw std::runtime_error("Cannot reassign uninitialized variables.");
                }

                llvm::Value* value = generate_expression(*variable->value);
                llvm::AllocaInst* allocation = iterator->second;

                builder->CreateStore(value, allocation);

                return;
            }

            if (const auto* if_statement = dynamic_cast<const IfStatement*>(&statement)) {
                llvm::Function* current_function = builder->GetInsertBlock()->getParent();
                llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*context, "if.then", current_function);
                llvm::BasicBlock* else_block = llvm::BasicBlock::Create(*context, "if.else", current_function);
                llvm::BasicBlock* after_block = llvm::BasicBlock::Create(*context, "if.after", current_function);

                llvm::Value* condition_value = generate_expression(*if_statement->condition);
                builder->CreateCondBr(condition_value, then_block, else_block);

                builder->SetInsertPoint(then_block);
                generate_block(*if_statement->then_body);

                bool then_terminator = builder->GetInsertBlock()->getTerminator() != nullptr;

                if (!then_terminator) {
                    builder->CreateBr(after_block);
                }

                builder->SetInsertPoint(else_block);

                if (if_statement->else_branch) {
                    generate_statement(*if_statement->else_branch);
                }

                bool else_terminator = builder->GetInsertBlock()->getTerminator() != nullptr;

                if (!else_terminator) {
                    builder->CreateBr(after_block);
                }

                if (then_terminator && else_terminator) {
                    after_block->eraseFromParent();
                } else {
                    builder->SetInsertPoint(after_block);
                }

                return;
            }

            if (const auto* break_statement = dynamic_cast<const BreakStatement*>(&statement)) {
                if (break_targets.empty()) {
                    throw std::runtime_error("Cannot use break outside of loops.");
                }

                builder->CreateBr(break_targets.back());
                return;
            }

            if (const auto* continue_statement = dynamic_cast<const ContinueStatement*>(&statement)) {
                if (continue_targets.empty()) {
                    throw std::runtime_error("Cannot use continue outside of loops.");
                }

                builder->CreateBr(continue_targets.back());
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

                llvm::AllocaInst* allocation = iterator->second;
                return builder->CreateLoad(allocation->getAllocatedType(), allocation, variable->name);
            }

            if (const auto* call = dynamic_cast<const FunctionCall*>(&expression)) {
                llvm::Function* function = module->getFunction(call->callee);

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

                if (binary->op.value == ">") {
                    if (is_double) {
                        return builder->CreateFCmpOGT(left_expression, right_expression);
                    }

                    return builder->CreateICmpSGT(left_expression, right_expression);
                } else if (binary->op.value == ">=") {
                    if (is_double) {
                        return builder->CreateFCmpOGE(left_expression, right_expression);
                    }

                    return builder->CreateICmpSGE(left_expression, right_expression);
                } else if (binary->op.value == "<") {
                    if (is_double) {
                        return builder->CreateFCmpOLT(left_expression, right_expression);
                    }

                    return builder->CreateICmpSLT(left_expression, right_expression);
                } else if (binary->op.value == "<=") {
                    if (is_double) {
                        return builder->CreateFCmpOLE(left_expression, right_expression);
                    }

                    return builder->CreateICmpSLE(left_expression, right_expression);
                } else if (binary->op.value == "==") {
                    if (is_double) {
                        return builder->CreateFCmpOEQ(left_expression, right_expression);
                    }

                    return builder->CreateICmpEQ(left_expression, right_expression);
                }

                if (binary->op.value == "&&") {
                    return builder->CreateAnd(left_expression, right_expression);
                } else if (binary->op.value == "||") {
                    return builder->CreateOr(left_expression, right_expression);
                }

                throw std::runtime_error("Unknown binary operation (LLVM)");
            }

            if (const auto* unary = dynamic_cast<const UnaryExpression*>(&expression)) {
                llvm::Value* value = generate_expression(*unary->right);

                if (unary->op.value == "!") {
                    return builder->CreateICmpEQ(value, llvm::ConstantInt::getFalse(*context));
                }

                if (unary->op.value == "-") {
                    if (value->getType()->isDoubleTy()) {
                        return builder->CreateFNeg(value);
                    }

                    return builder->CreateNeg(value);
                }

                throw std::runtime_error("Unknown unary operation");
            }

            if (const auto* str = dynamic_cast<const StringLiteral*>(&expression)) {
                return builder->CreateGlobalStringPtr(str->value);
            }

            throw std::runtime_error("Unknown expression (LLVM)");
        }

        void visualize_ir() const {
            module->print(llvm::outs(), nullptr);
        }

        void create_executable(const std::string& path) {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();

            auto target_triple = llvm::sys::getDefaultTargetTriple();
            module->setTargetTriple(target_triple);

            std::string error;
            const llvm::Target *target = llvm::TargetRegistry::lookupTarget(target_triple, error);

            if (!target) {
                throw std::runtime_error("LLVM target error: " + error);
            }

            std::string cpu = "generic";
            std::string features;

            llvm::TargetOptions options;
            auto relocation_model = std::optional<llvm::Reloc::Model>();

            std::unique_ptr<llvm::TargetMachine> target_machine(target->createTargetMachine(
                target_triple,
                cpu,
                features,
                options,
                relocation_model
            ));

            module->setDataLayout(target_machine->createDataLayout());

            std::error_code ec;
            llvm::raw_fd_ostream dest(path, ec, llvm::sys::fs::OF_None);

            if (ec) {
                throw std::runtime_error("Could not open output file.");
            }

            llvm::legacy::PassManager pass;

            if (target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
                throw std::runtime_error("TargetMachine cannot emit object file.");
            }

            pass.run(*module);
            dest.flush();
        }

        LLVMCodeGenerator() {
            context = std::make_unique<llvm::LLVMContext>();
            builder = std::make_unique<llvm::IRBuilder<>>(*context);
            module = std::make_unique<llvm::Module>("main", *context);
        }
};