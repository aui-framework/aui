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

//
// Created by alex2772 on 10/6/25.
//

#include "AHotCodeReload.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Platform/AProcess.h"
#include "AUI/IO/AFileInputStream.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/algorithm.hpp>

namespace {

static constexpr auto LOG_TAG = "Hot code reload";

struct Section {
    Elf64_Shdr header;
    AString name;
    std::vector<char> data;
};

AVector<Section> parseElf(const AString& path) {
    Elf64_Ehdr elfHeader;
    AFileInputStream fis(path);
    fis >> aui::serialize_raw(elfHeader);

    if (memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
        throw AException("Invalid ELF signature");
    }

    Elf64_Shdr stringTableHeader;
    fis.seek(elfHeader.e_shoff + elfHeader.e_shstrndx * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
    fis >> aui::serialize_raw(stringTableHeader);

    auto stringTable = std::string(stringTableHeader.sh_size, '\0');
    fis.seek(stringTableHeader.sh_offset, ASeekDir::BEGIN);
    fis.read(stringTable.data(), stringTableHeader.sh_size);

    AVector<Section> sections;

    for (int i = 0; i < elfHeader.e_shnum; ++i) {
        Elf64_Shdr sectionHeader;
        fis.seek(elfHeader.e_shoff + i * sizeof(Elf64_Shdr), ASeekDir::BEGIN);
        fis >> aui::serialize_raw(sectionHeader);
        AString name = static_cast<const char*>(stringTable.data() + sectionHeader.sh_name);
        std::vector<char> data(sectionHeader.sh_size);
        if (sectionHeader.sh_size != 0) {
            fis.seek(sectionHeader.sh_offset, ASeekDir::BEGIN);
            fis.read(data.data(), data.size());
            data.resize(sectionHeader.sh_size);
        }
        sections.emplace_back(Section {
          .header = sectionHeader,
          .name = std::move(name),
          .data = std::move(data),
        });
    }
    return sections;
}

template <typename T>
std::span<const T> asSpan(std::span<const char> view) {
    AUI_ASSERTX(view.size() % sizeof(T) == 0, "Invalid view size");
    return std::span<const T>((const T*) view.data(), view.size() / sizeof(T));
}

char* alignUpper(char* input, size_t alignment = sysconf(_SC_PAGESIZE)) {
    size_t offset = (size_t) input % alignment;
    if (offset == 0) {
        return input;
    }
    return input + alignment - offset;
}

void validateDistance(
    AStringView symname, char* addr1, char* addr2, size_t distance = std::numeric_limits<int32_t>::max()) {
    if (addr1 > addr2) {
        if (addr1 - addr2 > distance) {
            throw AException("Symbol too far: {}"_format(symname.data()));
        }
    } else {
        if (addr2 - addr1 > distance) {
            throw AException("Symbol too far: {}"_format(symname.data()));
        }
    }
}
}   // namespace

void AHotCodeReload::reload() {
    AThread::current()->enqueue([this] {
        AString input = "/home/alex2772/CLionProjects/aui/cmake-build-debug/examples/ui/hot_code_reload/CMakeFiles/aui.example.hot_code_reload.dir/src/main.cpp.o";
        emit patchBegin;
        AUI_DEFER { emit patchEnd; };

        struct MappedSection {
            Section section;
            _<void> page;
        };

        // 1. Load PROGBITS sections with ALLOC into memory
        std::vector<MappedSection> sections = [&] {
            auto pivot =
                ranges::max(mSymbols | ranges::view::transform([](const auto& i) { return (char*) i.second; }));
            pivot = alignUpper(pivot);
            auto sections = parseElf(input);
            return sections | ranges::view::transform([&](Section& section) {
                       _<void> page;
                       if (section.header.sh_flags & SHF_ALLOC && section.header.sh_type == SHT_PROGBITS &&
                           section.data.size() > 0) {
                           page = [&] {
                               tryAgainLol:
                                   void* p = mmap(
                                       pivot, section.data.size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
                                       -1, 0);
                                   if (p == MAP_FAILED) {
                                       throw AException("mmap fail");
                                   }
                                   pivot = alignUpper(pivot + section.data.size() + 1);
                                   if (p > pivot + sysconf(_SC_PAGESIZE)) {
                                       munmap(p, section.data.size());
                                       goto tryAgainLol;
                                   }
                                   return aui::ptr::manage_shared(p, [size = section.data.size()](void* ptr) {
                                       munmap(ptr, size);
                                   });
                           }();
                           memcpy(page.get(), section.data.data(), section.data.size());
                       }
                       return MappedSection { .section = std::move(section), .page = std::move(page) };
                   }) |
                   ranges::to_vector;
        }();

        bool errored = false;
        // 2. Process relocations
        for (const auto& mapped : sections) {
            if (mapped.section.header.sh_type != SHT_RELA)
                continue;
            if (mapped.section.name.contains(".debug")) {
                continue;
            }
            if (mapped.section.name.contains(".rela.eh_frame")) {
                printf("\n");
            }
            // Find target section
            size_t targetIdx = mapped.section.header.sh_info;
            auto& targetSection = sections[targetIdx];

            auto getSymbolName = [&](size_t index) {
                auto& symtab = sections.at(mapped.section.header.sh_link);
                auto st_name = asSpan<Elf64_Sym>(std::span(symtab.section.data))[index].st_name;
                return AStringView(&sections.at(symtab.section.header.sh_link).section.data.at(st_name));
            };


            // Each entry:
            auto i = asSpan<Elf64_Rela>(std::span(mapped.section.data));
            auto count = i.size();

            auto printSectionName = [&, printed = false]() mutable {
                if (printed) {
                    return;
                }
                printed = true;
                ALogger::info(LOG_TAG) << "In section: " << mapped.section.name << ":";
            };

            for (const auto& rela : i) {
                size_t offset = rela.r_offset;
                size_t symidx = ELF64_R_SYM(rela.r_info);

                // Lookup symbol name:
                auto symname = getSymbolName(symidx);
                auto symaddr = [&]() -> char* {
                    if (symname == "") {
                        return (char*)targetSection.page.get();
                    }
                    auto it = mSymbols.find(symname);
                    if (it == mSymbols.end()) {
                        printSectionName();
                        ALogger::err(LOG_TAG) << "Unresolved reference: \"" << symname.data() << "\"";
                        errored = true;
                        return nullptr;
                    }
                    return reinterpret_cast<char*>(it->second);
                }();
                if (symaddr == nullptr) {
                    continue;
                }

                auto writeAddr = reinterpret_cast<char*>(targetSection.page.get()) + offset;

                switch (auto type = ELF64_R_TYPE(rela.r_info)) {
                    case R_X86_64_PLT32:
                    case R_X86_64_PC32: {
                        validateDistance(symname, writeAddr, symaddr);
                        int32_t rel = symaddr - writeAddr + rela.r_addend;
                        memcpy(writeAddr, &rel, 4);
                        break;
                    }
                        /*
                    case R_X86_64_64: {
                        uint64_t abs = (uintptr_t) symaddr + rela->r_addend;
                        memcpy(writeAddr, &abs, 8);
                        break;
                    }*/
                        // Add more types here as needed
                    default: {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Unsupported relocation type: " << type << " in \"" << symname.data() << "\"";
                        errored = true;
                    }
                }
            }
        }
        if (errored) {
            ALogger::err(LOG_TAG) << "\"" << input << "\": refusing patch due to errors. Target is not changed.";
            return;
        }
    });
}

std::unordered_map<AString, void*> AHotCodeReload::extractSymbols() {
    std::unordered_map<AString, void*> symbols;
    auto myLocation = AProcess::self()->getPathToExecutable();
    auto sections = parseElf(myLocation);

    const Section* symtab = nullptr;   // regular symbol table
    const Section* dynsym = nullptr;   // dynamic symbol table
    const Section* rela = nullptr;     // relocation table

    for (const auto& section : sections) {
        switch (section.header.sh_type) {
            case SHT_SYMTAB:
                symtab = &section;
                break;
            case SHT_DYNSYM:
                dynsym = &section;
                break;
            case SHT_RELA:
                rela = &section;
                break;
            default:
                break;
        }
    }

    auto extractStringTable = [&](const Section& section) {
        return std::span(sections.at(section.header.sh_link).data);
    };
    if (symtab != nullptr) {
        auto stringTable = extractStringTable(*symtab);
        // Read symbols
        for (const auto& sym : asSpan<Elf64_Sym>(symtab->data)) {
            if (sym.st_value == 0) {
                continue;
            }

            auto name = std::string_view(&stringTable[sym.st_name]);
            auto value = reinterpret_cast<void*>(sym.st_value);
            symbols[name] = value;
            ALogger::info(LOG_TAG) << "Found symbol: " << name << " at " << value << " (size: " << sym.st_size << ")";
        }
    }

    if (dynsym != nullptr && rela != nullptr) {
        auto stringTable = extractStringTable(*dynsym);
        for (const auto& r : asSpan<Elf64_Rela>(rela->data)) {
            auto symIdx = ELF64_R_SYM(r.r_info);
            const auto& sym = asSpan<Elf64_Sym>(dynsym->data)[symIdx];
            auto name = std::string_view(&stringTable[sym.st_name]);
            auto value = reinterpret_cast<void*>(r.r_offset);   // address of plt stub
            symbols[name] = value;
            ALogger::info(LOG_TAG) << "Found dynamic symbol: " << name << " at " << value;
        }
    }

    return symbols;
}
