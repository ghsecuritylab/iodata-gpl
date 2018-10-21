#include <syslog.h>
#include <stdlib.h>
#include <upnp/ixml.h>
#include <string.h>
#include <time.h>
#include <upnp/upnp.h>
#include <upnp/upnptools.h>
#include <upnp/TimerThread.h>
#include "X_MBridgeControl.h"
#if HAS_MBRIDGE_CONTROL
#include "MBridgeControl.h"
#endif
#include "globals.h"
#include "gatedevice.h"
#include "pmlist.h"
#include "util.h"

#include <stdio.h>
#if HAS_OFFLINE_DOWNLOAD
#define NEW_STATUS_OK                                1
#define NEW_STATUS_ERROR                             0

#define STR_REPLY_OK                                 "OK"
#define STR_REPLY_ERROR                              "ERROR"
#define STR_REPLY_OPERATION_FAILED                   "ERROR_OPERATION_FAILED"
#define STR_REPLY_SUCCESS                            "Success"
#define STR_REPLY_FAILURE                            "Failure"
#define STR_REPLY_NEW                                "new"

#define TEMPORAL_FOLDER                              "/tmp/"

#define TORRENT_LIST_FILE_NAME_LEN                   64
#define MAX_TORRENT_CONTENT_LEN                      40*1024
#endif
extern int gAllowToSetIptables; //adam: 2008-11-28

//Definitions for mapping expiration timer thread
TimerThread gExpirationTimerThread;
static ThreadPool gExpirationThreadPool;

// MUTEX for locking shared state variables whenver they are changed
ithread_mutex_t DevMutex = PTHREAD_MUTEX_INITIALIZER;

// Main event handler for callbacks from the SDK.  Determine type of event
// and dispatch to the appropriate handler (Note: Get Var Request deprecated
int EventHandler(Upnp_EventType EventType, void *Event, void *Cookie)
{
	switch (EventType)
	{
		case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			HandleSubscriptionRequest((struct Upnp_Subscription_Request *) Event);
			break;
		// -- Deprecated --
		case UPNP_CONTROL_GET_VAR_REQUEST:
			HandleGetVarRequest((struct Upnp_State_Var_Request *) Event);
			break;
		case UPNP_CONTROL_ACTION_REQUEST:
			HandleActionRequest((struct Upnp_Action_Request *) Event);
			break;
		default:
			trace(1, "Error in EventHandler: Unknown event type %d",
						EventType);
	}
	return (0);
}

// Grab our UDN from the Description Document.  This may not be needed, 
// the UDN comes with the request, but we leave this for other device initializations
int StateTableInit(char *descDocUrl)
{
	IXML_Document *ixmlDescDoc;
	int ret;

	if ((ret = UpnpDownloadXmlDoc(descDocUrl, &ixmlDescDoc)) != UPNP_E_SUCCESS)
	{
		syslog(LOG_ERR, "Could not parse description document. Exiting ...");
		UpnpFinish();
		exit(0);
	}

	// Get the UDN from the description document, then free the DescDoc's memory
	gateUDN = GetFirstDocumentItem(ixmlDescDoc, "UDN");
	ixmlDocument_free(ixmlDescDoc);
		
	// Initialize our linked list of port mappings.
	pmlist_Head = pmlist_Current = NULL;
	PortMappingNumberOfEntries = 0;
#if HAS_MBRIDGE_CONTROL
	strcpy(configurationStatus, GetNewConfigurationStatus());
	strcpy(updateProgress, GetNewUpdateProgress());
	strcpy(updateStatus, GetNewUpdateStatus());
#endif
	return (ret);
}

// Handles subscription request for state variable notifications
int HandleSubscriptionRequest(struct Upnp_Subscription_Request *sr_event)
{
	IXML_Document *propSet = NULL;
	
	ithread_mutex_lock(&DevMutex);

	if (strcmp(sr_event->UDN, gateUDN) == 0)
	{
		// WAN Common Interface Config Device Notifications
		if (strcmp(sr_event->ServiceId, "urn:upnp-org:serviceId:WANCommonIFC1") == 0)
		{
		        trace(3, "Recieved request to subscribe to WANCommonIFC1");
			UpnpAddToPropertySet(&propSet, "PhysicalLinkStatus", "Up");
			UpnpAcceptSubscriptionExt(deviceHandle, sr_event->UDN, sr_event->ServiceId,
						  propSet, sr_event->Sid);
			ixmlDocument_free(propSet);
		}
		// WAN IP Connection Device Notifications
		else if (strcmp(sr_event->ServiceId, "urn:upnp-org:serviceId:WANIPConn1") == 0)
		{
			GetIpAddressStr(ExternalIPAddress, g_vars.extInterfaceName);
			trace(3, "Received request to subscribe to WANIPConn1");
			UpnpAddToPropertySet(&propSet, "PossibleConnectionTypes","IP_Routed");
			UpnpAddToPropertySet(&propSet, "ConnectionStatus","Connected");
			UpnpAddToPropertySet(&propSet, "ExternalIPAddress", ExternalIPAddress);
			UpnpAddToPropertySet(&propSet, "PortMappingNumberOfEntries","0");
			UpnpAcceptSubscriptionExt(deviceHandle, sr_event->UDN, sr_event->ServiceId,
						  propSet, sr_event->Sid);
			ixmlDocument_free(propSet);
		}
#if HAS_OFFLINE_DOWNLOAD
		// Vuze Offline Download Notifications
		else if (strcmp(sr_event->ServiceId, "urn:upnp-org:serviceId:VuzeOfflineDownloaderService") == 0)
		{
			trace(3, "Received request to subscribe to VuzeOfflineDownloaderService");
			UpnpAddToPropertySet(&propSet, "Status","OK");
			UpnpAcceptSubscriptionExt(deviceHandle, sr_event->UDN, sr_event->ServiceId,
									  propSet, sr_event->Sid);
			ixmlDocument_free(propSet);
		}
#endif
#if HAS_UPNP_API
		// X_MBridge Control Notifications
		else if (strcmp(sr_event->ServiceId, "urn:upnp-org:serviceId:X_MBridgeControlService") == 0)
		{
			trace(3, "Received request to subscribe to X_MBridgeControlService");
			UpnpAddToPropertySet(&propSet, "Status","OK");
			UpnpAcceptSubscriptionExt(deviceHandle, sr_event->UDN, sr_event->ServiceId,
									  propSet, sr_event->Sid);
			ixmlDocument_free(propSet);
		}
#endif
#if HAS_MBRIDGE_CONTROL
		// MBridge Control Notifications
		else if (strcmp(sr_event->ServiceId, "urn:schemas-denon-com:serviceId:MBridgeControl") == 0)
		{
			trace(3, "Received request to subscribe to MBridgeControl");
			UpnpAddToPropertySet(&propSet, "ConfigurationStatus", configurationStatus);
			UpnpAddToPropertySet(&propSet, "UpdateProgress", updateProgress);
			UpnpAddToPropertySet(&propSet, "UpdateStatus", updateStatus);
			UpnpAcceptSubscriptionExt(deviceHandle, sr_event->UDN, sr_event->ServiceId,
									  propSet, sr_event->Sid);
			ixmlDocument_free(propSet);
		}
#endif
	}
	ithread_mutex_unlock(&DevMutex);
	return(1);
}

int HandleGetVarRequest(struct Upnp_State_Var_Request *gv_request)
{
	// GET VAR REQUEST DEPRECATED FROM UPnP SPECIFICATIONS 
	// Report this in debug and ignore requests.  If anyone experiences problems
	// please let us know.
        trace(3, "Deprecated Get Variable Request received. Ignoring.");
	return 1;
}

int HandleActionRequest(struct Upnp_Action_Request *ca_event)
{
	int result = 0;
	
	ithread_mutex_lock(&DevMutex);

	if (strcmp(ca_event->DevUDN, gateUDN) == 0)
	{
		// Common debugging info, hopefully gets removed soon.
	        trace(3, "ActionName = %s", ca_event->ActionName);
		
		if (strcmp(ca_event->ServiceID, "urn:upnp-org:serviceId:WANIPConn1") == 0)
		{
			if (strcmp(ca_event->ActionName,"GetConnectionTypeInfo") == 0)
			  result = GetConnectionTypeInfo(ca_event);
			else if (strcmp(ca_event->ActionName,"GetNATRSIPStatus") == 0)
			  result = GetNATRSIPStatus(ca_event);
			else if (strcmp(ca_event->ActionName,"SetConnectionType") == 0)
			  result = SetConnectionType(ca_event);
			else if (strcmp(ca_event->ActionName,"RequestConnection") == 0)
			  result = RequestConnection(ca_event);
			else if (strcmp(ca_event->ActionName,"AddPortMapping") == 0)
			  result = AddPortMapping(ca_event);
			else if (strcmp(ca_event->ActionName,"GetGenericPortMappingEntry") == 0)
			  result = GetGenericPortMappingEntry(ca_event);
			else if (strcmp(ca_event->ActionName,"GetSpecificPortMappingEntry") == 0)
			  result = GetSpecificPortMappingEntry(ca_event);
			else if (strcmp(ca_event->ActionName,"GetExternalIPAddress") == 0)
			  result = GetExternalIPAddress(ca_event);
			else if (strcmp(ca_event->ActionName,"DeletePortMapping") == 0)
			  result = DeletePortMapping(ca_event);
			else if (strcmp(ca_event->ActionName,"GetStatusInfo") == 0)
			  result = GetStatusInfo(ca_event);
	
			// Intentionally Non-Implemented Functions -- To be added later
			/*else if (strcmp(ca_event->ActionName,"RequestTermination") == 0)
				result = RequestTermination(ca_event);
			else if (strcmp(ca_event->ActionName,"ForceTermination") == 0)
				result = ForceTermination(ca_event);
			else if (strcmp(ca_event->ActionName,"SetAutoDisconnectTime") == 0)
				result = SetAutoDisconnectTime(ca_event);
			else if (strcmp(ca_event->ActionName,"SetIdleDisconnectTime") == 0)
				result = SetIdleDisconnectTime(ca_event);
			else if (strcmp(ca_event->ActionName,"SetWarnDisconnectDelay") == 0)
				result = SetWarnDisconnectDelay(ca_event);
			else if (strcmp(ca_event->ActionName,"GetAutoDisconnectTime") == 0)
				result = GetAutoDisconnectTime(ca_event);
			else if (strcmp(ca_event->ActionName,"GetIdleDisconnectTime") == 0)
				result = GetIdleDisconnectTime(ca_event);
			else if (strcmp(ca_event->ActionName,"GetWarnDisconnectDelay") == 0)
				result = GetWarnDisconnectDelay(ca_event);*/
			else result = InvalidAction(ca_event);
		}
		else if (strcmp(ca_event->ServiceID,"urn:upnp-org:serviceId:WANCommonIFC1") == 0)
		{
			if (strcmp(ca_event->ActionName,"GetTotalBytesSent") == 0)
				result = GetTotal(ca_event, STATS_TX_BYTES);
			else if (strcmp(ca_event->ActionName,"GetTotalBytesReceived") == 0)
				result = GetTotal(ca_event, STATS_RX_BYTES);
			else if (strcmp(ca_event->ActionName,"GetTotalPacketsSent") == 0)
				result = GetTotal(ca_event, STATS_TX_PACKETS);
			else if (strcmp(ca_event->ActionName,"GetTotalPacketsReceived") == 0)
				result = GetTotal(ca_event, STATS_RX_PACKETS);
			else if (strcmp(ca_event->ActionName,"GetCommonLinkProperties") == 0)
				result = GetCommonLinkProperties(ca_event);
			else 
			{
				trace(1, "Invalid Action Request : %s",ca_event->ActionName);
				result = InvalidAction(ca_event);
			}
		}
#if HAS_OFFLINE_DOWNLOAD
		else if (strcmp(ca_event->ServiceID, "urn:upnp-org:serviceId:VuzeOfflineDownloaderService") == 0)
		{
			if(strcmp(ca_event->ActionName, "Activate") == 0)
				result = ActivateOfflineDownload(ca_event);
			else if(strcmp(ca_event->ActionName, "AddDownload") == 0)
				result = AddOfflineDownload(ca_event);
			else if(strcmp(ca_event->ActionName, "AddDownloadChunked") == 0)
				result = AddOfflineDownloadChunked(ca_event);
			else if(strcmp(ca_event->ActionName, "GetFreeSpace") == 0)
				result = GetFreeSpace(ca_event);
			else if(strcmp(ca_event->ActionName, "RemoveDownload") == 0)
				result = RemoveOfflineDownload(ca_event);
			else if(strcmp(ca_event->ActionName, "SetDownloads") == 0)
				result = SetOfflineDownload(ca_event);
			else if(strcmp(ca_event->ActionName, "StartDownload") == 0)
				result = StartOfflineDownload(ca_event);
			else if(strcmp(ca_event->ActionName, "UpdateDownload") == 0)
				result = UpdateOfflineDownload(ca_event);
			else
				result = InvalidAction(ca_event);
		}
#endif
#if HAS_UPNP_API
		else if (strcmp(ca_event->ServiceID, "urn:upnp-org:serviceId:WFADeviceService") == 0)
		{
			if(strcmp(ca_event->ActionName, "GetLANsettings") == 0)
				result = GetLANsettings(ca_event);
			else if(strcmp(ca_event->ActionName, "SetLANsettings") == 0)
				result = SetLANsettings(ca_event);
			else if(strcmp(ca_event->ActionName, "GetWLANSecuritySettings") == 0)
				result = GetWLANSecuritySettings(ca_event);
			else if(strcmp(ca_event->ActionName, "SetWLANSecuritySettings") == 0)
				result = SetWLANSecuritySettings(ca_event);
			else if(strcmp(ca_event->ActionName, "GetWLANAdvancedSettings") == 0)
				result = GetWLANAdvancedSettings(ca_event);
			else if(strcmp(ca_event->ActionName, "SetWLANAdvancedSettings") == 0)
				result = SetWLANAdvancedSettings(ca_event);
			else if(strcmp(ca_event->ActionName, "GetACLSettings") == 0)
				result = GetACLSettings(ca_event);
			else if(strcmp(ca_event->ActionName, "SetACLMode") == 0)
				result = SetACLMode(ca_event);
			else if(strcmp(ca_event->ActionName, "AddToACL") == 0)
				result = AddToACL(ca_event);
			else if(strcmp(ca_event->ActionName, "DeleteFromACL") == 0)
				result = DeleteFromACL(ca_event);
			else if(strcmp(ca_event->ActionName, "DeleteAllFromACL") == 0)
				result = DeleteAllFromACL(ca_event);
			else if(strcmp(ca_event->ActionName, "GetCurrentWDSAPList") == 0)
				result = GetCurrentWDSAPList(ca_event);
			else if(strcmp(ca_event->ActionName, "AddToWDSList") == 0)
				result = AddToWDSList(ca_event);
			else if(strcmp(ca_event->ActionName, "DeleteFromWDSList") == 0)
				result = DeleteFromWDSList(ca_event);
			else
				result = InvalidAction(ca_event);
		}
#endif
#if HAS_MBRIDGE_CONTROL
		else if (strcmp(ca_event->ServiceID, "urn:schemas-denon-com:serviceId:MBridgeControl") == 0)
		{
			if(strcmp(ca_event->ActionName, "GetConfigurationStatus") == 0)
				result = GetConfigurationStatus(ca_event);
			else if(strcmp(ca_event->ActionName, "SetConfigurationStatus") == 0)
				result = SetConfigurationStatus(ca_event);
			else if(strcmp(ca_event->ActionName, "GetConfigurationToken") == 0)
				result = GetConfigurationToken(ca_event);
			else if(strcmp(ca_event->ActionName, "ReleaseConfigurationToken") == 0)
				result = ReleaseConfigurationToken(ca_event);
			else if(strcmp(ca_event->ActionName, "StartInvitation") == 0)
				result = StartInvitation(ca_event);
			else if(strcmp(ca_event->ActionName, "StopInvitation") == 0)
				result = StopInvitation(ca_event);
			else if(strcmp(ca_event->ActionName, "GetAccessPointList") == 0)
				result = GetAccessPointList(ca_event);
			else if(strcmp(ca_event->ActionName, "GetMRHESSBRIDGEID") == 0)
				result = GetMRHESSBRIDGEID(ca_event);
			else if(strcmp(ca_event->ActionName, "SetMRHESSBRIDGEID") == 0)
				result = SetMRHESSBRIDGEID(ca_event);
			else if(strcmp(ca_event->ActionName, "GetHomeAPWirelessProfile") == 0)
				result = GetHomeAPWirelessProfile(ca_event);
			else if(strcmp(ca_event->ActionName, "SetHomeAPWirelessProfile") == 0)
				result = SetHomeAPWirelessProfile(ca_event);
			else if(strcmp(ca_event->ActionName, "GetBridgeWirelessProfile") == 0)
				result = GetBridgeWirelessProfile(ca_event);
			else if(strcmp(ca_event->ActionName, "GetFriendlyName") == 0)
				result = GetFriendlyName(ca_event);
			else if(strcmp(ca_event->ActionName, "SetFriendlyName") == 0)
				result = SetFriendlyName(ca_event);
			else if(strcmp(ca_event->ActionName, "GetSSIDPriority") == 0)
				result = GetSSIDPriority(ca_event);
			else if(strcmp(ca_event->ActionName, "SetSSIDPriority") == 0)
				result = SetSSIDPriority(ca_event);
			else if(strcmp(ca_event->ActionName, "GetMobileConfigURL") == 0)
				result = GetMobileConfigURL(ca_event);
			else if(strcmp(ca_event->ActionName, "GetFirmwareVersion") == 0)
				result = GetFirmwareVersion(ca_event);
			else if(strcmp(ca_event->ActionName, "UpdateFirmware") == 0)
				result = UpdateFirmware(ca_event);
			else if(strcmp(ca_event->ActionName, "CancelFirmwareUpdate") == 0)
				result = CancelFirmwareUpdate(ca_event);
			else if(strcmp(ca_event->ActionName, "GetUpdateProgress") == 0)
				result = GetUpdateProgress(ca_event);
			else if(strcmp(ca_event->ActionName, "GetUpdateStatus") == 0)
				result = GetUpdateStatus(ca_event);
			else if(strcmp(ca_event->ActionName, "ApplyChanges") == 0)
				result = ApplyChanges(ca_event);
			else
				result = InvalidAction(ca_event);
		}
#endif
	}

	ithread_mutex_unlock(&DevMutex);

	return (result);
}

// Default Action when we receive unknown Action Requests
int InvalidAction(struct Upnp_Action_Request *ca_event)
{
        ca_event->ErrCode = 401;
        strcpy(ca_event->ErrStr, "Invalid Action");
        ca_event->ActionResult = NULL;
        return (ca_event->ErrCode);
}

// As IP_Routed is the only relevant Connection Type for Linux-IGD
// we respond with IP_Routed as both current type and only type
int GetConnectionTypeInfo (struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN];
	IXML_Document *result;

	snprintf(resultStr, RESULT_LEN,
		"<u:GetConnectionTypeInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewConnectionType>IP_Routed</NewConnectionType>\n"
		"<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>"
		"</u:GetConnectionTypeInfoResponse>");

   // Create a IXML_Document from resultStr and return with ca_event
   if ((result = ixmlParseBuffer(resultStr)) != NULL)
   {
      ca_event->ActionResult = result;
      ca_event->ErrCode = UPNP_E_SUCCESS;
   }
   else
   {
      trace(1, "Error parsing Response to GetConnectionTypeinfo: %s", resultStr);
      ca_event->ActionResult = NULL;
      ca_event->ErrCode = 402;
   }

	return(ca_event->ErrCode);
}

// Linux-IGD does not support RSIP.  However NAT is of course
// so respond with NewNATEnabled = 1
int GetNATRSIPStatus(struct Upnp_Action_Request *ca_event)
{
   char resultStr[RESULT_LEN];
	IXML_Document *result;

   snprintf(resultStr, RESULT_LEN, "<u:GetNATRSIPStatusResponse xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
      							"<NewRSIPAvailable>0</NewRSIPAvailable>\n"
									"<NewNATEnabled>1</NewNATEnabled>\n"
								"</u:GetNATRSIPStatusResponse>");

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;	
	}
   else
	{
	        trace(1, "Error parsing Response to GetNATRSIPStatus: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}


// Connection Type is a Read Only Variable as linux-igd is only
// a device that supports a NATing IP router (not an Ethernet
// bridge).  Possible other uses may be explored.
int SetConnectionType(struct Upnp_Action_Request *ca_event)
{
	// Ignore requests
	ca_event->ActionResult = NULL;
	ca_event->ErrCode = UPNP_E_SUCCESS;
	return ca_event->ErrCode;
}

// This function should set the state variable ConnectionStatus to
// connecting, and then return synchronously, firing off a thread
// asynchronously to actually change the status to connected.  However, here we
// assume that the external WAN device is configured and connected
// outside of linux igd.
int RequestConnection(struct Upnp_Action_Request *ca_event)
{
	
	IXML_Document *propSet = NULL;
	
	//Immediatley Set connectionstatus to connected, and lastconnectionerror to none.
	strcpy(ConnectionStatus,"Connected");
	strcpy(LastConnectionError, "ERROR_NONE");
	trace(2, "RequestConnection recieved ... Setting Status to %s.", ConnectionStatus);

	// Build DOM Document with state variable connectionstatus and event it
	UpnpAddToPropertySet(&propSet, "ConnectionStatus", ConnectionStatus);
	
	// Send off notifications of state change
	UpnpNotifyExt(deviceHandle, ca_event->DevUDN, ca_event->ServiceID, propSet);

	ca_event->ErrCode = UPNP_E_SUCCESS;
	return ca_event->ErrCode;
}


int GetCommonLinkProperties(struct Upnp_Action_Request *ca_event)
{
   char resultStr[RESULT_LEN];
	IXML_Document *result;
	char ipAddr[40];
	int wanLink = getWanLinkStatus();
        
	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN,
		"<u:GetCommonLinkPropertiesResponse xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewWANAccessType>Cable</NewWANAccessType>\n"
		"<NewLayer1UpstreamMaxBitRate>%s</NewLayer1UpstreamMaxBitRate>\n"
		"<NewLayer1DownstreamMaxBitRate>%s</NewLayer1DownstreamMaxBitRate>\n"
		"<NewPhysicalLinkStatus>%s</NewPhysicalLinkStatus>\n"
		"</u:GetCommonLinkPropertiesResponse>", 
			 wanLink ? g_vars.upstreamBitrate : "0",
			 wanLink ? g_vars.downstreamBitrate : "0",
			 wanLink ? "Up" : "Down");

   // Create a IXML_Document from resultStr and return with ca_event
   if ((result = ixmlParseBuffer(resultStr)) != NULL)
   {
      ca_event->ActionResult = result;
      ca_event->ErrCode = UPNP_E_SUCCESS;
   }
   else
   {
      trace(1, "Error parsing Response to GetCommonLinkProperties: %s", resultStr);
      ca_event->ActionResult = NULL;
      ca_event->ErrCode = 402;
   }

	return(ca_event->ErrCode);
}

/* get specified statistic from /proc/net/dev */
int GetTotal(struct Upnp_Action_Request *ca_event, stats_t stat)
{
	char dev[IFNAMSIZ], resultStr[RESULT_LEN];
	const char *methods[STATS_LIMIT] =
		{ "BytesSent", "BytesReceived", "PacketsSent", "PacketsReceived" };
	unsigned long stats[STATS_LIMIT];
	FILE *proc;
	IXML_Document *result;
	int read;
	
	proc = fopen("/proc/net/dev", "r");
	if (!proc)
	{
		fprintf(stderr, "failed to open\n");
		return 0;
	}

	/* skip first two lines */
	fscanf(proc, "%*[^\n]\n%*[^\n]\n");

	/* parse stats */
	do
		read = fscanf(proc, "%[^:]:%lu %lu %*u %*u %*u %*u %*u %*u %lu %lu %*u %*u %*u %*u %*u %*u\n", dev, &stats[STATS_RX_BYTES], &stats[STATS_RX_PACKETS], &stats[STATS_TX_BYTES], &stats[STATS_TX_PACKETS]);
	while (read != EOF && (read == 5 && strncmp(dev, g_vars.extInterfaceName, IFNAMSIZ) != 0));

	fclose(proc);

	snprintf(resultStr, RESULT_LEN,
		"<u:GetTotal%sResponse xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewTotal%s>%lu</NewTotal%s>\n"
		"</u:GetTotal%sResponse>", 
		methods[stat], methods[stat], stats[stat], methods[stat], methods[stat]);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing response to GetTotal: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return (ca_event->ErrCode);
}

// Returns connection status related information to the control points
int GetStatusInfo(struct Upnp_Action_Request *ca_event)
{
   long int uptime;
   char resultStr[RESULT_LEN];
	IXML_Document *result = NULL;
	int wanLink = getWanLinkStatus();
	int connStatus = 0;
	char wanIPAddr[20];

#if USE_MYGETTIMEOFDAY
   uptime = (mytime() - startup_time);
#else
   uptime = (time(NULL) - startup_time);
#endif

   connStatus = GetIpAddressStr(wanIPAddr, g_vars.extInterfaceName);

   if(connStatus == 1 && strcmp(wanIPAddr, "10.64.64.64") == 0)
   {
	   connStatus = 0;
   }
   
	snprintf(resultStr, RESULT_LEN,
		"<u:GetStatusInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:GetStatusInfo:1\">\n"
		"<NewConnectionStatus>%s</NewConnectionStatus>\n"
		"<NewLastConnectionError>ERROR_NONE</NewLastConnectionError>\n"
		"<NewUptime>%li</NewUptime>\n"
		"</u:GetStatusInfoResponse>", 
		connStatus ? "Connected" : "Disconnected", 
		wanLink == 0 ? 0 : connStatus == 0 ? 0 : uptime);
   
	// Create a IXML_Document from resultStr and return with ca_event
   if ((result = ixmlParseBuffer(resultStr)) != NULL)
   {
      ca_event->ActionResult = result;
      ca_event->ErrCode = UPNP_E_SUCCESS;
   }
   else
   {
     trace(1, "Error parsing Response to GetStatusInfo: %s", resultStr);
      ca_event->ActionResult = NULL;
      ca_event->ErrCode = 402;
   }

   return(ca_event->ErrCode);
}

// Add New Port Map to the IGD
int AddPortMapping(struct Upnp_Action_Request *ca_event)
{
	char *remote_host=NULL;
	char *ext_port=NULL;
	char *proto=NULL;
	char *int_port=NULL;
	char *int_ip=NULL;
	char *int_duration=NULL;
	char *bool_enabled=NULL;
	char *desc=NULL;
  	struct portMap *ret, *new;
	int result;
	char num[5]; // Maximum number of port mapping entries 9999
	IXML_Document *propSet = NULL;
	int action_succeeded = 0;
	char resultStr[RESULT_LEN];

#if HAS_IO_REMOTE_ACCESS_FUNC
	FILE *fd;
	char buf[24];
#endif

	//adam 2008-11-28: Do not set any iptable rules and response action request failed.
	if (gAllowToSetIptables == 0)
	{
	  trace(1, "Not allow to configure iptable rules: Action Request Failed!");
	  ca_event->ErrCode = 501;
	  strcpy(ca_event->ErrStr, "Action Request Failed");
	  ca_event->ActionResult = NULL;
	  return(ca_event->ErrCode);
	}
	//~adam

	if ( (ext_port = GetFirstDocumentItemPort(ca_event->ActionRequest, "NewExternalPort") )
	     && (proto = GetFirstDocumentItem(ca_event->ActionRequest, "NewProtocol") )
	     && (int_port = GetFirstDocumentItemPort(ca_event->ActionRequest, "NewInternalPort") )
	     && (int_ip = GetFirstDocumentItem(ca_event->ActionRequest, "NewInternalClient") )
	     && (int_duration = GetFirstDocumentItem(ca_event->ActionRequest, "NewLeaseDuration") )
	     && (bool_enabled = GetFirstDocumentItem(ca_event->ActionRequest, "NewEnabled") )
	     && (desc = GetFirstDocumentItem(ca_event->ActionRequest, "NewPortMappingDescription") ))
	{
	  remote_host = GetFirstDocumentItem(ca_event->ActionRequest, "NewRemoteHost");
		// If port map with the same External Port, Protocol, and Internal Client exists
		// then, as per spec, we overwrite it (for simplicity, we delete and re-add at end of list)
		// Note: This may cause problems with GetGernericPortMappingEntry if a CP expects the overwritten
		// to be in the same place.
		if ((ret = pmlist_Find(ext_port, proto, int_ip)) != NULL)
		{
				trace(3, "Found port map to already exist.  Replacing");
				pmlist_Delete(ret);
		}

#ifdef UPNPD_MAX_PORTMAPPING_LIMIT
        /* 2011-08-03 Norkay, check list size  */
		if(pmlist_Size() >= UPNPD_MAX_PORTMAPPING_LIMIT)
		{
			trace(3, "Found port %s map to already exist.", ext_port);
			result = 401;
		}
		else
#endif
		{
/* 20121026 Jason: add for IODATA remote link function
 * It's will return error when user want use same port as remote link port */
#if HAS_IO_REMOTE_ACCESS_FUNC
			fd = fopen("/tmp/remotelink_port", "r");
			if(fd)
			{
				fgets(buf, sizeof(buf), fd);
				fclose(fd);
			}
		
			if(atoi(buf) == atoi(ext_port))
			{
				trace(3, "Found port %s map to already used by remotelink funcion.", ext_port);
				result = 718;
			}
			else
#endif
			{	
				/* xbox 360 does not keep track of the port it redirects and will
				 * redirect another port when receiving ConflictInMappingEntry */
				if ((ret = pmlist_FindSpecific(ext_port, proto)) != NULL)
				{
					trace(3, "Found port %s map to already exist.", ext_port);
					result = 718;
				}
				else
				{
					new = pmlist_NewNode(atoi(bool_enabled), atol(int_duration), "", ext_port, int_port, proto, int_ip, desc); 
					result = pmlist_PushBack(new);
				}
			}
		}

		if (result==1)
		{
		        ScheduleMappingExpiration(new,ca_event->DevUDN,ca_event->ServiceID);
			sprintf(num, "%d", pmlist_Size());
			trace(3, "PortMappingNumberOfEntries: %d", pmlist_Size());
			UpnpAddToPropertySet(&propSet, "PortMappingNumberOfEntries", num);				
			UpnpNotifyExt(deviceHandle, ca_event->DevUDN, ca_event->ServiceID, propSet);
			ixmlDocument_free(propSet);
			trace(2, "AddPortMap: DevUDN: %s ServiceID: %s RemoteHost: %s Prot: %s ExtPort: %s Int: %s.%s",
					    ca_event->DevUDN,ca_event->ServiceID,remote_host, proto, ext_port, int_ip, int_port);
			action_succeeded = 1;
		}
		else
		{
			if (result==718)
			{
				trace(1,"Failure in GateDeviceAddPortMapping: RemoteHost: %s Prot:%s ExtPort: %s Int: %s.%s\n",
						    remote_host, proto, ext_port, int_ip, int_port);
				ca_event->ErrCode = 718;
				strcpy(ca_event->ErrStr, "ConflictInMappingEntry");
				ca_event->ActionResult = NULL;
			}
#ifdef UPNPD_MAX_PORTMAPPING_LIMIT
			/* 2011-08-03 Norkay, check list size  */
			else if(result == 401)
			{
				InvalidAction(ca_event);
			}
#endif
 		}
	}
	else
	{
	  trace(1, "Failiure in GateDeviceAddPortMapping: Invalid Arguments!");
	  trace(1, "  ExtPort: %s Proto: %s IntPort: %s IntIP: %s Dur: %s Ena: %s Desc: %s",
		ext_port, proto, int_port, int_ip, int_duration, bool_enabled, desc);
	  ca_event->ErrCode = 402;
	  strcpy(ca_event->ErrStr, "Invalid Args");
	  ca_event->ActionResult = NULL;
	}
	
	if (action_succeeded)
	{
		ca_event->ErrCode = UPNP_E_SUCCESS;
		snprintf(resultStr, RESULT_LEN, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>",
			ca_event->ActionName, "urn:schemas-upnp-org:service:WANIPConnection:1", "", ca_event->ActionName);
		ca_event->ActionResult = ixmlParseBuffer(resultStr);
	}

	if (ext_port) free(ext_port);
	if (int_port) free(int_port);
	if (proto) free(proto);
	if (int_ip) free(int_ip);
	if (int_duration) free(int_duration);
	if (bool_enabled) free(bool_enabled);
	if (desc) free(desc);
	if (remote_host) free(remote_host);

	return(ca_event->ErrCode);
}

int GetGenericPortMappingEntry(struct Upnp_Action_Request *ca_event)
{
	char *mapindex = NULL;
	struct portMap *temp;
	char result_param[RESULT_LEN];
	char resultStr[RESULT_LEN];
	int action_succeeded = 0;

	if ((mapindex = GetFirstDocumentItem(ca_event->ActionRequest, "NewPortMappingIndex")))
	{
		temp = pmlist_FindByIndex(atoi(mapindex));
		if (temp)
		{
			snprintf(result_param, RESULT_LEN, "<NewRemoteHost>%s</NewRemoteHost><NewExternalPort>%s</NewExternalPort><NewProtocol>%s</NewProtocol><NewInternalPort>%s</NewInternalPort><NewInternalClient>%s</NewInternalClient><NewEnabled>%d</NewEnabled><NewPortMappingDescription>%s</NewPortMappingDescription><NewLeaseDuration>%li</NewLeaseDuration>", temp->m_RemoteHost, temp->m_ExternalPort, temp->m_PortMappingProtocol, temp->m_InternalPort, temp->m_InternalClient, temp->m_PortMappingEnabled, temp->m_PortMappingDescription, temp->m_PortMappingLeaseDuration);
			action_succeeded = 1;
		}
      if (action_succeeded)
      {
         ca_event->ErrCode = UPNP_E_SUCCESS;
                   snprintf(resultStr, RESULT_LEN, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
                           "urn:schemas-upnp-org:service:WANIPConnection:1",result_param, ca_event->ActionName);
                   ca_event->ActionResult = ixmlParseBuffer(resultStr);
      }
      else
      {
         ca_event->ErrCode = 713;
			strcpy(ca_event->ErrStr, "SpecifiedArrayIndexInvalid");
			ca_event->ActionResult = NULL;
      }

   }
   else
   {
            trace(1, "Failure in GateDeviceGetGenericPortMappingEntry: Invalid Args");
            ca_event->ErrCode = 402;
                 strcpy(ca_event->ErrStr, "Invalid Args");
                 ca_event->ActionResult = NULL;
   }
	if (mapindex) free (mapindex);
	return (ca_event->ErrCode);
 	
}
int GetSpecificPortMappingEntry(struct Upnp_Action_Request *ca_event)
{
   char *ext_port=NULL;
   char *proto=NULL;
   char result_param[RESULT_LEN];
   char resultStr[RESULT_LEN];
   int action_succeeded = 0;
	struct portMap *temp;

   if ((ext_port = GetFirstDocumentItemPort(ca_event->ActionRequest, "NewExternalPort"))
      && (proto = GetFirstDocumentItem(ca_event->ActionRequest,"NewProtocol")))
   {
      if ((strcmp(proto, "TCP") == 0) || (strcmp(proto, "UDP") == 0))
      {
			temp = pmlist_FindSpecific (ext_port, proto);
			if (temp)
			{
				snprintf(result_param, RESULT_LEN, "<NewInternalPort>%s</NewInternalPort><NewInternalClient>%s</NewInternalClient><NewEnabled>%d</NewEnabled><NewPortMappingDescription>%s</NewPortMappingDescription><NewLeaseDuration>%li</NewLeaseDuration>",
            temp->m_InternalPort,
            temp->m_InternalClient,
            temp->m_PortMappingEnabled,
				temp->m_PortMappingDescription,
            temp->m_PortMappingLeaseDuration);
            action_succeeded = 1;
			}
         if (action_succeeded)
         {
            ca_event->ErrCode = UPNP_E_SUCCESS;
	    snprintf(resultStr, RESULT_LEN, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
		    "urn:schemas-upnp-org:service:WANIPConnection:1",result_param, ca_event->ActionName);
	    ca_event->ActionResult = ixmlParseBuffer(resultStr);
         }
         else
         {
            trace(2, "GateDeviceGetSpecificPortMappingEntry: PortMapping Doesn't Exist...");
	    ca_event->ErrCode = 714;
	    strcpy(ca_event->ErrStr, "NoSuchEntryInArray");
	    ca_event->ActionResult = NULL;
         }
      }
      else
      {
              trace(1, "Failure in GateDeviceGetSpecificPortMappingEntry: Invalid NewProtocol=%s\n",proto);
	      ca_event->ErrCode = 402;
	      strcpy(ca_event->ErrStr, "Invalid Args");
	      ca_event->ActionResult = NULL;
      }
   }
   else
   {
      trace(1, "Failure in GateDeviceGetSpecificPortMappingEntry: Invalid Args");
      ca_event->ErrCode = 402;
      strcpy(ca_event->ErrStr, "Invalid Args");
      ca_event->ActionResult = NULL;
   }

   return (ca_event->ErrCode);


}
int GetExternalIPAddress(struct Upnp_Action_Request *ca_event)
{
   char resultStr[RESULT_LEN];
	IXML_Document *result = NULL;

   ca_event->ErrCode = UPNP_E_SUCCESS;
   GetIpAddressStr(ExternalIPAddress, g_vars.extInterfaceName);
   snprintf(resultStr, RESULT_LEN, "<u:GetExternalIPAddressResponse xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
										"<NewExternalIPAddress>%s</NewExternalIPAddress>\n"
								"</u:GetExternalIPAddressResponse>", ExternalIPAddress);

   // Create a IXML_Document from resultStr and return with ca_event
   if ((result = ixmlParseBuffer(resultStr)) != NULL)
   {
      ca_event->ActionResult = result;
      ca_event->ErrCode = UPNP_E_SUCCESS;
   }
   else
   {
      trace(1, "Error parsing Response to ExternalIPAddress: %s", resultStr);
      ca_event->ActionResult = NULL;
      ca_event->ErrCode = 402;
   }

   return(ca_event->ErrCode);
}

int DeletePortMapping(struct Upnp_Action_Request *ca_event)
{
   char *ext_port=NULL;
   char *proto=NULL;
   int result=0;
   char num[5];
   char resultStr[RESULT_LEN];
   IXML_Document *propSet= NULL;
   int action_succeeded = 0;
	struct portMap *temp;

   if (((ext_port = GetFirstDocumentItemPort(ca_event->ActionRequest, "NewExternalPort")) &&
      (proto = GetFirstDocumentItem(ca_event->ActionRequest, "NewProtocol"))))
   {

     if ((strcmp(proto, "TCP") == 0) || (strcmp(proto, "UDP") == 0))
     {
       if ((temp = pmlist_FindSpecific(ext_port, proto)))
	 result = pmlist_Delete(temp);

         if (result==1)
         {
            trace(2, "DeletePortMap: Proto:%s Port:%s\n",proto, ext_port);
            sprintf(num,"%d",pmlist_Size());
            UpnpAddToPropertySet(&propSet,"PortMappingNumberOfEntries", num);
            UpnpNotifyExt(deviceHandle, ca_event->DevUDN,ca_event->ServiceID,propSet);
            ixmlDocument_free(propSet);
            action_succeeded = 1;
         }
         else
         {
            trace(1, "Failure in GateDeviceDeletePortMapping: DeletePortMap: Proto:%s Port:%s\n",proto, ext_port);
            ca_event->ErrCode = 714;
            strcpy(ca_event->ErrStr, "NoSuchEntryInArray");
            ca_event->ActionResult = NULL;
         }
      }
      else
      {
         trace(1, "Failure in GateDeviceDeletePortMapping: Invalid NewProtocol=%s\n",proto);
         ca_event->ErrCode = 402;
			strcpy(ca_event->ErrStr, "Invalid Args");
			ca_event->ActionResult = NULL;
      }
   }
   else
   {
		trace(1, "Failiure in GateDeviceDeletePortMapping: Invalid Arguments!");
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
		ca_event->ActionResult = NULL;
   }

   if (action_succeeded)
   {
      ca_event->ErrCode = UPNP_E_SUCCESS;
      snprintf(resultStr, RESULT_LEN, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>",
         ca_event->ActionName, "urn:schemas-upnp-org:service:WANIPConnection:1", "", ca_event->ActionName);
      ca_event->ActionResult = ixmlParseBuffer(resultStr);
   }

   if (ext_port) free(ext_port);
   if (proto) free(proto);

   return(ca_event->ErrCode);
}
#if HAS_OFFLINE_DOWNLOAD
// --------------------------- START VuzeOfflineDownloader --------------------------- //

/*****************************************************************
 * NAME: hexChar2Int
 * ---------------------------------------------------------------
 * FUNCTION: Transfer the hexadecimal character into corresponding integer.
 * INPUT:    Hexadecimal character.
 * OUTPUT:   Corresponding integer.
 * Author: Zack 2011-0527
 * Modify:
 ******************************************************************/
int hexChar2Int(char hex_char)
{
	switch(hex_char)
	{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'a':
		case 'A':
			return 10;
		case 'b':
		case 'B':
			return 11;
		case 'c':
		case 'C':
			return 12;
		case 'd':
		case 'D':
			return 13;
		case 'e':
		case 'E':
			return 14;
		case 'f':
		case 'F':
			return 15;
		default:
			return -1;
	}
}

/*****************************************************************
 * NAME: hexStr2Char
 * ---------------------------------------------------------------
 * FUNCTION: Transfer the hexadecimal byte into corresponding character. (ASCII Mapping)
 * INPUT:    Hexadecimal byte.
 * OUTPUT:   Corresponding character.
 * Author: Zack 2011-0527
 * Modify:
 ******************************************************************/
char hexStr2Char(char *hex_byte)
{
	return (hexChar2Int(hex_byte[0])*16+hexChar2Int(hex_byte[1]));
}

int ActivateOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL;
	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");

	sprintf(sysconfigCommandString, "offline_download activate_download %s", client_id);
	sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:ActivateResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:ActivateResponse>\n", STR_REPLY_OK);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to ActivateOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

int AddOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL, *torrent_hash = NULL, *torrent_data = NULL;
	char tmpCharacterBuf[4] = {0};
	char torrent_file_name[TORRENT_FILE_NAME_LEN];
	FILE *fp = NULL;
	int i = 0;

	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHash");
	torrent_data = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentData");

	if(torrent_data != NULL)
	{
		sprintf(torrent_file_name, "%s%s.torrent", TEMPORAL_FOLDER, torrent_hash);
		if((fp = fopen(torrent_file_name, "w")) != NULL)
		{
			/********** Transfer the content of torrent from byte format into character format. **********/
			// In the packet, the torrent file is stored as the byte string which is "6162636465.....,"
			// but it must be stored as character which is "abcde....." in torrent file.
			// So the transformation is necessary.
			while(torrent_data[2*i] != '\0')
			{
				tmpCharacterBuf[0] = torrent_data[2*i];
				tmpCharacterBuf[1] = torrent_data[2*i+1];
				tmpCharacterBuf[2] = '\0';
				fprintf(fp, "%c", hexStr2Char(tmpCharacterBuf));
				i++;
			}

			if(fp == NULL)
			{
				trace(1, "Error to open: %s", torrent_file_name);
				ca_event->ActionResult = NULL;
				ca_event->ErrCode = UPNP_SOAP_E_ACTION_FAILED;
			}
			else
				fclose(fp);

			sprintf(sysconfigCommandString, "offline_download add_download %s %s", client_id, torrent_hash);
			sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);
		}
	}

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:AddDownloadResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:AddDownloadResponse>\n", sysconfigReturnString);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to AddOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

int AddOfflineDownloadChunked(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL, *torrent_hash = NULL, *torrent_data = NULL, *chunk_offset = NULL, *total_length_str = NULL;
	char tmpCharacterBuf[4] = {0};
	char torrent_file_name[TORRENT_FILE_NAME_LEN];
	FILE *fp = NULL;
	int i = 0, offset = 0, total_length = 0, got_all_torrent = 0;
	IXML_Document *result = NULL;

	sprintf(sysconfigReturnString, "%s", STR_REPLY_OK);

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHash");
	torrent_data = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentData");
	chunk_offset = GetFirstDocumentItem(ca_event->ActionRequest, "NewChunkOffset");
	offset = atoi(chunk_offset)/2;
	total_length_str = GetFirstDocumentItem(ca_event->ActionRequest, "NewTotalLength");
	total_length = atoi(total_length_str);

	// The left content which is smaller MAX_TORRENT_CONTENT_LEN means that the packet is the last packet of AddDownloadchunked.
	if((total_length - offset*2) < MAX_TORRENT_CONTENT_LEN)
	{
		got_all_torrent = 1;
	}

	if(torrent_data != NULL)
	{
		sprintf(torrent_file_name, "%s%s.torrent", TEMPORAL_FOLDER, torrent_hash);
		if(!sysIsFileExisted(torrent_file_name))
		{
			sprintf(sysconfigCommandString, "touch %s", torrent_file_name);
			system(sysconfigCommandString);
			memset(sysconfigCommandString, 0, sizeof(sysconfigCommandString));
		}

		if((fp = fopen(torrent_file_name, "r+")) != NULL)
		{
			fseek(fp, offset, SEEK_SET);
			/********** Transfer the content of torrent from byte format into character format. **********/
			// In the packet, the torrent file is stored as the byte string which is "6162636465.....,"
			// but it must be stored as character which is "abcde....." in torrent file.
			// So the transformation is necessary.
			while(torrent_data[2*i] != '\0')
			{
				tmpCharacterBuf[0] = torrent_data[2*i];
				tmpCharacterBuf[1] = torrent_data[2*i+1];
				tmpCharacterBuf[2] = '\0';
				fprintf(fp, "%c", hexStr2Char(tmpCharacterBuf));
				i++;
			}

			if(fp)
				fclose(fp);

			if(got_all_torrent)
			{
				memset(sysconfigReturnString, 0, SYSCONF_RETURN_LEN);
				sprintf(sysconfigCommandString, "offline_download add_download_chunked %s %s", client_id, torrent_hash);
				sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);
			}
		}
		else
		{
			memset(sysconfigReturnString, 0, SYSCONF_RETURN_LEN);
			sprintf(sysconfigReturnString, "%s", STR_REPLY_OPERATION_FAILED);
			trace(1, "Error to open: %s", torrent_file_name);
			ca_event->ActionResult = NULL;
			ca_event->ErrCode = UPNP_SOAP_E_ACTION_FAILED;
		}
	}

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:AddDownloadChunkedResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:AddDownloadChunkedResponse>\n", sysconfigReturnString);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		memset(sysconfigReturnString, 0, SYSCONF_RETURN_LEN);
		sprintf(sysconfigReturnString, "%s", STR_REPLY_OPERATION_FAILED);
		trace(1, "Error parsing Response to AddOfflineDownloadChunked: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

int GetFreeSpace(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN];
	char sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL;
	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");

	sysconf_util_cmd("offline_download get_free_space", sysconfigReturnString);

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:GetFreeSpaceResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewFreeSpace>%s</NewFreeSpace>\n"
									"    </u:GetFreeSpaceResponse>\n", sysconfigReturnString);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to GetFreeSpace: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

// No correct packet. This packet is constructed by my opinion
int RemoveOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL, *torrent_hash = NULL;
	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHash");

	sprintf(sysconfigCommandString, "offline_download remove_download %s %s", client_id, torrent_hash);
	sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:RemoveDownloadResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:RemoveDownloadResponse>\n", sysconfigReturnString);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to RemoveOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

int SetOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char torrent_list_file_name[TORRENT_LIST_FILE_NAME_LEN];
	char *client_id = NULL, *torrent_hash_list = NULL;
	IXML_Document *result = NULL;
	FILE *fp = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash_list = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHashList");

	sprintf(torrent_list_file_name, "%s%s", TEMPORAL_FOLDER, client_id);

	// Remove pervious torrent list file if it existed.
	if(sysIsFileExisted(torrent_list_file_name))
	{
		sprintf(sysconfigCommandString, "rm -rf %s", torrent_list_file_name);
		system(sysconfigCommandString);
		memset(sysconfigCommandString, 0, SYSCONF_COMMAND_LEN);
	}

	// NULL torrent_hash_list means that no torrent can be stored into file.
	if(torrent_hash_list != NULL)
	{
		if((fp = fopen(torrent_list_file_name, "w")) != NULL)
		{
			fprintf(fp, "%s", torrent_hash_list);

			if(fp)
				fclose(fp);
		}
	}

	sprintf(sysconfigCommandString, "offline_download set_downloads %s", client_id);
	sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);

	// Return value of NewSetDownloadsResultList :
	// 1. torrent_hash_list which is NULL means that there exists no torrent to be set. So it will return an empty string ""
	//    which means there exists neighter new torrent nor old torrent.
	// 2. If the torrent is sent at the first time, then it will return 1 which means there exists a new torrent to be added.
	// 3. If the torrent had been sent, then it will return 0 which means the torrent had been added before.
	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:SetDownloadsResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewSetDownloadsResultList>%s</NewSetDownloadsResultList>\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:SetDownloadsResponse>\n", (torrent_hash_list)?sysconfigReturnString:"", STR_REPLY_OK);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to SetOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}
	
	return(ca_event->ErrCode);
}

int StartOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL, *torrent_hash = NULL;
	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHash");

	sprintf(sysconfigCommandString, "offline_download start_download %s %s", client_id, torrent_hash);
	sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:StartDownloadResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewDataPort>%d</NewDataPort>\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:StartDownloadResponse>\n", 51413, STR_REPLY_OK);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to UpdateOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

int UpdateOfflineDownload(struct Upnp_Action_Request *ca_event)
{
	char resultStr[RESULT_LEN], sysconfigCommandString[SYSCONF_COMMAND_LEN], sysconfigReturnString[SYSCONF_RETURN_LEN];
	char *client_id = NULL, *torrent_hash  = NULL, *piece_required_map = NULL;
	char piece_map_file_name[PIECE_MAP_FILE_NAME_LEN];
	char buf_have_map[MAX_PIECE_MAP_LEN] = {0};
	FILE *fp = NULL;
	int new_status = NEW_STATUS_ERROR;
	IXML_Document *result = NULL;

	client_id = GetFirstDocumentItem(ca_event->ActionRequest, "NewClientID");
	torrent_hash = GetFirstDocumentItem(ca_event->ActionRequest, "NewTorrentHash");
	piece_required_map = GetFirstDocumentItem(ca_event->ActionRequest, "NewPieceRequiredMap");

	if(piece_required_map != NULL)
	{
		sprintf(piece_map_file_name, "%s%s", TEMPORAL_FOLDER, torrent_hash);
		if((fp = fopen(piece_map_file_name, "w")) != NULL)
		{
			fprintf(fp, "%s ", piece_required_map);

			if(fp == NULL)
			{
				trace(1, "Error to open: %s", piece_map_file_name);
				ca_event->ActionResult = NULL;
				ca_event->ErrCode = UPNP_SOAP_E_ACTION_FAILED;
			}
			else
				fclose(fp);

			sprintf(sysconfigCommandString, "offline_download update_download %s %s", client_id, torrent_hash);
			sysconf_util_cmd(sysconfigCommandString, sysconfigReturnString);

			if(strncmp(sysconfigReturnString, STR_REPLY_ERROR, strlen(STR_REPLY_ERROR)) != 0)
			{
				new_status = NEW_STATUS_OK;
				if(((fp = fopen(sysconfigReturnString, "r")) != NULL))
				{
					fscanf(fp, "%s", buf_have_map);

					if(fp)
						fclose(fp);

					memset(sysconfigCommandString, 0, SYSCONF_COMMAND_LEN);
					sprintf(sysconfigCommandString, "rm %s", sysconfigReturnString);
					system(sysconfigCommandString);
				}
			}
		}
	}

	ca_event->ErrCode = UPNP_E_SUCCESS;
	snprintf(resultStr, RESULT_LEN, "    <u:UpdateDownloadResponse xmlns:u=\"urn:schemas-upnp-org:service:VuzeOfflineDownloader:1\">\n"
									"      <NewPieceHaveMap>%s</NewPieceHaveMap>\n"
									"      <NewStatus>%s</NewStatus>\n"
									"    </u:UpdateDownloadResponse>\n",
                                    (strlen(buf_have_map))?buf_have_map:"",
									(new_status)?STR_REPLY_OK:STR_REPLY_ERROR);

	// Create a IXML_Document from resultStr and return with ca_event
	if ((result = ixmlParseBuffer(resultStr)) != NULL)
	{
		ca_event->ActionResult = result;
		ca_event->ErrCode = UPNP_E_SUCCESS;
	}
	else
	{
		trace(1, "Error parsing Response to UpdateOfflineDownload: %s", resultStr);
		ca_event->ActionResult = NULL;
		ca_event->ErrCode = 402;
	}

	return(ca_event->ErrCode);
}

// ---------------------------- END VuzeOfflineDownloader ---------------------------- //
#endif
int ExpirationTimerThreadInit(void)
{
  int retVal;
  ThreadPoolAttr attr;
  TPAttrInit( &attr );
  TPAttrSetMaxThreads( &attr, MAX_THREADS );
  TPAttrSetMinThreads( &attr, MIN_THREADS );
  TPAttrSetJobsPerThread( &attr, JOBS_PER_THREAD );
  TPAttrSetIdleTime( &attr, THREAD_IDLE_TIME );

  if( ThreadPoolInit( &gExpirationThreadPool, &attr ) != UPNP_E_SUCCESS ) {
    return UPNP_E_INIT_FAILED;
  }

  if( ( retVal = TimerThreadInit( &gExpirationTimerThread,
				  &gExpirationThreadPool ) ) !=
      UPNP_E_SUCCESS ) {
    return retVal;
  }
  
  return 0;
}

int ExpirationTimerThreadShutdown(void)
{
  return TimerThreadShutdown(&gExpirationTimerThread);
}


void free_expiration_event(expiration_event *event)
{
  if (event->mapping!=NULL)
    event->mapping->expirationEventId = -1;
  free(event);
}

void ExpireMapping(void *input)
{
  char num[5]; // Maximum number of port mapping entries 9999
  IXML_Document *propSet = NULL;
  expiration_event *event = ( expiration_event * ) input;
    
  ithread_mutex_lock(&DevMutex);

  trace(2, "ExpireMapping: Proto:%s Port:%s\n",
		      event->mapping->m_PortMappingProtocol, event->mapping->m_ExternalPort);

  //reset the event id before deleting the mapping so that pmlist_Delete
  //will not call CancelMappingExpiration
  event->mapping->expirationEventId = -1;
  pmlist_Delete(event->mapping);
  
  sprintf(num, "%d", pmlist_Size());
  UpnpAddToPropertySet(&propSet, "PortMappingNumberOfEntries", num);
  UpnpNotifyExt(deviceHandle, event->DevUDN, event->ServiceID, propSet);
  ixmlDocument_free(propSet);
  trace(3, "ExpireMapping: UpnpNotifyExt(deviceHandle,%s,%s,propSet)\n  PortMappingNumberOfEntries: %s",
		      event->DevUDN, event->ServiceID, num);
  
  free_expiration_event(event);
  
  ithread_mutex_unlock(&DevMutex);
}

int ScheduleMappingExpiration(struct portMap *mapping, char *DevUDN, char *ServiceID)
{
  int retVal = 0;
  ThreadPoolJob job;
  expiration_event *event;
  time_t curtime = time(NULL);
   
	
  if (mapping->m_PortMappingLeaseDuration > 0) {
    mapping->expirationTime = curtime + mapping->m_PortMappingLeaseDuration;
  }
  else {
    //client did not provide a duration, so use the default duration
    if (g_vars.duration==0) {
      return 1; //no default duration set
    }
    else if (g_vars.duration>0) {
      //relative duration
      mapping->expirationTime = curtime+g_vars.duration;
    }
    else { //g_vars.duration < 0
      //absolute daily expiration time

      long int expclock = -1*g_vars.duration;
      struct tm *loctime = localtime(&curtime);
      long int curclock = loctime->tm_hour*3600 + loctime->tm_min*60 + loctime->tm_sec;
      long int diff = expclock-curclock;

      if (diff<60) //if exptime is in less than a minute (or in the past), schedule it in 24 hours instead
        diff += 24*60*60;
      mapping->expirationTime = curtime+diff;
    }
  }

  event = ( expiration_event * ) malloc( sizeof( expiration_event ) );
  if( event == NULL ) {
    return 0;
  }
  event->mapping = mapping;
  if (strlen(DevUDN) < sizeof(event->DevUDN)) strcpy(event->DevUDN, DevUDN);
  else strcpy(event->DevUDN, "");
  if (strlen(ServiceID) < sizeof(event->ServiceID)) strcpy(event->ServiceID, ServiceID);
  else strcpy(event->ServiceID, "");
  
  TPJobInit( &job, ( start_routine ) ExpireMapping, event );
  TPJobSetFreeFunction( &job, ( free_routine ) free_expiration_event );
  if( ( retVal = TimerThreadSchedule( &gExpirationTimerThread,
				      mapping->expirationTime,
				      ABS_SEC, &job, SHORT_TERM,
				      &( event->eventId ) ) )
      != UPNP_E_SUCCESS ) {
    free( event );
    mapping->expirationEventId = -1;
    return 0;
  }
  mapping->expirationEventId = event->eventId;

  trace(3,"ScheduleMappingExpiration: DevUDN: %s ServiceID: %s Proto: %s ExtPort: %s Int: %s.%s at: %s eventId: %d",event->DevUDN,event->ServiceID,mapping->m_PortMappingProtocol, mapping->m_ExternalPort, mapping->m_InternalClient, mapping->m_InternalPort, ctime(&(mapping->expirationTime)), event->eventId);

  return event->eventId;
}

int CancelMappingExpiration(int expirationEventId)
{
  ThreadPoolJob job;
  if (expirationEventId<0)
    return 1;
  trace(3,"CancelMappingExpiration: eventId: %d",expirationEventId);
  if (TimerThreadRemove(&gExpirationTimerThread,expirationEventId,&job)==0) {
    free_expiration_event((expiration_event *)job.arg);
  }
  else {
    trace(1,"  TimerThreadRemove failed!");
  }
  return 1;
}

#if HAS_DELETE_ALL_PORT_MAPPINGS
void DeleteAllPortMappings(void)
{
  IXML_Document *propSet = NULL;

  ithread_mutex_lock(&DevMutex);

  pmlist_FreeList();

  UpnpAddToPropertySet(&propSet, "PortMappingNumberOfEntries", "0");
  UpnpNotifyExt(deviceHandle, gateUDN, "urn:upnp-org:serviceId:WANIPConn1", propSet);
  ixmlDocument_free(propSet);
  trace(2, "DeleteAllPortMappings: UpnpNotifyExt(deviceHandle,%s,%s,propSet)\n  PortMappingNumberOfEntries: %s",
	gateUDN, "urn:upnp-org:serviceId:WANIPConn1", "0");

  ithread_mutex_unlock(&DevMutex);
}
#endif
