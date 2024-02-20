/* 
 * $Id: cidclient.cc,v 1.1.2.1 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_CIDCLIENT_CC

#include <iostream>

extern "C" {
#include <errno.h>
}

#include "cidclient.hh"
#include "filecert.hh"
#include "socket.hh"
#include "functions.hh"
#include "defines.hh"

#define MD5_SIZE	16

#define CONTENT_PKTHDR_VERSION_ONE			1
#define CONTENT_PKTHDR_LEN		sizeof(ContentPktHeader)
#define CONTENT_PKTBODYHDR_LEN	sizeof(ContentPktBodyHeader)
#define CONTENT_PKTBODYHDR_VERSION_ONE		1
#define CONTENT_ID_QUERY_PKT  				0x0400
#define CONTENT_ID_UPDATE_PKT   			0x0410
#define CONTENT_ID_QUERY_RESPONSE_PKT		0x0401
#define CONTENT_ID_UPDATE_RESPONSE_PKT		0x0411
#define NAPHDR_LEN				8

struct ContentPktHeader {
	ContentPktHeader () : version(0), message_type(0), body_len(0)
          { *((U32 *)this) = 0; }

	void NtoH () {
		message_type = ntohs(message_type);
		body_len = ntohs(body_len);
	}
	
	void HtoN () { NtoH(); }

	U8 version;
	U8 reserved[3]; 
	U16 message_type;
	U16 body_len;
}; 
	

struct ContentPktBodyHeader {
public:
	U8 version;
	U8 pad[3];
	U16 username_offset;
	U16 certs_offset;
	U8 reserved[8];

	ContentPktBodyHeader () 
		: version(1), username_offset(0), certs_offset(0) {}

	void NtoH ()
	{
		username_offset = ntohs(username_offset);
		certs_offset = ntohs(certs_offset);
	}
	
	void HtoN ()
	{
		NtoH();
	}
};

CidClient::CidClient (const char* host_name, int port, bool net2host) 
  : sock(0), initialized(false), net_to_host(net2host)
{
  //crypto.SetKey("dummy key");
  in_buf = NULL;
  in_len = 0;
  Reconnect(host_name, port);
}

CidClient::~CidClient () {
  if (in_buf)
    delete[] in_buf;
  Close();
}

int CidClient::Reconnect (const char* host_name, int port) {
  Close();

  sock = new Socket(SOCK_DGRAM);
  sock->listen(0);
  sock->nonblock(true);
  
  polldesc.fd = sock->fd;
  polldesc.events = POLLIN;
  polldesc.revents = 0;

  return SetServer(host_name, port);
}

  
int CidClient::SetServer (const char* host_name, int port)
{
  if (!(*this)) {
    if ((host_name != NULL) && (port != 0)) {
      cliaddr = new Address(host_name, port);
      initialized = true;
      return 0;
    }
    return 1;
  }
  return 0;
}


void CidClient::Close ()
{
  if (sock) {
    sock->close();
    delete sock;
    sock = 0;
  }
  initialized = false;
}


int CidClient::Poll (int timeout)
{
  return ::poll (&polldesc, 1, timeout);
}


int CidClient::QuerySend (const U8 *md5, const char* username) {
  ContentPktHeader *pkt_hdr;

  int usernamelen;
  if ((usernamelen = strlen(username)) <= 0)
    return 0;

  int len = CONTENT_PKTHDR_LEN + MD5_SIZE + usernamelen + 1;
  U8 *buf = new U8[len+1];
  pkt_hdr = (ContentPktHeader*)buf;
  pkt_hdr->version = CONTENT_PKTHDR_VERSION_ONE;
  pkt_hdr->message_type = CONTENT_ID_QUERY_PKT; 
  pkt_hdr->body_len = MD5_SIZE;
  pkt_hdr->HtoN();

  memcpy(buf+CONTENT_PKTHDR_LEN, md5, MD5_SIZE);
  memcpy(buf+CONTENT_PKTHDR_LEN+MD5_SIZE, username, usernamelen + 1);
  
  int sentbytes = sock->send(buf, len, *cliaddr);
  delete[] buf;
  return sentbytes;
}


int CidClient::Receive (int timeout, int recv_size) {
  int len;
  in_buf = new U8[recv_size + 1];

  if ((len = sock->recv(in_buf, recv_size)) <= 0) {
    switch (errno) {
    case EWOULDBLOCK:
    case EINPROGRESS:
      len = 0;
      break;
    }
  }

  in_len = len;
  return len;
}

      
int CidClient::ResponseReceive (char** username, U8** certblob, 
                int& cblob_size, 
                int timeout, int recv_size)
{
  int len;
  ContentPktHeader *pkt_hdr;
  ContentPktBodyHeader *pbh;
  
  if ((len = Receive(timeout, recv_size)) <= 0)
    return len;
  
  pkt_hdr = (ContentPktHeader*) in_buf;
  pbh = (ContentPktBodyHeader*) (pkt_hdr+1);

  // Convert Headers back to Host Order
  pkt_hdr->NtoH();
  pbh->NtoH();
  
  *username = ((char*)(pbh+1)) + pbh->username_offset;
  *certblob = ((U8*)(pbh+1)) + pbh->certs_offset;

//  *certblob = (U8*) (in_buf + CONTENT_PKTHDR_LEN);
  cblob_size = len - CONTENT_PKTHDR_LEN - CONTENT_PKTBODYHDR_LEN 
         - pbh->certs_offset;

  return 1;
}


int CidClient::ResponseReceive (char** username, FileCert** fcert, 
                U8** scert,
                SongCertHead** scert_hd,
                int& scert_size,
                char* md5txtbuf,
                int timeout, int recv_size)
{
  ContentPktHeader *pkt_hdr;
  ContentPktBodyHeader *pbh;
  FileCert *fc;
  SongCertHead *sc;

  int len;
  if ((len = Receive(timeout, recv_size)) <= 0)
    return len;

  in_buf[len] = 0;
  pkt_hdr = (ContentPktHeader*) in_buf;
  pbh = (ContentPktBodyHeader*) (pkt_hdr+1);

  // Convert Headers to Host Order
  pkt_hdr->NtoH();
  pbh->NtoH();

  if ((len - CONTENT_PKTHDR_LEN) != pkt_hdr->body_len)
    return -1;

  *username = ((char*) (pbh+1)) + pbh->username_offset;
  
//  fc = *fcert = (FileCert*) (in_buf + CONTENT_PKTHDR_LEN); 
  fc = *fcert = (FileCert*) (((char*) (pbh+1)) + pbh->certs_offset);
  *scert = (U8*) (fc + 1);
  sc = *scert_hd = (SongCertHead*) (fc + 1);

  if (net_to_host) {
    fc->ntoh();
    sc->ntoh();
  }

  if (md5txtbuf)
    bin_to_hex(fc->md5, MD5_SIZE, md5txtbuf);

  scert_size = len - CONTENT_PKTHDR_LEN - CONTENT_PKTBODYHDR_LEN - pbh->certs_offset - sizeof(FileCert);
  
  return 1;
}
