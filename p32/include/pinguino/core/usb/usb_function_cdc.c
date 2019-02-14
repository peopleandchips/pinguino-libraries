/*
 * This file contains all of functions, macros, definitions, variables,
 * datatypes, etc. that are required for usage with the CDC function
 * driver. This file should be included in projects that use the CDC
 * function driver.
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the 'Company') for its PIC� Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN 'AS IS' CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */

#ifndef USBFUNCTIONCDC_C
#define USBFUNCTIONCDC_C

#include <typedef.h>
#include <usb/usb_device.h>
#include <usb/usb_function_cdc.h>

u8  cdc_trf_state;              // States are defined cdc.h
u8  cdc_rx_len;                 // total rx length
u8  cdc_tx_len;                 // total tx length
u32 cdc_bps;                    // CDC baud rate (cf. cdc.c)

LINE_CODING cdc_line_coding;    // Buffer to store line coding information

USB_HANDLE data_out;
USB_HANDLE data_in;

CONTROL_SIGNAL_BITMAP control_signal_bitmap;

// *** MUST BE VOLATILE ***
//USBVOLATILE u8 cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
//USBVOLATILE u8 cdc_data_tx[CDC_DATA_IN_EP_SIZE];
//volatile u8 cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
//volatile u8 cdc_data_tx[CDC_DATA_IN_EP_SIZE];
u8 cdc_data_rx[CDC_DATA_OUT_EP_SIZE];
u8 cdc_data_tx[CDC_DATA_IN_EP_SIZE];

/***********************************************************************
 * SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required
 * requests according to the CDC specification.
 * However, it is not really being used here, therefore a dummy buffer is
 * used for conformance.
 **********************************************************************/

#define DUMMY_LENGTH    0x08
u8 dummy_encapsulated_cmd_response[DUMMY_LENGTH];

/***********************************************************************
 * This routine checks the setup data packet to see if it
 * knows how to handle it.
 **********************************************************************/
 
void usb_cdc_check_request()
{
    #if 0 //def __DEBUG__
    debug("usb_check_cdc_request");
    #endif

    #if 0 //def __DEBUG__
    debug("Recipient=%d", usb_setup_pkt.Recipient);
    #endif

    // If request recipient is not an interface then return
    if (usb_setup_pkt.Recipient != RCPT_INTF)
        return;

    #if 0 //def __DEBUG__
    debug("Request type=%d", usb_setup_pkt.RequestType);
    #endif

    // If request type is not class-specific then return
    if (usb_setup_pkt.RequestType != CLASS)
        return;

    // Interface ID must match interface numbers associated with CDC class
     
    if ((usb_setup_pkt.bIntfID != CDC_COMM_INTF_ID) &&
        (usb_setup_pkt.bIntfID != CDC_DATA_INTF_ID))
        return;

    switch (usb_setup_pkt.bRequest)
    {
        // Required command
        case CDC_SEND_ENCAPSULATED_COMMAND:
            #ifdef __DEBUG__
            debug("SEND_ENCAPSULATED_COMMAND");
            #endif
            
            // send the packet
            usb_in_pipe.pSrc.bRam = (u8*) &dummy_encapsulated_cmd_response;
            usb_in_pipe.wCount = DUMMY_LENGTH;
            usb_in_pipe.info.bits.ctrl_trf_mem = USB_INPIPES_RAM;
            usb_in_pipe.info.bits.busy = 1;
            break;

        // Required command
        case CDC_GET_ENCAPSULATED_RESPONSE:
            #ifdef __DEBUG__
            debug("GET_ENCAPSULATED_RESPONSE");
            #endif
            
            // Populate dummy_encapsulated_cmd_response first.
            usb_in_pipe.pSrc.bRam = (u8*) &dummy_encapsulated_cmd_response;
            usb_in_pipe.info.bits.busy = 1;
            break;

        case CDC_SET_LINE_CODING:
            #ifdef __DEBUG__
            debug("SET_LINE_CODING");
            #endif
            
            usb_out_pipe.wCount = usb_setup_pkt.wLength;
            usb_out_pipe.pDst.bRam = (u8*) &cdc_line_coding._byte[0];
            usb_out_pipe.pFunc = 0;
            usb_out_pipe.info.bits.busy = 1;
            break;

        case CDC_GET_LINE_CODING:
            #ifdef __DEBUG__
            debug("GET_LINE_CODING");
            #endif
            
            usb_ep0_send_ram_ptr ((u8*) &cdc_line_coding,
                CDC_LINE_CODING_LENGTH, USB_EP0_INCLUDE_ZERO);
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            #ifdef __DEBUG__
            debug("SET_CONTROL_LINE_STATE");
            #endif
            
            control_signal_bitmap._byte = (u8)usb_setup_pkt.W_Value;
            //CONFIGURE_RTS(control_signal_bitmap.CARRIER_CONTROL);
            //CONFIGURE_DTR(control_signal_bitmap.DTE_PRESENT);
            
            if (usb_setup_pkt.W_Value == 0x03)
                cdc_trf_state = CDC_TX_BUSY;
            else
                cdc_trf_state = CDC_TX_READY;
            
            //usb_out_pipe.pDst.bRam = (u8*)&zlp;
            //usb_out_pipe.wCount = sizeof(Zero_Packet_Length) ;
            usb_in_pipe.info.bits.busy = 1;
            break;
    }
}

/***********************************************************************
 * This function initializes the CDC function driver. This function sets
 * the default line coding (baud rate, bit parity, number of data bits,
 * and format). This function also enables the endpoints and prepares for
 * the first transfer from the host.
 **********************************************************************/
 
void usb_cdc_init_endpoint()
{
    #ifdef __DEBUG__
    debug("cdc_init_endpoint");
    #endif

    // Abstract line coding information
    cdc_line_coding.dwDTERate   = cdc_bps; //CDC_DEFAULT_BPS;      // baud rate
    cdc_line_coding.bCharFormat = CDC_DEFAULT_FORMAT;   // 1 stop bit
    cdc_line_coding.bParityType = CDC_DEFAULT_PARITY;   // No parity
    cdc_line_coding.bDataBits   = CDC_DEFAULT_NUM_BITS; // 5,6,7,8, or 16

    // Flush all of the data in the CDC buffer
    cdc_trf_state = CDC_TX_READY;
    //cdc_tx_len = 0;
    cdc_rx_len = 0;

    /* Do not have to init Cnt of IN pipes here.
     * Reason:  Number of BYTEs to send to the host varies from one
     * transaction to another. Cnt should equal the exact number of
     * BYTEs to transmit for a given IN transaction. This number of
     * BYTEs will only be known right before the data is sent.
     */
     
    usb_enable_endpoint(CDC_COMM_EP, USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
    usb_enable_endpoint(CDC_DATA_EP, USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP | USB_OUT_ENABLED);
    //usb_enable_endpoint(CDC_COMM_EP, USB_IN_ENABLED | USB_HANDSHAKE_ENABLED);
    //usb_enable_endpoint(CDC_DATA_EP, USB_IN_ENABLED | USB_OUT_ENABLED | USB_HANDSHAKE_ENABLED);

    data_out = usb_rx_one_packet(CDC_DATA_EP, (u8*)&cdc_data_rx, sizeof(cdc_data_rx));
    data_in  = 0;
}

/***********************************************************************
 * 
 **********************************************************************/
 
#if 0
u8 cdc_consume (void (*func) (u32))
{
    u8 len;
    u32 n;

    if (! data_out || usb_handle_busy(data_out))
        return 0;

    // Pass received data to user function.
    len = usb_handle_get_length(data_out);
    if (func != 0)
    {
        for (n=0; n<len; n++)
            func(cdc_data_rx[n]);
    }

    // Prepare dual-ram buffer for next OUT transaction
    data_out = usb_rx_one_packet (CDC_DATA_EP, (u8*)&cdc_data_rx, sizeof(cdc_data_rx));

    return len;
}
#endif

/***********************************************************************
 * Get a string from CDC port
 * Return the number of bytes read
 **********************************************************************/
 
/*
u8 CDCgets(u8 *buffer)
{
    #if defined(__32MX220F032D__) || defined(__32MX220F032B__) || \
        defined(__32MX250F128B__) || defined(__32MX270F256B__)

        USB_Service();
        return USB_Service_CDC_GetString( buffer );

    #else

        CDCTxService();
        return getsUSBUSART(buffer, 64);

    #endif

    if (mUSBUSARTIsTxTrfReady())
    {
        CDCTxService();
        numBytesRead = getsUSBUSART(buffer, 64);
        CDCTxService();
        return numBytesRead;
    }
}
*/

u8 cdc_gets(char *buffer)
{
    u8 len = 0;
   
    cdc_rx_len = 0;

    //if (! data_out || usb_handle_busy(data_out))
    if (!usb_handle_busy(data_out))
    {
        len = usb_handle_get_length(data_out);

        // Copy data from dual-ram buffer to user's buffer
        for (cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
            buffer[cdc_rx_len] = cdc_data_rx[cdc_rx_len];
        
        data_out = usb_rx_one_packet (CDC_DATA_EP, (u8*)&cdc_data_rx, sizeof(cdc_data_rx));
    }

    return len;
}

/***********************************************************************
 * Get one char from the CDC port.
 **********************************************************************/

char cdc_getc()
{
    char buffer[64];
    u8 len = cdc_gets(buffer);

    if (len > 0)
        return buffer[0];
    else
        return 0;
}

/***********************************************************************
 * Handles device-to-host transaction(s)
 * This function should be called once per Main Program loop after the
 * device reaches the configured state.
 **********************************************************************/

void cdc_tx_service()
{
    u8 byte_to_send, i;
    
    #if 0 //def __DEBUG__
    debug("cdc_tx_service()");
    #endif

    // Check that USB connection is established.
    if (usb_device_state < CONFIGURED_STATE) // || U1PWRCbits.USUSPEND)
        return;

    if (usb_handle_busy(data_in))
        return;
        
    if (cdc_trf_state == CDC_TX_COMPLETING)
        cdc_trf_state = CDC_TX_READY;

    // If CDC_TX_READY state, nothing to do, just return.
    if(cdc_trf_state == CDC_TX_READY)
        return;
    
    // If CDC_TX_BUSY_ZLP state, send zero length packet
    if (cdc_trf_state == CDC_TX_BUSY_ZLP)
    {
        data_in = usb_tx_one_packet(CDC_DATA_EP, 0, 0);
        cdc_trf_state = CDC_TX_COMPLETING;
    }

    else if (cdc_trf_state == CDC_TX_BUSY)
    {
        // First, have to figure out how many byte of data to send.
        if (cdc_tx_len > sizeof(cdc_data_tx))
            byte_to_send = sizeof(cdc_data_tx);
        else
            byte_to_send = cdc_tx_len;

        // Subtract the number of bytes just about to be sent from the total.
        cdc_tx_len = cdc_tx_len - byte_to_send;
        
        // Lastly, determine if a zero length packet state is necessary.
        if (cdc_tx_len == 0)
        {
            if (byte_to_send == CDC_DATA_IN_EP_SIZE)
                cdc_trf_state = CDC_TX_BUSY_ZLP;
            else
                cdc_trf_state = CDC_TX_COMPLETING;
        }
        
        // send packet
        //data_in = usb_tx_one_packet(CDC_DATA_EP, (u8*)&cdc_data_tx, cdc_tx_len);
        data_in = usb_tx_one_packet(CDC_DATA_EP, (u8*)&cdc_data_tx, byte_to_send);
    }
}

#endif /* USBFUNCTIONCDC_C */
