#pragma once

#include "AUI/Audio/ISoundInputStream.h"

/**
 * @brief ISoundPipe accepts sound data and outputs sound samples, useful for decoded audio passed in packets
 * @note Pipe should decode audio on write() and give ready samples on read()
 */
class ISoundPipe : public ISoundInputStream, public IOutputStream {
};
