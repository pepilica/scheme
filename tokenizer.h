#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <regex>
#include "error.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

struct Emptiness {
    bool operator==(const Emptiness& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, Emptiness>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    std::basic_regex<char> symbols_begin_regex_;
    std::basic_regex<char> symbols_regex_;
    std::basic_regex<char> numbers_regex_;
    std::istream* stream_;
    Token token_;
    bool ended_;
};