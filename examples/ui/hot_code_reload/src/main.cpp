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
        ctx.render.string({ 0, 0 }, "Hello World!");
        ctx.render.string({ 4, 10 }, "Hello World!!");   // <<----- i added this line
    }
};

class AHotCodeReload : public AObject {
public:
    static AHotCodeReload& instance() {
        static AHotCodeReload instance;
        return instance;
    }

    void reload() {
        static constexpr auto LOG_TAG = "Hot code reload";
        auto newSymbols = extractSymbols();
        bool symbolsChanged = false;
        for (const auto& [name, addr] : mSymbols) {
            auto it = newSymbols.find(name);
            if (it == newSymbols.end()) {
                ALogger::err(LOG_TAG) << "Can't apply code: symbol \"" << name << "\" not found";
                symbolsChanged = true;
            }

            if (it->second != addr) {
                ALogger::err(LOG_TAG) << "Can't apply code: symbol \"" << name << "\" was relocated from " << addr << " to " << it->second;
                symbolsChanged = true;
            }
        }
        if (symbolsChanged) {
            return;
        }
        AUI_DEFER { mSymbols = std::move(newSymbols); };

        const auto PAGE_SIZE = sysconf(_SC_PAGESIZE);

        auto align = [&](uintptr_t addr) { return ((uintptr_t) addr & ~(PAGE_SIZE - 1)); };

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

        for (int i = 0; i < elfHeader.e_shnum; ++i) {
            Elf64_Shdr sectionHeader;
            fis.seek(elfHeader.e_shoff + i * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
            fis >> aui::serialize_raw(sectionHeader);

            if (sectionHeader.sh_addr == 0) {
                continue;
            }

            if (sectionHeader.sh_size == 0) {
                continue;
            }

            std::string_view sectionName = static_cast<const char*>(stringTable.data() + sectionHeader.sh_name);
            auto sectionStart = static_cast<char*>(reinterpret_cast<void*>(sectionHeader.sh_addr));
            auto sectionEnd = static_cast<char*>(static_cast<char*>(sectionStart) + sectionHeader.sh_size);

            if (sectionHeader.sh_type == SHT_PROGBITS && !(sectionHeader.sh_flags & SHF_WRITE) &&
                !(sectionHeader.sh_flags & SHF_EXECINSTR)) {
                goto patch;
            }

            if (sectionName == ".text") {
                goto patch;
            }

            continue;
        patch:

            ALogger::info(LOG_TAG)
                << "Patching: " << sectionName << " " << sectionHeader.sh_type << " " << sectionHeader.sh_flags << " "
                << (void*) sectionStart << "-" << (void*) sectionEnd;

            auto region = std::span<char>(sectionStart, sectionHeader.sh_size);
            auto aligned = reinterpret_cast<char*>(align(reinterpret_cast<std::uintptr_t>(region.data())));

            if (mprotect(aligned, sectionEnd - aligned, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
                ALogger::err(LOG_TAG) << "Failed to make memory writable";
                continue;
            }

            fis.seek(sectionHeader.sh_offset, ASeekDir::BEGIN);
            fis.read(region.data(), region.size());
            int prot = PROT_READ;
            if (sectionHeader.sh_flags & SHF_WRITE) {
                prot |= PROT_WRITE;
            }
            if (sectionHeader.sh_flags & SHF_EXECINSTR) {
                prot |= PROT_EXEC;
            }
            mprotect(aligned, sectionEnd - aligned, prot);
        }
        ALogger::info(LOG_TAG) << "Reloaded, good luck!";
        emit completed;
    }

    emits<> completed;

private:
    std::unordered_map<AString, void*> mSymbols = extractSymbols();

    std::unordered_map<AString, void*> extractSymbols() {
        std::unordered_map<AString, void*> symbols;
        auto myLocation = AProcess::self()->getPathToExecutable();
        AFileInputStream fis(myLocation);
        Elf64_Ehdr elfHeader;
        fis >> aui::serialize_raw(elfHeader);

        if (memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
            return symbols;
        }

        // Read section headers
        for (int i = 0; i < elfHeader.e_shnum; ++i) {
            Elf64_Shdr sectionHeader;
            fis.seek(elfHeader.e_shoff + i * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
            fis >> aui::serialize_raw(sectionHeader);


            if (sectionHeader.sh_type == SHT_SYMTAB) {
                auto stringTable = [&] {
                  fis.seek(elfHeader.e_shoff + sectionHeader.sh_link * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
                  Elf64_Shdr sectionHeaderStringTable;
                  fis >> aui::serialize_raw(sectionHeaderStringTable);
                  fis.seek(sectionHeaderStringTable.sh_offset, ASeekDir::BEGIN);
                  std::string sectionHeaderStringTableString(sectionHeaderStringTable.sh_size, '\0');
                  fis.read(sectionHeaderStringTableString.data(), sectionHeaderStringTable.sh_size);
                  return sectionHeaderStringTableString;
                }();

                // Read symbols
                size_t numSymbols = sectionHeader.sh_size / sizeof(Elf64_Sym);
                fis.seek(sectionHeader.sh_offset, ASeekDir::BEGIN);
                for (size_t j = 0; j < numSymbols; ++j) {
                    Elf64_Sym sym;
                    fis >> aui::serialize_raw(sym);

                    if (sym.st_value == 0) {
                        continue;
                    }

                    auto type = ELF64_ST_TYPE(sym.st_info);
                    if (type == STT_SECTION) {
                        continue;
                    }
                    if (type == STT_OBJECT) {
                        continue;
                    }

                    symbols["{} ({})"_format(std::string_view(&stringTable.at(sym.st_name)), sym.st_info)] = reinterpret_cast<void*>(sym.st_value);
                }
            }
        }
        return symbols;
    }

    AHotCodeReload() {}
};

AUI_ENTRY {
    auto window = _new<AWindow>("Hot code reload", 600_dp, 300_dp);
    window->setContents(Vertical {
      _new<TextRenderingView>() AUI_WITH_STYLE { MinSize { 32_dp } }, SpacerExpanding {},
      _new<AButton>("Reload")
          AUI_LET { AObject::connect(it->clicked, AUI_SLOT(AHotCodeReload::instance())::reload); } });
    window->show();
    return 0;
}
