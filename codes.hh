/* 
 * $Id: codes.hh,v 1.39.8.1.2.9 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_CODES_HH
#define _NAPD_CODES_HH

enum flood_handler_type { PASS, DROP, THROTTLE, LOG, _FHT_END };

struct _cmd_tab {
  unsigned code;
  char *name;
  enum flood_handler_type handler;
  bool payload_relevant;
  unsigned short threshold;
};


#define ERROR_MODAL 		0

#define ERROR_MESSAGE           SYSTEM_MESSAGE

#define LOGIN_REQUEST		2	// COMPNAME PASSWORD WAITPORT VERSION LINESPEED
#define LOGIN_SUCCESSFUL	3
#define AUTO_UPGRADE		5	// VERSION FILE

// New User Sequence.
#define CHECK_NAME		7	// COMPNAME
#define NAME_FREE		8
#define NAME_USED		9
#define NAME_INVALID		10
#define CHECK_PASS		11	// COMPNAME PASSWORD
#define PASS_OK			12
#define MUST_LOGIN		13

// Future New User and Login Sequence
#define USER_ADD                20      // COMPNAME PASSWORD WAITPORT VERSION LINESPEED EMAIL
#define USERADD_FAILED          21      // REASON#
#define LOGIN_USER              22      // COMPNAME PASSWORD WAITPORT VERSION LINESPEED
#define LOGIN_SUCCESS           23
#define LOGIN_FAILED            23      // REASON#
#define LOGIN_LOCKED            24
#define REGISTER_LOCKID         25      // LOCKID
#define REGLOCKID_FAILED        26
#define REFRESH_REQUEST		30	// (empty) /jdb
#define REFRESH_RESPONSE	31	// Same as LOGIN_SUCCESS /jdb

// Client Commands.
#define ADD_FILE		100	// FILENAME FILEID FILESIZE BITRATE FREQ LENGTH
#define ADDED_FILE		101
#define REMOVE_FILE		102	// FILENAME
#define REMOVED_FILE		103

#define REMOVE_ALL_FILES       110

#define SEARCH_CANCEL           199     // 
#define SEARCH_REQUEST		200	// STRING
#define SEARCH_REPLY		201	// FILENAME FILEID FILESIZE BITRATE FREQ LENGTH COMPNAME IP LINESPEED
#define SEARCH_COMPLETED	202
#define DOWNLOAD_REQUEST	203	// COMPNAME FILENAME
#define DOWNLOAD_RESPONSE	204	// COMPNAME IP PORT FILENAME FILEID LINESPEED
#define	COMP_MESSAGE		205	// DESTCOMP MESSAGE
#define DL_NOT_FOUND		206	// COMPNAME FILENAME

#define COMPUTER_ONLINE		209
#define COMPUTER_ONLINE_ARGS(c)	209, (c)->name, (c)->linespeed

#define	COMPUTER_OFFLINE	210	// COMPNAME

#define VIEW_FILES		211	// COMPNAME
#define NOTIFY_FILE		212	// COMPNAME FILENAME FILEID FILESIZE BITRATE FREQ LENGTH
#define VIEW_COMPLETED		213	// COMPNAME LINESPEED

#define SHARE_STATS		214	// LIBRARIES FILES GIGS

#define NEW_DOWNLOAD		218
#define DOWNLOAD_COMPLETE	219
#define NEW_UPLOAD		220
#define UPLOAD_COMPLETE		221

#define	ADD_NOTIFY		207	// COMPNAME
#define CHECK_ONLINE		208	// COMPNAME
#define NOTIFY_EXISTS		301	// COMPNAME
#define NOTIFY_UNKNOWN		302	// COMPNAME
#define REMOVE_NOTIFY		303	// COMPNAME

// Channel related codes.

#define JOIN_CHAN		400
#define JOIN_CHAN_ARGS(c)	400, (c)->name

#define PART_CHAN		401
#define PART_CHAN_ARGS(c)	401, (c)->name

#define CHANNEL_MESSAGE	        402	// CHANNAME CONTENT
#define RECV_CHAN_MESSAGE	403	// CHANNAME USER CONTENT
#define SYSTEM_MESSAGE		404	// MESSAGE
#define JOIN_ACCEPTED		405 	// CHANNEL

#define USER_JOIN		406
#define USER_JOIN_ARGS(c, u)	406, (c)->name, (u)->name, 0, (u)->linespeed

#define USER_PART		407
#define USER_PART_ARGS(c, u)	407, (c)->name, (u)->name, 0, (u)->linespeed

#define USER_HERE		408
#define USER_HERE_ARGS(c, u)	408, (c)->name, (u)->name, 0, (u)->linespeed

#define JOIN_COMPLETED		409	// CHAN
#define CHANNEL_TOPIC		410	// CHAN TOPIC

// SendFile request.
#define FIREWALL_TRANSFER	500	// COMPNAME FILENAME
#define FIREWALL_RESPONSE	501	// COMPNAME IP PORT FILENAME FILEID LINESPEED

// GET is a client question, USER is the server response
#define GET_LINESPEED		600	// COMPNAME
#define USER_LINESPEED		601
#define USER_LINESPEED_ARGS(u)	601, (u)->name, (u)->linespeed

#define SET_EMAILADDRESS	602	// EMAILADDRESS
#define SET_EMAIL		702	// EMAIL
#define SUBSCRIBE_NEWSLETTER    704     // EMAIL

#define USER_INFO		603	// COMPNAME
#define USER_INFO_ONLINE	604
#define USER_INFO_OFFLINE	605

#define SET_USER_LEVEL		606	// COMPNAME LEVEL
#define DOWNLOAD_ATTEMPT	607	// COMPNAME FILENAME LINESPEED
#define DOWNLOAD_ACCEPTED	608	// COMPNAME FILENAME
#define ACCEPT_FAILED		609	// COMPNAME FILENAME
#define KILL_USER		610	// COMPNAME

#define SET_USER_DATAPORT	613	// COMPNAME DATAPORT
#define SET_DATAPORT		703	// DATAPORT

#define CHANNEL_LIST		617

#define CHANNEL_ENTRY		618
#define CHANNEL_ENTRY_ARGS(c)	618, (c)->name, (c)->members.size(), (c)->topic

#define DELAY_DOWNLOAD		619	// COMPNAME FILENAME MAXUL
#define DOWNLOAD_DELAYED	620	// COMPNAME FILENAME FILESIZE MAXUL
#define SET_USER_LINESPEED	625	// USER LINESPEED
#define CONFIG_ERROR		626	// USER
#define BCAST_MODERATOR		627	// FROM MESSAGE
#define ANNOUNCE_LOCAL		628	// FROM MESSAGE
#define ANNOUNCE_ALL		629	// FROM MESSAGE

#define SET_LINESPEED		700	// LINESPEED
#define SET_PASSWORD		701	// PASSWORD

// New Command Codes (2.5)
//  
// Almost all new codes are singular atomics; they are used by both
// the client to ask the question, and the server to answer.


#define NOTIFY_LIST             310
#define NOTIFY_ENTRY            311
#define NOTIFY_CLEAR            316

#define IGNORE_LIST             320
#define IGNORE_ENTRY            321
#define IGNORE_ADD              322
#define IGNORE_REMOVE           323
#define IGNORE_UNKNOWN          324
#define IGNORE_EXISTS           325
#define IGNORE_CLEAR            326
#define IGNORE_FAIL             327

// #define USER_INFO2              650
#define USERFLAG_MUTE           651
#define USERFLAG_CLOAK          652

#define DONT_RECONNECT          748

#define SERVER_PING             750
#define CLIENT_PING             751
#define CLIENT_PONG             752

#define SET_USER_PASSWORD       753     

#define BAN_MODIFY              770
#define BAN_REMOVE_LEGACY       771

#define BAN_LIST		615
#define BAN_ADD 		612	// COMPNAME|ADDRESS
#define BAN_REMOVE		614	// COMPNAME|ADDRESS
#define BAN_ENTRY		616	// BAN(IP)


#define SERVER_RELOAD_DB        800     // all|bans|channels|config (default is all if ommitted)
#define SERVER_VERSION          801     // STRING|BUILD#
#define SERVER_RECONNECTDB      805     
#define SERVER_CONFIG           810     // STRING VALUE
#define SERVER_SHUTDOWN         811     
#define SERVER_RELAY		812	// CLIENTID RELAY_MSG
#define SERVER_CONNECT		813	// PASS
#define SERVER_CONNECT_OK	814

#define CHANNEL_CLEAR           820     // STRING, 0|1
#define CHANNEL_SETLEVEL        823     // STRING LEVEL, 0|1 
#define CHANNEL_EMOTE           824     // CHANNAME CONTENT, CHANNAME CLIENT CONTENT

#define CHANNEL_USERLIST        830     // CHANNEL, COMPLETION
#define CHANNEL_USER            825
#define CHANNEL_USER_ARGS(c, u)	825, \
  (c)->name, (u)->name, (u)->num_files, (u)->linespeed

#define CHANNEL_LIST2           827
#define CHANNEL_ENTRY2          828 
#define CHANNEL_ENTRY2_ARGS(c)	828, \
  (c)->name, (I32)(c)->size(), (I32)(c)->type, \
  (I32)(c)->required_level, 1000, (c)->topic

#define CHANNEL_KICK            829

#define CHANNEL_MOTD            425     // channel "line"


// this is to force a user to connect to another server
//#define CLIENT_REDIRECT         821     // USER IP PORT, IP PORT
//#define CLIENT_CYCLE            822     // USER IP, IP
//#define CLIENT_RECONNECT        819     // IP PORT

#define SERVER_USERLIST         831
#define SERVER_USER             832
#define SERVER_SEARCH           833

#define PORTD_CONNECT           850     // IP(STRING), IP(STRING) 0|1 (timeout)
#define PORTD_LISTEN            851     // IP(STRING), IP(STRING) 0|1 (timeout)

// some files stuff
#define ADD_PATH_BLOCK          870     // PATH FILE FILE FILE FILE ...

#define CAPABILITIES		920

#define FILEMETA		931
#define ADD_SONG		932
#define REMOVE_SONG		933
#define ADD_SONG2		934
#define SONG_CERT		944
#define SONG_CERT2		945
#define IDENTIFY		955

#define DL_DENIED		1300 //jdb

/******************
 * NAPD <-> NAPD  *
 ******************/

#define LINK_REQUEST            1 // PORT PASSWORD
#define LINK_SUCCESSFUL         2 //
#define LINK_DENIED             3
#define LINK_SHUTDOWN           4

#define LINK_HEARTBEAT          5

#define LINK_KILL               6

#define REMOTE_SHARE_STATS	10 // LIBRARIES FILES GIGS
#define REMOTE_BROADCAST	11 // LEVEL COMMAND CONTENT
#define REMOTE_USER_JOIN	12 // USER ADDRESS PORT LINESPEED LEVEL
#define REMOTE_USER_PART	13 // USER
#define REMOTE_USER_REPLACE	14 // COMPNAME
#define REMOTE_SYSTEM_MESSAGE	15 // USER MESSAGE
#define REMOTE_ERROR_MESSAGE    REMOTE_SYSTEM_MESSAGE
#define REMOTE_ERROR_MODAL      19
#define REMOTE_COMP_MESSAGE	16 // LOCALUSER FROMCOMP MESSAGE
#define REMOTE_SYNC             18

#define REMOTE_DL_RESPONSE	20 // LOCALUSER FROMCOMP IP PORT FILENAME FILEID LINESPEED
#define REMOTE_DL_ATTEMPT       11021 // LOCALUSER FROMCOMP FILENAME
#define REMOTE_DL_NOT_FOUND	23 // LOCALUSER FROMCOMP FILENAME
#define REMOTE_ACCEPT_FAILED	24 // LOCALUSER FROMCOMP FILENAME
#define REMOTE_DL_DELAYED	25 // LOCALUSER FROMCOMP FILENAME FILESIZE MAXUL
#define REMOTE_FW_RESPONSE	27 // LOCALUSER FROMCOMP IP PORT FILENAME LINESPEED

#define REMOTE_VIEW_FILES	30 // LOCALUSER FROMCOMP
#define REMOTE_NOTIFY_FILE	31 // LOCALUSER FROMCOMP FILENAME FILEID FILESIZE BITRATE FREQ LENGTH
#define REMOTE_VIEW_COMPLETED	32 // LOCALUSER FROMCOMP LINESPEED
#define REMOTE_NOTIFY_CHECK	33 // LOCALUSER FROMCOMP
#define REMOTE_NOTIFY_ONLINE	34 // LOCALUSER FROMCOMP LINESPEED
#define REMOTE_NOTIFY_OFFLINE	35 // LOCALUSER FROMCOMP
#define REMOTE_ADD_NOTIFY       36 // LOCALUSER FROMCOMP
#define REMOTE_REMOVE_NOTIFY    37 // LOCALUSER FROMCOMP

#define REMOTE_GET_LINESPEED	40 // LOCALUSER FROMCOMP
#define REMOTE_USER_LINESPEED	41 // LOCALUSER FROMCOMP LINESPEED

#define REMOTE_GET_USER_INFO	 50 // LOCALUSER FROMCOMP QUERYLEVEL
#define REMOTE_USER_INFO	 51 //

#define REMOTE_KILL_USER	60 // LOCALUSER FROMCOMP QUERYLEVEL [REASON]
#define REMOTE_BAN_MASK		61 // LOCALUSER ADDRESS|FROMCOMP [REASON]
#define REMOTE_UNBAN_MASK	62 // ADDRESS|FROMCOMP
#define REMOTE_NUKE_USER        63 // LOCALUSER FROMCOMP QUERYLEVEL [REASON]
#define REMOTE_MUZZLE_USER	64 // LOCALUSER FROMCOMP QUERYLEVEL [REASON]
#define REMOTE_UNMUZZLE_USER	65 // LOCALUSER FROMCOMP QUERYLEVEL [REASON]

#define REMOTE_SET_LINESPEED	70 // LOCALUSER FROMCOMP LINESPEED QUERYLEVEL
#define REMOTE_SET_DATAPORT	71 // LOCALUSER FROMCOMP DATAPORT QUERYLEVEL
#define REMOTE_SET_USERLEVEL	72 // LOCALUSER FROMCOMP LEVEL QUERYLEVEL

#define REMOTE_CLIENT_PING      80 // LOCALUSER FROMCOMP
#define REMOTE_CLIENT_PONG      81 // LOCALUSER FROMCOMP

#define REMOTE_CHANNEL_JOIN	91 // USER CHANNEL
#define REMOTE_CHANNEL_PART	92 // USER CHANNEL
#define REMOTE_CHANNEL_MESSAGE	93 // USER CHANNAME CONTENT
#define REMOTE_CHANNEL_EMOTE    94 // USER CHANNAME CONTENT

#define REMOTE_COMPUTER_ONLINE  100
#define REMOTE_COMPUTER_OFFLINE 101

#define REMOTE_SEARCH_REQUEST	200 // LOCALUSER STRING
#define REMOTE_SEARCH_REPLY	201 // LOCALUSER FILENAME FILEID FILESIZE BITRATE FREQ LENGTH COMPNAME LINESPEED
#define REMOTE_SEARCH_COMPLETED 202 // LOCALUSER MATCHES


#define ACCOUNT_UPDATE		10032
#define ACCOUNT_UPDATE_ARGS(c)	10032, \
  (c)->pn_service_level_id, 0, 0, \
  (c)->User::flags.bit.blockim, \
  (c)->User::flags.bit.blockchat, \
  (c)->User::flags.bit.badwords, \
  (c)->transfers, 0, 0, 0, 0, 0, "3.3-integrate"


//
// codes from shawn
//

// FIXME: this code should be deprecated and repositioned under the 1k
// limit once BETA 6 is no longer supported.
#define VIEW_URL                        1100 // TAB_POSITION URL ICON "TAB_TEXT"


#define F2_LOGIN_REQUEST		10400	// <UID> <MID> <CID> <ClientVersion>
#define F2_LOGIN_OK			10410	// <systime> (ok)
#define F2_LOGIN_SSL_CTX_EXP		10421	// <> (SSL context expired)
#define F2_LOGIN_ERROR			10430	// <> (unknown error)
#define F2_LOGIN_AUTH_DATA		10450	// <pswd> <transRes> <burnRes>
#define F2_LOGIN_AUTH_OK		10460	// <> ok
#define F2_LOGIN_AUTH_BAD_PSWD		10473	// <> (invalid password)
#define F2_DOWNLOAD_ATTEMPT		10607	// (ssl pp) <dUID> <md5> <keyId>

#define NAPNET_HELLO			12000
#define NAPNET_GOODBYE			12666

#endif
