#pragma once

#include <AUI/Core.h>

class API_AUI_CORE IEventLoop
{
public:
	IEventLoop() {}
	virtual ~IEventLoop() {}

	/**
	 * \brief Поток этого IEventLoop получил новое сообщение,
	 *		  которое необходимо обработать.
	 */
	virtual void notifyProcessMessages() = 0;

	/**
	 * \brief Начать обработку сообщений.
	 */
	virtual void loop() = 0;


    class API_AUI_CORE Handle {
	private:
        IEventLoop* mPrevEventLoop;
	    IEventLoop* mCurrentEventLoop;

	public:
        explicit Handle(IEventLoop* loop);
        ~Handle();
    };
};
