/*	----------------------------------------------------------------------------
    FILE:			interrupt.c
    PROJECT:		pinguino
    PURPOSE:		interrupt routines
    PROGRAMER:		regis blanchot <rblanchot@gmail.com>
    ----------------------------------------------------------------------------
    CHANGELOG :
    24 Déc. 2010 - Régis Blanchot - first release
    23 Nov. 2012 - Régis Blanchot - added __18f1220,1320,14k22,2455,4455,46j50 support
    24 Jan. 2013 - Régis Blanchot - added IntEnable and IntDisable functions
    25 Jan. 2013 - Régis Blanchot - added partial support to RTCC interrupt
    18 Apr. 2014 - Régis Blanchot - fixed OnTimer1 and 3 bug for x550 family
    20 Apr. 2014 - Régis Blanchot - added partial PIC18Fx7J53 support
    03 Feb. 2016 - Régis Blanchot - added partial PIC16F1459 support
    ----------------------------------------------------------------------------
    TODO :
    * INT3
    * INTEDG0, INTEDG1, INTEDG2, INTEDG3
    * BCL1, BCL2
    * CM1, CM2
    * RX1, RX2
    * TX1, TX2
    * SSP1, SSP2
    * CTMU
    * RTCC
    ----------------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    --------------------------------------------------------------------------*/

#ifndef __INTERRUPT_C
    #define __INTERRUPT_C

    #ifndef ON_EVENT
        #define ON_EVENT
    #endif

    #if defined(__PIC32MX__)
        #error "Error : this library is intended to be used only with 8-bit Pinguino" 
    #endif
    
    #include <compiler.h>
    #include <typedef.h>
    #include <interrupt.h>
    #include <macro.h>
    #include <oscillator.c>

    typedef void (*callback) (void);				// type of: void callback()

    static callback intFunction[INT_NUM];
    u8  intUsed[INT_NUM];

    // OnTimerX only
    #if defined(TMR0INT) || defined(TMR1INT) || \
        defined(TMR2INT) || defined(TMR3INT) || \
        defined(TMR4INT) || defined(TMR5INT) || \
        defined(TMR6INT) || defined(TMR8INT) 

    volatile u16 intCount[8];
    volatile u16 intCountLimit[8];
    volatile u8 preloadH[8];
    volatile u8 preloadL[8];

    #endif
    
/*	----------------------------------------------------------------------------
    ---------- Attach / Detach Interrupt
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Enable / Disable the interrupt
    @param		inter:		interrupt number
    --------------------------------------------------------------------------*/

#if defined(INTENABLE) || defined(INTDISABLE)
void IntSetEnable(u8 inter, u8 enable)
{
    if (enable == INT_DISABLE)
        intUsed[inter] = INT_NOT_USED;	// This interrupt is no longer used
    else
        intUsed[inter] = INT_USED;	    // This interrupt is used
        
    switch(inter)
    {
        #if defined(INT0INT)
        case INT_INT0:
            #ifdef __16F1459
            INTCONbits.INTE = enable;
            #else
            INTCONbits.INT0IE = enable;
            #endif
            break;
        #endif
        
        #if defined(INT1INT)
        case INT_INT1:
            INTCON3bits.INT1IE = enable;
            break;
        #endif

        #if defined(INT2INT)
        case INT_INT2:
            INTCON3bits.INT2IE = enable;
            break;
        #endif

        #if defined(TMR0INT)
        case INT_TMR0:
            #ifndef __16F1459
            INTCON2bits.TMR0IP = INT_LOW_PRIORITY;
            #endif
            INTCONbits.TMR0IE = enable;
            break;
        #endif

        #if defined(TMR1INT)
        case INT_TMR1:
            #ifndef __16F1459
            IPR1bits.TMR1IP = INT_LOW_PRIORITY;
            #endif
            PIE1bits.TMR1IE = enable;
            break;
        #endif

        #if defined(TMR2INT)
        case INT_TMR2:
            #ifndef __16F1459
            IPR1bits.TMR2IP = INT_LOW_PRIORITY;
            #endif
            PIE1bits.TMR2IE = enable;
            break;
        #endif

        #if defined(TMR3INT)
        #ifndef __16F1459
        case INT_TMR3:
            IPR2bits.TMR3IP = INT_LOW_PRIORITY;
            PIE2bits.TMR3IE = enable;
            break;
        #endif
        #endif

            
        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR4INT)
            case INT_TMR4:
                IPR3bits.TMR4IP = INT_LOW_PRIORITY;
                PIE3bits.TMR4IE = enable;
                break;
            #endif
        #endif

        
        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR5INT)
            case INT_TMR5:
                IPR5bits.TMR5IP = INT_LOW_PRIORITY;
                PIE5bits.TMR5IE = enable;
                break;
            #endif
        #endif
        
        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR6INT)
            case INT_TMR6:
                IPR5bits.TMR6IP = INT_LOW_PRIORITY;
                PIE5bits.TMR6IE = enable;
                break;
            #endif
        #endif
        
        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR8INT)
            case INT_TMR8:
                IPR5bits.TMR8IP = INT_LOW_PRIORITY;
                PIE5bits.TMR8IE = enable;
                break;
            #endif
        #endif

        #if defined(RBINT)
            case INT_RB:
            #if defined(__18f25k50) || defined(__18f45k50)
                INTCONbits.IOCIE = enable;
            #else
                INTCONbits.RBIE = enable;
            #endif
                break;
        #endif
            
        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(USBINT)
                case INT_USB:
                #if defined(__18f25k50) || defined(__18f45k50)
                    PIE3bits.USBIE = enable;
                #else
                    PIE2bits.USBIE = enable;
                #endif
                    break;
            #endif
        #endif
        
        #if defined(ADINT)
        case INT_AD:
            PIE1bits.ADIE = enable;
            break;
        #endif

        #if defined(RCINT)
        case INT_RC:
            PIE1bits.RCIE = enable;
            break;
        #endif

        #if defined(TXINT)
        case INT_TX:
            PIE1bits.TXIE = enable;
            break;
        #endif

        #if defined(CCP1INT)
        case INT_CCP1:
            PIE1bits.CCP1IE = enable;
            break;
        #endif
            
        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(CCP2INT)
            case INT_CCP2:
                PIE2bits.CCP2IE = enable;
                break;
            #endif
        #endif

        #if defined(OSCFINT)
        case INT_OSCF:
            PIE2bits.OSCFIE = enable;
            break;
        #endif

        #if defined(CMINT)
            case INT_CM:
            #if defined(__18f2455) || defined(__18f4455) || \
                defined(__18f2550) || defined(__18f4550) || \
                defined(__18f25k50) || defined(__18f45k50)
                PIE2bits.CMIE = enable;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                PIE2bits.CM1IE = enable;
            #endif
                break;
        #endif

            
        #if defined(__18f2455) || defined(__18f4455) || \
            defined(__18f2550) || defined(__18f4550) || \
            defined(__18f25k50) || defined(__18f45k50)
            #if defined(EEINT)
            case INT_EE:
                PIE2bits.EEIE = enable;
                break;
            #endif
        #endif
        
        #if defined(BCLINT)
            case INT_BCL:
                #if defined(__18f2455) || defined(__18f4455) || \
                    defined(__18f2550) || defined(__18f4550) || \
                    defined(__18f25k50) || defined(__18f45k50)
                PIE2bits.BCLIE = enable;
                #elif defined(__18f26j50) || defined(__18f46j50) || \
                      defined(__18f26j53) || defined(__18f46j53) || \
                      defined(__18f27j53) || defined(__18f47j53)
                PIE2bits.BCL1IE = enable;
                #endif
                break;
        #endif
            
        #if defined(HLVDINT)
            case INT_HLVD:
                #if defined(__18f2455) || defined(__18f4455) || \
                    defined(__18f2550) || defined(__18f4550) || \
                    defined(__18f25k50) || defined(__18f45k50)
                PIE2bits.HLVDIE = enable;
                #elif defined(__18f26j50) || defined(__18f46j50) || \
                      defined(__18f26j53) || defined(__18f46j53) || \
                      defined(__18f27j53) || defined(__18f47j53)
                PIE2bits.LVDIE = enable;
                #endif
                break;
        #endif
            
        #if defined(SSPINT)
            #if defined(__18f4550)
            case INT_SSP:
                PIE1bits.SSPIE = enable;
                break;
            #endif
        #endif
        
        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(RTCCINT)
            case INT_RTCC:
                PIE3bits.RTCCIE = enable;
                break;
            #endif
        #endif

        #if defined(__18f46j53) || defined(__18f47j53)
            #if defined(PMPINT)
            case INT_PMP:
                IPR1bits.PMPIP = INT_LOW_PRIORITY;
                PIE1bits.PMPIE = enable;
                break;
            #endif
        #endif
    }
}
#endif /* defined(INTENABLE) || defined(INTDISABLE) */

#if defined(INTDISABLE)
void IntDisable(u8 inter)
{
    IntSetEnable(inter, INT_DISABLE);
}
#endif /* defined(INTDISABLE) */

#if defined(INTENABLE)
void IntEnable(u8 inter)
{
    IntSetEnable(inter, INT_ENABLE);
    
    /*
    INTCONbits.GIEH = 1;   // Enable global HP interrupts
    INTCONbits.GIEL = 1;   // Enable global LP interrupts
    */
}
#endif /* defined(INTENABLE) */

#if 0
#if defined(INTSTART)
void IntStart()
{
    interrupts();
}
#endif /* defined(INTSTART) */

#if defined(INTSTOP)
void IntStop()
{
    noInterrupts();
}
#endif /* defined(INTSTOP) */
#endif

/*	----------------------------------------------------------------------------
    ---------- IntClearFlag
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Clear interrupt flag to allow interrupt again
    @param		inter:		interrupt number
    --------------------------------------------------------------------------*/

#if defined(INTCLEARFLAG)
void IntClearFlag(u8 inter)
{
    switch(inter)
    {
        #if defined(INT0INT)
        case INT_INT0: INTCONbits.INT0IF = 0; break;
        #endif

        #if defined(INT1INT)
        case INT_INT1: INTCON3bits.INT1IF = 0; break;
        #endif

        #if defined(INT2INT)
        case INT_INT2: INTCON3bits.INT2IF = 0; break;
        #endif

        #if defined(TMR0INT)
        case INT_TMR0: INTCONbits.TMR0IF = 0; break;
        #endif

        #if defined(TMR1INT)
        case INT_TMR1: PIR1bits.TMR1IF = 0; break;
        #endif

        #if defined(TMR2INT)
        case INT_TMR2: PIR1bits.TMR2IF = 0; break;
        #endif

        #if defined(TMR3INT)
        case INT_TMR3: PIR2bits.TMR3IF = 0; break;
        #endif

        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR4INT)
            case INT_TMR4: PIR3bits.TMR4IF = 0; break;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR5INT)
            case INT_TMR5: PIR5bits.TMR5IF = 0; break;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR6INT)
            case INT_TMR6: PIR5bits.TMR6IF = 0; break;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR8INT)
            case INT_TMR8: PIR5bits.TMR8IF = 0; break;
            #endif
        #endif

        #if defined(RBINT)
            case INT_RB:
            #if defined(__18f25k50) || defined(__18f45k50)
                INTCONbits.IOCIF = 0;
            #else
                INTCONbits.RBIF = 0;
            #endif
                break;
        #endif
        
        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(USBINT)
                case INT_USB:
                #if defined(__18f25k50) || defined(__18f45k50)
                    PIR3bits.USBIF = 0;
                #else
                    PIR2bits.USBIF = 0;
                #endif
                    break;
            #endif
        #endif

        #if defined(ADINT)
        case INT_AD: PIR1bits.ADIF = 0; break;
        #endif

        #if defined(RCINT)
        case INT_RC: PIR1bits.RCIF = 0; break;
        #endif

        #if defined(TXINT)
        case INT_TX: PIR1bits.TXIF = 0; break;
        #endif

        #if defined(CCP1INT)
        case INT_CCP1: PIR1bits.CCP1IF = 0; break;
        #endif

        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(CCP2INT)
            case INT_CCP2: PIR2bits.CCP2IF = 0; break;
            #endif
        #endif

        #if defined(OSCFINT)
        case INT_OSCF: PIR2bits.OSCFIF = 0; break;
        #endif

        #if defined(CMINT)
            case INT_CM:
            #if defined(__18f2455) || defined(__18f4455) || \
                defined(__18f2550) || defined(__18f4550) || \
                defined(__18f25k50) || defined(__18f45k50)
                PIR2bits.CMIF = 0;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                PIR2bits.CM1IF = 0;
            #endif
                break;
        #endif
        
        #if defined(__18f2455) || defined(__18f4455) || \
            defined(__18f2550) || defined(__18f4550) || \
            defined(__18f25k50) || defined(__18f45k50)
            #if defined(EEINT)
            case INT_EE: PIR2bits.EEIF = 0; break;
            #endif
        #endif
        
        #if defined(BCLINT)
            case INT_BCL:
            #if defined(__18f2455) || defined(__18f4455) || \
                defined(__18f2550) || defined(__18f4550) || \
                defined(__18f25k50) || defined(__18f45k50)
                PIR2bits.BCLIF = 0;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                PIR2bits.BCL1IF = 0;
            #endif
                break;
        #endif

        #if defined(HLVDINT)
            case INT_HLVD:
            #if defined(__18f2455) || defined(__18f4455) || \
                defined(__18f2550) || defined(__18f4550) || \
                defined(__18f25k50) || defined(__18f45k50)
                PIR2bits.HLVDIF = 0;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                PIR2bits.LVDIF = 0;
            #endif
                break;
        #endif
        
        #if defined(SSPINT)
            #if defined(__18f4550)
            case INT_SSP: PIR1bits.SSPIF = 0; break;
            #endif
        #endif
        
        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(RTCCINT)
            case INT_RTCC: PIR3bits.RTCCIF = 0; break;
            #endif
        #endif

        #if defined(__18f46j53) || defined(__18f47j53)
            #if defined(PMPINT)
            case INT_PMP: PIR1bits.PMPIF = 0; break;
            #endif
        #endif
    }
}
#endif /* defined(INTCLEARFLAG) */

/*	----------------------------------------------------------------------------
    ---------- IntIsFlagSet
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Read if interrupt flag is set
    @param		inter:		interrupt number
    @return     1 if set, 0 if not set
    --------------------------------------------------------------------------*/

#if defined(INTISFLAGSET)
u8 IntIsFlagSet(u8 inter)
{
    switch(inter)
    {
        #if defined(INT0INT)
        case INT_INT0:  return INTCONbits.INT0IF;
        #endif
        
        #if defined(INT1INT)
        case INT_INT1:	return INTCON3bits.INT1IF;
        #endif
        
        #if defined(INT2INT)
        case INT_INT2:	return INTCON3bits.INT2IF;
        #endif
        
        #if defined(TMR0INT)
        case INT_TMR0:	return INTCONbits.TMR0IF;
        #endif
        
        #if defined(TMR1INT)
        case INT_TMR1:	return PIR1bits.TMR1IF;
        #endif
        
        #if defined(TMR2INT)
        case INT_TMR2:	return PIR1bits.TMR2IF;
        #endif
        
        #if defined(TMR3INT)
        case INT_TMR3:	return PIR2bits.TMR3IF;
        #endif

        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR4INT)
            case INT_TMR4:	return PIR3bits.TMR4IF;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR5INT)
            case INT_TMR5:	return PIR5bits.TMR5IF;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR6INT)
            case INT_TMR6:	return PIR5bits.TMR6IF;
            #endif
        #endif

        #if defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(TMR8INT)
            case INT_TMR8:	return PIR5bits.TMR8IF;
            #endif
        #endif

        #if defined(RBINT)
        case INT_RB:
            #if defined(__18f25k50) || defined(__18f45k50)
                return INTCONbits.IOCIF;
            #else
                return INTCONbits.RBIF;
            #endif
        #endif

        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(USBINT)
            case INT_USB:
                #if defined(__18f25k50) || defined(__18f45k50)
                    return  PIR3bits.USBIF;
                #else
                    return  PIR2bits.USBIF;
                #endif
            #endif
        #else
            #error "No USB on this chip"
        #endif
        
        #if defined(ADINT)
        case INT_AD:	return PIR1bits.ADIF;
        #endif

        #if defined(RCINT)
        case INT_RC:	return PIR1bits.RCIF;
        #endif

        #if defined(TXINT)
        case INT_TX:	return PIR1bits.TXIF;
        #endif

        #if defined(CCP1INT)
        case INT_CCP1:	return PIR1bits.CCP1IF;
        #endif

        #if !defined(__18f1220) && !defined(__18f1320) && \
            !defined(__18f14k22) && !defined(__18lf14k22)
            #if defined(CCP2INT)
            case INT_CCP2:	return PIR2bits.CCP2IF;
            #endif
        #endif

        #if defined(OSCFINT)
        case INT_OSCF:	return PIR2bits.OSCFIF;
        #endif

        #if defined(CMINT)
            case INT_CM:
            #if   defined(__18f2455) || defined(__18f4455) || \
                  defined(__18f2550) || defined(__18f4550) || \
                  defined(__18f25k50) || defined(__18f45k50)
                return PIR2bits.CMIF;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                return PIR2bits.CM1IF;
            #endif
        #endif

        #if   defined(__18f2455) || defined(__18f4455) || \
              defined(__18f2550) || defined(__18f4550) || \
              defined(__18f25k50) || defined(__18f45k50)
            #if defined(EEINT)
            case INT_EE:	return PIR2bits.EEIF;
            #endif
        #endif
       
        #if defined(BCLINT)
            case INT_BCL:
            #if   defined(__18f2455) || defined(__18f4455) || \
                  defined(__18f2550) || defined(__18f4550) || \
                  defined(__18f25k50) || defined(__18f45k50)
                return PIR2bits.BCLIF;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                return PIR2bits.BCL1IF;
            #endif
        #endif

        #if defined(HLVDINT)
            case INT_HLVD:
            #if   defined(__18f2455) || defined(__18f4455) || \
                  defined(__18f2550) || defined(__18f4550) || \
                  defined(__18f25k50) || defined(__18f45k50)
                return PIR2bits.HLVDIF;
            #elif defined(__18f26j50) || defined(__18f46j50) || \
                  defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
                return PIR2bits.LVDIF;
            #endif
        #endif

        #if defined(__18f4455) || defined(__18f4550)
            #if defined(SSPINT)
            case INT_SSP:	return PIR1bits.SSPIF;
            #endif
        #endif

        #if defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            #if defined(RTCCINT)
            case INT_RTCC:	return PIR3bits.RTCCIF;
            #endif
        #endif

        #if defined(__18f46j53) || defined(__18f47j53)
            #if defined(PMPINT)
            case INT_PMP: return PIR1bits.PMPIF;
            #endif
        #endif

    }
    return 0;
}
#endif /* defined(INTISFLAGSET) */

/*	--------------------------------------------------------------------
    ---------- IntSetPriority
    --------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		set interrupt priority
    @param	    inter : interrupt num.
                pri : INT_LOW_PRIORITY or INT_LOW_PRIORITY
    ------------------------------------------------------------------*/

#ifdef INTSETPRIORITY
void IntSetPriority(u8 inter, u8 pri)
{
    #ifdef INT1INT
    INTCON3bits.INT1IP = pri;
    #endif
    
    #ifdef INT2INT
    INTCON3bits.INT2IP = pri;
    #endif
    
    #ifdef RBINT
    INTCONbits.RBIP = pri;
    #endif
    
    #if defined (TMR0INT) || defined(CNTR0INT)
    INTCON2bits.TMR0IP = pri;
    #endif
    
    #if defined (TMR1INT) || defined(CNTR1INT)
    PIR1bits.TMR1IP = pri;
    #endif
    
    #ifdef TMR2INT
    PIR1bits.TMR2IP = pri;
    #endif
    
    #if defined (TMR3INT) || defined(CNTR3INT)
    PIR2bits.TMR3IP = pri;
    #endif
    
    #ifdef TMR4INT
    PIR3bits.TMR4IP = pri;
    #endif
    
    #ifdef TMR5INT
    PIR5bits.TMR5IP = pri;
    #endif
    
    #ifdef TMR6INT
    PIR5bits.TMR6IP = pri;
    #endif
    
    #ifdef TMR8INT
    PIR5bits.TMR8IP = pri;
    #endif
    
    #ifdef ADINT
    PIR1bits.ADIP = pri;
    #endif
    
    #ifdef RCINT
    PIR1bits.RCIP = pri;
    #endif
    
    #ifdef TXINT
    PIR1bits.TXIP = pri;
    #endif
    
    #ifdef CCP1INT
    PIR1bits.CCP1IP = pri;
    #endif
    
    #ifdef CCP2INT
    PIR2bits.CCP2IP = pri;
    #endif
    
    #ifdef EEINT
    PIR2bits.EEIP = pri;
    #endif
    
    #ifdef USBINT
    PIR2bits.USBIP = pri;
    #endif
    
    #ifdef OSCFINT
    PIR2bits.OSCFIP = pri;
    #endif
    
    #ifdef CMINT
    PIR2bits.CMIP = pri;
    #endif
    
    #ifdef BCLINT
    PIR2bits.BCLIP = pri;
    #endif
    
    #ifdef HLVDINT
    PIR2bits.HLVDIP = pri;
    #endif
    
    #ifdef SSPINT
    PIR1bits.SSPIP = pri;
    #endif

    #if defined(__18f46j53) || defined(__18f47j53)
    #if defined(PMPINT)
    PIR1bits.PMPIP = pri;
    #endif
    #endif
}
#endif /* INTSETPRIORITY */

/*	----------------------------------------------------------------------------
    ---------- int_init
    ----------------------------------------------------------------------------
    @author		Régis Blanchot <rblanchot@gmail.com>
    @descr		Disable all the interrupt
    --------------------------------------------------------------------------*/
/*
void IntInit()
{
    u8 i;

    //RB : Should have been already defined in main.c
    //RCONbits.IPEN = 1;					// Enable HP/LP interrupts

    INTCONbits.GIEH = 0;                // Disable global HP interrupts
    INTCONbits.GIEL = 0;                // Disable global LP interrupts

    for (i = 0; i < INT_NUM; i++)		// Disable all interrupts
        IntSetEnable(i, INT_DISABLE);

    INTCONbits.GIEH = 1;				// Enable HP interrupts
    INTCONbits.GIEL = 1;				// Enable LP interrupts
}
*/
/*	----------------------------------------------------------------------------
    ---------- int_start
    ----------------------------------------------------------------------------
    @author		Régis Blanchot <rblanchot@gmail.com>
    @descr		Start interrupt
    @param      none
    --------------------------------------------------------------------------*/

void IntTimerStart()
{
    #ifdef TMR0INT
    #ifdef __16F1459
        OPTION_REGbits.TMR0CS = 0;  // Timer mode
    #else
        T0CONbits.TMR0ON = 1;
    #endif
    #endif

    #ifdef TMR1INT
        T1CONbits.TMR1ON = 1;
    #endif

    #ifdef TMR2INT
        T2CONbits.TMR2ON = 1;
    #endif

    #ifndef __16F1459
    #ifdef TMR3INT
        T3CONbits.TMR3ON = 1;
    #endif
    #endif

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #ifdef TMR4INT
            T4CONbits.TMR4ON = 1;
        #endif
    #endif

    #if defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #ifdef TMR5INT
            T5CONbits.TMR5ON = 1;
        #endif

        #ifdef TMR6INT
            T6CONbits.TMR6ON = 1;
        #endif

        #ifdef TMR8INT
            T8CONbits.TMR8ON = 1;
        #endif
    #endif
}

/*	----------------------------------------------------------------------------
    ---------- int_stop
    ----------------------------------------------------------------------------
    @author		Régis Blanchot <rblanchot@gmail.com>
    @descr		Stop all timers together
    --------------------------------------------------------------------------*/

void IntTimerStop()
{
    #ifdef TMR0INT
    #ifdef __16F1459
        OPTION_REGbits.TMR0CS = 1;  // Counter mode
    #else
        T0CONbits.TMR0ON = 0;
    #endif
    #endif

    #ifdef TMR1INT
        T1CONbits.TMR1ON = 0;
    #endif

    #ifdef TMR2INT
        T2CONbits.TMR2ON = 0;
    #endif

    #ifndef __16F1459
    #ifdef TMR3INT
        T3CONbits.TMR3ON = 0;
    #endif
    #endif

    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #ifdef TMR4INT
            T4CONbits.TMR4ON = 0;
        #endif
    #endif

    #if defined(__18f26j53) || defined(__18f46j53) || \
        defined(__18f27j53) || defined(__18f47j53)
        #ifdef TMR5INT
            T5CONbits.TMR5ON = 0;
        #endif

        #ifdef TMR6INT
            T6CONbits.TMR6ON = 0;
        #endif

        #ifdef TMR8INT
            T8CONbits.TMR8ON = 0;
        #endif
    #endif
}

/*	----------------------------------------------------------------------------
    ---------- OnTimer0
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR0INT
u8 OnTimer0(callback func, u8 timediv, u16 delay)
{
    u8 _t0con = 0;
    u16 _cycles_;
    
    if (intUsed[INT_TMR0] == INT_NOT_USED)
    {
        intUsed[INT_TMR0] = INT_USED;
        intCount[INT_TMR0] = 0;
        intCountLimit[INT_TMR0] = delay;
        intFunction[INT_TMR0] = func;

        switch(timediv)
        {
            case INT_MICROSEC:
                // 1 us = 1.000 ns = 12 cy @ Fosc = 48 MHz
                // 12
                _cycles_ = System_getPeripheralFrequency() / 1000 / 1000;
                #ifdef __16F1459
                _t0con = T0_SOURCE_INT | T0_PS_OFF;
                preloadL[INT_TMR0] =  0xFF - (u8)_cycles_; // 12
                #else
                _t0con = T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
                preloadH[INT_TMR0] = 0xFF;
                preloadL[INT_TMR0] = 0xFF - (u8)_cycles_;
                #endif
                break;
            case INT_MILLISEC:
                // 1 ms = 1.000.000 ns = 12.000 cy @ Fosc = 48 MHz
                // 12.000
                _cycles_ = System_getPeripheralFrequency() / 1000;
                #ifdef __16F1459
                _t0con = T0_SOURCE_INT | T0_PS_ON | T0_PS_1_64;
                preloadL[INT_TMR0] =  0xFF - (_cycles_/64); // 12000/64=187
                #else
                _t0con = T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
                preloadH[INT_TMR0] = high8(0xFFFF - _cycles_);
                preloadL[INT_TMR0] =  low8(0xFFFF - _cycles_);
                #endif
                break;
            case INT_SEC:
                // 1 sec = 1.000.000.000 ns = 12.000.000 cy @ Fosc = 48 MHz
                // 12.000.000 / 256 = 46875
                _cycles_ = System_getPeripheralFrequency() >> 8;
                #ifdef __16F1459
                _t0con = T0_SOURCE_INT | T0_PS_ON | T0_PS_1_256;
                intCountLimit[INT_TMR0] = 256 * delay;
                preloadL[INT_TMR0] =  0xFF - (_cycles_/256); // 46875/256 = 183
                #else
                _t0con = T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_ON | T0_PS_1_256;
                preloadH[INT_TMR0] = high8(0xFFFF - _cycles_);
                preloadL[INT_TMR0] =  low8(0xFFFF - _cycles_);
                #endif
                break;
        }

        #ifdef __16F1459
            OPTION_REG = _t0con;
            TMR0 = preloadL[INT_TMR0];
        #else
            T0CON = _t0con;
            INTCON2bits.TMR0IP = INT_LOW_PRIORITY;
            TMR0H = preloadH[INT_TMR0];
            TMR0L = preloadL[INT_TMR0];
        #endif

        INTCONbits.TMR0IF = 0;
        INTCONbits.TMR0IE = INT_ENABLE;
        return INT_TMR0;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER0 is already used !");
    #endif
        return INT_USED;
    }
}
#endif

/*	----------------------------------------------------------------------------
    ---------- OnTimer1
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR1INT
u8 OnTimer1(callback func, u32 timediv, u16 delay)
{
    u8  _presca_;
    u16 _cycles_;
    u32 fosc = System_getPeripheralFrequency();

    if (intUsed[INT_TMR1] == INT_NOT_USED)
    {
        intUsed[INT_TMR1] = INT_USED;
        intCount[INT_TMR1] = 0;
        intFunction[INT_TMR1] = func;

        // Tcy = 1/F s            = 1E09 / F ns
        // 1us = 1.000 ns         = 1E03 / (1E09 / F) = F/1E06 cy
        // 1ms = 1.000.000 ns     = 1E06 / (1E09 / F) = F/1E03 cy
        // 1s  = 1.000.000.000 ns = 1E09 / (1E09 / F) = F cy

        if ( timediv == INT_SEC )
        {
            _presca_ = T1_PS_1_8;
            intCountLimit[INT_TMR1] = delay * 25;
            _cycles_ = fosc / 8 / 25;
        }
        
        else // INT_MICROSEC or INT_MILLISEC
        {
            _presca_ = T1_PS_1_1;
            intCountLimit[INT_TMR1] = delay;
            _cycles_ = fosc / timediv;
        }
        
        _cycles_ = 0xFFFF - _cycles_;
        preloadH[INT_TMR1] = high8(_cycles_);
        preloadL[INT_TMR1] = low8(_cycles_);

        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            
        T1GCONbits.TMR1GE = 0;				/* First Ignore T1DIG effection */ 
        
        #endif

        TMR1H = preloadH[INT_TMR1];
        TMR1L = preloadL[INT_TMR1];
        #ifdef __16F1459
        T1CON = T1_ON | T1_SYNC_EXT_ON | _presca_ | T1_SOURCE_FOSCDIV4;
        #else
        T1CON = T1_ON | T1_16BIT | T1_SYNC_EXT_ON | _presca_ | T1_SOURCE_FOSCDIV4;
        IPR1bits.TMR1IP = INT_LOW_PRIORITY;
        #endif
        PIR1bits.TMR1IF = 0;
        PIE1bits.TMR1IE = INT_ENABLE;
        
        return INT_TMR1;
    }
    else
    {
        #ifdef DEBUG
        debug("Error : interrupt TIMER1 is already used !");
        #endif
        return INT_USED;
    }
}

void OnRTCC(callback func, u16 delay)
{
    #if defined(__18f2455) || defined(__18f4455) || \
        defined(__18f2550) || defined(__18f4550) || \
        defined(__18f25k50) || defined(__18f45k50)

        u8 _t1con = 0;

        if (intUsed[INT_TMR1] == INT_NOT_USED)
        {
            intUsed[INT_TMR1] = INT_USED;
            intCount[INT_TMR1] = 0;
            intCountLimit[INT_TMR1] = delay;
            intFunction[INT_TMR1] = func;

            // Preload TMR1 register pair for 1 mn (60 s = 60 x 1000 x 4 x 250 us) overflow
            preloadH[INT_TMR1] = 0x80;			// (0xFFFF - 0xFF0F) = 240
            preloadL[INT_TMR1] = 0x00;			// TMR1L is never preloaded or altered

            // RD16    = 1	Enables register read/write of Timer1 in one 16-bit operation
            // T1RUN   = 0	Device clock is derived from another source (le quartz 20MHz)
            // T1CKPS1 = 0	00 = 1:1 Prescale value
            // T1CKPS0 = 0
            // T1OSCEN = 1	Timer1 oscillator is on
            // T1SYNC  = 0	When TMR1CS = 1, Synchronize external clock input
            // TMR1CS  = 1	External clock from RC0/T1OSO/T13CKI pin (on the rising edge)
            // TMR1ON  = 0	Stops Timer1
            #if defined(__18f25k50) || defined(__18f45k50)
            _t1con = T1_ON | T1_16BIT | T1_SYNC_EXT_ON | T1_SOSC_ON | T1_PS_1_1 | T1_SOURCE_FOSC;
            #else
            _t1con = T1_ON | T1_16BIT | T1_SYNC_EXT_ON  | T1_OSC_ON | T1_PS_1_1 | T1_RUN_FROM_ANOTHER | T1_SOURCE_EXT;
          //_t1con = T1_ON | T1_16BIT | T1_SYNC_EXT_ON | T1_OSC_ON | T1_PS_1_8 | T1_SOURCE_FOSC;
            #endif
            // 18F26J50 -> _t1con = T1_ON | T1_16BIT | T1_PS_1_1 | T1_OSC_ON | T1_SYNC_EXT_ON | T1_SOURCE_EXT;

            IPR1bits.TMR1IP = INT_LOW_PRIORITY;
            PIE1bits.TMR1IE = INT_ENABLE;
            PIR1bits.TMR1IF = 0;
            TMR1H = preloadH[INT_TMR1];
            TMR1L = preloadL[INT_TMR1];
            T1CON = _t1con;
        }
        #ifdef DEBUG
        else
        {
            debug("Error : interrupt TIMER1 is already used !");
        }
        #endif

    #elif defined(__18f26j50) || defined(__18f46j50) || \
          defined(__18f26j53) || defined(__18f46j53) || \
          defined(__18f27j53) || defined(__18f47j53)

        // TODO

    #endif
}

#endif /* TMR1INT */

/*	----------------------------------------------------------------------------
    ---------- OnTimer2
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR2INT
u8 OnTimer2(callback func, u8 timediv, u16 delay)
{
    u8 _t2con = 0;
    u8 _pr2 = 0;
    u32 fosc = System_getPeripheralFrequency();
    
    if (intUsed[INT_TMR2] == INT_NOT_USED)
    {
        intUsed[INT_TMR2] = INT_USED;
        intCount[INT_TMR2] = 0;
        intCountLimit[INT_TMR2] = delay;
        intFunction[INT_TMR2] = func;

        // time = Tcy*postscaler*pr2*prescaler
        switch(timediv)
        {
            case INT_MICROSEC:
                // 1us = 12 cy
                _pr2 = fosc / 1000 / 1000;
                _t2con = T2_ON | T2_POST_1_1 | T2_PS_1_1;
                break;
            case INT_MILLISEC:
                // 1ms = 12.000 cy
                // 12.000 / 15 / 16 = 50
                _pr2 = fosc / 1000 / 240;
                _t2con = T2_ON | T2_POST_1_15 | T2_PS_1_16;
                break;
            case INT_SEC:
                #if defined(__16F1459)
                // 1sec = 12.000.000 cy
                // 12.000.000 / 15 / 64 = 12.500 = 250 * 50
                _pr2 = fosc / 250 / 50;
                intCountLimit[INT_TMR2] = delay * 50;
                _t2con = T2_ON | T2_POST_1_15 | T2_PS_1_64;
                #else
                // 1sec = 12.000.000 cy
                // 12.000.000 / 15 / 16 = 50.000 = 200 * 250
                _pr2 = fosc / 240 / 200;
                intCountLimit[INT_TMR2] = delay * 200;
                _t2con = T2_ON | T2_POST_1_15 | T2_PS_1_16;
                #endif
                break;
        }

        T2CON = _t2con;
        PR2 = _pr2;	// Timer2 Match value
        #ifndef __16F1459
        IPR1bits.TMR2IP = INT_LOW_PRIORITY;
        #endif
        PIR1bits.TMR2IF = 0;
        PIE1bits.TMR2IE = INT_ENABLE;
        return INT_TMR2;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER2 is already used !");
    #endif
        return INT_USED;
    }
}
#endif

/*	----------------------------------------------------------------------------
    ---------- OnTimer3
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR3INT
#ifndef __16F1459 
/* Note: This function needs T1OSC as its clock source */
u8 OnTimer3(callback func, u32 timediv, u16 delay)
{
    u8  _presca_;
    u16 _cycles_;
    u32 osc = System_getPeripheralFrequency();

    if (intUsed[INT_TMR3] == INT_NOT_USED)
    {
        intUsed[INT_TMR3] = INT_USED;
        intCount[INT_TMR3] = 0;
        intFunction[INT_TMR3] = func;

        // Tcy = 1/F s            = 1E09 / F ns
        // 1us = 1.000 ns         = 1E03 / (1E09 / F) = F/1E06 cy
        // 1ms = 1.000.000 ns     = 1E06 / (1E09 / F) = F/1E03 cy
        // 1s  = 1.000.000.000 ns = 1E09 / (1E09 / F) = F cy

        if ( timediv == INT_SEC )
        {
            _presca_ = T3_PS_1_8;
            intCountLimit[INT_TMR3] = delay * 25;
            _cycles_ = osc / 8 / 25;
        }
        
        else // INT_MICROSEC or INT_MILLISEC
        {
            _presca_ = T1_PS_1_1;
            intCountLimit[INT_TMR3] = delay;
            _cycles_ = osc / timediv;
        }
        
        preloadH[INT_TMR3] = high8(0xFFFF - _cycles_);
        preloadL[INT_TMR3] =  low8(0xFFFF - _cycles_);

        #if defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j50) || defined(__18f46j50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            
        T3GCONbits.TMR3GE = 0;				/* First Ignore T1DIG effection */ 
        
        #endif

        T3CON = T3_ON | T3_16BIT | T3_SYNC_EXT_ON | _presca_ | T3_SOURCE_FOSCDIV4;
        IPR2bits.TMR3IP = INT_LOW_PRIORITY;
        TMR3H = preloadH[INT_TMR3];
        TMR3L = preloadL[INT_TMR3];
        PIR2bits.TMR3IF = 0;
        PIE2bits.TMR3IE = INT_ENABLE;

        return INT_TMR3;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER3 is already used !");
    #endif
        return INT_USED;
    }
}
#else

    #error "Your processor don't have any Timer3."
    
#endif
#endif

/*	----------------------------------------------------------------------------
    ---------- OnTimer4
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR4INT

#if defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

u8 OnTimer4(callback func, u8 timediv, u16 delay)
{
    u8 _t4con = 0;
    u8 _pr4 = 0;

    if (intUsed[INT_TMR4] == INT_NOT_USED)
    {
        intUsed[INT_TMR4] = INT_USED;
        intCount[INT_TMR4] = 0;
        intCountLimit[INT_TMR4] = delay;
        intFunction[INT_TMR4] = func;

        // time = Tcy*postscaler*pr2*prescaler
        switch(timediv)
        {
            case INT_MICROSEC:
                // 1us = 12 cy
                _pr4 = System_getPeripheralFrequency() / 1000 / 1000;
                _t4con = T4_ON | T4_PS_1_1 | T4_POST_1_1;
                break;
            case INT_MILLISEC:
                // 1ms = 12.000 cy
                // 12.000 / 15 / 16 = 50
                _pr4 = System_getPeripheralFrequency() / 1000 / 240;
                _t4con = T4_ON | T4_POST_1_15 | T4_PS_1_16;
                break;
            case INT_SEC:
                // 1sec = 12.000.000 cy
                // 12.000.000 / 15 / 16 = 50.000 = 200 * 25
                _pr4 = System_getPeripheralFrequency() / 240 / 200;
                intCountLimit[INT_TMR4] = delay * 200;
                _t4con = T4_ON | T4_POST_1_15 | T4_PS_1_16;
                break;
        }

        T4CON = _t4con;
        IPR3bits.TMR4IP = INT_LOW_PRIORITY;
        PR4 = _pr4;	// Timer2 Match value
        PIR3bits.TMR4IF = 0;
        PIE3bits.TMR4IE = INT_ENABLE;
        return INT_TMR4;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER4 is already used !");
    #endif /* DEBUG */
        return INT_USED;
    }
}
    #else
        #error "Your processor don't have any Timer4."
    #endif /* defined(__18f26j50) || defined(__18f46j50) ... */
#endif /* TMR4INT */

/*	----------------------------------------------------------------------------
    ---------- OnTimer5
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR5INT

#if defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

u8 OnTimer5(callback func, u32 timediv, u16 delay)
{
    u8  _presca_;
    u16 _cycles_;
    u32 osc = System_getPeripheralFrequency();

    if (intUsed[INT_TMR5] == INT_NOT_USED)
    {
        intUsed[INT_TMR5] = INT_USED;
        intCount[INT_TMR5] = 0;
        intFunction[INT_TMR5] = func;

        // Tcy = 1/F s            = 1E09 / F ns
        // 1us = 1.000 ns         = 1E03 / (1E09 / F) = F/1E06 cy
        // 1ms = 1.000.000 ns     = 1E06 / (1E09 / F) = F/1E03 cy
        // 1s  = 1.000.000.000 ns = 1E09 / (1E09 / F) = F cy

        if ( timediv == INT_SEC )
        {
            _presca_ = T5_PS_1_8;
            intCountLimit[INT_TMR5] = delay * 25;
            _cycles_ = osc / 8 / 25;
        }
        
        else // INT_MICROSEC or INT_MILLISEC
        {
            _presca_ = T1_PS_1_1;
            intCountLimit[INT_TMR3] = delay;
            _cycles_ = osc / timediv;
        }
        
        preloadH[INT_TMR5] = high8(0xFFFF - _cycles_);
        preloadL[INT_TMR5] =  low8(0xFFFF - _cycles_);

        T5GCONbits.TMR5GE = 0;				/* First Ignore T1DIG effection */ 

        T5CON = T5_ON | T5_16BIT | T5_SYNC_EXT_ON | T5_SOURCE_T10 | _presca_ | T5_SOURCE_FOSCDIV4;
        IPR5bits.TMR5IP = INT_LOW_PRIORITY;
        TMR5H = preloadH[INT_TMR5];
        TMR5L = preloadL[INT_TMR5];
        PIR5bits.TMR5IF = 0;
        PIE5bits.TMR5IE = INT_ENABLE;
        return INT_TMR5;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER5 is already used !");
    #endif
        return INT_USED;
    }
}
    #else
        #error "Your processor don't have any Timer5."
    #endif /* defined(__18f27j53) || defined(__18f47j53) ... */
#endif /* TMR5INT */

/*	----------------------------------------------------------------------------
    ---------- OnTimer6
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR6INT

#if defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

u8 OnTimer6(callback func, u8 timediv, u16 delay)
{
    u8 _t6con = 0;
    u8 _pr6 = 0;

    if (intUsed[INT_TMR6] == INT_NOT_USED)
    {
        intUsed[INT_TMR6] = INT_USED;
        intCount[INT_TMR6] = 0;
        intCountLimit[INT_TMR6] = delay;
        intFunction[INT_TMR6] = func;

        // time = Tcy*postscaler*pr2*prescaler
        switch(timediv)
        {
            case INT_MICROSEC:
                // 1us = 12 cy
                _pr6 = System_getPeripheralFrequency() / 1000 / 1000;
                _t6con = T6_ON | T6_PS_1_1 | T6_POST_1_1;
                break;
            case INT_MILLISEC:
                // 1ms = 12.000 cy
                // 12.000 / 15 / 16 = 50
                _pr6 = System_getPeripheralFrequency() / 1000 / 240;
                _t6con = T6_ON | T6_POST_1_15 | T6_PS_1_16;
                break;
            case INT_SEC:
                // 1sec = 12.000.000 cy
                // 12.000.000 / 15 / 16 = 50.000 = 200 * 25
                _pr6 = System_getPeripheralFrequency() / 240 / 200;
                intCountLimit[INT_TMR6] = delay * 200;
                _t6con = T6_ON | T6_POST_1_15 | T6_PS_1_16;
                break;
        }

        T6CON = _t6con;
        IPR5bits.TMR6IP = INT_LOW_PRIORITY;
        PR6 = _pr6;	// Timer6 Match value
        PIR5bits.TMR6IF = 0;
        PIE5bits.TMR6IE = INT_ENABLE;
        return INT_TMR6;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER6 is already used !");
    #endif /* DEBUG */
        return INT_USED;
    }
}
    #else
        #error "Your processor don't have any Timer6."
    #endif /* defined(__18f27j53) || defined(__18f47j53) ... */
#endif /* TMR6INT */

/*	----------------------------------------------------------------------------
    ---------- OnTimer8
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure timers to execute function func every delay ms, us or sec
    @param		timediv:	INT_MICROSEC, INT_MILLISEC, INT_SEC
                func:		function called when interrupt occures
                delay:		delay before overload
    --------------------------------------------------------------------------*/

#ifdef TMR8INT

#if defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

u8 OnTimer8(callback func, u8 timediv, u16 delay)
{
    u8 _t8con = 0;
    u8 _pr8 = 0;

    if (intUsed[INT_TMR8] == INT_NOT_USED)
    {
        intUsed[INT_TMR8] = INT_USED;
        intCount[INT_TMR8] = 0;
        intCountLimit[INT_TMR8] = delay;
        intFunction[INT_TMR8] = func;

        // time = Tcy*postscaler*pr2*prescaler
        switch(timediv)
        {
            case INT_MICROSEC:
                // 1us = 12 cy
                _pr8 = System_getPeripheralFrequency() / 1000 / 1000;
                _t8con = T8_ON | T8_PS_1_1 | T8_POST_1_1;
                break;
            case INT_MILLISEC:
                // 1ms = 12.000 cy
                // 12.000 / 15 / 16 = 50
                _pr8 = System_getPeripheralFrequency() / 1000 / 240;
                _t8con = T8_ON | T8_POST_1_15 | T8_PS_1_16;
                break;
            case INT_SEC:
                // 1sec = 12.000.000 cy
                // 12.000.000 / 15 / 16 = 50.000 = 200 * 25
                _pr8 = System_getPeripheralFrequency() / 240 / 200;
                intCountLimit[INT_TMR8] = delay * 200;
                _t8con = T8_ON | T8_POST_1_15 | T8_PS_1_16;
                break;
        }

        T8CON = _t8con;
        IPR5bits.TMR8IP = INT_LOW_PRIORITY;
        PR8 = _pr8;	// Timer8 Match value
        PIR5bits.TMR8IF = 0;
        PIE5bits.TMR8IE = INT_ENABLE;
        return INT_TMR8;
    }
    else
    {
    #ifdef DEBUG
        debug("Error : interrupt TIMER8 is already used !");
    #endif /* DEBUG */
        return INT_USED;
    }
}

    #else

        #error "Your processor don't have any Timer8."

    #endif /* defined(__18f27j53) || defined(__18f47j53) ... */

#endif /* TMR8INT */

/*	----------------------------------------------------------------------------
    ---------- OnCounter
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure counters
    @param		inter:		interrupt number (INT_TMR0, INT_TMR1, or INT_TMR3)
                func:		function called when interrupt comes
    --------------------------------------------------------------------------*/

#ifdef CNTR0INT
// Not available on Pinguino with 2 buttons (RA4 = Run Led)
void OnCounter0(callback func, u8 config)
{
    if (intUsed[INT_TMR0] == INT_NOT_USED)
    {
        intUsed[INT_TMR0] = INT_USED;
        intFunction[INT_TMR0] = func;
        intCount[INT_TMR0] = 0;
        intCountLimit[INT_TMR0] = 0;
        INTCON2bits.TMR0IP = INT_LOW_PRIORITY;
        INTCONbits.TMR0IE = INT_ENABLE;
        preloadH[INT_TMR0] = 0;
        preloadL[INT_TMR0] = 0;
        TMR0H = 0;
        TMR0L = 0;
        T0CON = T0_ON | T0_16BIT | T0_SOURCE_EXT | T0_L2H | T0_PS_ON | T0_PS_1_2;
        T0CON |= config;
        INTCONbits.TMR0IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt TMR0 is already used !");
    }
    #endif
}
#endif

#ifdef CNTR1INT
void OnCounter1(callback func, u8 config)
{
    if (intUsed[INT_TMR1] == INT_NOT_USED)
    {
        intUsed[INT_TMR1] = INT_USED;
        intFunction[INT_TMR1] = func;
        intCount[INT_TMR1] = 0;
        intCountLimit[INT_TMR1] = 0;
        IPR1bits.TMR1IP = INT_LOW_PRIORITY;
        PIE1bits.TMR1IE = INT_ENABLE;
        preloadH[INT_TMR1] = 0;
        preloadL[INT_TMR1] = 0;
        TMR1H = 0;
        TMR1L = 0;
        T1CON = T1_ON | T1_16BIT | T1_PS_1_8 | T1_RUN_FROM_ANOTHER | T1_OSC_ON | T1_SYNC_EXT_ON | T1_SOURCE_EXT;
        T1CON |= config;
        PIR1bits.TMR1IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt TMR1 is already used !");
    }
    #endif
}
#endif

#ifdef CNTR3INT
void OnCounter3(callback func, u8 config)
{
    if (intUsed[INT_TMR3] == INT_NOT_USED)
    {
        intUsed[INT_TMR3] = INT_USED;
        intFunction[INT_TMR3] = func;
        intCount[INT_TMR3] = 0;
        intCountLimit[INT_TMR3] = 0;
        IPR2bits.TMR3IP = INT_LOW_PRIORITY;
        PIE2bits.TMR3IE = INT_ENABLE;
        preloadH[INT_TMR3] = 0;
        preloadL[INT_TMR3] = 0;
        TMR3H = 0;
        TMR3L = 0;
        T3CON = T3_ON | T3_16BIT | T3_PS_1_8 | T3_SOURCE_EXT; // default
        T3CON |= config;
        PIR2bits.TMR3IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt TMR3 is already used !");
    }
    #endif
}
#endif

/*	----------------------------------------------------------------------------
    ---------- OnChange
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure interrupt-on-change
    @param		inter:		interrupt number
                config:		INT_RISING_EDGE or INT_FALLING_EDGE
                func:		function called when interrupt comes
    --------------------------------------------------------------------------*/

#ifdef INT0INT
void OnChangePin0(callback func, u8 config)
{
    if (intUsed[INT_INT0] == INT_NOT_USED)
    {
        intUsed[INT_INT0] = INT_USED;
        intFunction[INT_INT0] = func;

        #ifdef __16F1459 // pin 0 = RA5

        if (config == INT_RISING_EDGE)
            IOCAPbits.IOCAP5 = 1;                   // INT_RISING_EDGE
        else
            IOCANbits.IOCAN5 = 1;                   // INT_FALLING_EDGE

        INTCONbits.IOCIE = INT_ENABLE;
        INTCONbits.IOCIF = 0;

        #else // pin 0 = RB0

        INTCON2bits.INTEDG0 = config;
        INTCON2bits.RBPU = 0;                       // PORTB pull-ups are enabled
        TRISBbits.TRISB0 = INPUT;
        //INTCON3bits.INT0IP = INT_LOW_PRIORITY;    // INT0 has always HIGH PRIORITY
        INTCONbits.INT0IE = INT_ENABLE;
        INTCONbits.INT0IF = 0;

        #endif
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt INT0 is already used !");
    }
    #endif
}
#endif

#ifdef INT1INT
void OnChangePin1(callback func, u8 config)
{
    if (intUsed[INT_INT1] == INT_NOT_USED)
    {
        intUsed[INT_INT1] = INT_USED;
        intFunction[INT_INT1] = func;
        INTCON2bits.INTEDG1 = config;
        INTCON2bits.RBPU = 0;						// PORTB pull-ups are enabled
        TRISBbits.TRISB1 = INPUT;
        INTCON3bits.INT1IP = INT_LOW_PRIORITY;
        INTCON3bits.INT1IE = INT_ENABLE;
        INTCON3bits.INT1IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt INT1 is already used !");
    }
    #endif
}
#endif

#ifdef INT2INT
void OnChangePin2(callback func, u8 config)
{
    if (intUsed[INT_INT2] == INT_NOT_USED)
    {
        intUsed[INT_INT2] = INT_USED;
        intFunction[INT_INT2] = func;
        INTCON2bits.INTEDG2 = config;
        INTCON2bits.RBPU = 0;						// PORTB pull-ups are enabled
        TRISBbits.TRISB2 = INPUT;
        INTCON3bits.INT2IP = INT_LOW_PRIORITY;
        INTCON3bits.INT2IE = INT_ENABLE;
        INTCON3bits.INT2IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt INT2 is already used !");
    }
    #endif
}
#endif

#ifdef INT3INT
void OnChangePin3(callback func, u8 config)
{
    if (intUsed[INT_INT3] == INT_NOT_USED)
    {
        intUsed[INT_INT3] = INT_USED;
        intFunction[INT_INT3] = func;
        INTCON2bits.INTEDG3 = config;
        INTCON2bits.RBPU = 0;						// PORTB pull-ups are enabled
        TRISBbits.TRISB3 = INPUT;
        INTCON3bits.INT3IP = INT_LOW_PRIORITY;
        INTCON3bits.INT3IE = INT_ENABLE;
        INTCON3bits.INT3IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt INT2 is already used !");
    }
    #endif
}
#endif

#ifdef RBINT
/*
    PORTB<7:4> Interrupt-on-Change
    Only pins configured as inputs can cause this interrupt to occur.
    Any RB7:RB4 pin configured as an output is excluded from the
    interrupt-on-change comparison. The pins are compared with the old
    value latched on the last read of PORTB.
    * pin:          INTRBx (can be ORed)
    * func:         function to call when interrupt occurs
    * ex. usage:    OnChangePin4to7(myInterrupt, INTRB4|INTRB7);
                    void myInterrupt()
                    {
                        if (INTRB & INTRB4)
                            do some stuff here
                        if (INTRB & INTRB7)
                            do some stuff here
                    }
*/

#define INTRB4      (1<<4)   // 16
#define INTRB5      (1<<5)   // 32
#define INTRB6      (1<<6)   // 64
#define INTRB7      (1<<7)   // 128
#define INTRBALL    0xF0

volatile u8 INTRB;

void OnChangePin4to7(callback func, u8 pin)
{
    if (intUsed[INT_RB] == INT_NOT_USED)
    {
        intUsed[INT_RB] = INT_USED;
        intFunction[INT_RB] = func;
        pin &= 0xF0;    // to prevent lower pins to change
        TRISB &= 0x0F;  // clears bit 7 to 4
        TRISB |= pin;   // pin as an INPUT
        INTCON2bits.RBIP = INT_LOW_PRIORITY;
        INTCONbits.RBIE  = INT_ENABLE;
        INTCONbits.RBIF  = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt RB is already used !");
    }
    #endif
}

/*
void OnChangePin4to7(callback func, u8 pin, u8 config)
{
    if (intUsed[INT_RB] == INT_NOT_USED)
    {
        intUsed[INT_RB] = INT_USED;
        intFunction[INT_RB] = func;
        INTCON2bits.INTEDG2 = config;
        TRISB |= (1 << pin);	// pin as an INPUT
        INTCON2bits.RBIP = INT_LOW_PRIORITY;
        INTCONbits.RBIE = INT_ENABLE;
        INTCONbits.RBIF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt RB is already used !");
    }
    #endif
}
*/

#endif

/*	----------------------------------------------------------------------------
    ---------- OnCompare
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure compare interrupt mode
    @param		inter:		interrupt number
                func:		function called when interrupt occured
    --------------------------------------------------------------------------*/

#ifdef CCP1INT
void OnCompare1(callback func, u8 config)
{
    if (intUsed[INT_CCP1] == INT_NOT_USED)
    {
        intUsed[INT_CCP1] = INT_USED;
        intFunction[INT_CCP1] = func;
        IPR1bits.CCP1IP = INT_LOW_PRIORITY;
        PIE1bits.CCP1IE = INT_ENABLE;
        CCP1CON &= config;
        PIR1bits.CCP1IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt CCP1 is already used !");
    }
    #endif
}
#endif

#ifdef CCP2INT
void OnCompare2(callback func, u8 config)
{
    if (intUsed[INT_CCP2] == INT_NOT_USED)
    {
        intUsed[INT_CCP2] = INT_USED;
        intFunction[INT_CCP2] = func;
        IPR2bits.CCP2IP = INT_LOW_PRIORITY;
        PIE2bits.CCP2IE = INT_ENABLE;
        CCP2CON &= config;
        PIR2bits.CCP2IF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt CCP2 is already used !");
    }
    #endif
}
#endif

#ifdef CMINT
void OnCompareAll(callback func, u8 config)
{
    if (intUsed[INT_CM] == INT_NOT_USED)
    {
        intUsed[INT_CM] = INT_USED;
        intFunction[INT_CM] = func;
        IPR2bits.CMIP = INT_LOW_PRIORITY;
        PIE2bits.CMIE = INT_ENABLE;
        PIR2bits.CMIF = 0;
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt CM is already used !");
    }
    #endif
}
#endif

/*	----------------------------------------------------------------------------
    ---------- OnEvent
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		Configure interrupt event
    @param		inter:		interrupt number
                func:		function called when interrupt occured
    --------------------------------------------------------------------------*/

#if defined(RCINT)   || defined(TXINT)  || defined(ADINT)   || \
    defined(OSCFINT) || defined(EEINT)  || defined(HLVDINT) || \
    defined(BCLINT)  || defined(USBINT) || defined(SSPINT)  || \
    defined(PMPINT)

void OnEvent(u8 inter, callback func)
{
    if (intUsed[inter] == INT_NOT_USED)
    {
        intUsed[inter] = INT_USED;
        intFunction[inter] = func;

        switch(inter)
        {
            #if defined(RCINT)
            case INT_RC:					// serial rx
                PIR1bits.RCIF = 0;
                break;
            #endif

            #if defined(TXINT)
            case INT_TX:					// serial tx
                PIR1bits.TXIF = 0;
                break;
            #endif

            #if defined(ADINT)
            case INT_AD:					// ad conversion complete
                PIR1bits.ADIF = 0;
                break;
            #endif

            #if defined(OSFINT)
            case INT_OSCF:					// oscillator failed
                PIR2bits.OSCFIF = 0;
                break;
            #endif

            #if defined(EEINT)
            case INT_EE:					// eeprom write operation
                PIR2bits.EEIF = 0;
                break;
            #endif

            #if defined(BCLINT)
            case INT_HLVD:					// high/low voltage detect
                PIR2bits.HLVDIF = 0;
                break;
            #endif

            #if defined(BCLINT)
            case INT_BCL:					// bus collision
                PIR2bits.BCLIF = 0;
                break;
            #endif

            #if defined(USBINT)
            case INT_USB:					// usb
            #if defined(__18f25k50) || defined(__18f45k50)
                PIR3bits.USBIF = 0;
            #else
                PIR2bits.USBIF = 0;
            #endif
                break;
            #endif

            #if defined(SSPINT)
            case INT_SSP:                   //
                PIR1bits.SSPIF = 0;
                break;
            #endif

            #if defined(__18f46j53) || defined(__18f47j53)
            #if defined(PMPINT)
            case INT_PMP:                   // parallel master port
                PIR1bits.PMPIF = 0;
                break;
            #endif
            #endif
        }
    }
    #ifdef DEBUG
    else
    {
        debug("Error : interrupt #%u is already used !", inter);
    }
    #endif
}
#endif

#ifdef RCINT
void OnSerialRX(callback func)	{	OnEvent(INT_RC, func);	}
#endif

#ifdef TXINT
void OnSerialTX(callback func)	{	OnEvent(INT_TX, func);	}
#endif

#ifdef ADINT
void OnADC(callback func)		{	OnEvent(INT_AD, func);	}
#endif

#ifdef OSCFINT
void OnOscFailed(callback func)	{	OnEvent(INT_OSCF, func);}
#endif

#ifdef EEINT
void OnEeprom(callback func)	{	OnEvent(INT_EE, func);	}
#endif

#ifdef HLVDINT
void OnLowVoltage(callback func){	OnEvent(INT_HLVD, func);}
#endif

#ifdef BCLINT
void OnBusCol(callback func)	{	OnEvent(INT_BCL, func);	}
#endif

#ifdef USBINT
void OnUSB(callback func)		{	OnEvent(INT_USB, func);	}
#endif

#ifdef SSPINT
//void OnParallel(callback func)	    {	OnEvent(INT_SSP, func);	}
void OnI2CRequest(callback func)	{	OnEvent(INT_SSP, func);	}
void OnI2CReceive(callback func)	{	OnEvent(INT_SSP, func);	}
#endif

#if defined(__18f46j53) || defined(__18f47j53)
#if defined(PMPINT)
void OnParallel(callback func)	    {	OnEvent(INT_PMP, func);	}
#endif
#endif
/*	----------------------------------------------------------------------------
    ---------- userhighinterrupt
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		function called by high_priority_isr
    @param		none
    --------------------------------------------------------------------------*/

void userhighinterrupt()
{
    #ifdef INT0INT
        #ifdef _16F1459
        if (INTCONbits.IOCIE && INTCONbits.IOCIF)
        {
            INTCONbits.IOCIF = 0;
            intFunction[INT_INT0]();
        }
        #else
        if (INTCONbits.INT0IE && INTCONbits.INT0IF)
        {
            INTCONbits.INT0IF = 0;
            intFunction[INT_INT0]();
        }
        #endif
    #endif
}

/*	----------------------------------------------------------------------------
    ---------- userlowinterrupt
    ----------------------------------------------------------------------------
    @author		Regis Blanchot <rblanchot@gmail.com>
    @descr		function called by low_priority_isr
    @param		none
    --------------------------------------------------------------------------*/

void userlowinterrupt()
{
    #ifdef INT1INT
    if (INTCON3bits.INT1IE && INTCON3bits.INT1IF)
    {
        INTCON3bits.INT1IF = 0;
        intFunction[INT_INT1]();
    }
    #endif
    
    #ifdef INT2INT
    if (INTCON3bits.INT2IE && INTCON3bits.INT2IF)
    {
        INTCON3bits.INT2IF = 0;
        intFunction[INT_INT2]();
    }
    #endif
    
    #ifdef RBINT
    u8 LastPortB;
    
    if (INTCONbits.RBIE && INTCONbits.RBIF)
    {
        INTRB = (PORTB & 0xF0) ^ LastPortB;
        LastPortB = (PORTB & 0xF0);
        intFunction[INT_RB]();
        INTCONbits.RBIF = 0;
    }
    #endif
    
    #if defined (TMR0INT) || defined(CNTR0INT)
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF)
    {
        //T0CONbits.TMR0ON = 0;
        #ifdef _16F1459
        TMR0 = preloadL[INT_TMR0];
        #else
        TMR0H = preloadH[INT_TMR0];
        TMR0L = preloadL[INT_TMR0];
        #endif
        INTCONbits.TMR0IF = 0;
        if (intCount[INT_TMR0]++ >= intCountLimit[INT_TMR0])
        {
            intCount[INT_TMR0] = 0;
            intFunction[INT_TMR0]();
        }
    }
    #endif
    
    #if defined (TMR1INT) || defined(CNTR1INT)
    if (PIE1bits.TMR1IE && PIR1bits.TMR1IF)
    {
        //T1CONbits.TMR1ON = 0;
        TMR1H = preloadH[INT_TMR1];
        TMR1L = preloadL[INT_TMR1];
        PIR1bits.TMR1IF = 0;
        if (intCount[INT_TMR1]++ >= intCountLimit[INT_TMR1])
        {
            intCount[INT_TMR1] = 0;
            intFunction[INT_TMR1]();
        }
    }
    #endif
    
    #ifdef TMR2INT
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF)
    {
        //T2CONbits.TMR2ON = 0;
        PIR1bits.TMR2IF = 0;
        if (intCount[INT_TMR2]++ >= intCountLimit[INT_TMR2])
        {
            intCount[INT_TMR2] = 0;
            intFunction[INT_TMR2]();
        }
        // NB : no need to reload PR2
    }
    #endif
    
    #if defined (TMR3INT) || defined(CNTR3INT)
    if (PIE2bits.TMR3IE && PIR2bits.TMR3IF)
    {
        //T3CONbits.TMR3ON = 0;
        TMR3H = preloadH[INT_TMR3];
        TMR3L = preloadL[INT_TMR3];
        PIR2bits.TMR3IF = 0;
        if (intCount[INT_TMR3]++ >= intCountLimit[INT_TMR3])
        {
            intCount[INT_TMR3] = 0;
            intFunction[INT_TMR3]();
        }
    }
    #endif
    
    #ifdef TMR4INT
    if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
    {
        //T4CONbits.TMR4ON = 0;
        PIR3bits.TMR4IF = 0;
        if (intCount[INT_TMR4]++ >= intCountLimit[INT_TMR4])
        {
            intCount[INT_TMR4] = 0;
            intFunction[INT_TMR4]();
        }
        // NB : no need to reload PR4
    }
    #endif
    
    #ifdef TMR5INT
    if (PIE5bits.TMR5IE && PIR5bits.TMR5IF)
    {
        //T5CONbits.TMR5ON = 0;
        PIR5bits.TMR5IF = 0;
        if (intCount[INT_TMR5]++ >= intCountLimit[INT_TMR5])
        {
            intCount[INT_TMR5] = 0;
            intFunction[INT_TMR5]();
        }
    }
    #endif
    
    #ifdef TMR6INT
    if (PIE5bits.TMR6IE && PIR5bits.TMR6IF)
    {
        //T6CONbits.TMR6ON = 0;
        PIR5bits.TMR6IF = 0;
        if (intCount[INT_TMR6]++ >= intCountLimit[INT_TMR6])
        {
            intCount[INT_TMR6] = 0;
            intFunction[INT_TMR6]();
        }
    }
    #endif
    
    #ifdef TMR8INT
    if (PIE5bits.TMR8IE && PIR5bits.TMR8IF)
    {
        //T8CONbits.TMR8ON = 0;
        PIR5bits.TMR8IF = 0;
        if (intCount[INT_TMR8]++ >= intCountLimit[INT_TMR8])
        {
            intCount[INT_TMR8] = 0;
            intFunction[INT_TMR8]();
        }
    }
    #endif
    
    #ifdef ADINT
    if (PIE1bits.ADIE && PIR1bits.ADIF)
    {
        PIR1bits.ADIF = 0;
        intFunction[INT_AD]();
    }
    #endif
    
    #ifdef RCINT
    if (PIE1bits.RCIE && PIR1bits.RCIF)
    {
        PIR1bits.RCIF = 0;
        intFunction[INT_RC]();
    }
    #endif
    
    #ifdef TXINT
    if (PIE1bits.TXIE && PIR1bits.TXIF)
    {
        PIR1bits.TXIF = 0;
        intFunction[INT_TX]();
    }
    #endif
    
    #ifdef CCP1INT
    if (PIE1bits.CCP1IE && PIR1bits.CCP1IF)
    {
        PIR1bits.CCP1IF = 0;
        intFunction[INT_CCP1]();
    }
    #endif
    
    #ifdef CCP2INT
    if (PIE2bits.CCP2IE && PIR2bits.CCP2IF)
    {
        PIR2bits.CCP2IF = 0;
        intFunction[INT_CCP2]();
    }
    #endif
    
    #ifdef EEINT
    
    if (PIE2bits.EEIE && PIR2bits.EEIF)
    {
        PIR2bits.EEIF = 0;
        intFunction[INT_EE]();
    }
    #endif
    
    #ifdef USBINT
    if (PIE2bits.USBIE && PIR2bits.USBIF)
    {
        PIR2bits.USBIF = 0;
        intFunction[INT_USB]();
    }
    #endif
    
    #ifdef OSCFINT
    if (PIE2bits.OSCFIE && PIR2bits.OSCFIF)
    {
        PIR2bits.OSCFIF = 0;
        intFunction[INT_OSCF]();
    }
    #endif
    
    #ifdef CMINT
    if (PIE2bits.CMIE && PIR2bits.CMIF)
    {
        PIR2bits.CMIF = 0;
        intFunction[INT_CM]();
    }
    #endif
    
    #ifdef BCLINT
    if (PIE2bits.BCLIE && PIR2bits.BCLIF)
    {
        PIR2bits.BCLIF = 0;
        intFunction[INT_BCL]();
    }
    #endif
    
    #ifdef HLVDINT
    if (PIE2bits.HLVDIE && PIR2bits.HLVDIF)
    {
        PIR2bits.HLVDIF = 0;
        intFunction[INT_HLVD]();
    }
    #endif
    
    #ifdef SSPINT
    if (PIE1bits.SSPIE && PIR1bits.SSPIF)
    {
        PIR1bits.SSPIF = 0;
        intFunction[INT_SSP]();
    }
    #endif

    #if defined(__18f46j53) || defined(__18f47j53)
    #if defined(PMPINT)
    if (PIE1bits.PMPIE && PIR1bits.PMPIF)
    {
        PIR1bits.PMPIF = 0;
        intFunction[INT_PMP]();
    }
    #endif
    #endif

    //int_start();
}

#endif /* __INTERRUPT_C */
