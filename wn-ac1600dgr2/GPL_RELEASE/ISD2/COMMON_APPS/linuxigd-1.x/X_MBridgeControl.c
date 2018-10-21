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
#include "X_MBridgeControl.h"
#include "globals.h"
#include "gatedevice.h"
#include "pmlist.h"
#include "util.h"

#define STR_REPLY_OK                                 "OK"
#define STR_REPLY_ERROR                              "ERROR"

#define MACFILTER_PATH "/tmp/apcfg_cli_firewall_macfilter_info"
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
/*****************************************************************
* NAME:    upnp_search_setting_value
* ---------------------------------------------------------------
* FUNCTION:	search the setting value by keyword
* INPUT:	context, keyword    	 
* OUTPUT:  	value 
* Author: 	Tim
* Modify:   
******************************************************************/
static char *upnp_search_setting_value(char *context, const char *keyword)
{
	char *ptr, *value_head, *value_end;
	static char value[256];

	memset(value, 0, sizeof(value));

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

				strncpy(value, value_head, (value_end-value_head));
				break;
			}
		}
	}
	return value;
}

//ToUpper
char *upper(char string[])
{
	int i;

	for (i=0; i<strlen(string); i++)
		string[i]=toupper(string[i]);

	return(string);
}

//ToLower
char *lower(char string[])
{
	int i;

	for (i=0; i<strlen(string); i++)
		string[i]=tolower(string[i]);

	return(string);
}

int GetLANsettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result;
	int j = 0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j = sprintf(resultStr,	"<u:GetLANsettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n");

	//apcfgCli_util_cmd(apcfgCliCommandString, apcfgCliReturnString);
	sprintf(apcfgCliCommandString, "apcfgCli_cli dhcp get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	j += sprintf(resultStr+j,	"<NewDHCPClient>%s</NewDHCPClient>\n", upnp_search_setting_value(apcfgCliReturnString, "Enable"));

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	sprintf(apcfgCliCommandString, "apcfgCli_cli lan get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	//apcfgCli_util_cmd(apcfgCliCommandString, apcfgCliReturnString);
	j += sprintf(resultStr+j,	"<NewIpAddress>%s</NewIpAddress>\n", upnp_search_setting_value(apcfgCliReturnString, "Ip"));
	j += sprintf(resultStr+j,	"<NewSubnetMask>%s</NewSubnetMask>\n", upnp_search_setting_value(apcfgCliReturnString, "Mask"));
	j += sprintf(resultStr+j,	"<NewDefaultGateway>%s</NewDefaultGateway>\n", upnp_search_setting_value(apcfgCliReturnString, "Gateway"));
	j += sprintf(resultStr+j,	"<NewDNSPrimary>%s</NewDNSPrimary>\n", upnp_search_setting_value(apcfgCliReturnString, "Dns1"));
	j += sprintf(resultStr+j,	"<NewDNSSecondary>%s</NewDNSSecondary>\n", upnp_search_setting_value(apcfgCliReturnString, "Dns2"));
	j += sprintf(resultStr+j,	"<NewMACAddress>%s</NewMACAddress>\n", upnp_search_setting_value(apcfgCliReturnString, "Mac"));
	//j += sprintf(resultStr+j,	"<NewStatus>%s</NewStatus>\n","OK");
	j += sprintf(resultStr+j,	"</u:GetLANsettingsResponse>");

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetLANsettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}
	return(ca_event->ErrCode);
}

int SetLANsettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *dhcpclient = NULL;
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	char *dns1 = NULL;
	char *dns2 = NULL;
	int j=0,cmdcount=0;

	memset(resultStr, 0, sizeof(resultStr));

	if(dhcpclient = GetFirstDocumentItem(ca_event->ActionRequest, "NewDHCPClient"))
	{
		memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
		memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

		sprintf(apcfgCliCommandString, "apcfgCli_cli dhcp set Enable:%s",dhcpclient);
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}

	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	j=sprintf(apcfgCliCommandString,"apcfgCli_cli lan set ");
	if(ip = GetFirstDocumentItem(ca_event->ActionRequest, "NewIpAddress"))
	{
		cmdcount++;
		j+=sprintf(j+apcfgCliCommandString, "Ip:%s ",ip);
	}
	if(mask = GetFirstDocumentItem(ca_event->ActionRequest, "NewSubnetMask"))
	{
		cmdcount++;
		j+=sprintf(j+apcfgCliCommandString, "Mask:%s ",mask);
	}
	if(gateway = GetFirstDocumentItem(ca_event->ActionRequest, "NewDefaultGateway"))
	{
		cmdcount++;
		j+=sprintf(j+apcfgCliCommandString, "Gateway:%s ",gateway);
	}
	if(dns1 = GetFirstDocumentItem(ca_event->ActionRequest, "NewDNSPrimary"))
	{
		cmdcount++;
		j+=sprintf(j+apcfgCliCommandString, "Dns1:%s ",dns1);
	}
	if(dns2 = GetFirstDocumentItem(ca_event->ActionRequest, "NewDNSSecondary"))
	{
		cmdcount++;
		j+=sprintf(j+apcfgCliCommandString, "Dns2:%s ",dns2);
	}
	if(cmdcount>0)
	{
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}
	
	
	snprintf(resultStr, RESULT_LEN, "<u:SetLANsettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									//"<NewStatus>%s</NewStatus>\n"
									"</u:SetLANsettingsResponse>");

	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to SetLANsettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (dhcpclient) free(dhcpclient);
	if (ip) free(ip);
	if (mask) free(mask);
	if (gateway) free(gateway);
	if (dns1) free(dns1);
	if (dns2) free(dns2);

	return(ca_event->ErrCode);
}

int GetWLANSecuritySettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result;
	int encrypt, auth, defaultKey;
	int j=0, securityType;

	char *supportEncrypt[] = {"NONE","WEP-OPEN","WEP-SHARED","WEP-AUTO","WPA-TKIP","WPA-AES","WPA-MIX","WPA2-TKIP","WPA2-AES","WPA2-MIX"};
	char *encrypt_key, str[10]={0};

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	
	j = sprintf(resultStr,	"<u:GetWLANSecuritySettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n");

	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 ssid[1] get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	j += sprintf(resultStr+j,	"<NewSSID>%s</NewSSID>\n", upnp_search_setting_value(apcfgCliReturnString, "Ssid"));
	j += sprintf(resultStr+j,	"<NewSSIDBroadcast>%s</NewSSIDBroadcast>\n", 
								(atoi(upnp_search_setting_value(apcfgCliReturnString, "BroadCast"))==0)?"1":"0");

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 ssid[1] security get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	encrypt = atoi(upnp_search_setting_value(apcfgCliReturnString, "EncType"));
	auth = atoi(upnp_search_setting_value(apcfgCliReturnString, "AuthType"));

	switch(encrypt)
	{
	case WLAN_ENC_NONE:
		securityType = WLAN_ENCRYPT_NONE;
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
	j += sprintf(resultStr+j,"<NewEncryptionType>%s</NewEncryptionType>\n", supportEncrypt[securityType]);
	switch(encrypt)
	{
	case WLAN_ENC_WEP:
		defaultKey = atoi(upnp_search_setting_value(apcfgCliReturnString, "DefaultTxKeyId"));
		sprintf(str, "Wepkey%d", defaultKey);
		encrypt_key = upnp_search_setting_value(apcfgCliReturnString, str);

		j += sprintf(resultStr+j,"<NewEncryptionKey>%s</NewEncryptionKey>\n", encrypt_key);
		break;
	case WLAN_ENC_TKIP:
	case WLAN_ENC_AES:
	case WLAN_ENC_TKIPAES:
	default:
		encrypt_key = upnp_search_setting_value(apcfgCliReturnString, "PskValue");
		j += sprintf(resultStr+j,"<NewEncryptionKey>%s</NewEncryptionKey>\n", encrypt_key);
		break;
	}

	//j += sprintf(resultStr+j,	"<NewStatus>%s</NewStatus>\n","OK");
	j += sprintf(resultStr+j,	"</u:GetWLANSecuritySettingsResponse>");

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetWLANSecuritySettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}
	return(ca_event->ErrCode);
}

int SetWLANSecuritySettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *ssid = NULL;
	char *broadcast = NULL;
	char *encrypttype = NULL;
	char *encryptkey = NULL;

	int encrypt, auth;
	int i,j=0,cmdcount=0,securityType;
	char *supportEncrypt[] = {"NONE","WEP-OPEN","WEP-SHARED","WEP-AUTO","WPA-TKIP","WPA-AES","WPA-MIX","WPA2-TKIP","WPA2-AES","WPA2-MIX"};

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j=sprintf(j+apcfgCliCommandString,"apcfgCli_cli wlan radio1 ssid[1] set ");

	if(ssid = GetFirstDocumentItem(ca_event->ActionRequest, "NewSSID"))
	{
		j+=sprintf(j+apcfgCliCommandString, "Ssid:%s ",ssid);
	}

	if(broadcast = GetFirstDocumentItem(ca_event->ActionRequest, "NewSSIDBroadcast"))
	{
		j+=sprintf(j+apcfgCliCommandString, "BroadCast:%s ",broadcast);
	}

	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		goto EXIT_SetWLANSecuritySettings;
	}

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	j=sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 ssid[1] security set ");

	if(encrypttype = GetFirstDocumentItem(ca_event->ActionRequest, "NewEncryptionType"))
	{
		for(i=0 ; i<sizeof(supportEncrypt)/sizeof(supportEncrypt[0]) ; i++)
		{
			if(strncmp(encrypttype, supportEncrypt[i], strlen(supportEncrypt[i])) ==0)
			{
				securityType = i;
			}
		}
		switch(securityType)
		{
		case WLAN_ENCRYPT_NONE:
			encrypt=WLAN_ENC_NONE;
			auth=WLAN_ENC_NONE;
			break;
		case WLAN_ENCRYPT_WEPOPEN:
			encrypt=WLAN_ENC_WEP;
			auth=WLAN_AUTH_OPEN;
			break;
		case WLAN_ENCRYPT_WEPSHARED:
			encrypt=WLAN_ENC_WEP;
			auth=WLAN_AUTH_SHARED;
			break;
		case WLAN_ENCRYPT_WEPAUTO:
			encrypt=WLAN_ENC_WEP;
			auth=WLAN_AUTH_WEPAUTO;
			break;
		case WLAN_ENCRYPT_WPATKIP:
			encrypt=WLAN_ENC_TKIP;
			auth=WLAN_AUTH_WPAPSK;
			break;
		case WLAN_ENCRYPT_WPA2AES:
			encrypt=WLAN_ENC_AES;
			auth=WLAN_AUTH_WPA2PSK;
			break;
		case WLAN_ENCRYPT_WPA2MIX:
			encrypt=WLAN_ENC_TKIPAES;
			auth=WLAN_AUTH_WPA1PSKWPA2PSK;
			break;
		}
		 
		j+=sprintf(j+apcfgCliCommandString, "EncType:%d ",encrypt);
		j+=sprintf(j+apcfgCliCommandString, "AuthType:%d ",auth);
	}
	if(encryptkey = GetFirstDocumentItem(ca_event->ActionRequest, "NewEncryptionKey"))
	{
		switch(securityType)
		{
		case WLAN_ENCRYPT_WEPOPEN:
		case WLAN_ENCRYPT_WEPSHARED:
		case WLAN_ENCRYPT_WEPAUTO:
			j+=sprintf(j+apcfgCliCommandString, "Length:64 ");
			j+=sprintf(j+apcfgCliCommandString, "KeyType:1 ");
			j+=sprintf(j+apcfgCliCommandString, "DefaultTxKeyId:1 ");
			j+=sprintf(j+apcfgCliCommandString, "Wepkey1:%s ",encryptkey);
			break;
		case WLAN_ENCRYPT_WPATKIP:
		case WLAN_ENCRYPT_WPA2AES:
		case WLAN_ENCRYPT_WPA2MIX:
			j+=sprintf(j+apcfgCliCommandString, "PskFormat:0 ");
			j+=sprintf(j+apcfgCliCommandString, "PskValue:%s ",encryptkey);
			break;
		}
	}
	
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

	snprintf(resultStr, RESULT_LEN, "<u:SetLANsettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:SetLANsettingsResponse>");

EXIT_SetWLANSecuritySettings:
	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to SetWLANSecuritySettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (ssid) free(ssid);
	if (broadcast) free(broadcast);
	if (encrypttype) free(encrypttype);
	if (encryptkey) free(encryptkey);

	return(ca_event->ErrCode);
}

int GetWLANAdvancedSettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result;
	int j=0, chan, autochan=-1;
	char *tmpStr;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j = sprintf(resultStr,	"<u:GetWLANAdvancedSettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n");

	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	
	chan = atoi(upnp_search_setting_value(apcfgCliReturnString, "Chan"));

	tmpStr = upnp_search_setting_value(apcfgCliReturnString, "AutoChan");
	if(tmpStr!=NULL)
	{
		autochan = atoi(tmpStr);
	}
	if(autochan==1)
	{
		j += sprintf(resultStr+j,"<NewChannelNumber>%s</NewChannelNumber>\n", "Auto");
	}
	else
	{
		j += sprintf(resultStr+j,"<NewChannelNumber>%d</NewChannelNumber>\n", chan);
	}

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 ssid[1] get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	j += sprintf(resultStr+j,	"<NewWMMEnabled>%s</NewWMMEnabled>\n", upnp_search_setting_value(apcfgCliReturnString, "Wmm"));

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 advance get");

	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

	j += sprintf(resultStr+j,	"<NewChannelWidth>%s</NewChannelWidth>\n", 
				 (atoi(upnp_search_setting_value(apcfgCliReturnString, "WlanNChanWidth"))==1)?"Auto 20/40 MHZ":"20 MHZ");
	j += sprintf(resultStr+j,	"<NewFragmentThreshold>%s</NewFragmentThreshold>\n", upnp_search_setting_value(apcfgCliReturnString, "FragThreshold"));
	j += sprintf(resultStr+j,	"<NewRTSThreshold>%s</NewRTSThreshold>\n", upnp_search_setting_value(apcfgCliReturnString, "RtsThreshold"));
	j += sprintf(resultStr+j,	"<NewBeaconInterval>%s</NewBeaconInterval>\n", upnp_search_setting_value(apcfgCliReturnString, "BeaconInterval"));
	j += sprintf(resultStr+j,	"<NewPreambleType>%s</NewPreambleType>\n", 
				 atoi(upnp_search_setting_value(apcfgCliReturnString, "Preamble"))==1?"Short":"Long");

	j += sprintf(resultStr+j,	"</u:GetWLANAdvancedSettingsResponse>");

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetWLANAdvancedSettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}
	return(ca_event->ErrCode);
}

int SetWLANAdvancedSettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *chan = NULL;
	char *chanWidth = NULL;
	char *wmm = NULL;
	char *fragThreshold = NULL;
	char *rtsThreshold = NULL;
	char *beaconInterval = NULL;
	char *preamble = NULL;
	int j=0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	if(chan = GetFirstDocumentItem(ca_event->ActionRequest, "NewChannelNumber"))
	{
		j=sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 set ");

		// set auto chan
		if(strcmp(chan,"Auto")==0)
		{
			j+=sprintf(j+apcfgCliCommandString, "AutoChan:1");
		}
		else
		{
			j+=sprintf(j+apcfgCliCommandString, "AutoChan:0 Chan:%s",chan);
		}
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}

	if(wmm = GetFirstDocumentItem(ca_event->ActionRequest, "NewWMMEnabled"))
	{
		memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
		memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

		sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 ssid[1] set Wmm:%s",wmm);
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	j=sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 advance set ");

	if(chanWidth = GetFirstDocumentItem(ca_event->ActionRequest, "NewChannelWidth"))
	{
		j+=sprintf(j+apcfgCliCommandString, "WlanNChanWidth:%s ",(strcmp(chanWidth,"20MHZ")==0)?"1":"0");
	}
	if(fragThreshold = GetFirstDocumentItem(ca_event->ActionRequest, "NewFragmentThreshold"))
	{
		j+=sprintf(j+apcfgCliCommandString, "FragThreshold:%s ",fragThreshold);
	}
	if(rtsThreshold = GetFirstDocumentItem(ca_event->ActionRequest, "NewRTSThreshold"))
	{
		j+=sprintf(j+apcfgCliCommandString, "RtsThreshold:%s ",rtsThreshold);
	}
	if(beaconInterval = GetFirstDocumentItem(ca_event->ActionRequest, "NewBeaconInterval"))
	{
		j+=sprintf(j+apcfgCliCommandString, "BeaconInterval:%s ",beaconInterval);
	}
	if(preamble = GetFirstDocumentItem(ca_event->ActionRequest, "NewPreambleType"))
	{
		j+=sprintf(j+apcfgCliCommandString, "Preamble:%s ",(strcmp(chanWidth,"Long")==0)?"2":"1");
	}

	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

	snprintf(resultStr, RESULT_LEN, "<u:SetLANsettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:SetLANsettingsResponse>");

	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to SetWLANAdvancedSettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (chan) free(chan);
	if (chanWidth) free(chanWidth);
	if (wmm) free(wmm);
	if (fragThreshold) free(fragThreshold);
	if (rtsThreshold) free(rtsThreshold);
	if (beaconInterval) free(beaconInterval);
	if (preamble) free(preamble);

	return(ca_event->ErrCode);
}

int GetACLSettings(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result;
	int j = 0, listcnt=0;
	int aclEnable=0,aclMode=0,enable;
	FILE *fp;
	char buf[256];
	char mac[18],maclist[RESULT_LEN];

	memset(resultStr, 0, sizeof(resultStr));
	memset(mac, 0, sizeof(mac));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j = sprintf(resultStr,	"<u:GetACLsettingsResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n");

	sprintf(apcfgCliCommandString, "apcfgCli_cli firewall macfilter get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	aclEnable = atoi(upnp_search_setting_value(apcfgCliReturnString, "Enable"));
	if (aclEnable==1)
	{
		//MAC Filter mode : Deny is 1, Allow is 0
		aclMode = atoi(upnp_search_setting_value(apcfgCliReturnString, "Mode"));
	}
	
	j += sprintf(resultStr+j,	"<NewACLControlMode>%s</NewACLControlMode>\n", (aclEnable==0)?"Disabled":(aclMode==1)?"Deny Listed":"Allowed Listed");

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	sprintf(apcfgCliCommandString, "apcfgCli_cli firewall macfilterlist get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

	fp = fopen(MACFILTER_PATH, "r");
	if (fp==NULL)
	{
		printf("failed to read %s\n", MACFILTER_PATH);
		goto EXIT_GetACLSettings;
	}
    while(fgets(buf, sizeof(buf), fp) != NULL) 
    {
		if(sscanf(buf,"\t%*s\tEnable:%d\tMac:%s\t%*s",&enable,mac)==2)
		{
			if (enable==1)
			{
				if (listcnt!=0)
				{
					listcnt+=sprintf(maclist+listcnt,",");
				}
				listcnt+=sprintf(maclist+listcnt,"%s",mac);
			}		
		}
	}
	fclose(fp);

	j += sprintf(resultStr+j,	"<NewACLList>%s</NewACLList>\n", maclist);
	j += sprintf(resultStr+j,	"</u:GetACLsettingsResponse>");

EXIT_GetACLSettings:
	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetACLsettings: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}
	return(ca_event->ErrCode);
}

int SetACLMode(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *aclMode = NULL;

	int j=0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	if(aclMode = GetFirstDocumentItem(ca_event->ActionRequest, "NewACLControlMode"))
	{
		j=sprintf(apcfgCliCommandString,"apcfgCli_cli firewall macfilter set ");

		j+=sprintf(j+apcfgCliCommandString, "Enable:%s ",(strcmp(aclMode,"Disabled")==0)?"0":"1");
		j+=sprintf(j+apcfgCliCommandString, "Mode:%s ",(strcmp(aclMode,"Allowed Listed")==0)?"0":"1");

		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}
	
	snprintf(resultStr, RESULT_LEN, "<u:SetACLModeResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:SetACLModeResponse>");

	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to SetACLMode: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (aclMode) free(aclMode);

	return(ca_event->ErrCode);
}

int AddToACL(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *mac = NULL;

	int j=0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	if(mac = GetFirstDocumentItem(ca_event->ActionRequest, "NewMACAddress"))
	{
		j=sprintf(apcfgCliCommandString,"apcfgCli_cli firewall macfilterlist set ");

		j+=sprintf(j+apcfgCliCommandString, "Action:1 ");
		j+=sprintf(j+apcfgCliCommandString, "Mac:%s ",mac);

		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}
	
	snprintf(resultStr, RESULT_LEN, "<u:AddToACLResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:AddToACLResponse>");

	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to AddToACL: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (mac) free(mac);

	return(ca_event->ErrCode);
}

int DeleteFromACL(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *mac = NULL;

	int j=0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	if(mac = GetFirstDocumentItem(ca_event->ActionRequest, "NewMACAddress"))
	{
		j=sprintf(apcfgCliCommandString,"apcfgCli_cli firewall macfilterlist set ");

		j+=sprintf(j+apcfgCliCommandString, "Action:0 ");
		j+=sprintf(j+apcfgCliCommandString, "Mac:%s ",mac);

		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}
	
	snprintf(resultStr, RESULT_LEN, "<u:DeleteFromACLResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:DeleteFromACLResponse>");

	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to DeleteFromACL: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (mac) free(mac);

	return(ca_event->ErrCode);
}

int DeleteAllFromACL(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;

	int j=0;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j=sprintf(apcfgCliCommandString,"apcfgCli_cli firewall macfilterlist set ");

	j+=sprintf(j+apcfgCliCommandString, "Action:2 ");

	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	
	snprintf(resultStr, RESULT_LEN, "<u:DeleteAllFromACLResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:DeleteAllFromACLResponse>");

	if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to DeleteAllFromACL: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	return(ca_event->ErrCode);
}

int GetCurrentWDSAPList(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result;
	int j = 0, listcnt=0, i;
	int wdsPhyMode=0,wdsNum;
	char wdslist[RESULT_LEN];

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	j = sprintf(resultStr,	"<u:GetCurrentWDSAPListResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n");

	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	wdsNum = atoi(upnp_search_setting_value(apcfgCliReturnString, "WdsNum"));
	wdsPhyMode = atoi(upnp_search_setting_value(apcfgCliReturnString, "WdsPhyMode"));

	listcnt+=sprintf(wdslist+listcnt,"Data Rate:%s,",(wdsPhyMode>=2)?"300M":(wdsPhyMode==1)?"54M":"11M");

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	for(i=1 ; i<=wdsNum ; i++)
	{
		sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds[%d] get",i);
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

		listcnt+=sprintf(wdslist+listcnt,"%s",upnp_search_setting_value(apcfgCliReturnString, "WlLinkMac"));
		if(i<wdsNum)
		{
			listcnt+=sprintf(wdslist+listcnt,",");
		}
	}


	j += sprintf(resultStr+j,	"<NewWDSAPList>%s</NewWDSAPList>\n", wdslist);
	j += sprintf(resultStr+j,	"</u:GetCurrentWDSAPListResponse>");

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetCurrentWDSAPList: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}
	return(ca_event->ErrCode);
}

int AddToWDSList(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *mac = NULL;
	int j=0, validIndex=0, i, wdsNum;

	memset(resultStr, 0, sizeof(resultStr));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	wdsNum = atoi(upnp_search_setting_value(apcfgCliReturnString, "WdsNum"));

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	for(i=1 ; i<=wdsNum ; i++)
	{
		sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds[%d] get",i);
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);

		if(strcmp(upnp_search_setting_value(apcfgCliReturnString, "WlLinkMac"),"000000000000")==0)
		{
			validIndex=i;
			break;
		}
	}

	// the table is full
	if(validIndex==0)
		goto EXIT_AddToWDSList;

	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	if(mac = GetFirstDocumentItem(ca_event->ActionRequest, "NewMACAddress"))
	{
		j=sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 wds[%d] set ",validIndex);
		j+=sprintf(j+apcfgCliCommandString, "WlLinkMac:%s ",mac);

		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	}
	
	snprintf(resultStr, RESULT_LEN, "<u:AddToWDSListResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
									"</u:AddToWDSListResponse>");

EXIT_AddToWDSList:
	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to AddToWDSList: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (mac) free(mac);

	return(ca_event->ErrCode);
}

int DeleteFromWDSList(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], apcfgCliCommandString[APCFGCLI_COMMAND_LEN], apcfgCliReturnString[APCFGCLI_RETURN_LEN];
	IXML_Document *result = NULL;
	char *mac = NULL;
	char tmp_mac[13]={0};
	int delIndex=0, i, wdsNum, len;
	unsigned int reg0, reg1, reg2, reg3, reg4, reg5;

	memset(resultStr, 0, sizeof(resultStr));
	memset(tmp_mac, 0, sizeof(tmp_mac));
	memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
	memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));

	sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds get");
	sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	wdsNum = atoi(upnp_search_setting_value(apcfgCliReturnString, "WdsNum"));

	if(mac = GetFirstDocumentItem(ca_event->ActionRequest, "NewMACAddress"))
	{
		len = strlen(mac);
		if(len==17)
		{
			if(sscanf(mac,"%02X:%02X:%02X:%02X:%02X:%02X", &reg0, &reg1, &reg2, &reg3, &reg4, &reg5)==6)
			{
				sprintf(tmp_mac,"%02X%02X%02X%02X%02X%02X",reg0,reg1,reg2,reg3,reg4,reg5);
			}
			else
				goto EXIT_DeleteFromWDSList;
		}
		else if(len==12)
		{
			sprintf(tmp_mac,"%s",mac);
			upper(tmp_mac);
		}
		else
			goto EXIT_DeleteFromWDSList;

		memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
		memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
		for(i=1 ; i<=wdsNum ; i++)
		{
			sprintf(apcfgCliCommandString, "apcfgCli_cli wlan radio1 wds[%d] get",i);
			sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
	
			if(strcmp(upnp_search_setting_value(apcfgCliReturnString, "WlLinkMac"),tmp_mac)==0)
			{
				delIndex=i;
				break;
			}
		}

		if(delIndex==0)
			goto EXIT_DeleteFromWDSList;

		memset(apcfgCliCommandString, 0, sizeof(apcfgCliCommandString));
		memset(apcfgCliReturnString, 0, sizeof(apcfgCliReturnString));
	
		sprintf(apcfgCliCommandString,"apcfgCli_cli wlan radio1 wds[%d] set WlLinkMac:000000000000",delIndex);
	
		sysinteract(apcfgCliReturnString, sizeof(apcfgCliReturnString), "%s", apcfgCliCommandString);
		
		snprintf(resultStr, RESULT_LEN, "<u:DeleteFromWDSListResponse xmlns:u=\"urn:schemas-upnp-org:service:WFADevice:1\">\n"
										"</u:DeleteFromWDSListResponse>");
	}
EXIT_DeleteFromWDSList:
	if(strncmp(apcfgCliReturnString,"command",7)==0)
	{
		trace(1, "Invalid Args");
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
	}
	else if((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		system("sysconf_cli applychanges checkAllModules &");
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to DeleteFromWDSList: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 501;
	}

	if (mac) free(mac);

	return(ca_event->ErrCode);
}
