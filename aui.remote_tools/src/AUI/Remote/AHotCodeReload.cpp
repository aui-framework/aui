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

void AHotCodeReload::reload() {
    static constexpr auto LOG_TAG = "Hot code reload";
    AThread::current()->enqueue([this] {
      auto newSymbols = extractSymbols();
      bool symbolsChanged = false;
      for (const auto& [name, addr] : mSymbols) {
          auto it = newSymbols.find(name);
          if (it == newSymbols.end()) {
              ALogger::err(LOG_TAG) << "Can't apply code: symbol \"" << name << "\" not found";
              symbolsChanged = true;
              continue;
          }

          if (it->second != addr) {
              ALogger::err(LOG_TAG) << "Can't apply code: symbol \"" << name << "\" was relocated from " << addr << " to " << it->second;
              symbolsChanged = true;
          }
      }
      if (symbolsChanged) {
          //            return;
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


          if (sectionHeader.sh_offset == 0) {
              continue;
          }

          AStringView sectionName = static_cast<const char*>(stringTable.data() + sectionHeader.sh_name);
          auto sectionStart = static_cast<char*>(reinterpret_cast<void*>(sectionHeader.sh_addr));
          auto sectionEnd = static_cast<char*>(static_cast<char*>(sectionStart) + sectionHeader.sh_size);

          if (sectionName.contains(".plt")) {
             // Overwriting .plt entries corrupts your program’s ability to perform dynamic calls. These entries are
             // set up at program start; overwriting them with on-disk data is almost always incorrect.
             continue;
          }
          if (sectionName.contains(".got")) {
              continue;
          }
          if (sectionName.contains(".bss")) {
             continue;
          }
          if (sectionName.contains(".rodata")) {
              continue;
          }
          if (sectionName.contains(".data.rel.ro")) {
              continue;
          }
          if (sectionName.contains(".data")) {
              continue;
          }

          ALogger::info(LOG_TAG)
              << "Patching: " << sectionName.data() << " " << sectionHeader.sh_type << " " << sectionHeader.sh_flags << " "
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
//          mprotect(aligned, sectionEnd - aligned, prot);
      }
      ALogger::info(LOG_TAG) << "Reloaded, good luck!";
      emit completed;
    });
}

std::unordered_map<AString, void*> AHotCodeReload::extractSymbols() {
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
                if (type == STT_NOTYPE) {
                    continue;
                }

                symbols["{} ({})"_format(std::string_view(&stringTable.at(sym.st_name)), sym.st_info)] = reinterpret_cast<void*>(sym.st_value);
            }
        }
    }
    return symbols;
}
