/*
 * SystemHeartbeat.h
 *
 *  Created on: 12.03.2019
 *      Author: dem34203
 */

#ifndef SYSTEMHEARTBEAT_H_
#define SYSTEMHEARTBEAT_H_

#include <chrono>


namespace Component {


template <typename TIoPin, typename TTimer>
class SystemHeartbeat
{
public:

	typedef typename TTimer::IdType IdType;

	/* Constructor */
	SystemHeartbeat(const TIoPin& ledPin, const TTimer& timer);

	/* Destructor */
	~SystemHeartbeat();

	template <typename TRep, typename TPeriod>
	void init(const std::chrono::duration<TRep, TPeriod>& interval) const;

	template <typename TRep, typename TPeriod>
	void setNewInterval(const std::chrono::duration<TRep, TPeriod>& newInterval) const;

private:

	const TIoPin& ledPin_;
	const TTimer& timer_;
	mutable IdType timerID_;

}; //Class Heartbeat


template <typename TIoPin, typename TTimer>
SystemHeartbeat<TIoPin, TTimer>::
SystemHeartbeat(const TIoPin& ledPin, const TTimer& timer) :
	ledPin_(ledPin),
	timer_(timer),
	timerID_(0)
{
	ledPin_.setHigh();
}


template <typename TIoPin, typename TTimer>
SystemHeartbeat<TIoPin, TTimer>::
~SystemHeartbeat()
{

}


template <typename TIoPin, typename TTimer>
template <typename TRep, typename TPeriod>
void SystemHeartbeat<TIoPin, TTimer>::
init(const std::chrono::duration<TRep, TPeriod>& interval) const
{
	timerID_ = timer_.asyncRepeat(interval, [this](){
		ledPin_.toggle();
	});
}


template <typename TIoPin, typename TTimer>
template <typename TRep, typename TPeriod>
void SystemHeartbeat<TIoPin, TTimer>::
setNewInterval(const std::chrono::duration<TRep, TPeriod>& newInterval) const
{
	timer_.abort(timerID_);

	timerID_ = timer_.asyncRepeat(newInterval, [this](){
		ledPin_.toggle();
	});
}


} /* namespace Component */

#endif /* SYSTEMHEARTBEAT_H_ */
