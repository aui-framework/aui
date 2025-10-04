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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AView.h>
#include "AUI/Platform/AProcess.h"
#include "AUI/IO/AFileInputStream.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

using namespace ass;
using namespace declarative;

class TextRenderingView : public AView {
public:
    void render(ARenderContext ctx) override {
        AView::render(ctx);
        ctx.render.setColor(AColor::BLACK);
        ctx.render.string({0, 0}, "Hello World!");
//        ctx.render.string({5, 10}, "Hello World"); // <<----- i added this line
    }

};

static void reload() {
    static constexpr auto LOG_TAG = "Hot code reload";
    void* addr = (void*) reload;
    const auto PAGE_SIZE = sysconf(_SC_PAGESIZE);

    auto align = [&](uintptr_t addr) {
        return ((uintptr_t) addr & ~(PAGE_SIZE - 1));
    };

    auto myLocation = AProcess::self()->getPathToExecutable();
    AFileInputStream fis(myLocation);
    Elf64_Ehdr elfHeader;
    fis >> aui::serialize_raw(elfHeader);

    if (memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
        ALogger::err(LOG_TAG) << "Invalid ELF signature";
        return;
    }

    Elf64_Shdr stringTableHeader;
    fis.seek(elfHeader.e_shoff + elfHeader.e_shstrndx * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
    fis >> aui::serialize_raw(stringTableHeader);

    std::string stringTable(stringTableHeader.sh_size, '\0');
    fis.seek(stringTableHeader.sh_offset, ASeekDir::BEGIN);
    fis.read(stringTable.data(), stringTableHeader.sh_size);

    auto regionToPatch = [&]() -> AOptional<std::span<char>> {
        for (int i = 0; i < elfHeader.e_shnum; ++i) {
            Elf64_Shdr sectionHeader;
            fis.seek(elfHeader.e_shoff + i * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
            fis >> aui::serialize_raw(sectionHeader);

            if (sectionHeader.sh_type == SHT_PROGBITS && (sectionHeader.sh_flags & SHF_EXECINSTR)) {
                const char* sectionName = static_cast<const char*>(stringTable.data() + sectionHeader.sh_name);

                auto sectionStart = static_cast<void*>(reinterpret_cast<void*>(sectionHeader.sh_addr));
                auto sectionEnd = static_cast<void*>(static_cast<char*>(sectionStart) + sectionHeader.sh_size);

                if (addr >= sectionStart && addr < sectionEnd) {
                    ALogger::info(LOG_TAG)
                        << "Patching: " << sectionName << " " << sectionHeader.sh_type << " " << sectionHeader.sh_flags
                        << " " << sectionStart << "-" << sectionEnd;
                    fis.seek(sectionHeader.sh_offset, ASeekDir::BEGIN);
                    return std::span<char>((char*)sectionStart, sectionHeader.sh_size);
                }
            }
        }
        return std::nullopt;
    }();
    if (!regionToPatch) {
        ALogger::err(LOG_TAG) << "Can't find section to patch";
        return;
    }
    auto alignedRegionToPatch = std::span<char>(reinterpret_cast<char*>(align(reinterpret_cast<std::uintptr_t>(regionToPatch->data()))), align(regionToPatch->size()));

    if (mprotect(alignedRegionToPatch.data(), alignedRegionToPatch.size(), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        ALogger::err(LOG_TAG) << "Failed to make memory writable";
        return;
    }

    AUI_DEFER { mprotect(alignedRegionToPatch.data(), alignedRegionToPatch.size(), PROT_READ | PROT_EXEC); };

    fis.read(regionToPatch->data(), regionToPatch->size());
    ALogger::info(LOG_TAG) << "Reloaded, good luck!";
}

AUI_ENTRY {
    auto window = _new<AWindow>("Hot code reload", 600_dp, 300_dp);
    window->setContents(Vertical {
      _new<TextRenderingView>() AUI_WITH_STYLE { MinSize { 32_dp } },
      SpacerExpanding {},
      _new<AButton>("Reload") AUI_LET {
          AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [] { reload(); });
      }
    });
    window->show();
    return 0;
}
