#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <upnp/upnp.h>
#include <upnp/ixml.h>
#include "globals.h"
#include <unistd.h>
#include <sys/un.h>

#define SYSCONFD_UTIL_PATH     "/var/sysconfd_if/if0"

static int get_sockfd(void)
{
   static int sockfd = -1;

   if (sockfd == -1)
   {
      if ((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
      {
         perror("user: socket creating failed");
         return (-1);
      }
   }
   return sockfd;
}

int GetIpAddressStr(char *address, char *ifname)
{
   struct ifreq ifr;
   struct sockaddr_in *saddr;
   int fd;
   int succeeded = 0;

   fd = get_sockfd();
   if (fd >= 0 )
   {
      strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
      ifr.ifr_addr.sa_family = AF_INET;
      if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
      {
         saddr = (struct sockaddr_in *)&ifr.ifr_addr;
         strcpy(address,inet_ntoa(saddr->sin_addr));
         succeeded = 1;
      }
      else
      {
         syslog(LOG_ERR, "Failure obtaining ip address of interface %s", ifname);
         strcpy(address, "0.0.0.0");
         succeeded = 0;
      }
   }
   return succeeded;
}

int getWanLinkStatus(void)
{
	FILE *fd;
	char buf[24];

	fd = fopen("/tmp/wanlink", "r");

	if(fd)
	{
		fgets(buf, sizeof(buf), fd);
		fclose(fd);

		return atoi(buf) == 0 ? 0 : 1;
	}

	return 1;
}

#if HAS_DEBUG /* cfho 2007-1121 */
void trace(int debuglevel, const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  if (g_vars.debug>=debuglevel) {
    vsyslog(LOG_DEBUG,format,ap);
  }
  va_end(ap);
}
#endif

/**
 * Trims leading and trailing white spaces from a string.
 *
 * @param str String to trim.
 * @return Trimmed string or NULL
 */
char *trimString(char *str)
{
    char *ptr;
    if (!str)
        return NULL;
    if (!*str)
        return str;

    // trim leading white spaces
    while(isspace(*str)) str++;

    // trim trailing white spaces
    ptr = str + strlen(str);
    while (isspace(*--ptr))
        *ptr = '\0';

    return str;
}

/**
 * THIS FUNCTION IS NOT ACTUALLY NEEDED, if you use UpnpMakeActionResponse and such
 * functions for creating responses. libupnp then takes care of escaping xmls.
 * Unescaping on the other hand must be done by ourself here.
 *
 * Change given XML string in escaped form.
 * Following characters are converted:
 *  '<'  -->  "&lt;"
 *  '>'  -->  "&gt;"
 *  '"'  -->  "&quot;"
 *  '''  -->  "&apos;"
 *  '&'  -->  "&amp;"
 *
 * User should free returned pointer.
 *
 * @param xml String to turn escaped xml.
 * @return Escaped xml string or NULL if failure.
 */
char* escapeXMLString(char *xml)
{
    xml = trimString(xml);
    if (xml == NULL)
        return NULL;

    char *escXML = NULL;
    size_t size = strlen(xml);
    size_t alloc = size +1;

    escXML = realloc(NULL, alloc);
    if (!escXML)
        return NULL;

    int i,j; // i goes through original xml and j through escaped escXML
    for (i=0,j=0; i < size; i++)
    {
        switch (xml[i])
        {
            case '<' :
            {
                char *new_buf;
                alloc += strlen("&lt;");
                new_buf = realloc (escXML, alloc);
                if (!new_buf) {
                    return NULL;
                }
                escXML = new_buf;

                strcpy(escXML+j, "&lt;");
                j += strlen("&lt;");
                break;
            }
            case '>' :
            {
                char *new_buf;
                alloc += strlen("&gt;");
                new_buf = realloc (escXML, alloc);
                if (!new_buf) {
                    return NULL;
                }
                escXML = new_buf;

                strcpy(escXML+j, "&gt;");
                j += strlen("&gt;");
                break;
            }
            case '"' :
            {
                char *new_buf;
                alloc += strlen("&quot;");
                new_buf = realloc (escXML, alloc);
                if (!new_buf) {
                    return NULL;
                }
                escXML = new_buf;

                strcpy(escXML+j, "&quot;");
                j += strlen("&quot;");
                break;
            }
            case '\'' :
            {
                char *new_buf;
                alloc += strlen("&apos;");
                new_buf = realloc (escXML, alloc);
                if (!new_buf) {
                    return NULL;
                }
                escXML = new_buf;

                strcpy(escXML+j, "&apos;");
                j += strlen("&apos;");
                break;
            }
            case '&' :
            {
                char *new_buf;
                alloc += strlen("&amp;");
                new_buf = realloc (escXML, alloc);
                if (!new_buf) {
                    return NULL;
                }
                escXML = new_buf;

                strcpy(escXML+j, "&amp;");
                j += strlen("&amp;");
                break;
            }
            default :
            {
                escXML[j++] = xml[i];
                break;
            }
        }
    }

    if (j > 0)
        escXML[j] = '\0';

    return escXML;
}

/**
 * Change given XML string in unescaped form.
 * Following characters are converted:
 *  "&lt;"    -->  '<'
 *  "&gt;"    -->  '>'
 *  "&quot;"  -->  '"'
 *  "&apos;"  -->  '''
 *  "&amp;"   -->  '&'
 *
 * User should free returned pointer.
 *
 * @param escXML String to turn unescaped xml.
 * @return Unescaped xml string or NULL if failure.
 */
char* unescapeXMLString(char *escXML)
{
    escXML = trimString(escXML);
    if (escXML == NULL)
        return NULL;

    char *xml = NULL;
    size_t size = strlen(escXML);

    xml = (char *)malloc(size);
    if (!xml)
        return NULL;

    memset(xml, '\0', size);

    int i,j; // i goes through unescaped xml and j through escaped escXML
    for (i=0,j=0; i < size && j < size; i++)
    {
        if (strncmp(escXML+j, "&lt;", strlen("&lt;")) == 0)
        {
            xml[i] = '<';
            j += strlen("&lt;");
        }
        else if (strncmp(escXML+j, "&gt;", strlen("&gt;")) == 0)
        {
            xml[i] = '>';
            j += strlen("&gt;");
        }
        else if (strncmp(escXML+j, "&quot;", strlen("&quot;")) == 0)
        {
            xml[i] = '"';
            j += strlen("&quot;");
        }
        else if (strncmp(escXML+j, "&apos;", strlen("&apos;")) == 0)
        {
            xml[i] = '\'';
            j += strlen("&apos;");
        }
        else if (strncmp(escXML+j, "&amp;", strlen("&amp;")) == 0)
        {
            xml[i] = '&';
            j += strlen("&amp;");
        }
        else
        {
            xml[i] = escXML[j];
            j++;
        }
    }

    xml[i] = '\0';

    return xml;
}

//-----------------------------------------------------------------------------
//
//                      Common extensions for ixml
//
//-----------------------------------------------------------------------------
/**
 * Get document item which is at position index in nodelist (all nodes with same name item).
 * Index 0 means first, 1 second, etc.
 *
 * @param doc XML document where item is fetched.
 * @param item Name of xml-node to fetch.
 * @param index Which one of nodes with same name is selected.
 * @return Value of desired node.
 */
char* GetDocumentItem(IXML_Document * doc, const char *item, int index)
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

    if ( nodeList )
    {
        if ( ( tmpNode = ixmlNodeList_item( nodeList, index ) ) )
        {
            textNode = ixmlNode_getFirstChild( tmpNode );
            if (textNode != NULL)
            {
                ret = strdup( ixmlNode_getNodeValue( textNode ) );
            }
            // if desired node exist, but textNode is NULL, then value of node propably is ""
            else
                ret = strdup("");
        }
    }

    ixmlNodeList_free( nodeList );
    return ret;
}

/**
 * Get first document item in nodelist with name given in item parameter.
 *
 * @param doc XML document where item is fetched.
 * @param item Name of xml-node to fetch.
 * @return Value of desired node.
 */
char* GetFirstDocumentItem( IN IXML_Document * doc,
                            IN const char *item )
{
    return GetDocumentItem(doc,item,0);
}

// 2011-10-17 Norkay, fix internalPort or externalPort has dummy "0" ex: 05004
char* GetFirstDocumentItemPort( IN IXML_Document * doc,
                                 IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

    if( nodeList )
       {
               if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) )
               {
                       textNode = ixmlNode_getFirstChild( tmpNode );
                       if (textNode != NULL)
                       {
                               const char *p = ixmlNode_getNodeValue( textNode );
                               // remove dummy "0" ex: 05004  ==> 5004
                               if(*p == '0' && *(p+1) != '\0')
                               {
                                       ret = strdup(p+1);
                               }
                               else
                               {
                                       ret = strdup(p);
                               }
                       }
               }
       }

    if( nodeList )
        ixmlNodeList_free( nodeList );
    return ret;
}
#if HAS_OFFLINE_DOWNLOAD
/*****************************************************************
 * NAME: sysIsFileExisted
 * ---------------------------------------------------------------
 * FUNCTION:  Check whether the file is existed
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ****************************************************************/
int sysIsFileExisted(const char *filename)
{
	int rval;
	// Check file existence.
	rval = access(filename, F_OK);

	return rval ? FALSE : TRUE;
}

/*****************************************************************
 * NAME: sysconf_util_request
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:   Norkay 2008-08-01
 * Modify:   Norkay 2008-08-22 '&'
 ****************************************************************/
static int sysconf_util_request(int s, const char *cmd, size_t cmd_len, char *reply, size_t *reply_len)
{
	struct timeval tv;
	int res;
	fd_set rfds;
	int len = cmd_len;

	// no send '&' if command string tail is '&'
	if(cmd[cmd_len-1] == '&')
	{
		len = cmd_len - 1;
	}

	if(send(s, cmd, len, 0) < 0)
		return -1;

	// no wait response if command string tail is '&'
		if(cmd[cmd_len-1] == '&')
		{
			strcpy(reply, "OK");
			*reply_len = 2;
			return 0;
		}

		for(;;)
		{
			tv.tv_sec = 40;
			tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(s, &rfds);
			res = select(s + 1, &rfds, NULL, NULL, &tv);
			if(FD_ISSET(s, &rfds))
			{
				res = recv(s, reply, *reply_len, 0);
				if(res < 0)
					return res;
				if(res > 0 && reply[0] == '<')
				{
					/* This is an unsolicited message from
					 * sysconfd, not the reply to the
					 * request. Use msg_cb to report this to the
					 * caller. */
					if ((size_t) res == *reply_len)
						res = (*reply_len) - 1;
					reply[res] = '\0';
					printf("[%s] [%d]\n", reply, res);
					continue;
				}
				*reply_len = res;
				break;
			}
			else
			{
				return -2;
			}
		}
		return 0;
}

/*****************************************************************
 * NAME: sysconf_util_cmd
 * ---------------------------------------------------------------
 * FUNCTION:  send command string to sysconfd and recevice response via sock
 * INPUT:
 * OUTPUT:
 * Author:   Norkay 2008-08-01
 * Modify:
 ****************************************************************/
int sysconf_util_cmd(char* cmdStr, char* replyStr)
{
	struct sockaddr_un local;
	struct sockaddr_un dest;
	int s = -1;
	int ret;
	char replyBuf[128];
	size_t replyLen;

	if(!sysIsFileExisted("/var/sysconfd_if/if0"))
	{
		return -3;
	}

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(s < 0)
	{
		return -1;
	}

	local.sun_family = AF_UNIX;
	snprintf(local.sun_path, sizeof(local.sun_path), "/var/sysconf_util_%d-%d", getpid(), 145);
	if(bind(s, (struct sockaddr *) &local, sizeof(local)) < 0)
	{
		close(s);
		return -1;
	}

	dest.sun_family = AF_UNIX;
	snprintf(dest.sun_path, sizeof(dest.sun_path), "%s", SYSCONFD_UTIL_PATH);
	if(connect(s, (struct sockaddr *) &dest, sizeof(dest)) < 0)
	{
		close(s);
		unlink(local.sun_path);
		return -1;
	}

	replyLen = sizeof(replyBuf) - 1;

	ret = sysconf_util_request(s, cmdStr, strlen(cmdStr), replyBuf, &replyLen);

	if(ret == -2)
	{
		printf("'%s' command timed out.\n", cmdStr);
		return -2;
	}
	else if(ret < 0)
	{
		printf("'%s' command failed.\n", cmdStr);
		return -1;
	}
	else
	{
		replyBuf[replyLen] = '\0';
		//printf("==>%s\n", replyBuf);
	}

	unlink(local.sun_path);
	close(s);

	if(replyStr)
	{
		strcpy(replyStr, replyBuf);
	}

	return 0;
}
#endif
#if HAS_UPNP_API || HAS_MBRIDGE_CONTROL
/*****************************************************************
* NAME: sysinteract
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int sysinteract(char *output, int outputlen, char *fmt, ...)
{
	static char cmd_for_sysinteract[256];
	FILE *pipe;
	int c;
	//char cmd[256];
	int i;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(cmd_for_sysinteract, sizeof(cmd_for_sysinteract), fmt, ap);
	va_end(ap);

	memset(output, 0, outputlen);

	if((pipe = popen(cmd_for_sysinteract, "r")) == NULL)
	{
		goto err;
	}

	for(i = 0; ((c = fgetc(pipe)) != EOF) && (i < outputlen - 1); i++)
	{
		output[i] = (char) c;
	}
	output[i] = '\0';

	pclose(pipe);

	if(strlen(output) == 0)
	{
		goto err;
	}

	return 0;

err:
	strcpy(output, "---");
	return -1;
}
#endif

#if HAS_MBRIDGE_CONTROL
/*****************************************************************
* NAME: getFirmwareHeaderCheck
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int getFirmwareHeaderCheckEx(char *filename, unsigned int magickey)
{
    char buf[100]={0};
    char *token = NULL;
    int rval;

    if(magickey != 0)
    {
        rval = sysinteract(buf, sizeof(buf), "header -x %s -m %x", filename, magickey);
    }
    else
    {
        rval = sysinteract(buf, sizeof(buf), "header -x %s", filename);
    }

    token = strstr(buf, "header: Return OK");

    if(token){
        return 1; /* OK */
    }

    return 0; /* Fail */
}
/*****************************************************************
* NAME: getFirmwareHeaderCheck
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int getFirmwareHeaderCheck(char *filename)
{
    return getFirmwareHeaderCheckEx(filename, 0);
}
/*****************************************************************
* NAME: sysGetMtdNumByName
* ---------------------------------------------------------------
* FUNCTION:  This API will return open the /proc/mtd file,
*           found the MTD# by the given name.
* INPUT:    inMtdName: The mtd name to be found. This API will do a exactly match.
* OUTPUT:   iMtdNum: The MTD number of the given "inMtdName". set to -1 if not found.
*           iMtdSize: The size of the given "inMtdName". set to 0 if not found.
*           return TRUE if found. Return False if not found.
* Author:
* Modify:   cfho 2006-0922
****************************************************************/
int sysGetMtdNumByName(const char *inMtdName, int *iMtdNum, unsigned int *iMtdSize)
{
    char buf[128];
    char strMtdName[16];
    FILE *fp;
    int nl = 0;
    char bufMtdName[32];

    // protect
    if(!inMtdName)
    {
        // printf("%s: inMtdName = NULL!\n", __FUNCTION__);
        return -1;
    }

    *iMtdNum=-1;
    // printf("inMtdName=%s\n",inMtdName);

    fp = fopen("/proc/mtd", "r");
    if(!fp)
    {
        return *iMtdNum;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        if(nl > 0)
        {
            //printf("nl %d buf[%s]\n",nl,buf);
            sscanf(buf, "%*s %*s %*s \"%s",bufMtdName);
            if(bufMtdName[strlen(bufMtdName)-1]=='\"') bufMtdName[strlen(bufMtdName)-1]=0;
            //printf("bufMtdName[%s]\n",bufMtdName);

            if(strcmp(bufMtdName,inMtdName)==0)
            {
                /* found */
                *iMtdNum=nl-1;
                sscanf(buf, "%s %x",strMtdName, iMtdSize);
                break;
            }
        }
        nl++;
    }
    // printf("iMtdNum=%d %s\n",*iMtdNum,strMtdName);
    fclose(fp);

    return *iMtdNum;
}
#endif
