#ifndef _MBridgeControl_H_
#define _MBridgeControl_H_ 1

#define APCFGCLI_CLI_GET(x, format, args...) \
{ \
    const char *F[] = {"apcfgCli_cli token get "format}; \
    char buf_for_SYSTEM[MAX_COMMAND_LEN]; \
    snprintf(buf_for_SYSTEM, MAX_COMMAND_LEN, *F, ##args); \
    sysinteract((x), sizeof((x)), buf_for_SYSTEM); \
    x[strlen(x)-1]='\0'; \
    dprintf("APCFGCLI_CLI_GET:[%s][%s]\n", buf_for_SYSTEM, x); \
}

#define APCFGCLI_CLI_SET(x, format, args...) \
{ \
    const char *F[] = {"apcfgCli_cli token set "format" '%s'"}; \
    char buf_for_SYSTEM[MAX_COMMAND_LEN]; \
    snprintf(buf_for_SYSTEM, MAX_COMMAND_LEN, *F, ##args, x); \
    system(buf_for_SYSTEM); \
    dprintf("APCFGCLI_CLI_SET:[%s]\n", buf_for_SYSTEM); \
}

#if 0//DEBUG
#define dprintf(msg...) printf(msg)
#else
#define dprintf(...)
#endif

#define HAP_24G_SSID_IDX 0
#define HAP_5G_SSID_IDX  1

#define DEV_MTD "/dev/mtd"
#define FWSTATUS "/tmp/dmFwStatus"
#define FWPROGRESS "/tmp/dmFwProgress"
#define FWURLFILE "/tmp/dmFwURL"

/* event stat */
char configurationStatus[8];
char updateProgress[8];
char updateStatus[32];

typedef struct _MBridgeCommonProfile
{
    char configStatus[8];
    char mrhessBridgeID[128];
    char friendlyName[32];
} MBridgeCommonProfile;

typedef struct _MBridgeWirelessProfile
{
    int  wpsStatus;
    char isEnable[8];
    char SSID[65];
    char AuthMode[64];
    char EncrypType[64];
    char WpaPsk[64];
    char WpaPskType[8]; /* Passphrase :0 , HEX(64):1 */
    char DefaultKeyId[8];
    char keyLength[8]; /*0,64,128*/
    char KeyType[8]; /*0:HEX 1:ASCII*/
    char key[4][128];
} MBridgeWirelessProfile;

struct dmlist {
    int event_id;
    int lease_time;
    char DevUDN[NAME_SIZE];
    char ServiceID[NAME_SIZE];
};

struct dm_expiration_event {
  int event_id;
  struct dmlist *node;
};
char* GetNewConfigurationStatus();
char* GetNewUpdateProgress();
char* GetNewUpdateStatus();
int GetConfigurationStatus(struct Upnp_Action_Request *ca_event);
int SetConfigurationStatus(struct Upnp_Action_Request *ca_event);
int GetConfigurationToken(struct Upnp_Action_Request *ca_event);
int ReleaseConfigurationToken(struct Upnp_Action_Request *ca_event);
int StartInvitation(struct Upnp_Action_Request *ca_event);
int StopInvitation(struct Upnp_Action_Request *ca_event);
int GetAccessPointList(struct Upnp_Action_Request *ca_event);
int GetMRHESSBRIDGEID(struct Upnp_Action_Request *ca_event);
int SetMRHESSBRIDGEID(struct Upnp_Action_Request *ca_event);
int GetHomeAPWirelessProfile(struct Upnp_Action_Request *ca_event);
int SetHomeAPWirelessProfile(struct Upnp_Action_Request *ca_event);
int GetBridgeWirelessProfile(struct Upnp_Action_Request *ca_event);
int GetFriendlyName(struct Upnp_Action_Request *ca_event);
int SetFriendlyName(struct Upnp_Action_Request *ca_event);
int GetSSIDPriority(struct Upnp_Action_Request *ca_event);
int SetSSIDPriority(struct Upnp_Action_Request *ca_event);
int GetMobileConfigURL(struct Upnp_Action_Request *ca_event);
int GetFirmwareVersion(struct Upnp_Action_Request *ca_event);
int UpdateFirmware(struct Upnp_Action_Request *ca_event);
int CancelFirmwareUpdate(struct Upnp_Action_Request *ca_event);
int GetUpdateProgress(struct Upnp_Action_Request *ca_event);
int GetUpdateStatus(struct Upnp_Action_Request *ca_event);
int ApplyChanges(struct Upnp_Action_Request *ca_event);
void dm_expiration(void *input);
void dm_freeEvent(struct dm_expiration_event *event);
int dm_scheduleExpiration(struct dmlist *event);
int dm_cancelExpiration(int eventid);
#endif // _MBridgeControl_H_
