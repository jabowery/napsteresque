/* -*-  Mode:C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:t -*- */
#ifndef CID_CLIENT_H_
#define CID_CLIENT_H_

/*
 * CidClient::QuerySend() -
 *	Expects a pointer to a md5 (ascii text for now), will return
 *	the length of "bytes" sent.  (This is not just the length of the
 *	md5, but also some pkt header for internal communications)
 *
 * CidClient::ResponseReceive(U8** certblob, int& cblob_size, ...) -
 *
 *  Caller can optionally specify a timeout in milliseconds.
 *
 *	Return Values:
 *	This function returns 0, if there is no incoming data....The caller 
 *	then should ignore values in *certblob and cblob_size.
 *	
 *	This function returns 1, if a certificate is received.
 *	*cerblob points to beginning of the FileCert, cblob_size is
 *  the size of FileCert + variable length SongCert
 *  If the caller needs to keep those values, the caller HAS to 
 *  copy them into the caller's buffer as the there is only 1 incoming 
 *  pkt buffer in this object, which gets overwritten each time 
 *  ResponseReceive() is called.
 *
 *	This function returns negative values when any of the system calls
 *	to poll() or recvfrom() failed.
 *
 */

extern "C" {
#include <unistd.h>
#include <sys/poll.h>
}

#include "defines.hh"
#include "filecert.hh"
#include "address.hh"
#include "socket.hh"

#define CID_CLIENT_DEFAULT_RECV_SIZE	4096	

struct CidClient {
	CidClient(
	  const char* host_name = 0, int port = 0, 
	  bool net2host = true
	);
	~CidClient();

	int Reconnect(const char* host_name, int port);
	int SetServer(const char* host_name, int port);
	void Close();

	bool operator!() const { return (initialized == false); }

	int Fd() { return polldesc.fd; }

	int QuerySend(
	  const U8* md5,
	  const char* username = "test"
	);

	int ResponseReceive(
	  char** username, FileCert** fcert, U8** scert,
	  SongCertHead** scert_hd, int& scert_size,
	  char* md5txtbuf = 0,
	  int timeout = 0,
	  int recv_size = CID_CLIENT_DEFAULT_RECV_SIZE
	);

	int ResponseReceive(
	  char** username, U8** certblob, 
	  int& cblob_size, int timeout = 0,
	  int recv_size = CID_CLIENT_DEFAULT_RECV_SIZE
	);

	void PollDescClear ();
	int Poll (int timeout);
	int Receive(int timeout, int recv_size);

	U8 *in_buf;
	int in_len;
	Socket *sock;
	Address *cliaddr;
	
	pollfd polldesc;

	bool initialized;
	bool net_to_host;
};

#endif
