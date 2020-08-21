#include "Data.h"

template<>
_<ASqlDatabase>& Autumn::detail::storage()
{
	static _<ASqlDatabase> s;
	return s;
}