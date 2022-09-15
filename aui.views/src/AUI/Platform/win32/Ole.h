#pragma once


class Ole {
public:
    ~Ole();

    static Ole& inst();

private:
    Ole();
};


