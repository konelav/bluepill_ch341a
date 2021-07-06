#ifndef __USBD_H
#define __USBD_H

#include "stm32f10x_conf.h"
#include "hw_config.h"

#include "usb_lib.h"
#include "usb_mem.h"
#include "usb_conf.h"

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

typedef enum _RESUME_STATE {
    RESUME_EXTERNAL,
    RESUME_INTERNAL,
    RESUME_LATER,
    RESUME_WAIT,
    RESUME_START,
    RESUME_ON,
    RESUME_OFF,
    RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE {
    UNCONNECTED,
    ATTACHED,
    POWERED,
    SUSPENDED,
    ADDRESSED,
    CONFIGURED
} DEVICE_STATE;

extern __IO bool fSuspendEnabled;  /* true when suspend is possible */

void config_usb(FunctionalState CableState);

void usbd_istr(void);

void EP1_IN_Callback(void);
void EP2_IN_Callback(void);
void EP2_OUT_Callback(void);

void SOF_Callback(void);

void usbd_suspend(void);
void usbd_resume_init(void);
void usbd_resume(RESUME_STATE eResumeSetVal);
RESULT usbd_power_on(void);
RESULT usbd_power_off(void);

#endif /* __USBD_H */
