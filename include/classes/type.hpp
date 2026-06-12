#pragma once

enum class TypeKind : unsigned char {
    Unknown,
    Boolean,
    Float,
    Error,
    Void,
    Int
};

enum class NumberType : unsigned char {
    Integer,
    Float
};

class Type {
    public:
        TypeKind kind;

        std::string_view to_string() const {
            if (kind == TypeKind::Boolean) {
                return "boolean";
            }

            if (kind == TypeKind::Int) {
                return "int";
            }

            if (kind == TypeKind::Void) {
                return "void";
            }

            return "unknown";
        }
};