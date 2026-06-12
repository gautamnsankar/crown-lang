#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "classes/parser.hpp"

class FunctionSymbol {
    public:
        std::vector<Type> parameter_types;
        std::string name;
        Type return_type;
};

class VariableSymbol {
    public:
        std::string name;
        Type type;
};

class SemanticAnalyzer {
    private:
        std::unordered_map<std::string, FunctionSymbol> functions;
        std::unordered_map<std::string, VariableSymbol> variables;

    public:
        SemanticAnalyzer() = default;
        
        void analyze(const Program& ast) {

        }
};