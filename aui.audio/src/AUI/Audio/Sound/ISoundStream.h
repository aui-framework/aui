#pragma once

#include "AUI/IO/IInputStream.h"
#include "AUI/Audio/AAudioFormat.h"

class ISoundStream: public IInputStream {
    public:
        /**
         * @brief Get general info about sound stream
         * @return AAudioFormat containing info about sound stream
         */
        virtual AAudioFormat info() = 0;

        /**
         * @brief Rewind audio stream to begin
         */
        virtual void rewind() = 0;


        virtual ~ISoundStream() = default;
};
