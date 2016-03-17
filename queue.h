/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
//#define MAX_QSIZE 5 /* 最大队列长度+1 */
#define MAX_BUF_LEN 2*1024

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct q_data{
	int len;
	char data[MAX_BUF_LEN];
}q_elem_t;

typedef struct queue{
	int front;
	int rear;
	int maxsize;
	q_elem_t *base;
}queue_t;

/******************************************************
 *                    Structures
 ******************************************************/



/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/


#ifdef __cplusplus
} /* extern "C" */
#endif

