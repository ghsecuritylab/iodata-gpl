#ifndef _X_MBridgeControl_H_
#define _X_MBridgeControl_H_
#include <upnp/upnp.h>

static char *upnp_search_setting_value(char *context, const char *keyword);
int GetLANsettings(struct Upnp_Action_Request *ca_event);
int GetWLANSecuritySettings(struct Upnp_Action_Request *ca_event);
int GetWLANAdvancedSettings(struct Upnp_Action_Request *ca_event);
int GetACLSettings(struct Upnp_Action_Request *ca_event);
int GetCurrentWDSAPList(struct Upnp_Action_Request *ca_event);

int SetLANsettings(struct Upnp_Action_Request *ca_event);
int SetWLANSecuritySettings(struct Upnp_Action_Request *ca_event);
int SetWLANAdvancedSettings(struct Upnp_Action_Request *ca_event);
int SetACLMode(struct Upnp_Action_Request *ca_event);
int AddToACL(struct Upnp_Action_Request *ca_event);
int DeleteFromACL(struct Upnp_Action_Request *ca_event);
int DeleteAllFromACL(struct Upnp_Action_Request *ca_event);
int AddToWDSList(struct Upnp_Action_Request *ca_event);
int DeleteFromWDSList(struct Upnp_Action_Request *ca_event);
#endif // _X_MBridgeControl_H_
