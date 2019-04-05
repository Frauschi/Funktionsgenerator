
#ifndef TIMERMGR_H_
#define TIMERMGR_H_


#include <cstdint>
#include <functional>
#include <array>
#include <chrono>
#include <algorithm>

#include "IdManager.h"


namespace Driver
{

template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits = 20>
class TimerMgr
{
public:

	typedef typename THardwareTimer::TimeUnitDuration TimeUnitDuration;

	typedef typename TEventLoop::Task::HandlerType CallbackHandlerType;

	typedef Util::IdManager::IdType IdType;


	/**	Constructor - creates a new TimerMgr object
	 * 	@param waitTimer - Reference to a device object for the wait timer
	 * 	@param repeatTimer - Reference to a device object for the repeat timer
	 */
	TimerMgr(const THardwareTimer& timer, const TEventLoop& el);

	/**	Destructor
	 */
	~TimerMgr();


	//----------------------- Wait timer -------------------------

	/**	 asynchronously wait for a specific time
	 * 	@param waitTime - object of type 'std::chrono::duration': specifies the time to wait
	 * 	@param callback - reference to a function object. Will be called when the wait is over
	 * 	@param repeated - flag; indicates whether the wait will be repeated when finished
	 * 	@return wait ID - ID of that specific wait
	 */
	template <typename TRep, typename TPeriod, typename TFunc>
	IdType asyncWait(const std::chrono::duration<TRep, TPeriod>& waitTime, TFunc&& callback, bool repeated = false) const;


	/**	asynchronously call a method periodically
	 * 	@param waitTime - object of type 'std::chrono::duration': specifies the time between the method calls
	 * 	@param func - reference to a function object. Will be called periodically with the given time
	*/
	template <typename TRep, typename TPeriod, typename TFunc>
	IdType asyncRepeat(const std::chrono::duration<TRep, TPeriod>& repeatTime, TFunc&& func) const;


	/**	abort specific wait and return the remaining time in milliseconds
	 * 	@param timerID - Reference to the ID of the wait that should be aborted.
	 * 			Is set to zero if wait is aborted successfully
	 * 	@return Integer - remaining time in milliseconds
	 */
	std::uint32_t abort(volatile IdType& timerID) const;


	/*	get remaining time of specific ongoing wait
	 * 	@param waitID - ID of the specific wait
	 * 	@return Integer - remaining time in milliseconds
	 */
	std::uint32_t getRemainingTime(volatile IdType timerID) const;


private:

	// Internal helper struct to hold the information about the current waits
	struct ActiveWait_t
	{
		CallbackHandlerType callback;
		TimeUnitDuration completeWaitTime;
		TimeUnitDuration remainingWaitTime;
		IdType id;
		bool repeating;
	};
	mutable std::array<ActiveWait_t, TMaxActiveWaits> activeWaits_;
	volatile mutable std::size_t currentActiveWaits_;

	Util::IdManager idMgr_;

	const THardwareTimer& timer_;
	const TEventLoop& el_;


	void interruptHandler(void) const;

	// sorts the activeWaits_ array and starts the hardware timer with the smallest valid wait time
	// @param lastIndexToSort - Array will be sorted from the beginning up to this index
	void sortAndStart(const std::size_t lastIndexToSort) const;
};



//---------------------------------------------------------------------------------------
// -------------------------------- Implementation --------------------------------------

template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
TimerMgr(const THardwareTimer& timer, const TEventLoop& el)  :
	currentActiveWaits_(0),
	timer_(timer),
	el_(el)
{
	// Fill waits array with default values
	for (std::size_t i = 0; i < activeWaits_.size(); i++) {
		activeWaits_[i].callback = nullptr;
		activeWaits_[i].completeWaitTime = TimeUnitDuration(0);
		activeWaits_[i].remainingWaitTime = TimeUnitDuration(0);
		activeWaits_[i].id = 0;
		activeWaits_[i].repeating = false;
	}


	// set Callback function for wait timer
	timer_.setCallbackHandler([this]() { this->interruptHandler(); });
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
~TimerMgr()
{

}


//------------------------------------------------------------
//----------------------- Wait timer -------------------------

template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
template <typename TRep, typename TPeriod, typename TFunc>
typename TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::IdType
TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
asyncWait(const std::chrono::duration<TRep, TPeriod>& waitTime, TFunc&& callback, bool repeated) const
{
	if (currentActiveWaits_ == TMaxActiveWaits) {
		// too many active waits -> return invalid id
		return 0;
	}

	IdType newID = idMgr_.getID();

	// check if there are active waits
	if (currentActiveWaits_ > 0) {
		// if yes, suspend
		timer_.stop();

		// get already waited time and substract it from all waits to perform
		auto waitedTime = timer_.getWaitedTime();
		for (std::size_t i = 0; i < currentActiveWaits_; i++)
			activeWaits_[i].remainingWaitTime -= waitedTime;
	}

	// add new wait time to the array
	activeWaits_[currentActiveWaits_].callback = std::forward<TFunc>(callback);
	activeWaits_[currentActiveWaits_].completeWaitTime = std::chrono::duration_cast<TimeUnitDuration>(waitTime);
	activeWaits_[currentActiveWaits_].remainingWaitTime = std::chrono::duration_cast<TimeUnitDuration>(waitTime);
	activeWaits_[currentActiveWaits_].id = newID;
	activeWaits_[currentActiveWaits_].repeating = repeated;

	currentActiveWaits_++;

	sortAndStart(currentActiveWaits_);

	return newID;
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
template <typename TRep, typename TPeriod, typename TFunc>
typename TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::IdType
TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
asyncRepeat(const std::chrono::duration<TRep, TPeriod>& repeatTime, TFunc&& func) const
{
	return asyncWait(repeatTime, func, true);
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
std::uint32_t TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
abort(volatile IdType& timerID) const
{
	// check if there is an ongoing wait
	if (currentActiveWaits_ > 0) {
		// if yes, suspend it
		timer_.stop();

		// get already waited time and substract it from all waits to perform
		auto waitedTime = timer_.getWaitedTime();
		for (std::size_t i = 0; i < currentActiveWaits_; i++) {
			activeWaits_[i].remainingWaitTime -= waitedTime;
		}
	}
	else
		return 0;


	std::uint32_t retVal = 0;

	// check if given ID is in waitsArr_
	for (std::size_t i = 0; i < currentActiveWaits_; i++) {
		if (activeWaits_[i].id == timerID) {
			// store remaining time
			retVal = activeWaits_[i].remainingWaitTime.count();

			// delete item
			activeWaits_[i].callback = nullptr;
			activeWaits_[i].completeWaitTime = TimeUnitDuration(0);
			activeWaits_[i].remainingWaitTime = TimeUnitDuration(0);
			activeWaits_[i].id = 0;
			activeWaits_[i].repeating = false;

			currentActiveWaits_ -= 1;

			// set given ID to zero (invalid ID) to indicate that the wait is aborted
			timerID = 0;
			break;
		}
	}

	// restart timer
	if (currentActiveWaits_ > 0)
		sortAndStart(currentActiveWaits_ + 1);

	return retVal;
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
std::uint32_t TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
getRemainingTime(volatile IdType timerID) const
{
	std::uint32_t retVal = 0;

	// check if there is an ongoing wait
	if (currentActiveWaits_ > 0) {
		// if yes, suspend it
		timer_.stop();

		// get already waited time and substract it from all waits to perform
		auto waitedTime = timer_.getWaitedTime();
		for (std::size_t i = 0; i < currentActiveWaits_; i++) {
			activeWaits_[i].remainingWaitTime -= waitedTime;

			// check if given ID is in waitsArr_
			if (activeWaits_[i].id == timerID) {
				// get remaining time
				retVal = activeWaits_[i].remainingWaitTime.count();
			}
		}

		// restart Wait
		timer_.start(activeWaits_[currentActiveWaits_ - 1].waitTime.count());
	}

	return retVal;
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
void TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
interruptHandler(void) const
{
	// Store index of last valid wait element in the array
	std::size_t finishedIndex = currentActiveWaits_ - 1;

	// handle last element that has just finished
	el_.addTaskToQueue(activeWaits_[finishedIndex].callback);

	// subtract waited time from the remaining waits
	for (std::size_t i = 0; i < finishedIndex; i++) {
		activeWaits_[i].remainingWaitTime -= activeWaits_[finishedIndex].remainingWaitTime;

		// If remainingWaitTime of another element in the array is also zero, we have to handle this wait, too
		if (activeWaits_[i].remainingWaitTime.count() <= 0) {
			// add callback handler to the EventLoop queue
			el_.addTaskToQueue(activeWaits_[i].callback);

			// if repeating flag is set, update remainingWaitTime
			if (activeWaits_[i].repeating == true) {
				activeWaits_[i].remainingWaitTime = activeWaits_[i].completeWaitTime;
			}
			else {
				// delete element
				activeWaits_[i].callback = nullptr;
				activeWaits_[i].completeWaitTime = TimeUnitDuration(0);
				activeWaits_[i].remainingWaitTime = TimeUnitDuration(0);
				activeWaits_[i].id = 0;

				currentActiveWaits_ -= 1;
			}
		}
	}

	// if repeating flag is set, update remainingWaitTime
	if (activeWaits_[finishedIndex].repeating == true) {
		activeWaits_[finishedIndex].remainingWaitTime = activeWaits_[finishedIndex].completeWaitTime;
	}
	else {
		// delete element
		activeWaits_[finishedIndex].callback = nullptr;
		activeWaits_[finishedIndex].completeWaitTime = TimeUnitDuration(0);
		activeWaits_[finishedIndex].remainingWaitTime = TimeUnitDuration(0);
		activeWaits_[finishedIndex].id = 0;

		currentActiveWaits_ -= 1;
	}

	// sort array and start next Wait
	if (currentActiveWaits_ > 0) {
		sortAndStart(finishedIndex + 1);
	}
}


template <typename THardwareTimer, typename TEventLoop, std::size_t TMaxActiveWaits>
void TimerMgr<THardwareTimer, TEventLoop, TMaxActiveWaits>::
sortAndStart(const std::size_t lastIndexToSort) const
{
	// sort waitsArr_ descending
	std::sort(activeWaits_.begin(), &activeWaits_[lastIndexToSort], [&](const ActiveWait_t& a, const ActiveWait_t& b) -> bool {
		return a.remainingWaitTime.count() > b.remainingWaitTime.count();
	});

	// start next Wait (last wait Time in array that is unequal to zero)
	timer_.start(activeWaits_[currentActiveWaits_ - 1].remainingWaitTime.count());
}


} /* namespace driver */

#endif /* TIMERMGR_H_ */
