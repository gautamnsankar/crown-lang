#pragma once

#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "classes/parser.hpp"
#include "classes/lexer.hpp"

class ImportResolver {
    private:
        std::unordered_set<std::string> loaded_files;

        std::filesystem::path resolve_import_path(const std::string& path) {
            if (path == "std") {
                return std::filesystem::path("stdlib") / "std.cw";
            }

            if (path == "math") {
                return std::filesystem::path("stdlib") / "math.cw";
            }

            return std::filesystem::path(path + ".cw");
        }

        std::string read_file(const std::filesystem::path& path) {
            std::ifstream file(path);

            if (!file) {
                throw std::runtime_error("Could not import: " + path.string());
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            return buffer.str();
        }

        Program parse_file(const std::filesystem::path& path) {
            std::string source = read_file(path);

            Lexer lexer(source);
            auto tokens = lexer.lex();

            Parser parser(tokens);
            return parser.parse();
        }
    public:
        Program resolve(Program program) {
            Program result;

            for (auto& declaration : program.declarations) {
                if (auto* import = dynamic_cast<ImportDeclaration*>(declaration.get())) {
                    std::filesystem::path path = resolve_import_path(import->path).string();
                    std::string real_path = std::filesystem::absolute(path).string();

                    if (loaded_files.contains(real_path)) {
                        continue;
                    }

                    loaded_files.insert(real_path);

                    Program imported_program = parse_file(path);
                    Program resolved_import = resolve(std::move(imported_program));

                    for (auto& imported : resolved_import.declarations) {
                        result.declarations.push_back(std::move(imported));
                    }

                    continue;
                }

                result.declarations.push_back(std::move(declaration));
            }

            return result;
        }
};