#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_


#include <cstdint>
#include <array>

namespace Util
{

template <typename TData, std::size_t TBufferSize>
class CircularBuffer
{

public:

	// Constructor
	CircularBuffer();

	// Destructor
	~CircularBuffer();

	inline constexpr std::size_t size(void) const { return _buffer.size() - 1; }

	inline bool isEmpty(void) const { return _bufferHead == _bufferTail; }

	inline bool isFull(void) const { return (_bufferHead + 1) % _buffer.size() == _bufferTail; }

	// returns the number of elements of type T currently in the buffer
	inline std::uint16_t available(void) const { return (_buffer.size() + _bufferHead - _bufferTail) % _buffer.size(); }

	// Returns the next object of type T in the buffer or a default constructed if buffer is empty
	// pop() also deletes the returned element in the buffer
	// peek() keeps it in the buffer. Returned object isn't writable!
	TData pop(void);
	TData const& peek(void);

	// Returns the next object in the buffer with write permissions
	TData& mutablePeek(void);

	// Just deletes the next object in the buffer without returning it
	void deleteNext(void);

	// Puts the given object of type TData in the buffer if it isn't full already

	bool push(const TData& newObject);
	bool push(TData&& newObject);

	// clears the buffer
	void clear(void) { _bufferHead = _bufferTail = 0; }


private:

	std::array<TData, TBufferSize+1> _buffer;

	volatile std::size_t _bufferHead; 	// index in the buffer where to add elements

	volatile std::size_t _bufferTail;	// index in the buffer where to read out elements


};


//---------------------------------------------------------------------------------------
// -------------------------------- Implementation --------------------------------------

template <typename TData, std::size_t TBufferSize>
CircularBuffer<TData, TBufferSize>::
CircularBuffer() :
	_bufferHead(0),
	_bufferTail(0)
{

}


template <typename TData, std::size_t TBufferSize>
CircularBuffer<TData, TBufferSize>::
~CircularBuffer()
{

}


template <typename TData, std::size_t TBufferSize>
TData CircularBuffer<TData, TBufferSize>::
pop(void)
{
	// check if buffer is empty
	if (_bufferHead == _bufferTail)
		return TData();

	// read element from the buffer
	TData retVal = _buffer[_bufferTail];

	// increment _bufferTail
	_bufferTail = ((_bufferTail + 1) % _buffer.size());

	// if buffer is empty now, reset index pointer
	if (_bufferHead == _bufferTail) {
		_bufferHead = 0;
		_bufferTail = 0;
	}

	return retVal;
}


template <typename TData, std::size_t TBufferSize>
void CircularBuffer<TData, TBufferSize>::
deleteNext(void)
{
	// check if buffer is empty
	if (_bufferHead == _bufferTail)
		return;

	// increment _bufferTail
	_bufferTail = ((_bufferTail + 1) % _buffer.size());

	// if buffer is empty now, reset index pointer
	if (_bufferHead == _bufferTail) {
		_bufferHead = 0;
		_bufferTail = 0;
	}
}



template <typename TData, std::size_t TBufferSize>
TData const& CircularBuffer<TData, TBufferSize>::
peek(void)
{
	// read element from the buffer
	return _buffer[_bufferTail];
}


template <typename TData, std::size_t TBufferSize>
TData& CircularBuffer<TData, TBufferSize>::
mutablePeek(void)
{
	// read element from the buffer
	return _buffer[_bufferTail];
}


template <typename TData, std::size_t TBufferSize>
bool CircularBuffer<TData, TBufferSize>::
push(const TData& newObject)
{
	// check if buffer is full
	if (((_bufferHead + 1) % _buffer.size()) == _bufferTail)
		return false;

	// add element to the buffer
	_buffer[_bufferHead] = newObject;

	// increment _bufferHead
	_bufferHead = ((_bufferHead + 1) % _buffer.size());

	return true;
}


template <typename TData, std::size_t TBufferSize>
bool CircularBuffer<TData, TBufferSize>::
push(TData&& newObject)
{
	// check if buffer is full
	if (((_bufferHead + 1) % _buffer.size()) == _bufferTail)
		return false;

	// add element to the buffer
	_buffer[_bufferHead] = newObject;

	// increment _bufferHead
	_bufferHead = ((_bufferHead + 1) % _buffer.size());

	return true;
}



} // end namspace Util

#endif /* CIRCULARBUFFER_H_ */
