
#ifndef HARDWARECORE_H_
#define HARDWARECORE_H_

#include <cstdint>
#include <array>
#include <functional>
#include "stm32l476xx.h"


namespace Device
{

enum InterruptId : std::uint8_t {
	I2C1_EVInt,
	I2C1_ERInt,
	Timer2Int,
	DMA1_CH6Int,
	DMA1_CH7Int,
	SPI1_Int,
	DMA1_CH2Int,
	DMA1_CH3Int,
	SPI2_Int,
	DMA1_CH4Int,
	DMA1_CH5Int,
	EncoderTimer_Int, 		//Encoder
	EXTI_Int,

	IntCount
};

enum GpioIntId : std::uint8_t {

	portExpander1Int,
	portExpander2Int,
	encoderInt,
	NoInterrupt	/* Also acts as the count of the valid GPIO Interrupt IDs */
};

enum GpioPin : std::uint8_t {
	PA0,  PB0,  PC0,  PD0,  PE0,  PF0,  PG0,  PH0,
	PA1,  PB1,  PC1,  PD1,  PE1,  PF1,  PG1,  PH1,
	PA2,  PB2,  PC2,  PD2,  PE2,  PF2,  PG2,  PH2,
	PA3,  PB3,  PC3,  PD3,  PE3,  PF3,  PG3,  PH3,
	PA4,  PB4,  PC4,  PD4,  PE4,  PF4,  PG4,  PH4,
	PA5,  PB5,  PC5,  PD5,  PE5,  PF5,  PG5,  PH5,
	PA6,  PB6,  PC6,  PD6,  PE6,  PF6,  PG6,  PH6,
	PA7,  PB7,  PC7,  PD7,  PE7,  PF7,  PG7,  PH7,
	PA8,  PB8,  PC8,  PD8,  PE8,  PF8,  PG8,  PH8,
	PA9,  PB9,  PC9,  PD9,  PE9,  PF9,  PG9,  PH9,
	PA10, PB10, PC10, PD10, PE10, PF10, PG10, PH10,
	PA11, PB11, PC11, PD11, PE11, PF11, PG11, PH11,
	PA12, PB12, PC12, PD12, PE12, PF12, PG12, PH12,
	PA13, PB13, PC13, PD13, PE13, PF13, PG13, PH13,
	PA14, PB14, PC14, PD14, PE14, PF14, PG14, PH14,
	PA15, PB15, PC15, PD15, PE15, PF15, PG15, PH15,

	PinCount
};

enum Edge : std::uint8_t {
	Falling,
	Rising,
	Both
};

/**	Class Interrupt Mananger - Controlls all interrupt related stuff in the project
 * 		- Enables and disables the interrupts in the NVIC
 * 		- Sets priority in the NVIC
 * 		- Call handler methods for the specific devices from the system interrupt service routines (at the end of the file)
 *		- Enable GPIO edge interrupts
 *
 * 	Uses the 'Singleton pattern': With the static member 'theInterruptMgr_' there can be only one instance of this class.
 * 	To use this instance, you call the methods with 'Device::InterruptMgr::reference().'
 */
class InterruptMgr
{
private:

	typedef std::function<void (void)> TInterruptHandler;

	static InterruptMgr const theInterruptMgr_;

	// Constructor (private to prevent more than one instance)
	InterruptMgr();

	// Destructor
	~InterruptMgr() { }

	// Copy Constructor to prevent more instances
	InterruptMgr(const InterruptMgr&);
	InterruptMgr(InterruptMgr&&);

	/* Handler method for the EXTI interrupts */
	void gpioInterruptHandler(void) const;

	mutable std::array<TInterruptHandler, InterruptId::IntCount> handlerArray_;

	struct GpioInterrupt_t {
		std::uint8_t line;
		TInterruptHandler callback;
		bool active;
	};
	mutable std::array<GpioInterrupt_t, GpioIntId::NoInterrupt> gpioHandlerArray_;

public:

	static inline const InterruptMgr& reference(void) { return theInterruptMgr_; }

	template <typename TFunc>
	void addHandlerForInterrupt(InterruptId const id, TFunc&& func) const
	{
		/* Store handler */
		handlerArray_[id] = std::forward<TFunc>(func);

		/* enable interrupt */
		enableInterrupt(id);
	}

	void enableInterrupt(InterruptId const id) const;
	void disableInterrupt(InterruptId const id) const;

	template <typename TFunc>
	void addHandlerForGpioInterrupt(GpioIntId const id, GpioPin const pin, TFunc&& func) const
	{
		/* Store callback in the handler array for GPIO interrupts */

		if (id < GpioIntId::NoInterrupt) {
			gpioHandlerArray_[id].line = (pin/8);
			gpioHandlerArray_[id].callback = std::forward<TFunc>(func);
			gpioHandlerArray_[id].active = true;
		}

	}

	void enableGpioIntForEdge(GpioPin const pin, Edge const edge) const;

	void handleInterrupt(InterruptId const id) const;

	void activateEdgeInterrupt(GpioIntId const id) const;
	void deactivateEdgeInterrupt(GpioIntId const id) const;

};


class Core
{
public:

	static inline void sleep(void) { __WFI(); }

	static inline void disableInterrupts(void) { __disable_irq(); }

	static inline void enableInterrupts(void) { __enable_irq(); }


	static std::uint32_t systemCoreClock(void);

	static void setSystemCoreClock(void);

private:

	// private Ctor to prevent instanciation
	Core();
	Core(const Core&);
	Core(const Core&&);

	~Core();

};


} /* namespace device */

#endif /* HARDWARECORE_H_ */
