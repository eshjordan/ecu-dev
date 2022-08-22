
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "fff.h"

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(Error_Handler);
FAKE_VOID_FUNC(vApplicationGetTimerTaskMemory, StaticTask_t **, StackType_t **,
	       uint32_t *);
FAKE_VOID_FUNC(vNetworkInterfaceAllocateRAMToBuffers,
	       NetworkBufferDescriptor_t *);

FAKE_VOID_FUNC(vApplicationTickHook);

FAKE_VOID_FUNC(vApplicationIPNetworkEventHook, eIPCallbackEvent_t);

FAKE_VALUE_FUNC(BaseType_t, xApplicationDNSQueryHook, const char *);

FAKE_VALUE_FUNC(uint32_t, ulApplicationGetNextSequenceNumber, uint32_t, uint16_t, uint32_t, uint16_t);

FAKE_VOID_FUNC(vApplicationGetIdleTaskMemory, StaticTask_t **, StackType_t **, uint32_t *);

FAKE_VOID_FUNC(vApplicationIdleHook);

FAKE_VOID_FUNC(vApplicationMallocFailedHook);

FAKE_VALUE_FUNC(BaseType_t, xApplicationGetRandomNumber, uint32_t *);

FAKE_VALUE_FUNC(const char *, pcApplicationHostnameHook);

FAKE_VOID_FUNC(vAssertCalled, const char *, unsigned long);

FAKE_VOID_FUNC(vApplicationDaemonTaskStartupHook);

extern const uint8_t ucMACAddress[6] = {0};

/* The definition of the timers themselves. */
typedef struct tmrTimerControl /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
	const char				*pcTimerName;		/*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
	ListItem_t				xTimerListItem;		/*<< Standard linked list item as used by all kernel features for event management. */
	TickType_t				xTimerPeriodInTicks;/*<< How quickly and often the timer expires. */
	void 					*pvTimerID;			/*<< An ID to identify the timer.  This allows the timer to be identified when the same callback is used for multiple timers. */
	TimerCallbackFunction_t	pxCallbackFunction;	/*<< The function that will be called when the timer expires. */
	#if( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t			uxTimerNumber;		/*<< An ID assigned by trace tools such as FreeRTOS+Trace */
	#endif
	uint8_t 				ucStatus;			/*<< Holds bits to say if the timer was statically allocated or not, and if it is active or not. */
} xTIMER;

/* The old xTIMER name is maintained above then typedefed to the new Timer_t
name below to enable the use of older kernel aware debuggers. */
typedef xTIMER Timer_t;


#ifdef __cplusplus
}
#endif
