//
// Created by alex2772 on 3/7/25.
//

#include <range/v3/all.hpp>
#include "Functions.h"

namespace {
template<aui::invocable<formula::Value> Callback>
void forEachArgAndRangeCell(const functions::Ctx& ctx, Callback&& callback) {
    for (const auto& arg : ctx.args) {
        if (auto rng = std::get_if<formula::Range>(&arg)) {
            for (const auto& cell : ctx.spreadsheet[*rng]) {
                callback(cell.value);
            }
            continue;
        }
        callback(arg);
    }
}
}

const AMap<AString, functions::Invocable>& functions::predefined() {
    static AMap<AString, functions::Invocable> out = {
        { "SUM",
          [](Ctx ctx) {
              double accumulator = 0.0;
              forEachArgAndRangeCell(ctx, [&](const formula::Value& v) {
                  if (auto d = std::get_if<double>(&v)) {
                      accumulator += *d;
                  }
              });
              return accumulator;
          } },
        { "COUNT",
            [](Ctx ctx) {
              int accumulator  = 0;
              forEachArgAndRangeCell(ctx, [&](const formula::Value& v) {
                if (std::holds_alternative<std::nullopt_t>(v)) {
                    return;
                }
                accumulator++;
              });
              return double(accumulator);
            } },
        { "IF",
            [](Ctx ctx) {
              if (ctx.args.size() != 3) {
                  throw AException("ARG");
              }
              auto condition = std::get_if<double>(&ctx.args[0]);
              if (condition == nullptr) {
                  throw AException("ARG0");
              }
              if (*condition == 0.0) {
                  return ctx.args[2];
              }
              return ctx.args[1];
            } },
    };
    return out;
}
