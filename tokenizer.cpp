#include "tokenizer.h"

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

bool SymbolToken::operator==(const SymbolToken &other) const {
    return (name == other.name);
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return (value == other.value);
}

Tokenizer::Tokenizer(std::istream *in)
    : symbols_begin_regex_(std::regex("[a-zA-Z<=>*/#]")),
      symbols_regex_(std::regex("[a-zA-Z<=>*/#0-9?!-]")),
      numbers_regex_(std::regex("[0-9]")),
      stream_(&(*in >> std::ws)),
      token_(),
      ended_(false) {
    Next();
}

bool Tokenizer::IsEnd() {
    return std::holds_alternative<Emptiness>(token_);
}

void Tokenizer::Next() {
    token_ = Emptiness();
    // while (std::isspace(stream_->peek()) && stream_) {
    //     char dummy;
    //     *stream_ >> dummy;
    // }
    *stream_ >> std::ws;
    bool is_symbol = false;
    bool is_value = false;
    std::string buffer;
    int res = stream_->peek();
    while (res != ' ' && !stream_->eof()) {
        if (res == EOF || res == '\n') {
            if (!stream_->get()) {
                ended_ = true;
                break;
            } else {
                stream_->get();
                res = stream_->peek();
                continue;
            }
        }
        char next = res;
        if (next == '.') {
            if (!is_value && !is_symbol) {
                token_ = DotToken();
                stream_->get();
            }
            break;
        } else if (next == '\'') {
            if (!is_value && !is_symbol) {
                token_ = QuoteToken();
                stream_->get();
            }
            break;
        } else if (next == '(') {
            if (!is_value && !is_symbol) {
                token_ = BracketToken::OPEN;
                stream_->get();
            }
            break;
        } else if (next == ')') {
            if (!is_value && !is_symbol) {
                token_ = BracketToken::CLOSE;
                stream_->get();
            }
            break;
        } else if (std::regex_match(&next, &next + 1, symbols_begin_regex_)) {
            is_symbol = true;
            buffer.push_back(next);
        } else if ((next == '+' || next == '-') && !is_symbol && !is_value) {
            is_symbol = true;
            is_value = true;
            buffer.push_back(next);
        } else if ((next == '+' || next == '-') && is_value) {
            break;
        } else if (std::regex_match(&next, &next + 1, numbers_regex_)) {
            if (is_value && is_symbol) {
                is_symbol = false;
                if (buffer.back() == '+') {
                    buffer.pop_back();
                }
                buffer.push_back(next);
            } else if (is_value || buffer.empty()) {
                is_value = true;
                buffer.push_back(next);
            } else if (std::regex_match(&buffer.back(), symbols_begin_regex_)) {
                is_value = false;
                buffer.push_back(next);
            } else if (is_symbol) {
                buffer.push_back(next);
            } else {
                throw SyntaxError(" ");
            }
        } else if (std::regex_match(&next, &next + 1, symbols_regex_) && is_symbol) {
            buffer.push_back(next);
        } else {
            throw SyntaxError(" ");
        }
        if (stream_->eof()) {
            ended_ = true;
            return;
        }
        stream_->get();
        res = stream_->peek();
    }
    if (res == EOF || res == '\n') {
        if (!stream_->get()) {
            ended_ = true;
        }
    }
    if (stream_->eof()) {
        ended_ = true;
    }
    if (is_symbol && is_value) {
        is_value = false;
    }
    if (is_value) {
        token_ = ConstantToken{std::stoi(buffer)};
    } else if (is_symbol) {
        token_ = SymbolToken{buffer};
    }
}

Token Tokenizer::GetToken() {
    return token_;
}

bool Emptiness::operator==(const Emptiness &other) const {
    return true;
}
