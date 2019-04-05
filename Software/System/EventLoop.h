
#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <functional>
#include <cstdint>

#include "CircularBuffer.h"



namespace System
{

/**	Class EventLoop
 *		Manages the execution of pending Tasks in order they were added to the queue.
 *		If no task is pending, the system uses the TDeviceCore class to put the µC
 *		into sleep mode until the next Task enters the queue.
 *
 *	@template TDeviceCore - Class to handle core device functionality
 *		Must implement the following STATIC methods:
 *		- void sleep(void)
 *		- void enableInterrupts(void)
 *		- void disableInterrupts(void)
 *
 *	@template TQueueSize - Sets the size of the Task queue. Default is 20
 */
template <typename TDeviceCore, std::size_t TQueueSize = 20>
class EventLoop_t
{
public:

	//---------------------------------------------------------------------------
	//--------------------------- Class 'Task' ----------------------------------
	// Wrapper class around a std::function object. Stores a callback method to
	// be called in the EventLoop.
	class Task {
	public:

		typedef std::function<void (void)> HandlerType;

		Task() : handler_(nullptr) {}
		explicit Task(nullptr_t) : handler_(nullptr) {}

		Task(const Task& rhs) : handler_(rhs.handler_) {}
		Task(Task&& rhs) : handler_(rhs.handler_) {}

		~Task() { }

		Task& operator=(const Task& rhs) { this->handler_ = rhs.handler_; return *this; }
		Task& operator=(Task&& rhs) { this->handler_ = rhs.handler_; return *this; }

		template <typename TFunc>
		static Task create(TFunc&& func) { Task t; t.handler_ = std::forward<TFunc>(func); return t; }

		template <typename TFunc>
		void addHandler(TFunc&& func) const {handler_ = std::forward<TFunc>(func); }

		void operator()(void) const { if(handler_) handler_(); }

	private:

		mutable HandlerType handler_;
	};
	//---------------------------------------------------------------------------

	// Constructor
	EventLoop_t();


	// Destructor
	~EventLoop_t();


	/**	Adds a new Task to the event queue
	 *	@param newTask - EventLoop::Task object for the queue
	 */
	template <typename FuncType>
	void addTaskToQueue(FuncType&& func) const;


	/** Start the Event Loop. Never returns exept you call stop()
	 */
	void run(void) const;


	/** Stop the execution of the Event Loop
	 * 	Wont stop immediately! Finishes executing the current task and stops afterwards
	 */
	void stop(void) const;


	/** Reset the Event Loop
	 */
	void reset(void) const;


	/**	Lock the Event Loop (disable all interrupts)
	 */
 	inline void lock(void) const { TDeviceCore::disableInterrupts(); }


	/**	Unlock the Event Loop (enable all interrupts)
	 */
	inline void unlock(void) const { TDeviceCore::enableInterrupts(); }


private:

	// Ringbuffer to store all pending tasks
	mutable Util::CircularBuffer<Task, TQueueSize> queue_;

	// Flag to indicate wheather the Event Loop is stopped
	volatile mutable bool stopped_;

};

//---------------------------------------------------------------------------------------
// --------------------- Implementation of Class 'EventLoop' ----------------------------

template <typename TDeviceCore, std::size_t TQueueSize>
EventLoop_t<TDeviceCore, TQueueSize>::
EventLoop_t() :
	stopped_(false)
{
}


template <typename TDeviceCore, std::size_t TQueueSize>
EventLoop_t<TDeviceCore, TQueueSize>::
~EventLoop_t()
{
}


template <typename TDeviceCore, std::size_t TQueueSize>
template <typename FuncType>
void EventLoop_t<TDeviceCore, TQueueSize>::
addTaskToQueue(FuncType&& func) const
{
	// Add given Task to the queue
	// Disable all interrupts prior to writing to the queue to prevent a race condition
	TDeviceCore::disableInterrupts();
	queue_.push(std::move(Task::create(func)));
	TDeviceCore::enableInterrupts();
}


template <typename TDeviceCore, std::size_t TQueueSize>
void EventLoop_t<TDeviceCore, TQueueSize>::
run(void) const
{
	while (true) {
		// Exit when stop flag is set
		if (stopped_)
			break;

		// Disable all interrupts while reading from the queue to prevent a race condition
		TDeviceCore::disableInterrupts();

		if (queue_.available()) {
			// Get next Task from the queue
			auto nextTask = queue_.peek();

			// Enable interrupts during Task execution
			TDeviceCore::enableInterrupts();

			// Execute next Task
			nextTask();

			// delete executed Task
			TDeviceCore::disableInterrupts();
			queue_.deleteNext();
			TDeviceCore::enableInterrupts();
		}
		else {
			// Enable interrupts during sleep mode
			TDeviceCore::enableInterrupts();

			// Enter sleep mode
			TDeviceCore::sleep();
		}
	}
}


template <typename TDeviceCore, std::size_t TQueueSize>
void EventLoop_t<TDeviceCore, TQueueSize>::
stop(void) const
{
	// set stopped flag. EventLoop will be stopped after current Task is executed completely
	stopped_ = true;
}


template <typename TDeviceCore, std::size_t TQueueSize>
void EventLoop_t<TDeviceCore, TQueueSize>::
reset(void) const
{
	// clear the Task queue
	queue_.clear();

	// Reset stopped flag
	stopped_ = false;
}


}	/* end namespace System */

#endif /* EVENTLOOP_H_ */
