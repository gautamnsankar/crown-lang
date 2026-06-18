#pragma once

enum class TypeKind : unsigned char {
    Unknown,
    Boolean,
    String,
    Double,
    Error,
    Class,
    Void,
    Int
};

enum class NumberType : unsigned char {
    Integer,
    Double
};

class Type {
    public:
        std::string class_name;
        TypeKind kind;

        Type() :
            kind(TypeKind::Unknown) {}

        Type(TypeKind kind) :
            kind(kind) {}

        Type(TypeKind kind, std::string class_name) :
            class_name(std::move(class_name)), kind(kind) {}

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

            if (kind == TypeKind::Class) {
                return "class";
            }

            return "unknown";
        }
};