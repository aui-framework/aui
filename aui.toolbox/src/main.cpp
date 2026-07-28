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

/*
                                     ,*%%%
                                      ,%%%
                                (%%%%%%%%%%%%%%%
                                    /%%#  %%
                                      (% %
                                      /%,&
                                      ,% &         (%%%,.%%%
                                      ,%,&     ,%%%        %&
                                %% .  %%.%  %&&%           %
                                  .# %% %  %#            *%
                                   %%%%%%%%%           #%.
                                  %%%%%%&(   &      %%%
                                   ,%%%%%%%%    *%%
                                    &#(    %#%%
                                   % .  /##.(
                                 %, /        %/
                            (%%. ,*(%&&&&&% (   ##
                       *%%/  /%% %,%%%%%%%%%%%/,%/%
                    %#.      % %%%%%%%%%%%%%%%.%% &
                   #           %/ &%%%%%%%%%%# %%
                  %(            %#&% .%%%% *#/%,
                   ,/#             *(######(*                ,
                       (&#,%(.                 #&,%%%%%%%%%.

*/

#include <AUI/Platform/Entry.h>
#include <Command/Pack.h>
#include <Command/Lang.h>
#include <Command/Css2ass.h>
#include <Command/PackManual.h>
#include <Command/Svg2ico.h>
#include <Command/ConvertImage.h>

#include "Toolbox.h"
#include "Command/Svg2png.h"
#include "Command/Auisl.h"
#include "Command/GenerateDlStubs.h"

Toolbox toolbox;

template <typename T>
void registerCommand() {
    ICommand* t = new T;
    toolbox.commands[t->getName()] = t;
}
Toolbox::Toolbox() {
    registerCommand<Pack>();
    registerCommand<Lang>();
    registerCommand<Css2ass>();
    registerCommand<PackManual>();
    registerCommand<Svg2png>();
    registerCommand<Svg2ico>();
    registerCommand<Auisl>();
    registerCommand<ConvertImage>();
    registerCommand<GenerateDlStubs>();
}
Toolbox::~Toolbox() {
    for (auto& c : commands) {
        delete c.second;
    }
}

void printAllCommands() {
    std::cout << "Available commands:" << std::endl;
    for (auto& c : toolbox.commands) {
        c.second->printHelp();
    }
}

AUI_ENTRY {
    std::cout << "Advanced Universal Interface toolbox" << std::endl;
    if (args.size() <= 1) {
        printAllCommands();
    } else {
        for (int i = 2; i < args.size(); ++i) {
            toolbox.args << args[i];
        }

        ICommand* command;
        try {
            if (auto c = toolbox.commands.contains(args[1])) {
                command = c->second;
                command->run(toolbox);
            } else {
                printAllCommands();
                return -1;
            }
        } catch (const IllegalArgumentsException& e) {
            std::cerr << std::endl << e.getMessage() << std::endl;
            command->printHelp();
            return -1;
        } catch (const AException& e) {
            std::cerr << std::endl << e << std::endl;
            return -1;
        } catch (...) {
            return -1;
        }
    }

    return 0;
}
