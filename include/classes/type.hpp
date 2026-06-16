#pragma once

enum class TypeKind : unsigned char {
    Unknown,
    Boolean,
    String,
    Double,
    Error,
    Void,
    Int
};

enum class NumberType : unsigned char {
    Integer,
    Double
};

class Type {
    public:
        TypeKind kind;

        std::string to_string() const {
            if (kind == TypeKind::Boolean) {
                return "boolean";
            }

            if (kind == TypeKind::Int) {
                return "int";
            }

            if (kind == TypeKind::String) {
                return "string";
            }

            if (kind == TypeKind::Double) {
                return "double";
            }

            if (kind == TypeKind::Error) {
                return "error";
            }

            if (kind == TypeKind::Void) {
                return "void";
            }

            return "unknown";
        }
};