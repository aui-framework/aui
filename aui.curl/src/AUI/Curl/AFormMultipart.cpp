//
// Created by alex2772 on 9/4/23.
//

#include <random>
#include <range/v3/view.hpp>
#include <range/v3/action.hpp>
#include "AUI/Common/AByteBufferView.h"
#include <AUI/Crypt/Random.h>
#include <AUI/IO/AConcatInputStream.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/IO/AStrongByteBufferInputStream.h>
#include <AUI/Common/ASmallVector.h>
#include "AFormMultipart.h"

_<IInputStream> AFormMultipart::makeInputStream() const {
    mBoundary = "----auiboundary";
    {
        static std::default_random_engine re;
        static std::uniform_int_distribution d(0, 255);
        std::array<uint8_t, 16> buf;
        ranges::generate(buf, []{ return uint8_t(d(re)); });

        mBoundary += AByteBufferView((const char*)buf.data(), buf.size()).toBase64String();
    }

    auto result = *this
                  | ranges::views::transform([&](const value_type& kv) {
                      std::string prefix = fmt::format("--{}\r\n"
                                                       "Content-Disposition: form-data; name=\"{}\"", mBoundary, kv.first.toStdString());

                      if (kv.second.filename) {
                          prefix += fmt::format("; filename=\"{}\"", *kv.second.filename);
                      }
                      prefix += "\r\n";

                      if (kv.second.mimeType) {
                          prefix += fmt::format("Content-Type: {}\r\n", *kv.second.mimeType);
                      }

                      prefix += "\r\n";

                      auto content = std::visit(aui::lambda_overloaded {
                          [](const AString& v) -> _<IInputStream> {
                              return _new<AStringStream>(v);
                          },
                          [](const AByteBuffer& v) -> _<IInputStream> {
                              return _new<AStrongByteBufferInputStream>(v);
                          },
                          [&](const _<IInputStream>& is) -> _<IInputStream> {
                              return is;
                          },
                      }, kv.second.value);

                      return std::array{
                          (_<IInputStream>)_new<AStringStream>(std::move(prefix)),
                          content,
                          (_<IInputStream>)_new<AStringStream>(fmt::format("\r\n")),
                      };
                    })
                  | ranges::actions::join
                  | ranges::to<ADeque<_<IInputStream>>>()
                  //| ranges::actions::push_front({(_<IInputStream>)_new<AStringStream>(fmt::format("\r\n", mBoundary))})
                  | ranges::actions::push_back({(_<IInputStream>)_new<AStringStream>(fmt::format("--{}--\r\n", mBoundary))});

    return _new<AConcatInputStream>(std::move(result));
}
