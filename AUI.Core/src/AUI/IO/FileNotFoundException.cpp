#include "FileNotFoundException.h"

FileNotFoundException::FileNotFoundException(const AString& message) : IOException(message) {}
