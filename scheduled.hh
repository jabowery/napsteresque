/*
 * $Id: scheduled.hh,v 1.10.6.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_SCHEDULED_HH
#define _NAPD_SCHEDULED_HH

// only bool(*)(void) prototypes should go here

bool ReconnectServers(void);
bool RandomizeServers(void);

bool UpdateUserStats(void);
bool UpdateServerStats(void);

bool ReloadBans(void);
bool ReloadServers(void);

bool CheckIntegrity(void);

bool TimeoutBans(void);
bool TimeoutBlocks(void);

bool SendLinkHeartbeat(void);

bool VerifyDBConnection(void);

#ifdef RIAALOG
bool RIAALogOn(void);
bool FlushRIAALog(void);
#endif

#endif
