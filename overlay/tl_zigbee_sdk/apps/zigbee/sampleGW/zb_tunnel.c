/********************************************************************************************************
 * @file    zb_tunnel.c
 *
 * @brief   Raw APS tunnel for the gateway host.
 *
 *          Every incoming APS message on the gateway endpoint is mirrored to
 *          the host as 0x8902 (see zbhci_tunnelMsgRecvPush), then processed
 *          normally by the ZCL layer. This carries manufacturer-specific
 *          clusters (e.g. 0xFCC0, 0xEF00) and any custom frames that the
 *          stock ZBHCI commands cannot express. Sending is done with 0x0901.
 *
 *******************************************************************************************************/
#if (__PROJECT_TL_GW__)

#include "tl_common.h"
#include "zb_api.h"
#include "zcl_include.h"

#if ZBHCI_EN
#include "zbhci.h"

void zcl_rx_handler(void *pData);
void zbhci_tunnelMsgRecvPush(void *arg);

/*********************************************************************
 * @fn      sampleGW_tunnelRxHandler
 *
 * @brief   AF receive hook: tunnel a copy to the host, then continue
 *          with the normal ZCL processing.
 *
 * @param   pData - apsdeDataInd_t
 *
 * @return  None
 */
void sampleGW_tunnelRxHandler(void *pData)
{
    zbhci_tunnelMsgRecvPush(pData);
    zcl_rx_handler(pData);
}

#endif /* ZBHCI_EN */

#endif /* __PROJECT_TL_GW__ */
