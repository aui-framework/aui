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
#include <range/v3/view/enumerate.hpp>

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

uintptr_t alignUpper(uintptr_t input, size_t alignment = sysconf(_SC_PAGESIZE)) {
    size_t offset = (size_t) input % alignment;
    if (offset == 0) {
        return input;
    }
    return input + alignment - offset;
}

bool validateDistance(
    AStringView symname, char* addr1, char* addr2, size_t distance = std::numeric_limits<int32_t>::max()) {
    if (addr1 > addr2) {
        if (addr1 - addr2 > distance) {
            return false;
        }
    } else {
        if (addr2 - addr1 > distance) {
            return false;
        }
    }
    return true;
}

auto extractStringTable(const AVector<Section>& allSections, const Section& targetSection) {
    return std::span(allSections.at(targetSection.header.sh_link).data);
}


void hook(void* source, void* destination) {
    static const auto PAGE_SIZE = sysconf(_SC_PAGESIZE);
    auto alignedAddr = reinterpret_cast<uintptr_t>(source) & ~(PAGE_SIZE - 1);
    auto alignedSize = alignUpper(reinterpret_cast<uintptr_t>(source) - alignedAddr + 8, PAGE_SIZE);
    if (mprotect(reinterpret_cast<void*>(alignedAddr), alignedSize, PROT_READ | PROT_WRITE) != 0) {
        throw AException("mprotect failed at {}"_format(reinterpret_cast<void*>(alignedAddr)));
    }
    AUI_DEFER { mprotect(reinterpret_cast<void*>(alignedAddr), alignedSize, PROT_READ | PROT_EXEC); };

    auto* sourceCode = reinterpret_cast<uint8_t*>(source);

#if AUI_ARCH_X86 || AUI_ARCH_X86_64
    static constexpr auto JMP_OPCODE = 0xE9;
    *(sourceCode++) = JMP_OPCODE;
    int32_t offset = reinterpret_cast<uintptr_t>(destination) - (reinterpret_cast<uintptr_t>(source)) - 5;
    std::memcpy(sourceCode, &offset, sizeof(offset));
#endif
}

template<aui::invocable<AStringView, const Elf64_Sym> F>
void extractSymbols(const AVector<Section>& sections, F&& destination) {
    const Section* symtab = nullptr;   // regular symbol table

    for (const auto& section : sections) {
        switch (section.header.sh_type) {
            case SHT_SYMTAB:
                symtab = &section;
                break;
        }
    }
    if (symtab == nullptr) {
        return;
    }
    auto stringTable = extractStringTable(sections, *symtab);
    // Read symbols
    for (const auto& sym : asSpan<Elf64_Sym>(symtab->data)) {
        auto name = AStringView(&stringTable[sym.st_name]);
        if (name.empty()) {
            continue;
        }
        auto value = reinterpret_cast<void*>(sym.st_value);
        ALOG_DEBUG(LOG_TAG) << fmt::format(
            "    Found symbol: at base+{:<16p} (size: {:<8}, bind: {:<8}, type: {:<8}, visibility: {:<8}, shndx: {:<8}) "
            "{:<40}",
            value, sym.st_size, ELF64_ST_BIND(sym.st_info), ELF64_ST_TYPE(sym.st_info),
            ELF64_ST_VISIBILITY(sym.st_other), sym.st_shndx, name);
        destination(name, sym);
    }
}

}   // namespace

void AHotCodeReload::reload() {
    AThread::current()->enqueue([this] {
        AString input =
            "/home/alex2772/CLionProjects/aui/cmake-build-debug/examples/ui/hot_code_reload/CMakeFiles/"
            "aui.example.hot_code_reload.dir/src/main.cpp2.o";
        emit patchBegin;
        AUI_DEFER { emit patchEnd; };

        struct MappedSection {
            Section section;
            _<void> page;
        };

        struct LocalSymbol {
            /**
             * @brief Section index this symbol is located in.
             */
            size_t sectionIdx;

            /**
             * @brief Offset in bytes from the beginning of the section.
             */
            size_t offset;


            /**
             * @brief Size of the symbol in bytes.
             */
            size_t size;
        };
        
        std::unordered_map<AString, LocalSymbol> localSymbols;

        // 1. Load PROGBITS sections with ALLOC into memory
        std::vector<MappedSection> sections = [&] {
            auto pivot =
                ranges::min(mSymbols | ranges::view::transform([](const auto& i) { return reinterpret_cast<uintptr_t>(i.second); }));
            pivot = alignUpper(pivot);
            auto sections = parseElf(input);
            ALOG_DEBUG(LOG_TAG) << input << " :";
            ::extractSymbols(sections, [&](AStringView name, const Elf64_Sym& sym) {
                localSymbols[name] = LocalSymbol{
                    .sectionIdx = sym.st_shndx,
                    .offset = sym.st_value,
                    .size = sym.st_size,
                };
            });

            return sections | ranges::view::transform([&](Section& section) {
                       _<void> page;
                       if (section.header.sh_flags & SHF_ALLOC && section.header.sh_type == SHT_PROGBITS &&
                           section.data.size() > 0) {
                           page = [&] {
                               tryAgainLol:
                                   void* p = mmap(
                                       reinterpret_cast<void*>(pivot), section.data.size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
                                       -1, 0);
                                   if (p == MAP_FAILED) {
                                       throw AException("mmap fail");
                                   }
                                   pivot = alignUpper(pivot + section.data.size() + 1);
                                   if (p > reinterpret_cast<char*>(pivot) + sysconf(_SC_PAGESIZE)) {
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
        aui::lazy<MappedSection*> textSection = [&] {
            auto it = ranges::find_if(sections, [&](const MappedSection& section) {
                return section.section.name == ".text";
            });
            if (it == sections.end()) {
                throw AException("Failed to find .text section");
            }
            if (it->page == nullptr) {
                throw AException(".text section is not mapped");
            }
            return &*it;
        };
        for (const auto& mapped : sections) {
            if (mapped.section.header.sh_type != SHT_RELA)
                continue;
            if (mapped.section.name.contains(".debug")) {
                continue;
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

            auto printSectionName = [&, printed = false]() mutable {
                if (printed) {
                    return;
                }
                printed = true;
                ALogger::info(LOG_TAG) << "In section: " << mapped.section.name << " :";
            };

            for (const auto& rela : i) {
                size_t offset = rela.r_offset;
                size_t symidx = ELF64_R_SYM(rela.r_info);

                // Lookup symbol name:
                auto symname = getSymbolName(symidx);
                auto writeAddr = reinterpret_cast<char*>(targetSection.page.get()) + offset;
                if (writeAddr == nullptr) {
                    continue;
                }

                auto getAddr = [&](const auto& table) -> char* {
                    if (symname == "") {
                        symname = ".text";
                        return static_cast<char*>((*textSection)->page.get());
                    }
                    if (auto it = table.find(symname); it != table.end()) {
                        return reinterpret_cast<char*>(it->second);
                    }
                    if (auto it = localSymbols.find(symname); it != localSymbols.end() && it->second.sectionIdx != SHN_UNDEF) {
                        auto& targetSection = sections.at(it->second.sectionIdx);
                        if (targetSection.page.get() == nullptr) {
                            printSectionName();
                            ALogger::err(LOG_TAG) << "Target section of this symbol is not mapped: \"" << symname << "\"";
                            errored = true;
                            return nullptr;
                        }
                        return reinterpret_cast<char*>(targetSection.page.get()) + it->second.offset;
                    }
                    printSectionName();
                    ALogger::err(LOG_TAG) << "Unresolved reference: \"" << symname << "\"";
                    errored = true;
                    return nullptr;
                };
#if AUI_DEBUG
                printSectionName();
                ALOG_DEBUG(LOG_TAG) << "Patching relocation: \"" << symname << "\" : " << (void*) writeAddr;
#endif

                switch (auto type = ELF64_R_TYPE(rela.r_info)) {
                    case R_X86_64_PLT32:
                    case R_X86_64_PC32: {
                        auto symaddr = getAddr(mSymbols);
                        if (symaddr == nullptr) {
                            continue;
                        }
                        if (!validateDistance(symname, writeAddr, symaddr)) {
                            printSectionName();
                            ALogger::err(LOG_TAG)
                                << "Relocation is too far away: symname=\"" << symname
                                << "\", symaddr=" << (void*) symaddr << ", writeAddr=" << (void*) writeAddr;
                            errored = true;
                        }
                        int32_t rel = symaddr - writeAddr + rela.r_addend;
                        memcpy(writeAddr, &rel, 4);
                        break;
                    }
                    case R_X86_64_64: {
                        auto symaddr = getAddr(mSymbols);
                        if (symaddr == nullptr) {
                            continue;
                        }
                        uint64_t abs = (uintptr_t) symaddr + rela.r_addend;
                        memcpy(writeAddr, &abs, 8);
                        break;
                    }
                    case R_X86_64_REX_GOTPCRELX:
                    case R_X86_64_GOTPCREL:
                    case R_X86_64_GOTPCRELX: {
                        auto symaddr = getAddr(mGot);
                        if (symaddr == nullptr) {
                            continue;
                        }
                        if (!validateDistance(symname, writeAddr, symaddr)) {
                            printSectionName();
                            ALogger::err(LOG_TAG)
                                << "Relocation is too far away: symname=\"" << symname
                                << "\", symaddr=" << (void*) symaddr << ", writeAddr=" << (void*) writeAddr;
                            errored = true;
                        }
                        int32_t rel = symaddr - writeAddr + rela.r_addend;
                        memcpy(writeAddr, &rel, 4);
                        break;
                    }
                    default: {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Unsupported relocation type: " << type << " in \"" << symname << "\"";
                        errored = true;
                    }
                }
            }
        }
        if (errored) {
            printErrorAndExit:
            ALogger::err(LOG_TAG) << "\"" << input << "\": refusing patch due to errors. Target is not changed.";
            return;
        }
        
        // 3. Set proper page permissions
        for (const auto& mapped : sections) {
            if (mapped.page == nullptr) {
                continue;
            }
            int prot = PROT_READ;
            if (mapped.section.header.sh_flags & SHF_WRITE) {
                prot |= PROT_WRITE;
            }
            if (mapped.section.header.sh_flags & SHF_EXECINSTR) {
                prot |= PROT_EXEC;
            }
            if (auto result = mprotect(mapped.page.get(), mapped.section.data.size(), prot); result != 0) {
                ALogger::err(LOG_TAG) << "mprotect failed: " << (void*)mapped.page.get() << " : " << strerror(errno);
                goto printErrorAndExit;
            }
        }

        // 4. Hook old symbols with newly loaded ones
        for (const auto&[name, symbol]: localSymbols) {
            auto destinationAddr = reinterpret_cast<char*>(symbol.offset);
            if (symbol.sectionIdx == SHN_UNDEF) {
                continue;
            }
            if (symbol.sectionIdx == SHN_COMMON) {
                continue;
            }
            if (symbol.sectionIdx != SHN_ABS) {
                auto& destinationSection = sections.at(symbol.sectionIdx);
                if (destinationSection.page == nullptr) {
                    continue;
                }
                destinationAddr += reinterpret_cast<uintptr_t>(destinationSection.page.get());
            }
            ALOG_DEBUG(LOG_TAG) << "Mapping: " << name << " : " << (void*)destinationAddr;

            auto sourceAddr = mSymbols.find(name);
            if (sourceAddr == mSymbols.end()) {
                continue;
            }

            if (std::memcmp(sourceAddr->second, destinationAddr, symbol.size) == 0) {
                continue;
            }

            if (!name.contains("inflate")) {
                continue;
            }

            ALOG_DEBUG(LOG_TAG) << "Hooking symbol: " << name << " : " << (void*)sourceAddr->second << " -> " << (void*)destinationAddr;
            hook(sourceAddr->second, destinationAddr);
        }
        mAllocatedPages.insertAll(sections | ranges::view::transform([](auto& section) -> decltype(auto) { return std::move(section.page); }));
        ALOG_DEBUG(LOG_TAG) << "Done";
        emit patchEnd;
    });
}

AHotCodeReload::AHotCodeReload() {
    APath objectPath = AProcess::self()->getPathToExecutable();

    if (!objectPath.isRegularFileExists()) {
        return;
    }
    ALOG_DEBUG(LOG_TAG) << "Object: \"" << objectPath << "\"";

    auto sections = parseElf(objectPath);

    ::extractSymbols(sections, [&](AStringView name, const Elf64_Sym& sym) {
        auto resolved = reinterpret_cast<char*>(0) + sym.st_value;
        if (resolved == nullptr) {
            return;
        }
        if (auto& v = mSymbols[name]; v > resolved || v == nullptr) {
            v = resolved;
        };
    });

    auto plt = ranges::find_if(sections, [&](const auto& section) {
        return section.name == ".plt";
    });
    if (plt == sections.end()) {
        return;
    }

    for (const auto& section : sections) {
        if (section.header.sh_type != SHT_RELA) {
            continue;
        }
        const auto& dynsym = sections.at(section.header.sh_link);
        auto stringTable = extractStringTable(sections, dynsym);
        for (const auto& [i, r] : asSpan<Elf64_Rela>(section.data) | ranges::view::enumerate) {
            auto symIdx = ELF64_R_SYM(r.r_info);
            const auto& sym = asSpan<Elf64_Sym>(dynsym.data)[symIdx];
            auto name = std::string_view(&stringTable[sym.st_name]);
            auto got = reinterpret_cast<void**>(r.r_offset);   // address of plt stub

#if AUI_ARCH_X86 || AUI_ARCH_X86_64
            static constexpr auto PLT_ENTRY_SIZE = 16;
#endif

            auto pltFunc = (void*)(plt->header.sh_addr + PLT_ENTRY_SIZE * (i + 1)); // +1 for initial PLT0 entry
            mGot[name] = got;
            mSymbols[name] = pltFunc;
            ALOG_DEBUG(LOG_TAG) << "    Found dynamic symbol: " << name << ", got: " << (void*)got << ", @plt func: " << pltFunc << " actual location: " << *got;
        }
    }
}
