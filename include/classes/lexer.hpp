#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include "common/constants.hpp"
#include "classes/token.hpp"

class Lexer {
    private:
        std::string raw_source;
        std::size_t cursor;

        bool is_character(unsigned char c) {
            if (c >= 65 && c <= 90) {
                return true;
            }

            if (c >= 97 && c <= 122) {
                return true;
            }

            return false;
        }

        bool is_digit(unsigned char c) {
            return c >= 48 && c <= 57;
        }

        bool is_operator(unsigned char c) {
            char ch = static_cast<char>(c);
            std::string_view view {&ch, 1};

            return std::any_of(OPERATORS.begin(), OPERATORS.end(), [&](std::string_view op) {
                return op.starts_with(view);
            });
        }

        bool is_boolean(std::string_view buffer) {
            return (buffer == "true" || buffer == "false");
        }

        unsigned char peek() const {
            if ((cursor + 1) >= raw_source.size()) {
                return '\0';
            }

            return raw_source[cursor + 1];
        }

        unsigned char current() const {
            if (is_end()) {
                return '\0';
            }

            return raw_source[cursor];
        }

        unsigned char advance() {
            if (is_end()) {
                return '\0';
            }

            unsigned char c = current();
            ++cursor;

            return c;
        }

        bool is_end() const {
            return cursor >= raw_source.size();
        }

        Token lex_identifier() {
            std::string buffer;

            while (is_character(current())) {
                unsigned char c = advance();
                buffer += c;

                if (std::find(KEYWORDS.begin(), KEYWORDS.end(), buffer) != KEYWORDS.end()) {
                    return Token(TokenType::Keyword, buffer);
                }
            }

            if (is_boolean(buffer)) {
                return Token(TokenType::BooleanLiteral, buffer);
            }

            return Token(TokenType::Identifier, buffer);
        }

        Token lex_string(unsigned char string_start) {
            advance();
            std::string buffer;

            while (!is_end() && current() != string_start) {
                unsigned char c = advance();
                buffer += c;
            }

            advance();

            return Token(TokenType::String, buffer);
        }

        bool is_operator_prefix(const std::string& text){
            return std::any_of(
                OPERATORS.begin(),
                OPERATORS.end(),
                [&](const std::string_view& op) {
                    return op.starts_with(text);
                }
            );
        }

        Token lex_operator() {
            std::string best_match;
            std::string buffer;

            while (!is_end()) {
                std::string next = buffer;
                next += current();

                if (!is_operator_prefix(next)) {
                    break;
                }

                advance();
                buffer = next;

                if (std::find(OPERATORS.begin(), OPERATORS.end(), buffer) != OPERATORS.end()) {
                    best_match = buffer;
                }
            }

            if (best_match.empty()) {
                throw std::runtime_error("Error while lexing operator.");
            }

            return Token(TokenType::Operator, best_match);
        }

        Token lex_digits() {
            std::string buffer;
            bool seen_decimal = false;

            while (is_digit(current())) {
                if (peek() == '.') {
                    if (seen_decimal) {
                        throw std::runtime_error("Invalid floating point number.");
                    }

                    buffer += advance();
                    seen_decimal = true;
                }

                unsigned char c = advance();
                buffer += c;
            }

            return Token(TokenType::NumberLiteral, buffer);
        }
        
    public:
        Lexer(std::string source) : raw_source(std::move(source)), cursor(0) {}

        std::vector<Token> lex() {
            std::vector<Token> tokens;

            while (!is_end()) {
                unsigned char c = current();

                if (c == WHITESPACE) {
                    advance();
                    continue;
                }

                if (is_character(c)) {
                    tokens.push_back(lex_identifier());
                    continue;
                }

                if (is_operator(c)) {
                    tokens.push_back(lex_operator());
                    continue;
                }

                if (is_digit(c)) {
                    tokens.push_back(lex_digits());
                    continue;
                }

                if (c == '(') {
                    tokens.push_back(Token(TokenType::LeftParenthesis, "("));
                    advance();
                    continue;
                }

                if (c == ')') {
                    tokens.push_back(Token(TokenType::RightParenthesis, ")"));
                    advance();
                    continue;
                }

                if (c == '{') {
                    tokens.push_back(Token(TokenType::LeftCurlyBraces, "{"));
                    advance();
                    continue;
                }

                 if (c == '}') {
                    tokens.push_back(Token(TokenType::RightCurlyBraces, "}"));
                    advance();
                    continue;
                }

                if (c == '"' || c == '\'' || c == '`') {
                    tokens.push_back(lex_string(c));
                    continue;
                }

                if (c == ',') {
                    tokens.push_back(Token(TokenType::Comma, ","));
                    advance();
                    continue;
                }

                if (c == ';') {
                    tokens.push_back(Token(TokenType::Semicolon, ";"));
                    advance();
                    continue;
                }

                    advance(); // TODO: error handle properly
            }

            tokens.push_back(Token(TokenType::EndOfFile, "EOF"));

            return tokens;
        }
};