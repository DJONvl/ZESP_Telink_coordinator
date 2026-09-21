/********************************************************************************************************
 * @file    zb_ieee.h
 *
 * @brief   Coordinator IEEE control: fixed at boot + settable via ZBHCI 0x0903.
 *
 *******************************************************************************************************/
#pragma once

#include "tl_common.h"

void zb_ieeeApplyFixed(void);
void zbhci_ieeeSetHandle(void *arg);
