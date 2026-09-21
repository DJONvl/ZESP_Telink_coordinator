/********************************************************************************************************
 * @file    zb_ieee.c
 *
 * @brief   Coordinator IEEE control for the gateway host.
 *
 *          Background: the stack takes the extended address from the MAC flash
 *          sector (0xFF000 on 1 MB flash) and caches it in NVRAM. If the sector
 *          is blank (e.g. wiped by a bootloader reflash) a random EUI-64 is
 *          generated, and every factory reset mints a fresh one. The
 *          coordinator then looks like a brand-new device each time.
 *
 *          Fix: user_app_init() runs after stack_init() (MAC/NVRAM restore)
 *          and before bdb_init() (formation), so unconditionally overwriting
 *          NIB here pins the identity every boot, immune to NVRAM wipes,
 *          blank MAC sector and reflashes.
 *
 *          0x0903 sets the IEEE at runtime (payload: 8 bytes in DISPLAY order,
 *          i.e. exactly as netinfo 0x8045 shows it). Takes effect immediately
 *          for subsequent stack ops; for a clean state send it after factory
 *          reset / before formation. RSP 0x8903: [status, echoed 8 bytes].
 *          RAM-only: the next reboot re-applies FIXED_IEEE_DISP.
 *
 *******************************************************************************************************/
#if (__PROJECT_TL_GW__)

#include "tl_common.h"
#include "zb_common.h"
#include "zbhci.h"
#include "zb_ieee.h"

/* Display order (MSB first), as shown in netinfo 0x8045 / driver logs.
 * NIB stores it little-endian, hence the reversal on write. */
static const u8 FIXED_IEEE_DISP[8] = {0x3C, 0x0B, 0x4F, 0xFF, 0xFE, 0x05, 0xE2, 0x6F};

void zb_ieeeApplyFixed(void)
{
    u8 i;
    for (i = 0; i < 8; i++) {
        g_zbMacPib.extAddress[i] = FIXED_IEEE_DISP[7 - i];
    }
}

void zbhci_ieeeSetHandle(void *arg)
{
    zbhci_cmdHandler_t *cmdInfo = (zbhci_cmdHandler_t *)arg;
    u8 rsp[9] = {0};

    if (cmdInfo->payloadLen >= 8) {
        u8 i;
        u8 *p = cmdInfo->payload;
        for (i = 0; i < 8; i++) {
            g_zbMacPib.extAddress[i] = p[7 - i];
        }
        rsp[0] = ZBHCI_MSG_STATUS_SUCCESS;
        memcpy(&rsp[1], p, 8);
    } else {
        rsp[0] = ZBHCI_MSG_STATUS_INCORRECT_PARAMETERS;
    }

    zbhciTx(ZBHCI_CMD_MAC_SET_EXT_ADDR_RSP, sizeof(rsp), rsp);
    ev_buf_free(arg);
}

#endif /* __PROJECT_TL_GW__ */
