#pragma once

#include <AUI/Core.h>
#include <AUI/Common/AString.h>

#include "AUI/Autumn/Autumn.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/i18n/AI18n.h"


class AException;

namespace AError
{
	class IErrorHandler
	{
	public:
		virtual void handle(const AException& exception, const AString& furtherInfo)= 0;
		virtual ~IErrorHandler() = default;
	};
	
	inline void handle(const AException& exception, const AString& furtherInfo = {})
	{
		if (furtherInfo.empty()) {
			ALogger::err(u8"Произошла ошибка: "_i18n + exception.getMessage());
		}
		else
		{
			ALogger::err(furtherInfo + ": " + exception.getMessage());
		}
		if (auto handler = Autumn::get<AError::IErrorHandler>())
			handler->handle(exception, furtherInfo);

		// todo stacktrace
	}
}
