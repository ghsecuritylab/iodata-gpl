/* All the codes are reference to the "dproxy-nexgen" project */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <dnsproxy.h>

fd_set rfds;
int dns_sock;


static dummy_dns_answer[20];
void dns_decode_name(char *name, char **buf)
{
  int i, k, len, j;

  i = k = 0;
  while( **buf ){
         len = *(*buf)++;
         for( j = 0; j<len ; j++)
          name[k++] = *(*buf)++;
         name[k++] = '.';
  }
  name[k-1] = *(*buf)++;
}

/*******************************************************/
void dns_decode_rr(struct dns_rr *rr, char **buf, int is_question,char *header)
{
  /* if the first two bits the of the name are set, then the message has been
     compressed and so the next byte is an offset from the start of the message
     pointing to the start of the name */
  if( **buf & 0xC0 ){
    (*buf)++;
    header += *(*buf)++;
    dns_decode_name( rr->name, &header );
  }else{
    /* ordinary decode name */
    dns_decode_name( rr->name, buf );
  }  

  SET_UINT16( rr->type, buf );
  SET_UINT16( rr->class, buf);

  if( is_question != 1 ){
    SET_UINT32( rr->ttl, buf );
    SET_UINT16( rr->rdatalen, buf );

    memcpy( rr->data, *buf, rr->rdatalen );
    *buf += rr->rdatalen;
    /*
    for(i = 0; i < rr->rdatalen; i+=4 )
      SET_UINT32( (uint32)rr->data[i], buf );
    */
  }

  

}
/*******************************************************/
int dns_decode_message(struct dns_message *m, char **buf)
{
  int i;
  char *header_start = *buf;

  SET_UINT16( m->header.id, buf );
  SET_UINT16( m->header.flags.flags, buf );
  SET_UINT16( m->header.qdcount, buf );
  SET_UINT16( m->header.ancount, buf );
  SET_UINT16( m->header.nscount, buf );
  SET_UINT16( m->header.arcount, buf );

  if( m->header.ancount > 1 ){
    printf("Lotsa answers\n");
  }

#if 1
  /* decode all the question rrs */
  for( i = 0; i < m->header.qdcount && i < NUM_RRS; i++){
    dns_decode_rr( &m->question[i], buf, 1, header_start );
  }  
  /* decode all the answer rrs */
  for( i = 0; i < m->header.ancount && i < NUM_RRS; i++){
    dns_decode_rr( &m->answer[i], buf, 0, header_start );
  }  
#endif
  return 0;
}
/*****************************************************************************/
void dns_decode_request(dns_request_t *m)
{
  struct in_addr *addr;
  char *ptr;
  int i;

  m->here = m->original_buf;

  dns_decode_message( &m->message, &m->here );

  
  if( m->message.question[0].type == PTR ){
    strncpy( m->ip, m->message.question[0].name, 20 );
  }else if ( m->message.question[0].type == A || 
         m->message.question[0].type == AAA){ 
    strncpy( m->cname, m->message.question[0].name, NAME_SIZE );
  }
#if 1
    strcpy( m->ip, dummy_dns_answer);
#else
  /* set according to the answer */
  for( i = 0; i < m->message.header.ancount && i < NUM_RRS; i++){
    /* make sure we ge the same type as the query incase there are multiple
       and unrelated answers */
    if( m->message.question[0].type == m->message.answer[i].type ){

      if( m->message.answer[i].type == A
      || m->message.answer[i].type == AAA ){
    /* Standard lookup so convert data to an IP */
    addr = (struct in_addr *)m->message.answer[i].data;
    //strncpy( m->ip, inet_ntoa( addr[0] ), 20 );
    strcpy( m->ip, "192.168.123.1" );
    break;
    
      }else if( m->message.answer[i].type == PTR ){
    /* Reverse lookup so convert data to a nume */
    ptr = m->message.answer[i].data;
    dns_decode_name( m->cname, &ptr );
    strncpy( m->ip, m->message.answer[i].name, 20 );
    break;
      }

    } /* if( question == answer ) */
  } /* for */
#endif
}
/*****************************************************************************/
int dns_read_packet(int sock, dns_request_t *m)
{
  struct sockaddr_in sa;
  int salen;
  
  /* Read in the actual packet */
  salen = sizeof(sa);
  
  m->numread = recvfrom(sock, m->original_buf, sizeof(m->original_buf), 0,
             (struct sockaddr *)&sa, &salen);
  
  if ( m->numread < 0) {
    printf("dns_read_packet: recvfrom\n");
    return -1;
  }
  
  /* TODO: check source addr against list of allowed hosts */

  /* record where it came from */
  memcpy( (void *)&m->src_addr, (void *)&sa.sin_addr, sizeof(struct in_addr));
  m->src_port = ntohs( sa.sin_port );
    //printf("dns_read_packet m->src_port %d\n",m->src_port);

  /* check that the message is long enough */
  if( m->numread < sizeof (m->message.header) ){
    //printf("dns_read_packet: packet from '%s' to short to be dns packet", 
//      inet_ntoa (sa.sin_addr) );
    return -1;
  }

  /* pass on for full decode */
  dns_decode_request( m );

  return 0;
}
/*****************************************************************************/
int dns_construct_header(dns_request_t *m)
{
  char *ptr = m->original_buf;
  int dummy;

  SET_UINT16_TO_N( ptr, m->message.header.id, dummy );
  SET_UINT16_TO_N( ptr, m->message.header.flags.flags, dummy );
  SET_UINT16_TO_N( ptr, m->message.header.qdcount, dummy );
  SET_UINT16_TO_N( ptr, m->message.header.ancount, dummy );
  SET_UINT16_TO_N( ptr, m->message.header.nscount, dummy );
  SET_UINT16_TO_N( ptr, m->message.header.arcount, dummy );
  
  return 0;
}
/*****************************************************************************/
int dns_construct_name(char *name, char *encoded_name)
{
  int i,j,k,n;

  k = 0; /* k is the index to temp */
  i = 0; /* i is the index to name */
  while( name[i] ){

     /* find the dist to the next '.' or the end of the string and add it*/
     for( j = 0; name[i+j] && name[i+j] != '.'; j++);
     encoded_name[k++] = j;

     /* now copy the text till the next dot */
     for( n = 0; n < j; n++)
        encoded_name[k++] = name[i+n];
    
     /* now move to the next dot */ 
     i += j + 1;

     /* check to see if last dot was not the end of the string */
     if(!name[i-1])break;
  }
  encoded_name[k++] = 0;
  return k;
}
/*****************************************************************************/
void dns_construct_reply( dns_request_t *m )
{
  int len;

  /* point to end of orginal packet */ 
  m->here = &m->original_buf[m->numread];

  m->message.header.ancount = 1;
  m->message.header.flags.f.question = 1;
  dns_construct_header( m );

  if( m->message.question[0].type == A ){
    /* standard lookup so return and IP */
    struct in_addr in;

    inet_aton( m->ip, &in );
    SET_UINT16_TO_N( m->here, 0xc00c, m->numread ); /* pointer to name */
    SET_UINT16_TO_N( m->here, A, m->numread );      /* type */
    SET_UINT16_TO_N( m->here, IN, m->numread );     /* class */
    SET_UINT32_TO_N( m->here, 10000, m->numread );  /* ttl */
    SET_UINT16_TO_N( m->here, 4, m->numread );      /* datalen */
    memcpy( m->here, &in.s_addr, sizeof(in.s_addr) ); /* data */
    m->numread += sizeof( in.s_addr);
  }else if ( m->message.question[0].type == PTR ){
    /* reverse look up so we are returning a name */
    SET_UINT16_TO_N( m->here, 0xc00c, m->numread ); /* pointer to name */
    SET_UINT16_TO_N( m->here, PTR, m->numread );    /* type */
    SET_UINT16_TO_N( m->here, IN, m->numread );     /* class */
    SET_UINT32_TO_N( m->here, 10000, m->numread );  /* ttl */
    len = dns_construct_name( m->cname, m->here + 2 );
    SET_UINT16_TO_N( m->here, len, m->numread );      /* datalen */
    m->numread += len;
  }
}
/*****************************************************************************/
int dns_write_packet(int sock, struct in_addr in, int port, dns_request_t *m)
{
  struct sockaddr_in sa;
  int retval;

  /* Zero it out */
  memset((void *)&sa, 0, sizeof(sa));

  /* Fill in the information */
  //inet_aton( "203.12.160.35", &in );
  memcpy( &sa.sin_addr.s_addr, &in, sizeof(in) );
    //printf("dns_write_packet dst port %d\n",port);
  sa.sin_port = htons(port);
  sa.sin_family = AF_INET;

  retval = sendto(sock, m->original_buf, m->numread, 0, 
        (struct sockaddr *)&sa, sizeof(sa));
  
  if( retval < 0 ){
    printf("dns_write_packet: sendto");
  }

  return retval;
}
/*****************************************************************************/
int dns_init()
{
  struct sockaddr_in sa;
  struct in_addr ip;

  /* Clear it out */
  memset((void *)&sa, 0, sizeof(sa));
    
  dns_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  /* Error */
  if( dns_sock < 0 ){
     printf("Could not create socket");
     exit(1);
  } 

  ip.s_addr = INADDR_ANY;
  sa.sin_family = AF_INET;
  memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
  sa.sin_port = htons(PORT);
  
  /* bind() the socket to the interface */
  if (bind(dns_sock, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0){
     printf("dns_init: bind: Could not bind to port");
     exit(1);
  }



  FD_ZERO( &rfds );
  FD_SET( dns_sock, &rfds );

  
  return 1;
}
/*****************************************************************************/

int dns_main_loop()
{
  struct timeval tv;
  fd_set active_rfds;
  int retval;
  dns_request_t m;
  dns_request_t *ptr, *next;
  struct in_addr in;

  while( 1 ){
    
    /* set the one second time out */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* now copy the main rfds in the active one as it gets modified by select*/
    active_rfds = rfds;

    retval = select( FD_SETSIZE, &active_rfds, NULL, NULL, &tv );

    if (retval){
        //printf("***  data is now available dns_sock %d\n", dns_sock);
        /* data is now available */
        dns_read_packet( dns_sock, &m );
        //dns_handle_request( &m );

        dns_construct_reply(&m);
        dns_write_packet( dns_sock,  m.src_addr, m.src_port, &m );
        
    }
  }  
  return 0;
}
int main(int argc, char* argv[])
{
//printf("argc %d\n", argc);
if (argc!=2)
{
    printf("usage: dnsproxy [dummy IP]\n");
    exit(0);
}
//printf("argv[1] %s\n", argv[1]);
strcpy(dummy_dns_answer, argv[1]);
dns_init();
dns_main_loop();

return 1;
}






