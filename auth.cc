/* 
 * $Id: auth.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_AUTH_CC

#include "server.hh"
#include "functions.hh"
#include "command.hh"

#if 0

bool Server::F2Login(Client* caller, Command *com) {
  char *uid = NULL, *mid = NULL, *cid = NULL, *version = NULL;
  UNPACK_COMMAND(com, &uid, &mid, &cid, &version);
  sprintf(caller->pv_user_machine_name, "%d", (int)mid[0]);

  if (caller->load(uid)) {
    // Check the validity of the session //vn 
    if (caller->pd_ssl_expiration_date <= NOW) {
      SAY(caller, F2_LOGIN_SSL_CTX_EXP);
      return false;
    }

    //vn
    //TODO Check for the certificate and pass the new certificate if any
    //TODO Check for CodeID and valicate the client
    caller->version = strdup(version);

    caller->pv_code_identifier[sizeof(caller->pv_code_identifier) - 1] = '\0';
    strncpy(caller->pv_code_identifier, cid, sizeof(caller->pv_code_identifier) - 1);

    //Set a pointer to the previously saved SSL Session (to be used on callback for SSL Handshake)
/*
    if (!SecureConnection::storeSSLSession(caller->GetSSLHandle(), &(caller->po_ssl_session))) {
      SAY(caller, F2_LOGIN_SSL_CTX_EXP);
      return false;
    }
*/

    SAY(caller, F2_LOGIN_OK, "OK");
    return true;
  } else {
    SAY(caller, F2_LOGIN_ERROR);
    return false;
  }
}

bool Server::F2AuthData(Client* caller, Command *com) {
  char *passwd = NULL, *cid = NULL, *mid = NULL, *csn = NULL, *port = NULL;
  I32 lineSpeed = 0;

  UNPACK_COMMAND(com,
    &passwd, &cid, &mid, 
    (I32 *)NULL, (I32 *)NULL, (I32 *)NULL, (I32 *)NULL,
    &csn, &port, &lineSpeed
  );

  if (!passwd || !caller->pv_user_password) {
    SAY(caller, F2_LOGIN_AUTH_BAD_PSWD);
    return false;
  } 

  if (strcmp(crypt_password(passwd), caller->pv_user_password)) {
    SAY(caller, F2_LOGIN_AUTH_BAD_PSWD);
    return false;
  }

  caller->linespeed	= lineSpeed;
  caller->dataport	= atoi(port);

  caller->login(caller->name, passwd);
  SAY(caller, ACCOUNT_UPDATE_ARGS(caller));
  SAY(caller, F2_LOGIN_AUTH_OK);

  return true;                   
}

#endif
