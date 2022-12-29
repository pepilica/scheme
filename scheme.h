#pragma once

#include <string>
#include <sstream>
#include "tokenizer.h"
#include "parser.h"
#include "error.h"
#include "functions.h"

class Interpreter {
public:
    std::string Run(const std::string& stream);

private:
    std::shared_ptr<Scope> global_scope_;
};
