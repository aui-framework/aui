//
// Created by alex2772 on 11/6/20.
//

#include "ICommand.h"

void ICommand::printHelp() {
    std::cout << getName() << " " << getSignature() << std::endl << "\t";
    size_t counter = 0;
    auto desc = getDescription().toStdString();
    for (auto i = desc.begin(); i != desc.end(); ++i, ++counter) {
        std::cout << (*i);
        if (counter == 80) {
            counter = 0;
            std::cout << std::endl << "\t";
        }
    }
    std::cout << std::endl;
}
