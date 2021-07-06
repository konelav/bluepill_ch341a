#include "usbd.h"

#include "console.h"
#include "ch341a.h"

static uint8_t CH341A_USB_Rx_Buffer[CH341A_DATA_SIZE];


__IO uint16_t wIstr;  /* ISTR register last read value */
__IO uint8_t bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */
__IO uint32_t esof_counter =0; /* expected SOF counter */
__IO uint32_t wCNTR=0;

__IO bool fSuspendEnabled = TRUE;  /* true when suspend is possible */
__IO uint32_t EP[8];

struct {
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
} ResumeS;

__IO uint32_t remotewakeupon=0;


void (*pEpInt_IN[7])(void) = {
    EP1_IN_Callback,
    EP2_IN_Callback,
    EP3_IN_Callback,
    EP4_IN_Callback,
    EP5_IN_Callback,
    EP6_IN_Callback,
    EP7_IN_Callback,
};

void (*pEpInt_OUT[7])(void) = {
    EP1_OUT_Callback,
    EP2_OUT_Callback,
    EP3_OUT_Callback,
    EP4_OUT_Callback,
    EP5_OUT_Callback,
    EP6_OUT_Callback,
    EP7_OUT_Callback,
};

void config_usb(FunctionalState CableState) {
    NVIC_InitTypeDef s;

    INF_VAL("config_usb(CableState = ", CableState, 10, ")");

    NVIC_PriorityGroupConfig(IRQ_PRIO_GROUP_CFG);
    s.NVIC_IRQChannel = USB_IRQ;
    s.NVIC_IRQChannelPreemptionPriority = USB_IRQ_PRIO;
    s.NVIC_IRQChannelSubPriority = 0;
    s.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&s);
}

void usbd_istr(void) {
    static uint32_t sof_counter = 0;

    uint32_t i = 0;
    __IO uint32_t EP[8];

    wIstr = _GetISTR();

    if (wIstr != ISTR_SOF) {
        DBG_VAL("usbd_istr(wIstr = 0x", wIstr, 16, ")");
    }
    else {
        sof_counter++;
        if (sof_counter >= 60000) {
            DBG_VAL("usbd_istr(SOF) x ", sof_counter, 10, "");
            sof_counter = 0;
        }
    }

    #if (IMR_MSK & ISTR_SOF)
    if (wIstr & ISTR_SOF & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_SOF);
        bIntPackSOF++;
        #ifdef SOF_CALLBACK
        SOF_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_CTR)
    if (wIstr & ISTR_CTR & wInterrupt_Mask) {
        /* servicing of the endpoint correct transfer interrupt */
        /* clear of the CTR flag into the sub */
        CTR_LP();
        #ifdef CTR_CALLBACK
        CTR_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_RESET)
    if (wIstr & ISTR_RESET & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_RESET);
        Device_Property.Reset();
        #ifdef RESET_CALLBACK
        RESET_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_DOVR)
    if (wIstr & ISTR_DOVR & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_DOVR);
        #ifdef DOVR_CALLBACK
        DOVR_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_ERR)
    if (wIstr & ISTR_ERR & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_ERR);
        #ifdef ERR_CALLBACK
        ERR_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_WKUP)
    if (wIstr & ISTR_WKUP & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_WKUP);
        usbd_resume(RESUME_EXTERNAL);
        #ifdef WKUP_CALLBACK
        WKUP_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_SUSP)
    if (wIstr & ISTR_SUSP & wInterrupt_Mask) {
        /* check if SUSPEND is possible */
        if (fSuspendEnabled) {
            usbd_suspend();
        }
        else {
        /* if not possible then resume after xx ms */
            usbd_resume(RESUME_LATER);
        }
        /* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
        _SetISTR((uint16_t)CLR_SUSP);
        #ifdef SUSP_CALLBACK
        SUSP_Callback();
        #endif
    }
    #endif

    #if (IMR_MSK & ISTR_ESOF)
    if (wIstr & ISTR_ESOF & wInterrupt_Mask) {
        /* clear ESOF flag in ISTR */
        _SetISTR((uint16_t)CLR_ESOF);
        if ((_GetFNR()&FNR_RXDP)!=0) {
            /* increment ESOF counter */
            esof_counter++;
            /* test if we enter in ESOF more than 3 times with FSUSP =0 and RXDP =1=>> possible missing SUSP flag*/
            if ((esof_counter > 3 ) && ((_GetCNTR() & CNTR_FSUSP) == 0)) {
                /* this a sequence to apply a force RESET*/
                /*Store CNTR value */
                wCNTR = _GetCNTR();
                /*Store endpoints registers status */
                for (i = 0; i < 8; i++)
                    EP[i] = _GetENDPOINT(i);
                /*apply FRES */
                wCNTR |= CNTR_FRES;
                _SetCNTR(wCNTR);
                /*clear FRES*/
                wCNTR &= ~CNTR_FRES;
                _SetCNTR(wCNTR);
                /*poll for RESET flag in ISTR*/
                while ((_GetISTR()&ISTR_RESET) == 0)
                    ;
                /* clear RESET flag in ISTR */
                _SetISTR((uint16_t)CLR_RESET);
                /*restore Enpoints*/
                for (i = 0; i < 8; i++)
                    _SetENDPOINT(i, EP[i]);
                esof_counter = 0;
            }
        }
        else {
            esof_counter = 0;
        }
        /* resume handling timing is made with ESOFs */
        usbd_resume(RESUME_ESOF); /* request without change of the machine state */
        #ifdef ESOF_CALLBACK
        ESOF_Callback();
        #endif
    }
    #endif
}

RESULT usbd_power_on(void) {
    uint16_t wRegVal;

    config_usb(ENABLE);

    /*** CNTR_PWDN = 0 ***/
    wRegVal = CNTR_FRES;
    _SetCNTR(wRegVal);

    /* The following sequence is recommended:
        1- FRES = 0
        2- Wait until RESET flag = 1 (polling)
        3- clear ISTR register */

    /*** CNTR_FRES = 0 ***/
    wInterrupt_Mask = 0;

    _SetCNTR(wInterrupt_Mask);

    /* Wait until RESET flag = 1 (polling) */
    /*while ((_GetISTR() & ISTR_RESET) != 0)
        ;*/

    /*** Clear pending interrupts ***/
    SetISTR(0);

    /*** Set interrupt mask ***/
    wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
    _SetCNTR(wInterrupt_Mask);

    return USB_SUCCESS;
}

RESULT usbd_power_off() {
    /* disable all interrupts and force USB reset */
    _SetCNTR(CNTR_FRES);
    /* clear interrupt status register */
    _SetISTR(0);
    config_usb(DISABLE);
    /* switch-off device */
    _SetCNTR(CNTR_FRES + CNTR_PDWN);
    /* sw variables reset */
    return USB_SUCCESS;
}

void usbd_suspend(void) {
    uint32_t i = 0;
    uint16_t wCNTR;

    /* suspend preparation */

    /*Store CNTR value */
    wCNTR = _GetCNTR();

    /* This a sequence to apply a force RESET to handle a robustness case */

    /*Store endpoints registers status */
    for (i = 0; i < 8; i++)
        EP[i] = _GetENDPOINT(i);

    /* unmask RESET flag */
    wCNTR |= CNTR_RESETM;
    _SetCNTR(wCNTR);

    /*apply FRES */
    wCNTR |= CNTR_FRES;
    _SetCNTR(wCNTR);

    /*clear FRES*/
    wCNTR &= ~CNTR_FRES;
    _SetCNTR(wCNTR);

    /*poll for RESET flag in ISTR*/
    while ((_GetISTR() & ISTR_RESET) == 0)
        ;

    /* clear RESET flag in ISTR */
    _SetISTR((uint16_t)CLR_RESET);

    /*restore Enpoints*/
    for (i = 0; i < 8; i++)
        _SetENDPOINT(i, EP[i]);

    /* Now it is safe to enter macrocell in suspend mode */
    wCNTR |= CNTR_FSUSP;
    _SetCNTR(wCNTR);

    /* force low-power mode in the macrocell */
    wCNTR = _GetCNTR();
    wCNTR |= CNTR_LPMODE;
    _SetCNTR(wCNTR);
}

void usbd_resume_init(void) {
    uint16_t wCNTR;
    /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
    /* restart the clocks */
    /* CNTR_LPMODE = 0 */
    wCNTR = _GetCNTR();
    wCNTR &= (~CNTR_LPMODE);
    _SetCNTR(wCNTR);

    /* reset FSUSP bit */
    _SetCNTR(IMR_MSK);
    /* reverse suspend preparation */
}

void usbd_resume(RESUME_STATE eResumeSetVal) {
    uint16_t wCNTR;

    if (eResumeSetVal != RESUME_ESOF)
        ResumeS.eState = eResumeSetVal;
    switch (ResumeS.eState) {
    case RESUME_EXTERNAL:
        if (remotewakeupon == 0) {
            usbd_resume_init();
            ResumeS.eState = RESUME_OFF;
        }
        else { /* RESUME detected during the RemoteWAkeup signalling => keep RemoteWakeup handling*/
            ResumeS.eState = RESUME_ON;
        }
        break;
    case RESUME_INTERNAL:
        usbd_resume_init();
        ResumeS.eState = RESUME_START;
        remotewakeupon = 1;
        break;
    case RESUME_LATER:
        ResumeS.bESOFcnt = 2;
        ResumeS.eState = RESUME_WAIT;
        break;
    case RESUME_WAIT:
        ResumeS.bESOFcnt--;
        if (ResumeS.bESOFcnt == 0)
            ResumeS.eState = RESUME_START;
        break;
    case RESUME_START:
        wCNTR = _GetCNTR();
        wCNTR |= CNTR_RESUME;
        _SetCNTR(wCNTR);
        ResumeS.eState = RESUME_ON;
        ResumeS.bESOFcnt = 10;
        break;
    case RESUME_ON:
        ResumeS.bESOFcnt--;
        if (ResumeS.bESOFcnt == 0) {
            wCNTR = _GetCNTR();
            wCNTR &= (~CNTR_RESUME);
            _SetCNTR(wCNTR);
            ResumeS.eState = RESUME_OFF;
            remotewakeupon = 0;
        }
        break;
    case RESUME_OFF:
    case RESUME_ESOF:
    default:
        ResumeS.eState = RESUME_OFF;
        break;
    }
}

void EP1_IN_Callback (void) {
    ch341a_on_ep1_in();
}

void EP2_IN_Callback (void) {
    ch341a_on_packet_transmitted();
}

void EP2_OUT_Callback(void) {
    uint16_t USB_Rx_Cnt;
    USB_Rx_Cnt = USB_SIL_Read(EP2_OUT, CH341A_USB_Rx_Buffer);
    ch341a_on_packet_received(CH341A_USB_Rx_Buffer, USB_Rx_Cnt);
}

void SOF_Callback(void) {
    static uint32_t sof_counter = 0;
    
    if (sof_counter++ == CH341A_FRAME_INTERVAL) {
        sof_counter = 0;
        ch341a_on_frame_interval();
    }
}
