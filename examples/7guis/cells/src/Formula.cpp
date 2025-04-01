//
// Created by alex2772 on 3/7/25.
//

#include "Formula.h"
#include "AUI/Common/AVector.h"
#include "AUI/Util/ATokenizer.h"
#include "Tokens.h"
#include "AUI/Logging/ALogger.h"
#include "AST.h"
#include "AUI/Util/AEvaluationLoopException.h"

formula::Value formula::evaluate(const Spreadsheet& spreadsheet, const AString& expression) {
    return precompile(expression)(spreadsheet);
}

formula::Precompiled formula::precompile(const AString& expression) {
    if (expression.empty()) {
        return [](const Spreadsheet&) { return std::nullopt; };
    }
    if (auto d = expression.toDouble()) {
        return [d = *d](const Spreadsheet&) { return d; };
    }
    if (!expression.startsWith('=')) {
        return [stringConstant = expression](const Spreadsheet&) { return stringConstant; };
    }
    try {
        auto tokens = token::parse(ATokenizer(expression));
        auto p = ast::parseExpression(tokens);

        return [p = std::shared_ptr(std::move(p))](const Spreadsheet& ctx) -> formula::Value {
            try {
                return p->evaluate(ctx);
            } catch (const AEvaluationLoopException& e) {
                return "#LOOP!";
            } catch (const AException& e) {
                return "#{}!"_format(e.getMessage());
            }
        };
    } catch (const AException& e) {
        ALogger::err("Formula") << "Can't parse expression " << expression << "\n" << e;
        return [msg = e.getMessage()](const Spreadsheet&) { return "#{}!"_format(msg); };
    }
}
