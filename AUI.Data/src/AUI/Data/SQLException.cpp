#include "SQLException.h"

SQLException::SQLException()
{
}

SQLException::SQLException(const AString& message): AException(message)
{
}

SQLException::~SQLException()
{
}
