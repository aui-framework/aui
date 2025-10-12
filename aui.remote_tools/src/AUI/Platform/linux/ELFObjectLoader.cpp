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
// Created by alex2772 on 10/11/25.
//

#include "ELFObjectLoader.h"
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
#include <range/v3/view/drop.hpp>

namespace {

static constexpr auto LOG_TAG = "ELFObjectLoader";

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

template <aui::invocable<AStringView, const Elf64_Sym> F>
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
        ALOG_TRACE(LOG_TAG) << fmt::format(
            "    Found symbol: at base+{:<16p} (size: {:<8}, bind: {:<8}, type: {:<8}, visibility: {:<8}, shndx: "
            "{:<8}) "
            "{:<40}",
            value, sym.st_size, ELF64_ST_BIND(sym.st_info), ELF64_ST_TYPE(sym.st_info),
            ELF64_ST_VISIBILITY(sym.st_other), sym.st_shndx, name);
        destination(name, sym);
    }
}

void cxa_pure_virtual() { throw AException("Pure virtual function called"); }

}   // namespace

ELFObjectLoader::ELFObjectLoader(const APath& baseFilePath) {
    ALOG_TRACE(LOG_TAG) << "Object: \"" << baseFilePath << "\"";

    auto sections = parseElf(baseFilePath);

    mSymbols["__cxa_pure_virtual"] = (void*) cxa_pure_virtual;

    ::extractSymbols(sections, [&](AStringView name, const Elf64_Sym& sym) {
        auto resolved = reinterpret_cast<char*>(0) + sym.st_value;
        if (resolved == nullptr) {
            return;
        }
        if (auto& v = mSymbols[name]; v > resolved || v == nullptr) {
            v = resolved;
        };
    });

    auto plt = ranges::find_if(sections, [&](const auto& section) { return section.name == ".plt"; });
    if (plt == sections.end()) {
        return;
    }

    for (const auto& section : sections) {
        if (section.header.sh_type != SHT_RELA) {
            continue;
        }
        const auto& dynsym = sections.at(section.header.sh_link);
        auto stringTable = extractStringTable(sections, dynsym);

        const bool isRelatedToPlt = section.name.contains(".plt");   // .rela.plt
        ALOG_TRACE(LOG_TAG) << "In section: " << section.name << " :";

        for (const auto& [i, r] : asSpan<Elf64_Rela>(section.data) | ranges::view::enumerate) {
            auto symIdx = ELF64_R_SYM(r.r_info);
            const auto& sym = asSpan<Elf64_Sym>(dynsym.data)[symIdx];
            auto name = std::string_view(&stringTable[sym.st_name]);
            auto got = reinterpret_cast<void**>(r.r_offset);   // address of plt stub
            void* pltFunc = nullptr;
            if (isRelatedToPlt) {
#if AUI_ARCH_X86 || AUI_ARCH_X86_64
                static constexpr auto PLT_ENTRY_SIZE = 16;
                pltFunc = (void*) (plt->header.sh_addr + PLT_ENTRY_SIZE * (i + 1));   // +1 for initial PLT0 entry
#endif
            }
            mGot[name] = got;
            if (pltFunc != nullptr)
                mSymbols[name] = pltFunc;
            ALOG_TRACE(LOG_TAG)
                << "    Found dynamic symbol name=" << name << ", got=" << (void*) got << ", *got=" << *got
                << ", pltFunc=" << (void*) pltFunc;
        }
    }
}

void ELFObjectLoader::load(const APath& objectFile) {
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
        auto pivot = ranges::min(
            mSymbols | ranges::view::transform([](const auto& i) { return reinterpret_cast<uintptr_t>(i.second); }));
        pivot = alignUpper(pivot);
        auto sections = parseElf(objectFile);
        ALOG_TRACE(LOG_TAG) << objectFile << " :";
        ::extractSymbols(sections, [&](AStringView name, const Elf64_Sym& sym) {
            localSymbols[name] = LocalSymbol {
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
                                   reinterpret_cast<void*>(pivot), section.data.size(), PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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

        for (auto rela : i) {
            size_t offset = rela.r_offset;
            size_t symidx = ELF64_R_SYM(rela.r_info);

            // Lookup symbol name:
            auto symname = getSymbolName(symidx);
            if (symname == "") {
                symname = ".text";
            }
            auto writeAddr = reinterpret_cast<char*>(targetSection.page.get()) + offset;
            if (writeAddr == nullptr) {
                continue;
            }

            auto getAddr = [&](const auto& table, bool strict = true) -> char* {
                if (symname == ".text") {
                    return static_cast<char*>((*textSection)->page.get());
                }
                if (auto it = table.find(symname); it != table.end()) {
                    return reinterpret_cast<char*>(it->second);
                }
                if (auto it = localSymbols.find(symname);
                    it != localSymbols.end() && it->second.sectionIdx != SHN_UNDEF) {
                    auto& targetSection = sections.at(it->second.sectionIdx);
                    if (targetSection.page.get() == nullptr) {
                        printSectionName();
                        ALogger::err(LOG_TAG) << "Target section of this symbol is not mapped: \"" << symname << "\"";
                        errored = true;
                        return nullptr;
                    }
                    return reinterpret_cast<char*>(targetSection.page.get()) + it->second.offset;
                }
                if (strict) {
                    printSectionName();
                    ALogger::err(LOG_TAG) << "Unresolved reference to \"" << symname << "\"";
                    errored = true;
                }
                return nullptr;
            };

            auto type = ELF64_R_TYPE(rela.r_info);
            if (ALogger::global().isTrace()) {
                printSectionName();
                ALOG_TRACE(LOG_TAG)
                    << "Patching relocation: symname=\"" << symname << "\", writeAddr=" << (void*) writeAddr
                    << ", type=" << type;
            }

            switch (type) {
                case R_X86_64_PC32: {
                    // doesn't care plt or got, just an offset
                    char* symaddr = nullptr;
                    if (auto i = mGot.find(symname);
                        i != mGot.end() && validateDistance(symname, writeAddr, reinterpret_cast<char*>(i->second))) {
                        symaddr = (char*) i->second;
                        ALOG_TRACE(LOG_TAG) << "Using got";
                    } else if (auto addr = getAddr(mSymbols)) {
                        symaddr = addr;
                        ALOG_TRACE(LOG_TAG) << "Using abs";
                    }
                    if (symaddr == nullptr) {
                        continue;
                    }
                    if (!validateDistance(symname, writeAddr, symaddr)) {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Relocation is too far away: symname=\"" << symname << "\", symaddr=" << (void*) symaddr
                            << ", writeAddr=" << (void*) writeAddr;
                        errored = true;
                    }

                    ALOG_TRACE(LOG_TAG) << "symaddr=" << (void*) symaddr;
                    int32_t rel = symaddr - writeAddr + rela.r_addend;
                    memcpy(writeAddr, &rel, 4);
                    break;
                }
                case R_X86_64_PLT32: {
                    auto symaddr = getAddr(mSymbols);
                    if (symaddr == nullptr) {
                        continue;
                    }
                    if (!validateDistance(symname, writeAddr, symaddr)) {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Relocation is too far away: symname=\"" << symname << "\", symaddr=" << (void*) symaddr
                            << ", writeAddr=" << (void*) writeAddr;
                        errored = true;
                    }
                    ALOG_TRACE(LOG_TAG) << "symaddr=" << (void*) symaddr;
                    int32_t rel = symaddr - writeAddr + rela.r_addend;
                    memcpy(writeAddr, &rel, 4);
                    break;
                }

                case R_X86_64_64: {
                    auto symaddr = getAddr(mSymbols);
                    if (symaddr == nullptr) {
                        continue;
                    }
                    ALOG_TRACE(LOG_TAG) << "symaddr=" << (void*) symaddr;
                    uint64_t abs = (uintptr_t) symaddr + rela.r_addend;
                    memcpy(writeAddr, &abs, 8);
                    break;
                }

                case R_X86_64_GOTPCREL:
                case R_X86_64_GOTPCRELX: {
                do_R_X86_64_GOTPCRELX:
                    // R_X86_64_GOTPCREL(X)`** is used for position-independent code to access globals via the
                    // Global Offset Table (GOT), usually with instructions like mov foo@GOTPCREL(%rip),%reg.
                    auto symaddr = getAddr(mGot);
                    if (symaddr == nullptr) {
                        continue;
                    }

                    if (!validateDistance(symname, writeAddr, symaddr)) {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Relocation is too far away: symname=\"" << symname << "\", symaddr=" << (void*) symaddr
                            << ", writeAddr=" << (void*) writeAddr << ", type=" << type;
                        errored = true;
                    }
                    ALOG_TRACE(LOG_TAG) << "symaddr=" << (void*) symaddr;
                    int32_t rel = symaddr - writeAddr + rela.r_addend;
                    memcpy(writeAddr, &rel, 4);
                    break;
                }

                case R_X86_64_REX_GOTPCRELX: {
                    // R_X86_64_REX_GOTPCRELX and R_X86_64_GOTPCRELX are very similar—"X" represents an "relaxed" or
                    // optimized version of the relocation, where the linker is allowed to optimize away the load
                    // from the GOT and use a direct move if possible.
                    //
                    // The relocation is not only about patching addresses; it may actually require rewriting
                    // the instruction opcode and operands, not just a simple offset/fixup.
                    auto symaddr = getAddr(mSymbols, false);
                    if (symaddr == nullptr) {
                        goto do_R_X86_64_GOTPCRELX;
                    }
                    {
                        // Get pointer to opcode bytes (3 bytes before the immediate)
                        uint8_t* op = reinterpret_cast<uint8_t*>(writeAddr) - 3;
                        // Preserve register bits: original MODRM is op[2], lower 2 bits = reg index
                        uint8_t regIndex = op[2] & 0x03;
                        // Patch: if it's a MOV r, imm32 ("48 C7 Cx ...")
                        op[0] = 0x48;              // REX.W
                        op[1] = 0xC7;              // Opcode for MOV r/m64, imm32
                        op[2] = 0xC0 | regIndex;   // MODRM: direct to register
                        rela.r_addend = 0;
                    }

                    if (!validateDistance(symname, 0, symaddr)) {
                        printSectionName();
                        ALogger::err(LOG_TAG)
                            << "Relocation is too far away: symname=\"" << symname << "\", symaddr=" << (void*) symaddr
                            << ", writeAddr=" << (void*) writeAddr << ", type=" << type;
                        errored = true;
                    }
                    ALOG_TRACE(LOG_TAG) << "symaddr=" << (void*) symaddr;
                    int32_t rel = reinterpret_cast<uintptr_t>(symaddr) + rela.r_addend;
                    memcpy(writeAddr, &rel, 4);
                    break;
                }
                default: {
                    printSectionName();
                    ALogger::err(LOG_TAG) << "Unsupported relocation type: " << type << " in \"" << symname << "\"";
                    errored = true;
                }
            }
        }
    }
    if (errored) {
    printErrorAndExit:
        ALogger::err(LOG_TAG) << "\"" << objectFile << "\": refusing patch due to errors. Target is not changed.";
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
            ALogger::err(LOG_TAG) << "mprotect failed: " << (void*) mapped.page.get() << " : " << strerror(errno);
            goto printErrorAndExit;
        }
    }

    // 4. Hook old symbols with newly loaded ones
    for (const auto& [name, symbol] : localSymbols) {
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
        ALOG_TRACE(LOG_TAG) << "New function: " << (void*) destinationAddr << " : " << name;

        auto sourceAddr = mSymbols.find(name);
        if (sourceAddr == mSymbols.end()) {
            continue;
        }

        if (std::memcmp(sourceAddr->second, destinationAddr, symbol.size) == 0) {
            continue;
        }

        ALOG_TRACE(LOG_TAG)
            << "Hooking symbol: " << name << " : " << (void*) sourceAddr->second << " -> " << (void*) destinationAddr;
        hook(sourceAddr->second, destinationAddr);
    }
    mAllocatedPages.insertAll(
        sections | ranges::view::transform([](auto& section) -> decltype(auto) { return std::move(section.page); }));
    ALOG_TRACE(LOG_TAG) << "Done";
}
