/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <range/v3/all.hpp>
#include "AST.h"
#include "AUI/Traits/variant.h"
#include "Functions.h"
#include <AUI/Common/AMap.h>

using namespace ast;

namespace {

template <typename type>
constexpr size_t got = aui::variant::index_of<token::Any, type>::value;

template <typename T, typename Variant>
const T& expect(const Variant& variant) {
    if (std::holds_alternative<T>(variant)) {
        return std::get<T>(variant);
    }
    throw AException("VALUE {}"_format(AClass<T>::name()).uppercase());
}

struct BinaryOperatorNode : public INode {
    _unique<INode> left;
    _unique<INode> right;
};

template <typename F>
struct BinaryOperatorNodeImpl : BinaryOperatorNode {
    virtual ~BinaryOperatorNodeImpl() = default;

    formula::Value evaluate(const Spreadsheet& ctx) override {
        return double(F {}(expect<double>(left->evaluate(ctx)), expect<double>(right->evaluate(ctx))));
    }
};

struct DoubleNode : INode {
    double value;
    explicit DoubleNode(double value) : value(value) {}
    ~DoubleNode() override = default;

    formula::Value evaluate(const Spreadsheet& ctx) override { return value; }
};

struct StringLiteralNode : INode {
    AString value;
    explicit StringLiteralNode(AString value) : value(std::move(value)) {}
    ~StringLiteralNode() override = default;

    formula::Value evaluate(const Spreadsheet& ctx) override { return value; }
};

struct RangeNode : INode {
    formula::Range range;
    explicit RangeNode(const formula::Range& range) : range(range) {}

    ~RangeNode() override = default;

    formula::Value evaluate(const Spreadsheet& ctx) override { return range; }
};

struct IdentifierNode : INode {
    AString name;
    explicit IdentifierNode(AString name) : name(std::move(name)) {}
    ~IdentifierNode() = default;
    formula::Value evaluate(const Spreadsheet& ctx) override {
        auto result = *ctx[Cell::fromName(name)].value;
        if (std::holds_alternative<std::nullopt_t>(result)) {
            return 0.0;
        }
        return result;
    }
};

class AstState {
public:
    explicit AstState(std::span<token::Any> tokens) : mTokens(tokens) {}

    _unique<INode> parseExpression() {
        _unique<INode> temporaryValue;   // storage for temporary non-binary nodes such ast constants, function calls,
                                         // etc

        struct BinaryOperatorAndItsPriority {
            BinaryOperatorNode* op;
            int priority = -1;
            _unique<BinaryOperatorNode> owning;
        };

        AVector<BinaryOperatorAndItsPriority> binaryOperators;

        auto putValue = [&](_unique<INode> node) {
            if (temporaryValue) {
                throw AException("SYNTAX");
            }
            if (!binaryOperators.empty()) {
                if (binaryOperators.last().op->right) {
                    throw AException {};
                }
                binaryOperators.last().op->right = std::move(node);
                return;
            }
            temporaryValue = std::move(node);
        };

        auto takeValue = [&] {
            if (!temporaryValue) {
                throw AException {};
            }
            auto v = std::move(temporaryValue);
            temporaryValue = nullptr;   // to be sure
            return v;
        };

        enum class Priority {
            // to do last
            ASSIGNMENT,
            COMPARISON,
            BINARY_SHIFT,
            PLUS_MINUS,
            ASTERISK_SLASH,
            ARRAY_ACCESS,
            MEMBER_ACCESS,
            // to do first
        };

        auto handleBinaryOperator = [&]<aui::derived_from<BinaryOperatorNode> T>(Priority p) {
            mIterator++;
            const int currentPriority = int(p);

            if (temporaryValue) {
                auto out = std::make_unique<T>();
                out->left = std::move(temporaryValue);
                binaryOperators << BinaryOperatorAndItsPriority {
                    .op = out.get(),
                    .priority = currentPriority,
                    .owning = std::move(out),
                };
                AUI_ASSERT(temporaryValue == nullptr);
                return;
            }

            for (const auto& o : binaryOperators | ranges::views::reverse) {
                if (o.priority < currentPriority && o.op->right) {
                    // steal rhs
                    auto currentOperator = std::make_unique<T>();
                    currentOperator->left = std::move(o.op->right);
                    auto ptr = currentOperator.get();
                    o.op->right = std::move(currentOperator);
                    binaryOperators << BinaryOperatorAndItsPriority {
                        .op = ptr,
                        .priority = currentPriority,
                    };
                    return;
                }
            }
            if (!binaryOperators.empty()) {
                auto root = std::min_element(
                    binaryOperators.begin(), binaryOperators.end(),
                    [](const BinaryOperatorAndItsPriority& lhs, const BinaryOperatorAndItsPriority& rhs) {
                        return lhs.priority < rhs.priority;
                    });
                auto out = std::make_unique<T>();
                AUI_ASSERT(root->owning != nullptr);
                out->left = std::move(root->owning);
                binaryOperators << BinaryOperatorAndItsPriority {
                    .op = out.get(),
                    .priority = currentPriority,
                    .owning = std::move(out),
                };
                AUI_ASSERT(temporaryValue == nullptr);
                return;
            }

            throw AException {};
        };

        auto handleUnaryOperator = [&]<aui::derived_from<INode> T>() {

        };

        for (; mIterator != mTokens.end();) {
            const auto& currentTokenValue = currentToken();
            switch (currentTokenValue.index()) {
                case got<token::Identifier>: {
                    if (auto it = std::next(mIterator);
                        it != mTokens.end()) {
                        switch (it->index()) {
                            case got<token::LPar>:
                                putValue(parseFunctionCall());
                                continue;
                            case got<token::Colon>:
                                putValue(parseRange());
                                continue;
                            default:
                                break;
                        }
                    }
                    putValue(parseIdentifier());
                    mIterator++;
                    break;
                }

                case got<token::Plus>: {
                    handleBinaryOperator.operator()<BinaryOperatorNodeImpl<std::plus<>>>(Priority::PLUS_MINUS);
                    break;
                }

                case got<token::Minus>: {
                    handleBinaryOperator.operator()<BinaryOperatorNodeImpl<std::minus<>>>(Priority::PLUS_MINUS);
                    break;
                }

                case got<token::Asterisk>: {   // pointer dereference or multiply
                    handleBinaryOperator.
                    operator()<BinaryOperatorNodeImpl<std::multiplies<>>>(Priority::ASTERISK_SLASH);
                    break;
                }

                case got<token::Slash>: {   // divide
                    handleBinaryOperator.
                    operator()<BinaryOperatorNodeImpl<std::divides<>>>(Priority::ASTERISK_SLASH);
                    break;
                }

                case got<token::LAngle>: {
                    handleBinaryOperator.
                        operator()<BinaryOperatorNodeImpl<std::less<>>>(Priority::COMPARISON);
                    break;
                }

                case got<token::RAngle>: {
                    handleBinaryOperator.
                        operator()<BinaryOperatorNodeImpl<std::greater<>>>(Priority::COMPARISON);
                    break;
                }

                case got<token::Double>: {
                    putValue(parseDouble());
                    ++mIterator;
                    break;
                }

                case got<token::StringLiteral>: {
                    putValue(parseStringLiteral());
                    ++mIterator;
                    break;
                }

                case got<token::LPar>: {
                    ++mIterator;
                    putValue(parseExpression());
                    expect<token::RPar>();
                    ++mIterator;
                    break;
                }

                default:
                    goto naxyi;
            }
        }
    naxyi:
        if (temporaryValue && !binaryOperators.empty()) {
            // should assign it to some operator
            for (const auto& o : binaryOperators | ranges::views::reverse) {
                if (o.op->right == nullptr) {
                    o.op->right = std::move(temporaryValue);
                    AUI_ASSERT(binaryOperators.first().owning != nullptr);
                    return std::move(binaryOperators.first().owning);
                }
            }
            throw AException {};
        }
        if (!binaryOperators.empty()) {
            auto e = ranges::min_element(binaryOperators, [](const auto& l, const auto& r) {
                return l.priority <= r.priority;
            });
            AUI_ASSERT(e->owning != nullptr);
            return std::move(e->owning);
        }
        if (temporaryValue) {
            return temporaryValue;
        }
        throw AException {};
    }

private:
    std::span<token::Any> mTokens;
    std::span<token::Any>::iterator mIterator = mTokens.begin();

    const token::Any& currentToken() { return *safeIteratorRead(mIterator); }

    std::span<token::Any>::iterator safeIteratorRead(std::span<token::Any>::iterator it) {
        if (it == mTokens.end()) {
            throw AException("END");
        }
        return it;
    }

    template <typename T>
    const T& expect() {
        return ::expect<T>(currentToken());
    }

    _unique<INode> parseFunctionCall() {
        struct FunctionCall : INode {
            functions::Invocable function;
            AVector<_unique<INode>> args;

            ~FunctionCall() override = default;
            formula::Value evaluate(const Spreadsheet& ctx) override {
                return function(functions::Ctx {
                  .spreadsheet = ctx,
                  .args = AVector(
                      args | ranges::view::transform([&](const _unique<INode>& node) { return node->evaluate(ctx); }) |
                      ranges::to_vector),
                });
            }
        };
        auto out = std::make_unique<FunctionCall>();
        out->function = functions::predefined().at(expect<token::Identifier>().name.uppercase());
        mIterator++;
        expect<token::LPar>();
        mIterator++;

        for (;;) {
            switch (auto it = safeIteratorRead(mIterator); it->index()) {
                case got<token::RPar>:
                    ++mIterator;
                    return out;
                case got<token::Semicolon>:
                    ++mIterator;
                    break;
                default:
                    out->args << parseExpression();
            }
        }

        return out;
    }

    _unique<INode> parseIdentifier() { return std::make_unique<IdentifierNode>(expect<token::Identifier>().name); }

    _unique<INode> parseDouble() { return std::make_unique<DoubleNode>(expect<token::Double>().value); }
    _unique<INode> parseStringLiteral() { return std::make_unique<StringLiteralNode>(expect<token::StringLiteral>().value); }

    _unique<INode> parseRange() {
        formula::Range rng;
        rng.from = Cell::fromName(expect<token::Identifier>().name);
        mIterator++;
        expect<token::Colon>();
        mIterator++;
        rng.to = Cell::fromName(expect<token::Identifier>().name);
        mIterator++;
        return std::make_unique<RangeNode>(rng);
    }
};

}   // namespace

_unique<INode> ast::parseExpression(std::span<token::Any> tokens) { return AstState { tokens }.parseExpression(); }
