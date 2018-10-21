#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <upnp/ixml.h>
#include <string.h>
#include <time.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <upnp/TimerThread.h>
#include "pmlist.h"
#include "MBridgeControl.h"
#include "globals.h"
#include "gatedevice.h"
#include "util.h"
#include "gconfig.h"
#include "tokens.h"
#include "appsver.h"
#include "apcfg.h"
#include "firmwareconfig.h"
static struct dmlist dmevent;

enum{
    WLAN_ENC_NONE=0,
    WLAN_ENC_WEP,
    WLAN_ENC_TKIP,
    WLAN_ENC_AES,
    WLAN_ENC_TKIPAES,
    MAX_WLAN_ENC
};

enum{
    WLAN_AUTH_OPEN=0,
    WLAN_AUTH_WEPAUTO,
    WLAN_AUTH_SHARED,
    WLAN_AUTH_WPAPSK,
    WLAN_AUTH_WPA,
    WLAN_AUTH_WPA2PSK,
    WLAN_AUTH_WPA2,
    WLAN_AUTH_WPA1WPA2,
    WLAN_AUTH_WPA1PSKWPA2PSK
};

enum{
    WLAN_ENCRYPT_NONE=0,
    WLAN_ENCRYPT_WEPOPEN,
    WLAN_ENCRYPT_WEPSHARED,
    WLAN_ENCRYPT_WEPAUTO,
    WLAN_ENCRYPT_WPATKIP,
    WLAN_ENCRYPT_WPAAES,
    WLAN_ENCRYPT_WPAMIX,
    WLAN_ENCRYPT_WPA2TKIP,
    WLAN_ENCRYPT_WPA2AES,
    WLAN_ENCRYPT_WPA2MIX
};

static MBridgeCommonProfile mbConfigToken =
{
    .mrhessBridgeID = {0},
    .friendlyName = {0}
};

extern TimerThread gExpirationTimerThread;
extern ithread_mutex_t DevMutex;
static char mbSSIDPriorityToken[2][8][8] = {[0]={[0 ... 7]=0}, [1]={[0 ... 7]=0}}; //[SSID][CATNUM][RET_STRING]
static int mbSSIDPriorityModified = 0;
/*********************************************************
 *  SSID0:  5G MRHESS network
 *  SSID1:  5G HomeAP network
 *  SSID2:2.4G MRHESS network
 *  SSID3:2.4G HomeAP network
 **********************************************************/
//HomeAPWirelessProfile [0]:2.4G, [1]:5G
static MBridgeWirelessProfile mbHomeAPToken[] =
{
    {
        .wpsStatus = 0,
        .isEnable = {0},
        .SSID = {0},
        .AuthMode = {0},
        .EncrypType = {0},
        .WpaPsk = {0},
        .WpaPskType = {0},
        .DefaultKeyId = {0},
        .keyLength = {0},
        .KeyType = {0},
        .key = {{0}}
    },{
        .wpsStatus = 0,
        .isEnable = {0},
        .SSID = {0},
        .AuthMode = {0},
        .EncrypType = {0},
        .WpaPsk = {0},
        .WpaPskType = {0},
        .DefaultKeyId = {0},
        .keyLength = {0},
        .KeyType = {0},
        .key = {{0}}
    }
};
/*****************************************************************
 * NAME:    upnp_search_setting_value
 * ---------------------------------------------------------------
 * FUNCTION: search the setting value by keyword
 * INPUT:    context, keyword
 * OUTPUT:   value
 * Author:   Tim
 * Modify:
 ******************************************************************/
static char *upnp_search_setting_value(char *context, const char *keyword)
{
    char *ptr, *value_head, *value_end;
    static char value[256] = {0};

    //memset(value, 0, sizeof(value));

    if (context==NULL || keyword==NULL)
    {
        return NULL;
    }

    while(context != NULL)
    {
        ptr = strstr(context, keyword);

        if (ptr==NULL || (ptr==context))
        {
            return NULL;
        }

        if (*(ptr-1)!='\t')
        {
            context = ptr+strlen(keyword);
        }
        else
        {
            if (*(ptr+strlen(keyword))!=':')
            {
                context = ptr+strlen(keyword);
            }
            else
            {
                value_head = ptr+strlen(keyword)+1;
                value_end = strstr(value_head, "\t");

                if (value_end == NULL)
                {
                    return NULL;
                }
                //strncpy(value, value_head, (value_end-value_head));
                // Yolin: snprintf is better
                snprintf(value, value_end-value_head+1, value_head);
                break;
            }
        }
    }
    return value;
}
/*****************************************************************
 * NAME:    initMBridgeTokenInfo
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void initMBridgeTokenInfo(void)
{
    memset(mbConfigToken.mrhessBridgeID, 0, sizeof(mbConfigToken.mrhessBridgeID));
    memset(mbConfigToken.friendlyName, 0, sizeof(mbConfigToken.friendlyName));
    memset(&mbHomeAPToken[HAP_24G_SSID_IDX], 0, sizeof(MBridgeWirelessProfile));
    memset(&mbHomeAPToken[HAP_5G_SSID_IDX], 0, sizeof(MBridgeWirelessProfile));
    mbSSIDPriorityModified = 0;
    memset(mbSSIDPriorityToken, 0, sizeof(mbSSIDPriorityToken));
}
/*****************************************************************
 * NAME:    setMBridgeTokenInfo
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void setMBridgeTokenInfo(void)
{
    int m = 0, n = 0;
    char tmp[16] = {0};

    if(strlen(mbConfigToken.mrhessBridgeID))
    {
        APCFGCLI_CLI_SET(mbConfigToken.mrhessBridgeID, WLAN_R1_S1_AP_SSID_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbConfigToken.mrhessBridgeID, WLAN_R2_S1_AP_SSID_TOK);
#endif
        //20130827 Jason: set PSK when we get SSID token
        SYSTEM("sysconf_cli inform gen_pskey");
    }
    if(strlen(mbConfigToken.friendlyName))
    {
        APCFGCLI_CLI_SET(mbConfigToken.friendlyName, MRHESS_FRIENDLY_NAME_TOK);
    }

    //TODO:we just save the same value for 2.4g and 5g
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].SSID))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, WLAN_R1_S2_AP_SSID_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, WLAN_R2_S2_AP_SSID_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode, WLAN_R1_S2_AP_AUTH_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode, WLAN_R2_S2_AP_AUTH_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType, WLAN_R1_S2_AP_ENC_TYPE_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType, WLAN_R2_S2_AP_ENC_TYPE_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength, WLAN_R1_S2_AP_WEP_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength, WLAN_R2_S2_AP_WEP_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType, WLAN_R1_S2_AP_KEYTYPE_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType, WLAN_R2_S2_AP_KEYTYPE_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk, WLAN_R1_S2_AP_WPAPASSPHRASE_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk, WLAN_R2_S2_AP_WPAPASSPHRASE_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType, WLAN_R1_S2_AP_WPAPSK_KEYTYPE_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType, WLAN_R2_S2_AP_WPAPSK_KEYTYPE_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[0]))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[0], WLAN_R1_S2_AP_WEPKEY_0_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[0], WLAN_R2_S2_AP_WEPKEY_0_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[1]))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[1], WLAN_R1_S2_AP_WEPKEY_1_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[1], WLAN_R2_S2_AP_WEPKEY_1_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[2]))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[2], WLAN_R1_S2_AP_WEPKEY_2_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[2], WLAN_R2_S2_AP_WEPKEY_2_TOK);
#endif
    }
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[3]))
    {
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[3], WLAN_R1_S2_AP_WEPKEY_3_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET(mbHomeAPToken[HAP_24G_SSID_IDX].key[3], WLAN_R2_S2_AP_WEPKEY_3_TOK);
#endif
    }
    if(mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus == 1)
    {
        APCFGCLI_CLI_SET("1", WLAN_R1_ALL_WPS_STATUS_TOK);
#if _HAS_WLAN_5G_SETTING
        APCFGCLI_CLI_SET("1", WLAN_R2_ALL_WPS_STATUS_TOK);
#endif
    }


    if(mbSSIDPriorityModified == 1)
    {
        for (m=0; m<2; m++)
        {
            for (n=0; n<8; n++)
            {
                if(!strcmp(mbSSIDPriorityToken[m][n], "AC_VO"))
                {
                    snprintf(tmp, sizeof(tmp), "%d", USERPRIORITY_TO_AC_VO);
                    APCFGCLI_CLI_SET(tmp, WLAN_ _R1_ "%d" _AP_ "%d" _USER_TO_AC_TOK, m+1, n+1);
                }
                else if(!strcmp(mbSSIDPriorityToken[m][n], "AC_VI"))
                {
                    snprintf(tmp, sizeof(tmp), "%d", USERPRIORITY_TO_AC_VI);
                    APCFGCLI_CLI_SET(tmp, WLAN_ _R1_ "%d" _AP_ "%d" _USER_TO_AC_TOK, m+1, n+1);
                }
                else if(!strcmp(mbSSIDPriorityToken[m][n], "AC_BE"))
                {
                    snprintf(tmp, sizeof(tmp), "%d", USERPRIORITY_TO_AC_BE);
                    APCFGCLI_CLI_SET(tmp, WLAN_ _R1_ "%d" _AP_ "%d" _USER_TO_AC_TOK, m+1, n+1);
                }
                else //AC_BK
                {
                    snprintf(tmp, sizeof(tmp), "%d", USERPRIORITY_TO_AC_BK);
                    APCFGCLI_CLI_SET(tmp, WLAN_ _R1_ "%d" _AP_ "%d" _USER_TO_AC_TOK, m+1, n+1);
                }
            }
        }
    }

    // we append G_UPNPD to force reload upnpd
    SYSTEM("sysconf_cli applychanges checkAllModules %s&", G_MACRO_TO_STR(G_UPNPD));
}
/*****************************************************************
 * NAME:    convInputString
 * ---------------------------------------------------------------
 * FUNCTION: Use this function to avoid shell input unsafe value
 * INPUT:
 * OUTPUT: REMEMBER TO FREE THE OUTPUT STRING
 * Author:
 * Modify:
 ******************************************************************/
static char * convInputString(char *inputStr)
{
    char *sp, *ret, *pOut;
    char tmp[MAX_COMMAND_LEN] = {0};

    ret = inputStr;
    sp = pOut = tmp;
    //protect
    if(!ret || !strlen(ret))
        return;

    while (*ret != '\0')
    {
        switch (*ret)
        {
            case '`':
                /* pass through */
            case '\\':
                /* pass through */
            case '\'':
                /* pass through */
            case '\"':
                *pOut = '\\';
                pOut++;
            default:
                break;
        }
        *pOut = *ret;
        pOut++;
        ret++;
    }
    *pOut = 0;
    //printf("==> convInputString:[%s] \n", sp);
    return strdup(sp);
}
/*****************************************************************
 * NAME:    GetNewConfigurationStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
char* GetNewConfigurationStatus()
{
    static char NewConfigurationStatus[8];
    APCFGCLI_CLI_GET(NewConfigurationStatus, MRHESS_CONF_STATUS_TOK);
    return NewConfigurationStatus;
}
/*****************************************************************
 * NAME:    GetNewUpdateProgress
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
char* GetNewUpdateProgress()
{
    static char NewUpdateProgress[8]={0};
    sysinteract(NewUpdateProgress, sizeof(NewUpdateProgress), "dmFwUpgrade.sh %s %s", "GetUpdateProgress", FWPROGRESS);
    return NewUpdateProgress;
}
/*****************************************************************
 * NAME:    GetNewUpdateStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
char* GetNewUpdateStatus()
{
    FILE *fp;
    static char NewUpdateStatus[32]={0};

    if(!access(FWSTATUS, F_OK))
    {
        if ((fp = fopen(FWSTATUS, "r")) != NULL)
        {
            fscanf(fp, "%s", NewUpdateStatus);
            fclose(fp);
        }
    }
    return NewUpdateStatus;
}
/*****************************************************************
 * NAME:    GetConfigurationStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetConfigurationStatus(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    strcpy(configurationStatus, GetNewConfigurationStatus());
    snprintf(resultStr, RESULT_LEN,
            "<u:GetConfigurationStatusResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<configurationStatus>%s</configurationStatus>"
            "</u:GetConfigurationStatusResponse>",
            configurationStatus);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    SetConfigurationStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int SetConfigurationStatus(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationStatus = NULL;
    int val = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationStatus = GetFirstDocumentItem(ca_event->ActionRequest, "configurationStatus");
    if(pConfigurationStatus)
    {
        val = atoi(pConfigurationStatus);
        if(strlen(pConfigurationStatus))
        {
            APCFGCLI_CLI_SET(pConfigurationStatus, MRHESS_CONF_STATUS_TOK);
            snprintf(resultStr, RESULT_LEN,
                    "<u:SetConfigurationStatusResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                    "</u:SetConfigurationStatusResponse>");
        }
    }

    if((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
        strcpy(ca_event->ErrStr, "Action Failed");
    }

    if (pConfigurationStatus) free(pConfigurationStatus);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    SetConfigurationStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetConfigurationToken(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char tmp[8] = {0};
    char ConfigurationToken[32] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check");
    if(!strcmp(tmp, "pass") || !strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else // none --> unlock
    {
        // reset all temp value when GetConfigurationToken, ReleaseConfigurationToken and ApplyChanges
        initMBridgeTokenInfo();
        // use "GetConfigurationToken.sh lock 300 &" to lock, use "GetConfigurationToken.sh unlock" to unlock
        system("GetConfigurationToken.sh lock 300 &");
        // GetConfigurationTokenToken.sh get
        // 1. if lock: return value in /tmp/linuxigd/ConfigurationToken.lock
        // 2. if unlock: lock and gen new token to ConfigurationToken.lock
        sysinteract(ConfigurationToken, sizeof(ConfigurationToken), "GetConfigurationToken.sh get");

        snprintf(resultStr, RESULT_LEN,
                "<u:GetConfigurationTokenResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
                "<configurationToken>%s</configurationToken>"
                "</u:GetConfigurationTokenResponse>",
                ConfigurationToken);

        if ((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
        }
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    ReleaseConfigurationToken
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int ReleaseConfigurationToken(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char tmp[8] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else // none or pass
    {
        // release the ConfigurationToken
        sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh unlock");
        initMBridgeTokenInfo();
        snprintf(resultStr, RESULT_LEN,
                "<u:ReleaseConfigurationTokenResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
                "</u:ReleaseConfigurationTokenResponse>");

        if ((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
        }
    }

    if(pConfigurationToken) free(pConfigurationToken);
    if(pConvConfigurationToken) free(pConvConfigurationToken);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    StartInvitation
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int StartInvitation(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *ptimeout = NULL;
    int timeout = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    ptimeout = GetFirstDocumentItem(ca_event->ActionRequest, "timeout");

    if(ptimeout)
        timeout=atoi(ptimeout);

    if(!timeout && timeout < 0)
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 402;
        strcpy(ca_event->ErrStr, "Invalid Args");
    }
    else
    {
        SYSTEM("sysconf_cli ie_inform startInvite %d &", timeout);
        snprintf(resultStr, RESULT_LEN,
                "<u:StartInvitationResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
                "</u:StartInvitationResponse>");

        if ((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }

    if(ptimeout) free(ptimeout);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    StopInvitation
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int StopInvitation(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    system("sysconf_cli ie_inform stopInvite &");
    snprintf(resultStr, RESULT_LEN,
            "<u:StopInvitationResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "</u:StopInvitationResponse>");

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetAccessPointList
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetAccessPointList(struct Upnp_Action_Request *ca_event)
{
    //TODO:resultStr buf might overflow!!!
    char resultStr[RESULT_LEN*2] = {0};
    char resultTmp[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pOutString = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pAccessPointList = NULL;
    char tmp[8] = {0};
    int j = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        system("sysconf_cli inform upnp_sitesurvey"); //It will cause 3 sec to sitesurvey.

#define UPNP_SITE_SURVEY_FILE "/mnt/upnp_sitesurvey"
        //cmd [rm -f /mnt/sitesurvey_tmp ]
        j = sprintf(resultTmp, "<APInfoList>");

        FILE *fp = NULL;
        char buf[512]={0};
        if((fp=fopen(UPNP_SITE_SURVEY_FILE, "r")) != NULL)
        {
            while(fgets(buf, sizeof(buf), fp) != NULL)
            {
                // remove '\n'
                *(buf+strlen(buf)-1) = '\0';
                j += sprintf(resultTmp+j, "<APInfo>");
                j += sprintf(resultTmp+j, "%s", buf);
                j += sprintf(resultTmp+j, "</APInfo>");
                //TODO: to avoid overflow
                if(j > (RESULT_LEN - 540))
                    break;
            }
        }
        if(fp) fclose(fp);
        j += sprintf(resultTmp+j, "</APInfoList>");
        pOutString = escapeXMLString(resultTmp);

        j = sprintf(resultStr, "<u:GetAccessPointListResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">");
        j += sprintf(resultStr+j, "<accessPointList>");
        j += sprintf(resultStr+j, "%s", pOutString);
        j += sprintf(resultStr+j, "</accessPointList>");
        j += sprintf(resultStr+j, "</u:GetAccessPointListResponse>");

        if((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pAccessPointList) free(pAccessPointList);
    if (pOutString) free(pOutString);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetMRHESSBRIDGEID
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetMRHESSBRIDGEID(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pMRHESSBRIDGEID = NULL;
    char token[128] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    if(strlen(mbConfigToken.mrhessBridgeID))
    {
        strcpy(token, mbConfigToken.mrhessBridgeID);
    }
    else
    {
        APCFGCLI_CLI_GET(token, WLAN_R1_S1_AP_SSID_TOK);
    }

    // SSID contains "&", we must translate it to "&amp;"
    pMRHESSBRIDGEID = escapeXMLString(token);
    snprintf(resultStr, RESULT_LEN,
            "<u:GetMRHESSBRIDGEIDResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<MRHESSBRIDGEID>%s</MRHESSBRIDGEID>"
            "</u:GetMRHESSBRIDGEIDResponse>",
            pMRHESSBRIDGEID);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    if(pMRHESSBRIDGEID) free(pMRHESSBRIDGEID);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    SetMRHESSBRIDGEID
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int SetMRHESSBRIDGEID(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pMRHESSBRIDGEID = NULL;
    char tmp[8] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        pMRHESSBRIDGEID = GetFirstDocumentItem(ca_event->ActionRequest, "MRHESSBRIDGEID");
        strcpy(mbConfigToken.mrhessBridgeID, pMRHESSBRIDGEID);
        snprintf(resultStr, RESULT_LEN,
                "<u:SetMRHESSBRIDGEIDResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                "</u:SetMRHESSBRIDGEIDResponse>");
        if((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }
    else if(!strcmp(tmp, "invalid"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 402;
        strcpy(ca_event->ErrStr, "Invalid Args");//invalid ConfigurationToken
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pMRHESSBRIDGEID) free(pMRHESSBRIDGEID);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetHomeAPWirelessProfile
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetHomeAPWirelessProfile(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    char resultTmp[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    char *pOutString = NULL;
    char *pOutSSID = NULL;
    //char *supportEncrypt[] = {"NONE","WEP-OPEN","WEP-SHARED","WEP-AUTO","WPA-TKIP","WPA-AES","WPA-MIX","WPA2-TKIP","WPA2-AES","WPA2-MIX"};
    char *supportEncrypt[] = {"NONE","WEP","WEP","WEP","WPA-TKIP","WPA-AES","WPA-AES","WPA2-TKIP","WPA2-AES","WPA2-AES"};
    char secEnable[16] = {0};
    char passPhrase[128] = {0};
    int auth = 0, encrypt = 0, securityType = 0, defkey = 0;
    int j = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    dprintf("isEnable:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].isEnable);
    dprintf("SSID:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].SSID);
    dprintf("AuthMode:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode);
    dprintf("EncrypType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType);
    dprintf("WpaPsk:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk);
    dprintf("WpaPskType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType);
    dprintf("DefaultKeyId:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId);
    dprintf("keyLength:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].keyLength); /*0,64,128*/
    dprintf("KeyType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].KeyType); /*0:HEX 1:ASCII*/
    dprintf("key:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].key);

    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].isEnable) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].isEnable, WLAN_R1_S2_AP_ENABLE_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].SSID) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, WLAN_R1_S2_AP_SSID_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode, WLAN_R1_S2_AP_AUTH_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType, WLAN_R1_S2_AP_ENC_TYPE_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk, WLAN_R1_S2_AP_WPAPASSPHRASE_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType, WLAN_R1_S2_AP_WPAPSK_KEYTYPE_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId, WLAN_R1_S2_AP_KEYID_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength, WLAN_R1_S2_AP_WEP_TOK); /*0,64,128*/
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType, WLAN_R1_S2_AP_KEYTYPE_TOK); /*0:HEX 1:ASCII*/
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[0]) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].key[0], WLAN_R1_S2_AP_WEPKEY_0_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[1]) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].key[1], WLAN_R1_S2_AP_WEPKEY_1_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[2]) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].key[2], WLAN_R1_S2_AP_WEPKEY_2_TOK);
    if(strlen(mbHomeAPToken[HAP_24G_SSID_IDX].key[3]) == 0)
        APCFGCLI_CLI_GET(mbHomeAPToken[HAP_24G_SSID_IDX].key[3], WLAN_R1_S2_AP_WEPKEY_3_TOK);

    encrypt=atoi(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType);
    auth=atoi(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode);

    strcpy(secEnable, "true");
    switch(encrypt)
    {
        case WLAN_ENC_NONE:
            securityType = WLAN_ENCRYPT_NONE;
            strcpy(secEnable, "false");
            break;
        case WLAN_ENC_WEP:
            securityType = (auth==WLAN_AUTH_OPEN)?WLAN_ENCRYPT_WEPOPEN:
                (auth==WLAN_AUTH_WEPAUTO)?WLAN_ENCRYPT_WEPAUTO:WLAN_ENCRYPT_WEPSHARED;
            break;
        case WLAN_ENC_TKIP:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPATKIP:WLAN_ENCRYPT_WPA2TKIP;
            break;
        case WLAN_ENC_AES:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPAAES:WLAN_ENCRYPT_WPA2AES;
            break;
        case WLAN_ENC_TKIPAES:
        default:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPAMIX:WLAN_ENCRYPT_WPA2MIX;
            break;
    }

    switch(encrypt)
    {
        case WLAN_ENC_WEP:
            if(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId)
                defkey = atoi(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId);
            strcpy(passPhrase, mbHomeAPToken[HAP_24G_SSID_IDX].key[defkey]);
            break;
        case WLAN_ENC_TKIP:
        case WLAN_ENC_AES:
        case WLAN_ENC_TKIPAES:
        default:
            strcpy(passPhrase, mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk);
            break;
    }

    pOutSSID = escapeXMLString(mbHomeAPToken[HAP_24G_SSID_IDX].SSID);
    j = sprintf(resultTmp, "<wirelessProfile SSID=\"%s\">", pOutSSID);
    j += sprintf(resultTmp+j, "<wirelessSecurity enabled=\"%s\">", secEnable);
    j += sprintf(resultTmp+j, "<Mode passPhrase=\"%s\">%s</Mode>", passPhrase, supportEncrypt[securityType]);
    j += sprintf(resultTmp+j, "</wirelessSecurity></wirelessProfile>");
    pOutString = escapeXMLString(resultTmp);

    //TODO: 5G profile how to show?
    j = sprintf(resultStr, "<u:GetHomeAPWirelessProfileResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">");
    j += sprintf(resultStr+j, "<HomeAPWirelessProfile>");
    j += sprintf(resultStr+j, "%s", pOutString);
    j += sprintf(resultStr+j, "</HomeAPWirelessProfile>");
    j += sprintf(resultStr+j, "</u:GetHomeAPWirelessProfileResponse>");

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    if(pOutString) free(pOutString);
    if(pOutSSID) free(pOutSSID);

    return(ca_event->ErrCode);

}
/*****************************************************************
 * NAME:    SetHomeAPWirelessProfile
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int SetHomeAPWirelessProfile(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pInputString = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pHomeAPWirelessProfile = NULL;
    char SSID[128] = {0};
    char secEnable[16] = {0};
    char passPhrase[128] = {0};
    char securityType[16] = {0};
    char tailString[64]= {0};
    char tmp[8] = {0};
    int ret = 0, defkey = 0;
    int passPhraseLen = 0;
    int invalidArg = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        pHomeAPWirelessProfile = GetFirstDocumentItem(ca_event->ActionRequest, "HomeAPWirelessProfile");

        ret = sscanf(pHomeAPWirelessProfile,
                "<wirelessProfile SSID=\"%64[^\"]\"><wirelessSecurity enabled=\"%6[^\"]\"><Mode passPhrase=\"%64[^\"]\">%[^<]%44s",
                SSID, secEnable, passPhrase, securityType, tailString);

        pInputString = unescapeXMLString(SSID);
        //TODO: we need check the value before save to struct.
        if(ret == 5 && !strcmp(tailString, "</Mode></wirelessSecurity></wirelessProfile>"))
        {
            passPhraseLen = strlen(passPhrase);
            if(!strcmp(secEnable, "true"))
            {
                //TODO: the securityType TBD
                if(!strcmp(securityType, "WEP"))
                {
                    if(passPhraseLen==5 || passPhraseLen==13) //ASCII
                    {
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, pInputString);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType, "1");
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength, (passPhraseLen==5)?"64":"128");
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType), "%d", WLAN_ENC_WEP);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode), "%d", WLAN_AUTH_OPEN);
                        if(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId)
                            defkey = atoi(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].key[defkey], passPhrase);
                        mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus = 1;
                    }
                    else if(passPhraseLen==10 || passPhraseLen==26) //HEX
                    {
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, pInputString);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].KeyType, "0");
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].keyLength, (passPhraseLen==10)?"64":"128");
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType), "%d", WLAN_ENC_WEP);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode), "%d", WLAN_AUTH_OPEN);
                        if(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId)
                            defkey = atoi(mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].key[defkey], passPhrase);
                        mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus = 1;
                    }
                    else
                    {
                        invalidArg = 1;
                    }
                }
                else if(!strcmp(securityType, "WPA-TKIP")||
                        !strcmp(securityType, "WPA2-TKIP"))
                {
                    if(passPhraseLen<8 || passPhraseLen > 64)
                    {
                        invalidArg = 1;
                    }
                    else
                    {
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, pInputString);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType), "%d", WLAN_ENC_TKIP);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk, passPhrase);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode), "%d", WLAN_AUTH_WPAPSK);
                        mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus = 1;
                        /* Set WPA Key type : Passphrase :0 , HEX(64):1 */
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType, (passPhraseLen == 64)?"1":"0");
                    }
                }
                else if(!strcmp(securityType, "WPA-AES")||
                        !strcmp(securityType, "WPA2-AES"))
                {
                    if(passPhraseLen<8 || passPhraseLen > 64)
                    {
                        invalidArg = 1;
                    }
                    else
                    {
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, pInputString);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType), "%d", WLAN_ENC_AES);
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk, passPhrase);
                        snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode,
                                sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode), "%d", WLAN_AUTH_WPAPSK);
                        mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus = 1;
                        /* Set WPA Key type : Passphrase :0 , HEX(64):1 */
                        strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType, (passPhraseLen == 64)?"1":"0");
                    }
                }
            }
            else
            {
                strcpy(mbHomeAPToken[HAP_24G_SSID_IDX].SSID, pInputString);
                snprintf(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType, sizeof(mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType),
                        "%d", WLAN_ENC_NONE);
                mbHomeAPToken[HAP_24G_SSID_IDX].wpsStatus = 1;
            }

            dprintf("HAP_24G_SSID_IDX\n");
            dprintf("isEnable:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].isEnable);
            dprintf("SSID:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].SSID);
            dprintf("AuthMode:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].AuthMode);
            dprintf("EncrypType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].EncrypType);
            dprintf("WpaPsk:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].WpaPsk);
            dprintf("WpaPskType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].WpaPskType);
            dprintf("DefaultKeyId:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].DefaultKeyId);
            dprintf("keyLength:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].keyLength); /*0,64,128*/
            dprintf("KeyType:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].KeyType); /*0:HEX 1:ASCII*/
            dprintf("key:[%s]\n", mbHomeAPToken[HAP_24G_SSID_IDX].key);

            snprintf(resultStr, RESULT_LEN,
                    "<u:SetHomeAPWirelessProfileResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                    "</u:SetHomeAPWirelessProfileResponse>");
            if((result = ixmlParseBuffer(resultStr)) != NULL)
            {
                ca_event->ActionResult = result;
                ca_event->ErrCode = UPNP_E_SUCCESS;
            }
            else
            {
                ca_event->ActionResult = NULL;
                ca_event->ErrCode = 501;
                strcpy(ca_event->ErrStr, "Action Failed");
            }
        }
        else
        {
            invalidArg = 1;
        }
    }
    else if(!strcmp(tmp, "invalid"))
    {
        invalidArg = 1;
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if( invalidArg == 1)
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 402;
        strcpy(ca_event->ErrStr, "Invalid Args");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pHomeAPWirelessProfile) free(pHomeAPWirelessProfile);
    if (pInputString) free(pInputString);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetBridgeWirelessProfile
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetBridgeWirelessProfile(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    char resultTmp[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    char *pOutString = NULL;
    char *pOutSSID = NULL;
    //char *supportEncrypt[] = {"NONE","WEP-OPEN","WEP-SHARED","WEP-AUTO","WPA-TKIP","WPA-AES","WPA-MIX","WPA2-TKIP","WPA2-AES","WPA2-MIX"};
    char *supportEncrypt[] = {"NONE","WEP","WEP","WEP","WPA-TKIP","WPA-AES","WPA-AES","WPA2-TKIP","WPA2-AES","WPA2-AES"};
    char secEnable[16] = {0};
    char passPhrase[128] = {0};
    int auth = 0, encrypt = 0, securityType = 0, defkey = 0;
    int j = 0;

    MBridgeWirelessProfile mbBridgeToken =
    {
        .isEnable = {0},
        .SSID = {0},
        .AuthMode = {0},
        .EncrypType = {0},
        .WpaPsk = {0},
        .WpaPskType = {0},
        .DefaultKeyId = {0},
        .keyLength = {0},
        .KeyType = {0},
        .key = {[0 ... 3] = {0}}
    };

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    //BridgeWirelessProfile cannot set by MBridgeControler, we needn't check temp value
    APCFGCLI_CLI_GET(mbBridgeToken.isEnable, WLAN_R1_APCLI_ENABLE_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.SSID, WLAN_R1_APCLI_SSID_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.AuthMode, WLAN_R1_APCLI_AUTH_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.EncrypType, WLAN_R1_APCLI_ENC_TYPE_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.WpaPsk, WLAN_R1_APCLI_WPAPASSPHRASE_TOK);
    // WLAN_R1_APCLI_WPAPSK_KEYTYPE_TOK not use
    //APCFGCLI_CLI_GET(mbBridgeToken.WpaPskType, WLAN_R1_APCLI_WPAPSK_KEYTYPE_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.DefaultKeyId, WLAN_R1_APCLI_KEYID_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.keyLength, WLAN_R1_APCLI_WEP_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.KeyType, WLAN_R1_APCLI_KEYTYPE_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.key[0], WLAN_R1_APCLI_WEPKEY_0_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.key[1], WLAN_R1_APCLI_WEPKEY_1_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.key[2], WLAN_R1_APCLI_WEPKEY_2_TOK);
    APCFGCLI_CLI_GET(mbBridgeToken.key[3], WLAN_R1_APCLI_WEPKEY_3_TOK);

    encrypt=atoi(mbBridgeToken.EncrypType);
    auth=atoi(mbBridgeToken.AuthMode);

    strcpy(secEnable, "true");
    switch(encrypt)
    {
        case WLAN_ENC_NONE:
            securityType = WLAN_ENCRYPT_NONE;
            strcpy(secEnable, "false");
            break;
        case WLAN_ENC_WEP:
            securityType = (auth==WLAN_AUTH_OPEN)?WLAN_ENCRYPT_WEPOPEN:
                (auth==WLAN_AUTH_WEPAUTO)?WLAN_ENCRYPT_WEPAUTO:WLAN_ENCRYPT_WEPSHARED;
            break;
        case WLAN_ENC_TKIP:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPATKIP:WLAN_ENCRYPT_WPA2TKIP;
            break;
        case WLAN_ENC_AES:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPAAES:WLAN_ENCRYPT_WPA2AES;
            break;
        case WLAN_ENC_TKIPAES:
        default:
            securityType = (auth==WLAN_AUTH_WPAPSK)?WLAN_ENCRYPT_WPAMIX:WLAN_ENCRYPT_WPA2MIX;
            break;
    }

    switch(encrypt)
    {
        case WLAN_ENC_WEP:
            if(mbBridgeToken.DefaultKeyId)
                defkey = atoi(mbBridgeToken.DefaultKeyId);
            strcpy(passPhrase, mbBridgeToken.key[defkey]);
            break;
        case WLAN_ENC_TKIP:
        case WLAN_ENC_AES:
        case WLAN_ENC_TKIPAES:
        default:
            strcpy(passPhrase, mbBridgeToken.WpaPsk);
            break;
    }

    pOutSSID = escapeXMLString(mbBridgeToken.SSID);
    j = sprintf(resultTmp, "<wirelessProfile SSID=\"%s\"", pOutSSID);
    j += sprintf(resultTmp+j, "<wirelessSecurity enabled=\"%s\">", secEnable);
    j += sprintf(resultTmp+j, "<Mode passPhrase=\"%s\">%s</Mode>", passPhrase, supportEncrypt[securityType]);
    j += sprintf(resultTmp+j, "</wirelessSecurity></wirelessProfile>");
    pOutString = escapeXMLString(resultTmp);

    // TODO: 5G profile how to show?
    j = sprintf(resultStr, "<u:GetBridgeWirelessProfileResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">");
    j += sprintf(resultStr+j, "<BridgeWirelessProfile>");
    j += sprintf(resultStr+j, "%s", pOutString);
    j += sprintf(resultStr+j, "</BridgeWirelessProfile>");
    j += sprintf(resultStr+j, "</u:GetBridgeWirelessProfileResponse>");

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    if(pOutString) free(pOutString);
    if(pOutSSID) free(pOutSSID);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetFriendlyName
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetFriendlyName(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char token[32] = {0};
    char *pOutString = NULL;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    if(strlen(mbConfigToken.friendlyName))
    {
        strcpy(token, mbConfigToken.friendlyName);
    }
    else
    {
        APCFGCLI_CLI_GET(token, MRHESS_FRIENDLY_NAME_TOK);
    }
    pOutString = escapeXMLString(token);
    snprintf(resultStr, RESULT_LEN,
            "<u:GetFriendlyNameResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<friendlyName>%s</friendlyName>"
            "</u:GetFriendlyNameResponse>",
            pOutString);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }
    if(pOutString) free(pOutString);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    SetFriendlyName
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int SetFriendlyName(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pFriendlyName = NULL;
    char tmp[8] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        pFriendlyName = GetFirstDocumentItem(ca_event->ActionRequest, "friendlyName");
        strcpy(mbConfigToken.friendlyName, pFriendlyName);
        snprintf(resultStr, RESULT_LEN,
                "<u:SetFriendlyNameResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                "</u:SetFriendlyNameResponse>");

        if((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }
    else //if(!strcmp(tmp, "none")) or "invalid"
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pFriendlyName) free(pFriendlyName);

    return(ca_event->ErrCode);

}
/*****************************************************************
 * NAME:    GetSSIDPriority
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetSSIDPriority(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    char resultTmp[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pOutString = NULL;
    char token[2][8][8] = {[0]={[0 ... 7]=0}, [1]={[0 ... 7]=0}}; //[SSID][CATNUM][RET_INT]
    char accessCategory[2][64] = {[0]={0}, [1]={0}}; //[SSID][RET_STRING]
    int val = 0;
    int j = 0;
    int m = 0, n = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
#if HAS_SUPPORT_PRIORITY_TO_ACCESS_CATEGORY_MAPPING
    for (m=0; m<2; m++)
    {
        for (n=0; n<8; n++)
        {
            if(mbSSIDPriorityModified == 0)
            {
                APCFGCLI_CLI_GET(token[m][n], WLAN_ _R1_ "%d" _AP_ "%d" _USER_TO_AC_TOK, m+1, n+1);
                switch (atoi(token[m][n]))
                {
                    case USERPRIORITY_TO_AC_VO:
                        strcat(accessCategory[m], "AC_VO");
                        break;
                    case USERPRIORITY_TO_AC_VI:
                        strcat(accessCategory[m], "AC_VI");
                        break;
                    case USERPRIORITY_TO_AC_BE:
                        strcat(accessCategory[m], "AC_BE");
                        break;
                    default:
                    case USERPRIORITY_TO_AC_BK:
                        strcat(accessCategory[m], "AC_BK");
                        break;
                }
            }
            else
            {
                strcat(accessCategory[m], mbSSIDPriorityToken[m][n]);
            }

            if(n!=7)
                strcat(accessCategory[m], ",");
        }
    }
    j = sprintf(resultTmp, "<SSIDNo>%d</SSIDNo>", 1);
    j += sprintf(resultTmp+j, "<UserPriorityToAccessCategory>%s</UserPriorityToAccessCategory>", accessCategory[0]);
    j += sprintf(resultTmp+j, "<SSIDNo>%d</SSIDNo>", 2);
    j += sprintf(resultTmp+j, "<UserPriorityToAccessCategory>%s</UserPriorityToAccessCategory>", accessCategory[1]);
    pOutString = escapeXMLString(resultTmp);
#endif
    j = sprintf(resultStr, "<u:GetSSIDPriorityResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">");
    //j += sprintf(resultStr+j, "<SSIDPriorities>");
    j += sprintf(resultStr+j, "<SSIDPriority>");
#if HAS_SUPPORT_PRIORITY_TO_ACCESS_CATEGORY_MAPPING
    j += sprintf(resultStr+j, "%s", pOutString);
#endif
    j += sprintf(resultStr+j, "</SSIDPriority>");
    //j += sprintf(resultStr+j, "</SSIDPriorities>");
    j += sprintf(resultStr+j, "</u:GetSSIDPriorityResponse>");

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    if(pOutString) free(pOutString);

    return(ca_event->ErrCode);

}
/*****************************************************************
 * NAME:    SetSSIDPriority
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int SetSSIDPriority(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pSSIDPriority = NULL;
    char tmp[8] = {0};
    char tailString[64]= {0};
    int ret = 0, m = 0, n = 0;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        pSSIDPriority = GetFirstDocumentItem(ca_event->ActionRequest, "SSIDPriority");

#if HAS_SUPPORT_PRIORITY_TO_ACCESS_CATEGORY_MAPPING
        ret = sscanf(pSSIDPriority,
                "<SSIDNo>1</SSIDNo><UserPriorityToAccessCategory>"
                "%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^<]"
                "</UserPriorityToAccessCategory>"
                "<SSIDNo>2</SSIDNo><UserPriorityToAccessCategory>"
                "%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^,],%6[^<]%42s",
                mbSSIDPriorityToken[0][0], mbSSIDPriorityToken[0][1], mbSSIDPriorityToken[0][2], mbSSIDPriorityToken[0][3],
                mbSSIDPriorityToken[0][4], mbSSIDPriorityToken[0][5], mbSSIDPriorityToken[0][6], mbSSIDPriorityToken[0][7],
                mbSSIDPriorityToken[1][0], mbSSIDPriorityToken[1][1], mbSSIDPriorityToken[1][2], mbSSIDPriorityToken[1][3],
                mbSSIDPriorityToken[1][4], mbSSIDPriorityToken[1][5], mbSSIDPriorityToken[1][6], mbSSIDPriorityToken[1][7],
                tailString);

        if(ret == 17 && !strcmp(tailString, "</UserPriorityToAccessCategory>"))
        {
            //TODO: check value
            mbSSIDPriorityModified = 1;
            snprintf(resultStr, RESULT_LEN,
                    "<u:SetSSIDPriorityResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                    "</u:SetSSIDPriorityResponse>");
            if((result = ixmlParseBuffer(resultStr)) != NULL)
            {
                ca_event->ActionResult = result;
                ca_event->ErrCode = UPNP_E_SUCCESS;
            }
            else
            {
                ca_event->ActionResult = NULL;
                ca_event->ErrCode = 501;
                strcpy(ca_event->ErrStr, "Action Failed");
            }
        }
        else
        {
            mbSSIDPriorityModified = 0;
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 402;
            strcpy(ca_event->ErrStr, "Invalid Args");
        }
#endif
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pSSIDPriority) free(pSSIDPriority);

    return(ca_event->ErrCode);

}
/*****************************************************************
 * NAME:    GetMobileConfigURL
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetMobileConfigURL(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char mobileurl[64] = {0};
    char apcfgCliReturnString[APCFGCLI_COMMAND_LEN] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    strcpy(mobileurl, "http://");
    sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "apcfgCli_cli lan get");
    strcat(mobileurl, upnp_search_setting_value(apcfgCliReturnString, "Ip"));
    strcat(mobileurl, "/mrhess.mobileconfig");

    snprintf(resultStr, RESULT_LEN,
            "<u:GetMobileConfigURLResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<URL>%s</URL>"
            "</u:GetMobileConfigURLResponse>",
            mobileurl);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    SetConfigurationStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetFirmwareVersion(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    snprintf(resultStr, RESULT_LEN,
            "<u:GetFirmwareVersionResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<firmwareVersion>%d.%d.%d</firmwareVersion>"
            "</u:GetFirmwareVersionResponse>",
            APPS_MAJOR_VERSION__, APPS_MINOR_VERSION__, APPS_RELEASE_VERSION__);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    UpdateFirmware
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int UpdateFirmware(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char *pURL = NULL;
    char *pSize = NULL;
    char *pDate = NULL;
    char *pChecksum = NULL;
    char tmp[8] = {0};
    char md5[64] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);

    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    pURL = GetFirstDocumentItem(ca_event->ActionRequest, "URL");
    pSize = GetFirstDocumentItem(ca_event->ActionRequest, "size");
    pDate = GetFirstDocumentItem(ca_event->ActionRequest, "date");
    pChecksum = GetFirstDocumentItem(ca_event->ActionRequest, "checksum");

    if(!(pURL == NULL || pSize == NULL || pDate == NULL || pChecksum == NULL))
    {
        sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);
    }
    else
    {
        strcpy(tmp, "invalid");
    }

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        //TODO: check pURL, pSize, pDate, pChecksum value
        /***********************************
         *   UPDATE_INITIAL
         *   UPDATE_IN_PROGRESS
         *   UPDATE_DONE
         *   UPDATE_DOWNLOAD_FAILED
         *   UPDATE_CANCELLED
         ***********************************/
        unsigned int iMtdSize;
        int iMtdNum=-1;

        sysGetMtdNumByName("Kernel",&iMtdNum,&iMtdSize);

        // write url to file, avoid special character error
        FILE *fp;
        if ((fp = fopen(FWURLFILE, "w+")) != NULL)
        {
            fprintf(fp, "%s", pURL);
            fclose(fp);
            //dmFwUpgrade.sh download_progress_file status_file md5string dev_mtdX
            SYSTEM("dmFwUpgrade.sh %s %s %s %s %s%d 0x%x 0x%x&", FWURLFILE, FWPROGRESS, FWSTATUS, pChecksum, DEV_MTD, iMtdNum, VENDOR_ID, PRODUCT_ID);
        }

        snprintf(resultStr, RESULT_LEN,
                "<u:GetMobileConfigURLResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
                "</u:GetMobileConfigURLResponse>");

        if((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            //start to notify event
            strcpy(dmevent.DevUDN, ca_event->DevUDN);
            strcpy(dmevent.ServiceID, ca_event->ServiceID);
            dmevent.lease_time = 2;
            if(dmevent.event_id>=0)
            {
                dm_cancelExpiration(dmevent.event_id);
            }
            dmevent.event_id=dm_scheduleExpiration(&dmevent);

            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }
    else if(!strcmp(tmp, "invalid"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 402;
        strcpy(ca_event->ErrStr, "Invalid Args");
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);
    if (pURL) free(pURL);
    if (pSize) free(pSize);
    if (pDate) free(pDate);
    if (pChecksum) free(pChecksum);

    return(ca_event->ErrCode);

}
/*****************************************************************
 * NAME:    CancelFirmwareUpdate
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int CancelFirmwareUpdate(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char cancelProgress[8] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    sysinteract(cancelProgress, sizeof(cancelProgress), "dmFwUpgrade.sh %s %s %s", "CancelFirmwareUpdate", FWPROGRESS, FWSTATUS);
    snprintf(resultStr, RESULT_LEN,
            "<u:CancelFirmwareUpdateResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "</u:CancelFirmwareUpdateResponse>");

    if(!strcmp(cancelProgress, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }
    else if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        //cancel notify event
        dm_cancelExpiration(dmevent.event_id);
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetUpdateProgress
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetUpdateProgress(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    strcpy(updateProgress, GetNewUpdateProgress());

    snprintf(resultStr, RESULT_LEN,
            "<u:GetUpdateProgressResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<progress>%s</progress>"
            "</u:GetUpdateProgressResponse>",
            updateProgress);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    GetUpdateStatus
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int GetUpdateStatus(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;

    strcpy(updateStatus, GetNewUpdateStatus());
    snprintf(resultStr, RESULT_LEN,
            "<u:GetUpdateStatusResponse xmlns:u=\"urn:schemas-denon-com:service:MBridgeControl:1\">"
            "<UpdateStatus>%s</UpdateStatus>"
            "</u:GetUpdateStatusResponse>",
            updateStatus);

    if ((result = ixmlParseBuffer(resultStr)) != NULL)
    {
        ca_event->ActionResult = result;
        ca_event->ErrCode = UPNP_E_SUCCESS;
    }
    else
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 501;
    }

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    ApplyChanges
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int ApplyChanges(struct Upnp_Action_Request *ca_event)
{
    char resultStr[RESULT_LEN] = {0};
    IXML_Document *result = NULL;
    char *pConfigurationToken = NULL;
    char *pConvConfigurationToken = NULL;
    char tmp[8] = {0};

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    pConfigurationToken = GetFirstDocumentItem(ca_event->ActionRequest, "configurationToken");
    pConvConfigurationToken = convInputString(pConfigurationToken);
    sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh check \"%s\"", pConvConfigurationToken);

    if(!strcmp(tmp, "fail"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 801;
        strcpy(ca_event->ErrStr, "Configuration locked");
    }
    else if(!strcmp(tmp, "pass"))
    {
        //save change and reload modules
        setMBridgeTokenInfo();
        initMBridgeTokenInfo();
        sysinteract(tmp, sizeof(tmp), "GetConfigurationToken.sh unlock");
        snprintf(resultStr, RESULT_LEN,
                "<u:SetApplyChangesResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">"
                "</u:SetApplyChangesResponse>");

        if((result = ixmlParseBuffer(resultStr)) != NULL)
        {
            ca_event->ActionResult = result;
            ca_event->ErrCode = UPNP_E_SUCCESS;
        }
        else
        {
            ca_event->ActionResult = NULL;
            ca_event->ErrCode = 501;
            strcpy(ca_event->ErrStr, "Action Failed");
        }
    }
    else if(!strcmp(tmp, "invalid"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 402;
        strcpy(ca_event->ErrStr, "Invalid Args");
    }
    else //if(!strcmp(tmp, "none"))
    {
        ca_event->ActionResult = NULL;
        ca_event->ErrCode = 802;
        strcpy(ca_event->ErrStr, "Unsafe access");
    }

    if (pConfigurationToken) free(pConfigurationToken);
    if (pConvConfigurationToken) free(pConvConfigurationToken);

    return(ca_event->ErrCode);
}
/*****************************************************************
 * NAME:    dm_scheduleExpiration
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int dm_scheduleExpiration(struct dmlist *node)
{
    ThreadPoolJob job;
    struct dm_expiration_event *event;

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    event = (struct dm_expiration_event *)malloc(
            sizeof(struct dm_expiration_event));
    if(event == NULL)
    {
        return 0;
    }

    event->node = node;

    TPJobInit( &job, ( start_routine ) dm_expiration, event );
    TPJobSetFreeFunction( &job, ( free_routine ) dm_freeEvent );

    if( TimerThreadSchedule(&gExpirationTimerThread,
                event->node->lease_time,
                REL_SEC,
                &job,
                SHORT_TERM,
                &(event->event_id))
            != UPNP_E_SUCCESS )
    {
        free( event );
        return 0;
    }
    node->event_id = event->event_id;

    return event->event_id;
}
/*****************************************************************
 * NAME:    dm_expiration
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void dm_expiration(void *input)
{
    IXML_Document *propSet = NULL;
    struct dm_expiration_event *event = (struct dm_expiration_event * ) input;
    char NewConfigurationStatus[8];
    char NewUpdateProgress[8];
    char NewUpdateStatus[32];

    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    ithread_mutex_lock(&DevMutex);
    strcpy(NewConfigurationStatus, GetNewConfigurationStatus());
    strcpy(NewUpdateProgress, GetNewUpdateProgress());
    strcpy(NewUpdateStatus, GetNewUpdateStatus());

    if(strcmp(NewConfigurationStatus, configurationStatus))
    {
        dprintf("Notify:configurationStatus change to %s\n", NewConfigurationStatus);
        strcpy(configurationStatus, NewConfigurationStatus);
        UpnpAddToPropertySet(&propSet, "ConfigurationStatus", configurationStatus);
    }

    if(strcmp(NewUpdateProgress, updateProgress))
    {
        dprintf("Notify:updateProgress change to %s\n", NewUpdateProgress);
        strcpy(updateProgress, NewUpdateProgress);
        UpnpAddToPropertySet(&propSet, "UpdateProgress", updateProgress);
    }

    if(strcmp(NewUpdateStatus, updateStatus))
    {
        dprintf("Notify:updateStatus change to %s\n", NewUpdateStatus);
        strcpy(updateStatus, NewUpdateStatus);
        UpnpAddToPropertySet(&propSet, "UpdateStatus", updateStatus);
    }

    UpnpNotifyExt(deviceHandle, event->node->DevUDN, event->node->ServiceID, propSet);
    ixmlDocument_free(propSet);
    dm_freeEvent(event);

    //XXX: loop.
    dm_scheduleExpiration(&dmevent);
    ithread_mutex_unlock(&DevMutex);
}
/*****************************************************************
 * NAME:    dm_freeEvent
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void dm_freeEvent(struct dm_expiration_event *event)
{
    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    if (event != NULL && event->node !=NULL)
        event->node->event_id = -1;
    free(event);
}
/*****************************************************************
 * NAME:    dm_cancelExpiration
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int dm_cancelExpiration(int expirationEventId)
{
    ThreadPoolJob job;
    dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    if (expirationEventId<0)
    {
        dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
        return 1;
    }
    if (TimerThreadRemove(&gExpirationTimerThread,expirationEventId,&job)==0) {
        dm_freeEvent((struct dm_expiration_event *)job.arg);
    }
    else {
        dprintf("[%s] %s\n", __FILE__, __FUNCTION__);
    }
    return 1;
}
