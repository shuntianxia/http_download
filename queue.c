/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 * WICED Configuration Mode Application
 *
 * This application demonstrates how to use WICED Configuration Mode
 * to automatically configure application parameters and Wi-Fi settings
 * via a softAP and webserver
 *
 * Features demonstrated
 *  - WICED Configuration Mode
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. After the download completes, the terminal displays WICED startup
 *      information and starts WICED configuration mode.
 *
 * In configuration mode, application and Wi-Fi configuration information
 * is entered via webpages using a Wi-Fi client (eg. your computer)
 *
 * Use your computer to step through device configuration using WICED Config Mode
 *   - Connect the computer using Wi-Fi to the config softAP "WICED Config"
 *     The config AP name & passphrase is defined in the file <WICED-SDK>/include/default_wifi_config_dct.h
 *     The AP name/passphrase is : Wiced Config / 12345678
 *   - Open a web browser and type wiced.com in the URL
 *     (or enter 192.168.0.1 which is the IP address of the softAP interface)
 *   - The Application configuration webpage appears. This page enables
 *     users to enter application specific information such as contact
 *     name and address details for device registration
 *   - Change one of more of the fields in the form and then click 'Save settings'
 *   - Click the Wi-Fi Setup button
 *   - The Wi-Fi configuration page appears. This page provides several options
 *     for configuring the device to connect to a Wi-Fi network.
 *   - Click 'Scan and select network'. The device scans for Wi-Fi networks in
 *     range and provides a webpage with a list.
 *   - Enter the password for your Wi-Fi AP in the Password box (top left)
 *   - Find your Wi-Fi AP in the list, and click the 'Join' button next to it
 *
 * Configuration mode is complete. The device stops the softAP and webserver,
 * and attempts to join the Wi-Fi AP specified during configuration. Once the
 * device completes association, application configuration information is
 * printed to the terminal
 *
 * The wiced.com URL reference in the above text is configured in the DNS
 * redirect server. To change the URL, edit the list in
 * <WICED-SDK>/Library/daemons/dns_redirect.c
 * URLs currently configured are:
 *      # http://www.broadcom.com , http://broadcom.com ,
 *      # http://www.facebook.com , http://facebook.com ,
 *      # http://www.google.com   , http://google.com   ,
 *      # http://www.bing.com     , http://bing.com     ,
 *      # http://www.apple.com    , http://apple.com    ,
 *      # http://www.wiced.com    , http://wiced.com    ,
 *
 *  *** IMPORTANT NOTE ***
 *   The config mode API will be integrated into Wi-Fi Easy Setup when
 *   WICED-SDK-3.0.0 is released.
 *
 */

#include "wiced.h"
#include "queue.h"

/******************************************************
 *                      Macros
 ******************************************************/


/******************************************************
 *                    Constants
 ******************************************************/


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/


/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
wiced_result_t create_queue(queue_t *q, int maxsize)
{  
	q->base=(int *)malloc(sizeof(q_elem_t)*maxsize);
	if(NULL == q->base)
	{
		WPRINT_APP_INFO(("Memory allocation failure"));
		return WICED_ERROR;
	}
	q->front = q->rear = 0;
	q->maxsize = maxsize;
	return WICED_SUCCESS;
}

void destroy_queue(queue_t *q)
{
	if(q->base)
		free(q->base);
	q->base = NULL;
	q->front = q->rear = 0;
}

wiced_bool_t queue_is_full(queue_t q)
{
	if((q->rear+1)%q->maxsize == q->front)
		return WICED_TRUE;
	else
		return WICED_FALSE;
}

wiced_bool_t queue_is_empty(queue_t q)  
{  
	if(q->front == q->rear)
		return WICED_TRUE;
	else
		return WICED_FALSE;
}

q_elem_t *get_front_elem(queue_t *q)
{
	if(queue_is_empty(q)) {
		return NULL;
	} else {
		return &q->base[q->front];
	}
}

q_elem_t *get_rear_elem(queue_t *q)
{
	if(queue_is_empty(q)) {
		return NULL;
	} else {
		return &q->base[q->rear];
	}
}

wiced_result_t en_queue(queue_t q, int val)  
{  
	if(queue_is_full(q)) {
		return WICED_ERROR;
	} else {
		q->base[q->rear] = val;
		q->rear = (q->rear+1)%q->maxsize;
		return WICED_SUCCESS;
	}
}

wiced_result_t de_queue(queue_t q, int *val)
{  
	if(queue_is_empty(q)) {
		return WICED_ERROR;
	} else {
		*val = q->base[q->front];
		q->front = (q->front+1)%q->maxsize;
		return WICED_SUCCESS;  
	}
}
