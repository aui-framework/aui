#pragma once
#include <thread>
#include <utility>
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AMutex.h"
#include "AConditionVariable.h"
#include <functional>

class IEventLoop;

/**
 * \brief Абстрактный поток. Не все потоки созданы через AThread,
 *        а это - по сути абстрактный интерфейс.
 */
class API_AUI_CORE AAbstractThread
{
	friend class IEventLoop;
	friend class AThread;
public:

	/**
	 * \brief Идентификатор потока.
	 */
	typedef std::thread::id id;

private:
	/**
	 * \brief Очередь сообщений.
	 */
	ADeque<std::function<void()>> mMessageQueue;

	/**
	 * \brief Мьютекс для потокобезопасности очереди сообщений
	 */
	AMutex mQueueLock;
	/**
	 * \brief Мьютекс для потокобезопасности поля mCurrentEventLoop.
	 */
	AMutex mEventLoopLock;

	/**
	 * \brief Идентификатор этого потока.
	 */
	id mId;

	AConditionVariable mSleepCV;
	
	/**
	 * \brief Текущий IEventLoop для этого потока. Требуется
	 *		  для межпоточной доставки сообщений.
	 */
	IEventLoop* mCurrentEventLoop = nullptr;

	AAbstractThread() = default;
	AAbstractThread(const id& id);


	/**
	 * \brief Хранилище указателя на AAbstractThread для текущего потока.
	 */
	static _<AAbstractThread>& threadStorage();

public:
	/**
	 * \return Идентификатор этого потока.
	 */
	id getId() const;

	/**
	 * \brief Передать задание на обработку (сообщение) этому
	 *		  потоку.
	 *        Сообщения обрабатываются самим фреймворком при помощи
	 *        AEventLoop и участие пользователя не требуется.
	 *        Если очень хочется, можно обработать эти сообщения,
	 *        вызвав функцию AThread::processMessages().
	 *
	 */
	void enqueue(const std::function<void()>& f);

	/**
	 * \brief Обработать сообщения от других потоков.
	 *        Вызывается самим фреймворком при помощи
	 *        IEventLoop и участие пользователя не требуется.
	 *        Если очень хочется, можно вызвать самому, фреймворк
	 *        от этого не помрёт.
	 */
	void processMessages();

	virtual ~AAbstractThread();

	/**
	 * \return true, если для потока запрошено прерывание.
	 */
	virtual bool isInterrupted();
	/**
	 * \brief сбросить флаг прерывания.
	 */
	virtual void resetInterrupted();


    template <class Callable>
    inline void operator<<(Callable fun)
    {
        enqueue(fun);
    }
    template <class Callable>
    inline void operator*(Callable fun)
    {
        enqueue(fun);
    }
};

/**
 * \brief Поток.
 */
class API_AUI_CORE AThread : public AAbstractThread, public AObject, public std::enable_shared_from_this<AThread>
{
public:
	/**
	 * \brief Исключение, которое выплёвывается из точки прерывания,
	 *        если для потока было запрошено это самое прерывание.
	 *        Обрабатывается в AThread::start. Может быть обработано
	 *        пользователем.
	 */
	class AInterrupted
	{
	};

private:
	/**
	 * \brief Нативный хендл потока.
	 */
	std::thread* mThread = nullptr;

	/**
	 * \brief Функция, запускаемая при выполнении этого потока.
	 *	      Становится nullptr после start().
	 */
	std::function<void()> mFunctor;

	/**
	 * \brief true, если для этого потока было запрошено
	 *		  прерывание.
	 */
	bool mInterrupted = false;

public:

	AThread(std::function<void()> functor)
		: mFunctor(std::move(functor))
	{
	}

	/**
	 * \brief Запустить выполнение потока.
	 */
	void start();

	/**
	 * \brief Прервать выполнение потока.
	 *	      Естественно, так как С++ - компилируемый язык, то для
	 *	      корректной работы этой функции прерываемый код должен
	 *	      содержать вызовы функции AThread::interruptionPoint().
	 */
	void interrupt();


	/**
	 * \brief Заснуть на указаную длительность.
	 *	      В большинстве операционных систем гарантируется, что
	 *	      пройдёт указаное количество времени, но не гарантируется,
	 *	      что пройдёт больше времени, чем нужно.
	 *	      
	 *	      Есть поддержка interrupt().
	 * \param durationInMs длительность в миллисекундах
	 */
	static void sleep(unsigned durationInMs);

	/**
	 * \return Текущий поток.
	 */
	static _<AAbstractThread> current();

	/**
	 * \return Точка прерывания. Необходима для работы функции
	 *		   AThread::interrupt.
	 */
	static void interruptionPoint();

	bool isInterrupted() override;
	void resetInterrupted() override;
	
	void join();

	virtual ~AThread();
};
