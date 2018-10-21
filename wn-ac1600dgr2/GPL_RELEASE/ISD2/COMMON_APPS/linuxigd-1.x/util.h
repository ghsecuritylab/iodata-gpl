#ifndef _UTIL_H_
#define _UTIL_H_

int get_sockfd(void);
char* escapeXMLString(char *xml);
char* unescapeXMLString(char *escXML);
int GetIpAddressStr(char *address, char *ifname);
int getWanLinkStatus(void);
#if HAS_DEBUG /* cfho 2007-1121 */
void trace(int debuglevel, const char *format, ...);
#else
#define trace(...)
#endif
#if HAS_OFFLINE_DOWNLOAD
int sysIsFileExisted(const char *filename);
static int sysconf_util_request(int s, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len);
int sysconf_util_cmd(char* cmdStr, char* replyStr);
#endif
#if HAS_UPNP_API || HAS_MBRIDGE_CONTROL
int sysinteract(char *output, int outputlen, char *fmt, ...);
#endif

#define MAX_COMMAND_LEN 256
#define SYSTEM(format,args...) \
{ \
	const char *F[] = {format}; \
	char buf_for_SYSTEM[MAX_COMMAND_LEN]; \
	char cmd[MAX_COMMAND_LEN]; \
	sprintf(buf_for_SYSTEM, *F, ##args); \
	system(buf_for_SYSTEM); \
}
char* GetFirstDocumentItem( IN IXML_Document * doc, const char *item );
// 2011-10-17 Norkay, fix internalPort or externalPort has dummy "0" ex: 05004
char* GetFirstDocumentItemPort( IN IXML_Document * doc, const char *item );
char* GetDocumentItem(IXML_Document * doc, const char *item, int index);
#endif //_UTIL_H_
