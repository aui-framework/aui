#pragma once

struct AModel {
private:
    static AModel*& currentModel();
public:
    AModel();
};

