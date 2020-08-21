#pragma once
#pragma warning(disable: 4251)

#include "AUI/api.h"
#include "AUI/Autumn/Autumn.h"
#include "AUI/Common/SharedPtrTypes.h"

class ASqlDatabase;

namespace Autumn
{
	namespace detail
	{
		template<>
		API_AUI_DATA _<ASqlDatabase>& storage();
	}
}


typedef unsigned id_t;
