//
// Created by alex2772 on 2020-08-12.
//

#include "AModel.h"

#define ui

AModel::AModel() {
    ui {

    };
}

AModel*& AModel::currentModel() {
    thread_local AModel* storage = nullptr;
    return storage;
}
