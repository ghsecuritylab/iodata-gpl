#ifndef _GATEDEVICE_H_
#define _GATEDEVICE_H_ 1

#include <upnp/upnp.h>

/* interface statistics */
typedef enum {
        STATS_TX_BYTES,
        STATS_RX_BYTES,
        STATS_TX_PACKETS,
        STATS_RX_PACKETS,
        STATS_LIMIT
} stats_t;

// IGD Device Globals
UpnpDevice_Handle deviceHandle;
char *gateUDN;
long int startup_time;

// State Variables
char ConnectionType[50];
char PossibleConnectionTypes[50];
char ConnectionStatus[20];
long int StartupTime;
char LastConnectionError[35];
long int AutoDisconnectTime;
long int IdleDisconnectTime;
long int WarnDisconnectDelay;
int RSIPAvailable;
int NATEnabled;
char ExternalIPAddress[20];
int PortMappingNumberOfEntries;
int PortMappingEnabled;

// Linked list for portmapping entries
struct portMap *pmlist_Head;
struct portMap *pmlist_Current;

// WanIPConnection Actions 
int EventHandler(Upnp_EventType EventType, void *Event, void *Cookie);
int StateTableInit(char *descDocUrl);
int HandleSubscriptionRequest(struct Upnp_Subscription_Request *sr_event);
int HandleGetVarRequest(struct Upnp_State_Var_Request *gv_event);
int HandleActionRequest(struct Upnp_Action_Request *ca_event);

int GetConnectionTypeInfo(struct Upnp_Action_Request *ca_event);
int GetNATRSIPStatus(struct Upnp_Action_Request *ca_event);
int SetConnectionType(struct Upnp_Action_Request *ca_event);
int RequestConnection(struct Upnp_Action_Request *ca_event);
int GetTotal(struct Upnp_Action_Request *ca_event, stats_t stat);
int GetCommonLinkProperties(struct Upnp_Action_Request *ca_event);
int InvalidAction(struct Upnp_Action_Request *ca_event);
int GetStatusInfo(struct Upnp_Action_Request *ca_event);
int AddPortMapping(struct Upnp_Action_Request *ca_event);
int GetGenericPortMappingEntry(struct Upnp_Action_Request *ca_event);
int GetSpecificPortMappingEntry(struct Upnp_Action_Request *ca_event);
int GetExternalIPAddress(struct Upnp_Action_Request *ca_event);
int DeletePortMapping(struct Upnp_Action_Request *ca_event);

#if defined (NEW_THREAD_IDLE_TIME)
#define THREAD_IDLE_TIME NEW_THREAD_IDLE_TIME
#else
#define THREAD_IDLE_TIME 5000
#endif

#if defined (NEW_JOBS_PER_THREAD)
#define JOBS_PER_THREAD NEW_JOBS_PER_THREAD
#else
#define JOBS_PER_THREAD 10
#endif

#if defined (NEW_MIN_THREADS)
#define MIN_THREADS NEW_MIN_THREADS
#else
#define MIN_THREADS 2
#endif

#if defined (NEW_MAX_THREADS)
#define MAX_THREADS NEW_MAX_THREADS
#else
#define MAX_THREADS 12
#endif



#if 0 /** cfho 2010-0707 */
// Definitions for mapping expiration timer thread
#define THREAD_IDLE_TIME 5000
#define JOBS_PER_THREAD 10
#define MIN_THREADS 2 
#define MAX_THREADS 12 
#endif /* cfho */



int ExpirationTimerThreadInit(void);
int ExpirationTimerThreadShutdown(void);
int ScheduleMappingExpiration(struct portMap *mapping, char *DevUDN, char *ServiceID);
int CancelMappingExpiration(int eventId);
#if HAS_DELETE_ALL_PORT_MAPPINGS
void DeleteAllPortMappings(void);
#endif

#if HAS_OFFLINE_DOWNLOAD
int GetFreeSpace(struct Upnp_Action_Request *ca_event);
int ActivateOfflineDownload(struct Upnp_Action_Request *ca_event);
int SetOfflineDownload(struct Upnp_Action_Request *ca_event);
int AddOfflineDownload(struct Upnp_Action_Request *ca_event);
int AddOfflineDownloadChunked(struct Upnp_Action_Request *ca_event);
int RemoveOfflineDownload(struct Upnp_Action_Request *ca_event);
int UpdateOfflineDownload(struct Upnp_Action_Request *ca_event);
int StartOfflineDownload(struct Upnp_Action_Request *ca_event);
#endif

#endif //_GATEDEVICE_H
