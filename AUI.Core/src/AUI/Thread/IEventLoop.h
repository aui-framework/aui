#pragma once

#include <AUI/Core.h>

class API_AUI_CORE IEventLoop
{
private:
	IEventLoop* mPrevEventLoop;
	
public:
	IEventLoop();
	virtual ~IEventLoop();

	/**
	 * \brief Поток этого IEventLoop получил новое сообщение,
	 *		  которое необходимо обработать.
	 */
	virtual void notifyProcessMessages() = 0;
};
