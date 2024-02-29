// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/Platform/Entry.h>
#include <Command/Pack.h>
#include <Command/Lang.h>
#include <Command/Css2ass.h>
#include <Command/PackManual.h>
#include <Command/Svg2ico.h>

#include "Toolbox.h"
#include "Command/Svg2png.h"
#include "Command/Auisl.h"

Toolbox toolbox;

template<typename T>
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

AUI_ENTRY
{
	std::cout << "Advanced Universal Interface toolbox" << std::endl;
	if (args.size() <= 1)
	{
		printAllCommands();
	}
	else
	{
		for (int i = 2; i < args.size(); ++i)
		{
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
		    std::cerr << std::endl << e.getMessage() << std::endl;
		    return -1;
        } catch (...) {
		    return -1;
        }
	}

	return 0;
}
