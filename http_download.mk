#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_http_download

$(NAME)_SOURCES := http_download.c
				   

				   
ifeq ($(NETWORK),NetX)
#GLOBAL_DEFINES += NX_EXTENDED_BSD_SOCKET_SUPPORT
endif

ifeq ($(NETWORK),NetX_Duo)
#GLOBAL_DEFINES += NXD_EXTENDED_BSD_SOCKET_SUPPORT
endif

#GLOBAL_DEFINES := WICED_DISABLE_SSID_BROADCAST

WIFI_CONFIG_DCT_H := wifi_config_dct.h