
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


#ifdef __cplusplus
}
#endif
