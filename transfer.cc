/* 
 * $Id: transfer.cc,v 1.34.4.1.2.5 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_TRANSFER_CC

#include "server.hh"
#include "functions.hh"
#if 0
#include <fil_ssl_rt.h>

// caller has the file, he wants to tell the recipient to delay a while
void Server::DelayDownload(Client *caller, Command *com) {
  char *username, *filename;
  U32 maxdl;
  UNPACK_COMMAND(com, &username, &filename, &maxdl);

  File *file;
  if (!(file = caller->FindFile(filename))) {
    log.debug("Server::DelayDownload: %s asked %s to delay download of nonexistant file %s", 
	      caller->name, username, filename);
    return;
  }

  Client *lc;
  RemoteClient *rc;

  // Note: Remote ignore handled on the other end
  if ((lc = GetLocalUser(username)) && 
      !lc->IsIgnoring(caller) && lc->IsVisibleTo(caller)) {
    lc->Say(DOWNLOAD_DELAYED, "%s \"%s\" %lu %d", caller->name, filename, file->size, maxdl);
  } else if (LinkAllowDownloads &&
	     (rc = GetRemoteUser(username))) {
    rc->Say(REMOTE_DL_DELAYED, "%s \"%s\" %lu %d", caller->name, filename, file->size, maxdl);
  } else {
    caller->Say(DL_NOT_FOUND, "%s \"%s\"", caller->name, filename);
  }
}

void Server::DownloadRequest(Client *caller, Command *com) {
  char *username, *filename;
  Client *lc;
  RemoteClient *rc;

  if (!caller->AnyTransferResources()) {
    caller->Say(DL_DENIED, "%s", com->data);
    return;
  }

  UNPACK_COMMAND(com, &username, &filename);

  if (xferlog)
    fprintf(xferlog, "%ld %s\n", NOW, filename);

  if ((lc = GetLocalUser(username))) {
    if (!lc->IsIgnoring(caller) && lc->IsVisibleTo(caller)) {
      if (!caller->DataPort() && !lc->DataPort())  {
        caller->Say(DOWNLOAD_RESPONSE, "%s %lu %d \"%s\" firewallerror 0",
                    lc->name, lc->IP(), lc->DataPort(), filename);
      } else {
        char* CID = caller->pv_code_identifier;
        Certificate* Cert = NULL;
        OctetString* ans1EncodedPK = NULL;

        if ((Cert = d_Certificate(&caller->po_sec_certificate))) {
          if (!(ans1EncodedPK = e_KeyInfo(Cert->tbs->subjectPK)))
            log.error("Failed to get PK from Cert");
        } else {
          log.error("Failed to create cert object from cert bytestream");
        }

#warning "KLUDGE /jdb"
        CID[9] = 0;
        Command c(F2_DOWNLOAD_ATTEMPT);
        PACK_COMMAND(&c,
          caller->name, filename, Data(ans1EncodedPK->octets, ans1EncodedPK->noctets), CID, (I32)caller->LineSpeed());
        lc->Say(c, SSL_RT_F2_DOWNLOAD_ATTEMPT);
      }

 /********************************************************************/
    }
  } else if (LinkAllowDownloads &&
	     (rc = GetRemoteUser(username))) {
    if (!caller->DataPort() && !rc->DataPort()) 
      caller->Say(DOWNLOAD_RESPONSE, "%s %lu %d \"%s\" firewallerror 0",
  	 	  rc->name, rc->IP(), rc->DataPort(), filename);
    else {
      char* CID = caller->pv_code_identifier;
      Certificate* Cert = NULL;
      OctetString* ans1EncodedPK = NULL;

      if ((Cert = d_Certificate(&caller->po_sec_certificate))) {
        if (!(ans1EncodedPK = e_KeyInfo(Cert->tbs->subjectPK)))
          log.error("Failed to get PK from Cert");
      } else {
        log.error("Failed to create cert object from cert bytestream");
      }

#warning "KLUDGE /jdb"
      CID[9] = 0;
      Command c(REMOTE_DL_ATTEMPT);
      PACK_COMMAND(&c, rc->name,
        caller->name, filename, Data(ans1EncodedPK->octets, ans1EncodedPK->noctets), CID, (I32)caller->LineSpeed());
      rc->Server()->Say(c);
    }
  } else {
    log.debug("Server::DownloadRequest: %s asked %s to download nonexistant file %s", 
	      caller->name, username, filename);
    caller->Say(DL_NOT_FOUND, "%s \"%s\"", username, filename);
  }
}

void Server::AcceptFailed(Client *caller, Command *com) {
  char *username, *filename;
  UNPACK_COMMAND(com, &username, &filename);

  Client *lc = GetLocalUser(username);
  if (!lc || lc->IsIgnoring(caller) || !lc->IsVisibleTo(caller)) {
    if (RemoteClient *rc = GetRemoteUser(username))
      rc->Server()->Say(REMOTE_ACCEPT_FAILED, "%s %s \"%s\"", rc->name, caller->name, filename);
    return;
  }

  Command c(ACCEPT_FAILED); 
  PACK_COMMAND(&c, (char *)caller->name, filename);
  lc->Say(c);
}

void Server::DownloadAccepted(Client *caller, Command *com) {
  char *username, *filename;
  UNPACK_COMMAND(com, &username, &filename);

  cerr << "DOWNLOAD_ACCEPTED: " << username << " " << filename << "\n";
  cerr << "caller->Name: " << caller->name << "\n";
  cerr << "AllowDownloading: " << (AllowDownloading?1:0) << "\n";

  File *file;
  Client *lc;
  RemoteClient *rc;

  // Note: Ignore for remote handled on the other end

  if (AllowDownloading && (lc = GetLocalUser(username)) && lc->IsVisibleTo(caller)) {
    if (!(file = caller->FindFile(filename))) {
      log.debug("Server::DownloadAccepted: %s was asked for nonexistant file %s",
        caller->name, filename);
      lc->Say(DL_NOT_FOUND, "%s \"%s\"", lc->name, filename);
    } else {

fprintf(stderr, "DOWNLOAD_RESPONSE: %s %lu %d \"%s\" %s %d\n",
  caller->name, caller->IP(), caller->DataPort(),
  filename, file->id, caller->LineSpeed());

      lc->Say(DOWNLOAD_RESPONSE, "%s %lu %d \"%s\" %s %d",
        caller->name, caller->IP(), caller->DataPort(),
        filename, file->id, caller->LineSpeed());
      NumTransfers++;
      NumBytesTransfers += file->size;
    }
  } else if (AllowDownloading && LinkAllowDownloads && (rc = GetRemoteUser(username))) {
    log.debug("Server::DownloadAccepted: Found remote client");
    if ((file = caller->FindFile(filename))) {
      log.debug("Server::DownloadAccepted: Sending Download Response");
      rc->Say(REMOTE_DL_RESPONSE, "%s %lu %d \"%s\" %s %d", 
        caller->name, caller->IP(),
        caller->DataPort(), filename, file->id, caller->LineSpeed());
    } else {
      log.debug("Server::DownloadAccepted: Couldn't find file in user's list");
      rc->Say(REMOTE_DL_NOT_FOUND, "%s \"%s\"", caller->name, filename);
    }
  } else
    caller->Say(ACCEPT_FAILED, "%s \"%s\"", username, filename);
}

void Server::DownloadComplete(Client *caller, Command *com) {
  I32 success;
  UNPACK_COMMAND(com, &success);

  if (caller->Downloading)
    caller->Downloading--; 
  if (success)
    caller->DecrementTransferResources();
}

void Server::FirewallTransfer(Client *caller, Command *com) {
  char *username, *filename;
  Client *lc;
  RemoteClient *rc;
  UNPACK_COMMAND(com, &username, &filename);

  char fileid[FILEID_MAXBUF]; 

  // Note: Ignore handled on remote end
  if ((lc = GetLocalUser(username)) && 
      !lc->IsIgnoring(caller) &&
      lc->IsVisibleTo(caller)) {
    if (lc->FindFileID(filename, fileid)) {
      lc->Say(FIREWALL_RESPONSE, "%s %lu %d \"%s\" %s %d", 
	      caller->name, caller->IP(),
	      caller->DataPort(), filename, fileid,
	      (unsigned)caller->LineSpeed()); 
      fprintf(stderr, "FIREWALL RESPONSE: %s %lu %d \"%s\" %s %d\n", 
	      caller->name, caller->IP(),
	      caller->DataPort(), filename, fileid,
	      (unsigned)caller->LineSpeed()); 
    }
  } else if ((rc = GetRemoteUser(username))) { 
    rc->Server()->Say(REMOTE_FW_RESPONSE, "%s %s %lu %d \"%s\" %d", username, 
                      caller->name, caller->IP(), caller->DataPort(),
                      filename, (unsigned)caller->LineSpeed());
  } else caller->Say(DL_NOT_FOUND, "%s \"%s\"", username, filename);
}

#endif
