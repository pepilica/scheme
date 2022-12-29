#include "parser.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (!tokenizer->IsEnd()) {
        bool was_quote = false;
        std::shared_ptr<Object> res;
        Token next = tokenizer->GetToken();
        if (std::holds_alternative<SymbolToken>(next)) {
            SymbolToken next_token = std::get<SymbolToken>(next);
            if (next_token.name == "#f" || next_token.name == "#t") {
                res = std::make_shared<Bool>(next_token.name);
            } else {
                res = std::make_shared<Symbol>(next_token.name);
            }
        } else if (std::holds_alternative<ConstantToken>(next)) {
            ConstantToken next_token = std::get<ConstantToken>(next);
            res = std::make_shared<Number>(next_token.value);
        } else if (std::holds_alternative<BracketToken>(next)) {
            BracketToken next_token = std::get<BracketToken>(next);
            if (next_token == BracketToken::OPEN) {
                tokenizer->Next();
                res = ReadList(tokenizer);
            } else {
                throw SyntaxError(" ");
            }
        } else if (std::holds_alternative<QuoteToken>(next)) {
            std::shared_ptr<Cell> ans_cell = std::make_shared<Cell>();
            ans_cell->GetFirst() = std::make_shared<Symbol>("quote");
            tokenizer->Next();
            std::shared_ptr<Cell> right_cell = std::make_shared<Cell>();
            right_cell->GetFirst() = Read(tokenizer);
            right_cell->GetSecond() = nullptr;
            ans_cell->GetSecond() = right_cell;
            res = ans_cell;
            was_quote = true;
        } else {
            throw SyntaxError(" ");
        }
        if (!was_quote) {
            tokenizer->Next();
        }
        return res;
    } else {
        throw SyntaxError(" ");
    }
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    bool was_dot = false;
    if (!tokenizer->IsEnd()) {
        std::shared_ptr<Cell> cell = std::make_shared<Cell>();
        if (std::holds_alternative<BracketToken>(tokenizer->GetToken())) {
            if (std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE) {
                return nullptr;
            }
        }
        cell->GetFirst() = Read(tokenizer);
        if (tokenizer->IsEnd()) {
            throw SyntaxError(" ");
        }
        if (std::holds_alternative<DotToken>(tokenizer->GetToken())) {
            was_dot = true;
            tokenizer->Next();
        }
        if (std::holds_alternative<BracketToken>(tokenizer->GetToken())) {
            if (std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE && was_dot) {
                throw SyntaxError(" ");
            }
        }
        if (was_dot) {
            cell->GetSecond() = Read(tokenizer);
        } else {
            cell->GetSecond() = ReadList(tokenizer);
        }
        if (tokenizer->IsEnd()) {
            throw SyntaxError(" ");
        } else {
            if (std::holds_alternative<BracketToken>(tokenizer->GetToken())) {
                if (std::get<BracketToken>(tokenizer->GetToken()) != BracketToken::CLOSE) {
                    throw SyntaxError(" ");
                }
            } else {
                throw SyntaxError(" ");
            }
        }
        return cell;
    } else {
        throw SyntaxError(" ");
    }
}