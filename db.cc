/*
 * $Id: db.cc,v 1.1.2.2 2001/08/24 02:19:17 dbrumleve Exp $
 */

#define _NAPD_DB_CC
#include "db.hh"
#include "client.hh"
#include "defines.hh"
#include "server.hh"
#include "channel.hh"

int db_init(char *db_info) {return 0;};
#if 0
bool Client::login(const char *name, const char *password) { return -1; }
bool Client::load(const char *n) { return -1; }
bool Client::update() { return -1; }

#include <DBOra.hh>

DBConn *db = NULL;

#ifdef ORACLE_STATIC
int wtcLerr, wtcMerr, wtcstu, wtclkm, wtcsrin;
#endif

int db_init(char *db_info) {
  char *db_user = db_info, *db_pass, *db_tns;

  if (char *p = strchr(db_user, '/')) {
    *p++ = '\0';
    db_pass = p;
  } else
    return -1;

  if (char *p = strchr(db_pass, '@')) {
    *p++ = '\0';
    db_tns = p;
  } else
    return -1;

  db = new DBConn (db_user, db_pass, db_tns);

  if (db->isErr()){
    LOG(ERR, "db connect to TNS %s as %s failed", db_tns, db_user);
    return false;
  }

  db->setErrWriteMessages(&cerr);
  log.info("Server::Init: DB server:\n\n%s", db->getVersion().c_str());
  return true;
}


bool LoadBans(slist<Ban*>* bans) {
  // cache the host bans, load them first
  DBQuery *q = db->query(
    "SELECT TRIM(ip) as ip, banner, reason, time, flags FROM " DBT_HOSTBANS
  );

  Ban *ban;

  while ((q->fetch())) {
    ban = new Ban(
      (const char *)(*q)[DBF_HOSTBANS_IP], 
      (const char *)(*q)[DBF_HOSTBANS_BANNER],
      (const char *)(*q)[DBF_HOSTBANS_REASON],
      (time_t)(*q)[DBF_HOSTBANS_TIME]
    );

    *((U8 *)&ban->flags) = (unsigned)(*q)[DBF_HOSTBANS_FLAGS];
    bans->push_front(ban);
  }

  return true;
}

// expect only hostname bans, since we dump username bans down
// immediately

bool UpdateBans(slist<BanDelta> *todo) {
  if (todo->empty())
    return true;

  Ban *b;
  bool ret = true;
  DBQuery *q ;

  foreach (i, *todo) {
    b = (*i).ban;

    switch ((*i).op) {
    case ADD:
      q = db->query(
        "INSERT INTO hostbans (ip, banner, reason) " 
        "VALUES (:ip, :banner, :reason)"
      );

      q->bind(":ip", b->address);
      q->bind(":reason", b->reason);
      q->bind(":banner", b->banner);

      if (!q->run()) {
        log.error("BanList::Update: query run failed "
          "ip[%s] reason[%s] banner[%s]", b->address, b->reason, b->banner
        );
        ret = false;
      } 

      break;
    case REMOVE:
      q = db->query( "DELETE FROM hostbans WHERE ip = RPAD(:ip, 15)");
      q->bind(":ip", b->address);
  
      if (!q->run()) {
        log.error("BanList::Update: query run failed with ip[%s]", b->address);
        ret = false;
      }

      delete b;
      break;
    }
  }
  
  todo->clear();

  return ret;
}

bool LoadChannels(ChannelList* channels) {
  DBQuery *q = db->query("SELECT * FROM " DBT_CHANNELS);
  
  Channel *channel;

  while (q->fetch()) {
    channel = new Channel(
      (const char *)(*q)[DBF_CHANNELS_NAME],
      (unsigned)(*q)[DBF_CHANNELS_LIMIT], 
      (unsigned)(*q)[DBF_CHANNELS_LEVEL]
    );

    channel->type = Channel::STATIC;
    channel->SetMOTD((const char *)(*q)[DBF_CHANNELS_MOTD]);
    channel->Topic((const char *)(*q)[DBF_CHANNELS_TOPIC]);

    channels->push_back(channel);
  }
  
  return true;
}

bool LoadIgnores(IgnoreList *ignores) {
  Client *ignorer = ignores->ignorer;
  
  if (!ignorer)
    return false;

  DBQuery *q = db->query(
    "SELECT username "
    "FROM ignores, users "
    "WHERE ignores.id = :id AND users.id = ignores.idtoignore"
  );

  long id = ignorer->id();
  q->bind(":id", id);

  while (q->fetch()) {
    const char *ignoree = (const char *)(*q)[DBF_USERS_NAME];
    ignores->push_front(strdup(ignoree));
  }

  return !db->isErr();
}

bool UpdateIgnores(slist<IgnoreDelta>* todo, Client *ignorer) {
  if (todo->empty())
    return true;

  char *ignoree;
  DBQuery *q;
  unsigned long dbID;

  foreach (i, *todo) {
    ignoree = (*i).ignoree;

    switch ((*i).op) {
    case ADD:
      q = db->query(
        "INSERT INTO ignores (id, idtoignore) "
        "SELECT :ignorerID, id FROM users "
        "WHERE ucasename = UPPER(:nametoignore)"
      );

      if (!(dbID = ignorer->id()))
        log.error("IgnoreList::Update: goddamit jim!");
      if (!ignoree)
        log.error("IgnoreList::Update: fucking christ, jim! (%s:%d)",
          ignorer->name, ignorer->id());

      q->bind(":ignorerID", dbID);
      q->bind(":nametoignore", ignoree);

      if (!q->run()) 
        log.error("IgnoreList::Update: query run (ADD) failed with "
          "dbID %d and nametoignore %s", ignorer->id(), ignoree);

      break;

    case REMOVE:
      q = db->query(
        "DELETE FROM ignores "
        "WHERE id = :ignorerID AND idtoignore = ("
        "  SELECT id FROM users WHERE ucasename = UPPER(:nametoignore)"
        ")"
      );

      long id = ignorer->id();
      q->bind(":ignorerID", id);
      q->bind(":nametoignore", ignoree);

      if (!q->run())
        log.error("IgnoreList::Update: query run (REMOVE) failed with ",
          "dbID %d and nametoignore %s", ignorer->id(), ignoree);

      delete ignoree;
      break;
    }
  }

  todo->clear();

  return true;
}



bool Client::load(const char *n) {
  //typedef char varchar2[4096];
  DBQuery *q;

  pn_port = 0, pn_user_level = 0, 
  pn_server_id = 0, pn_user_id = 0, pn_parent_id = 0, pn_service_level_id = 0,
  pn_user_machine_id = 0, pn_code_id = 0, pn_error_code = 0,
  pn_sec_public_key = 0, pn_sec_certificate = 0, pn_ssl_session = 0, pn_sec_ca_certificate = 0;
  
#define ZERO(x) memset((x), 0, sizeof(x))
  ZERO(pv_user_name);			//ZERO(pv_user_machine_name);	
  ZERO(pv_user_password);	
  ZERO(pv_parent_user_name);		/*ZERO(pv_port);*/			ZERO(pv_muted);
  ZERO(pv_muzzled);			ZERO(pv_cloaked);			/*ZERO(pv_user_level);*/
  ZERO(pr_sec_public_key);		ZERO(pv_sec_cert_serial_num);		ZERO(pr_sec_certificate);
  ZERO(pv_level_code);			ZERO(pr_sec_ca_certificate);
  ZERO(pr_ssl_session);			/*ZERO(pd_ssl_expiration_date);*/	ZERO(pv_enforce_code_id_flag);
  ZERO(pv_enforce_code_update_flag);	ZERO(pv_enforce_user_auth_flag);	ZERO(pv_blocked_flag);
  ZERO(pv_upload_blocked_flag);		ZERO(pv_low_bandwidth_flag);		ZERO(pv_child_login_flag);
  ZERO(pv_parent_not_fnd_flag);		ZERO(pv_service_level_not_fnd_flag);	ZERO(pv_user_machine_not_fnd_flag);
  ZERO(pv_block_im_flag);
  ZERO(pv_block_chat_flag);		ZERO(pv_enable_prof_flag);		ZERO(pv_transition_flag);
  ZERO(pv_ub_banner);			ZERO(pv_ub_reason);			ZERO(pv_ub_permanent);
  ZERO(pv_code_identifier);
#undef ZERO

  
  pn_server_id = 0;

  strncpy(pv_user_name, n, sizeof(pv_user_name) - 1);

  q = db->query(
    "BEGIN OLTP_NAPD_PKG.RETRIEVE_USER_INFO("
    "  :pv_user_name, :pv_user_machine_name, :pn_server_id, "
    "  :pn_user_id, :pv_user_password, :pn_parent_id, :pv_parent_user_name, "
    "  :pn_port, :pv_muted, :pv_muzzled, :pv_cloaked, "
    "  :pn_user_level, :pr_sec_public_key, :pn_sec_public_key, "
    "  :pr_sec_certificate, :pn_sec_certificate, :pr_sec_ca_certificate, :pn_sec_ca_certificate, "
    "  :pv_sec_cert_serial_num, :pn_service_level_id, "
    "  :pv_block_im_flag, :pv_block_chat_flag, :pv_enable_prof_flag, "
    "  :pv_transition_flag, :pv_level_code, :pn_user_machine_id, "
    "  :pn_code_id, :pv_code_identifier,  :pr_ssl_session, :pn_ssl_session, "
    "  :pd_ssl_expiration_date, :pv_enforce_code_id_flag, :pv_enforce_code_update_flag, "
    "  :pv_enforce_user_auth_flag, :pv_blocked_flag, :pv_upload_blocked_flag, "
    "  :pv_low_bandwidth_flag, :pv_child_login_flag, "
    "  :pv_parent_not_fnd_flag, :pv_service_level_not_fnd_flag, "
    "  :pv_user_machine_not_fnd_flag, "
    "  :pv_ub_banner, :pv_ub_reason, :pv_ub_permanent, "
    "  :pn_error_code"
    "); END;"
  );

  BIND3(q,pv_user_name);		BIND3(q,pv_user_machine_name);		BIND2(q,pn_server_id);
  BIND2(q,pn_user_id);			BIND3(q,pv_user_password);		BIND2(q,pn_parent_id);
  BIND3(q,pv_parent_user_name);		BIND2(q,pn_port);			BIND3(q,pv_muted);
  BIND3(q,pv_muzzled);			BIND3(q,pv_cloaked);			BIND2(q,pn_user_level);
  BIND4(q,pr_sec_public_key);		BIND3(q,pv_sec_cert_serial_num);	BIND4(q,pr_sec_certificate);
  BIND4(q,pr_sec_ca_certificate);	BIND2(q,pn_sec_ca_certificate);		BIND3(q,pv_level_code);	
  BIND2(q,pn_user_machine_id);		BIND2(q,pn_code_id);			BIND3(q,pv_code_identifier);
  BIND4(q,pr_ssl_session);		BINDT(q,pd_ssl_expiration_date);	BIND3(q,pv_enforce_code_id_flag);
  BIND3(q,pv_enforce_code_update_flag);	BIND3(q,pv_enforce_user_auth_flag);	BIND3(q,pv_blocked_flag);
  BIND3(q,pv_upload_blocked_flag);	BIND3(q,pv_low_bandwidth_flag);		BIND3(q,pv_child_login_flag);
  BIND3(q,pv_parent_not_fnd_flag);	BIND3(q,pv_service_level_not_fnd_flag);	BIND3(q,pv_user_machine_not_fnd_flag);
  BIND2(q,pn_error_code);		BIND2(q,pn_service_level_id);		BIND3(q,pv_block_im_flag);
  BIND3(q,pv_block_chat_flag);		BIND3(q,pv_enable_prof_flag);		BIND3(q,pv_transition_flag);
  BIND3(q,pv_ub_banner);		BIND3(q,pv_ub_reason);			BIND3(q,pv_ub_permanent);
  BIND2(q,pn_sec_public_key);		BIND2(q,pn_ssl_session);		BIND2(q,pn_sec_certificate);

cerr << *q << "\n";
cerr << "query running!\n";
cerr << "Name is: [" << pv_user_name << "]\n";

  if (q->run(), db->isErr()) {
    log.error("Client::Load: query run failed");
    cerr << "{" << pv_user_name << "} {" << pv_user_machine_name << "} {" << pn_server_id << "}\n";
    cerr << *q << "\n";
    return false;
  }
cerr << "query ran!\n";
cerr << "PN_ERROR_CODE = " << pn_error_code << "\n";
cerr << "{" << pv_user_name << "} {" << pv_user_machine_name << "} {" << pn_server_id << "}\n";

 if (!pn_user_id)
     return false;

  flags.muzzled			= (*pv_muzzled == 'Y');
  flags.cloaked			= (*pv_cloaked == 'Y');
  flags.muted			= (*pv_muted   == 'Y');

  if (!(load_options & LOAD_LASTLOGIN))
    _LastLogin			= NOW;

  // changed to use the username (case) they login with
  name = strdup(n);

  po_ssl_session.octets = pr_ssl_session;
  po_ssl_session.noctets = pn_ssl_session;
  po_sec_certificate.octets = pr_sec_certificate;
  po_sec_certificate.noctets = pn_sec_certificate;
  po_sec_public_key.octets = pr_sec_public_key;
  po_sec_public_key.noctets = pn_sec_public_key;
  
  if (!ignores.load(this))
    log.error("Client::Load: IgnoreList::Load failed for %s", name);

  return true;
}

int Client::login(const char *name, const char *password) {
  typedef char varchar2[256];
  DBQuery *q;

  varchar2 pv_user_name;
  time_t pd_transfer_date, pd_burn_date, pd_export_date;
  long pn_server_id = 0, pn_transfer_count = 0,
    pn_export_count = 0, pn_burn_count = 0, pn_error_code = 0;
  char hash[17];

#define ZERO(x) memset((x), 0, sizeof(x))
  ZERO(pv_user_name);	ZERO(pd_transfer_date);		ZERO(pd_burn_date);
  ZERO(pd_export_date);
#undef ZERO

  assert(name && strlen(name));
  assert(password && strlen(password));
  strcpy(hash, crypt_password(password));

  strncpy(pv_user_name, name, sizeof(pv_user_name) - 1);
  pn_server_id = 0;

  q = db->query(
    "BEGIN OLTP_NAPD_PKG.USER_LOGIN("
    "  :pv_user_name, :pn_server_id, :hash, "
    "  :pn_transfer_count, :pd_transfer_date, "
    "  :pn_export_count, :pd_export_date, "
    "  :pn_burn_count, :pd_burn_date, "
    "  :pn_error_code, 'Y'"
    "); END;"
  );

  BIND3(q,pv_user_name);
  BIND2(q,pn_server_id);
  BIND3(q,hash);
  BIND2(q,pn_transfer_count); 
  BIND2(q,pn_export_count);
  BIND2(q,pn_burn_count);
  BINDT(q,pd_transfer_date); 
  BINDT(q,pd_export_date);
  BINDT(q,pd_burn_date);
  BIND2(q,pn_error_code);

  if (q->run(), db->isErr()) {
    log.error("Client::Load: query run failed");
    return false;
  }

  if (pn_error_code)
    return false;

  transfers_used = 0;
  transfers = pn_transfer_count;

  return true;
}

// update the client and login record in the db.  it is up to the
// caller (Generally class Server) to call Update() everytime it
// changes something about the client.
bool Client::Update(void) {
  extern Server server;

  if (!_changed || !server.AllowDBWrites)
    return true;

  char *noyes[] = { "N", "Y" };

  DBQuery *q = db->query(
    "UPDATE users SET "
    "muted = :muted, "
    "muzzled = :muzzled, "
    "cloaked = :cloaked, "
    "userlevel = :userlevel "
    "WHERE id = :id"
  );

  q->bind(":muted", noyes[flags.muted]);
  q->bind(":muzzled", noyes[flags.muzzled]);
  q->bind(":cloaked", noyes[flags.cloaked]);
  q->bind(":userlevel", (unsigned &)_Level);

  q->bind(":id", pn_user_id);

  if (!q->run())
    log.error("Client::Update: query run failed");

  ignores.update();

  _changed = false;

  { // jdb
    typedef char varchar2[256];
    varchar2 pv_user_name, pd_transfer_date, pd_export_date, pd_burn_date;
    int pn_transfer_count, pn_export_count, pn_burn_count, pn_error_code;

    memset(pv_user_name, 0, sizeof(pv_user_name) - 1);
    strncpy(pv_user_name, name, sizeof(pv_user_name) - 1);
    pn_transfer_count = _TransferResourcesDelta;
    pn_export_count = _ExportResourcesDelta;
    pn_burn_count = _BurnResourcesDelta;
    pn_error_code = 0;

    DBQuery *q = db->query(
      "BEGIN "
      "OLTP_NAPD_PKG.REFRESH_USER_RESOURCES("
      "  :pv_user_name, "
      "  :pn_transfer_count, :pd_transfer_date, "
      "  :pn_export_count, :pd_export_date, "
      "  :pn_burn_count, :pd_burn_date, "
      "  :pn_error_code "
      "); "
      "END;"
    );

    BIND3(q,pv_user_name);
    BIND2(q,pn_transfer_count);
    BIND2(q,pn_burn_count);
    BIND2(q,pn_export_count);
    BIND3(q,pd_transfer_date);
    BIND3(q,pd_burn_date);
    BIND3(q,pd_export_date);
    BIND2(q,pn_error_code);

    if (q->run(), db->isErr())
      log.error("Client::Update: could not refresh user");

    _TransferResourcesDelta = 0;
    _ExportResourcesDelta = 0;
    _BurnResourcesDelta = 0;

    _TransferResources = pn_transfer_count;
    _ExportResources = pn_export_count;
    _BurnResources = pn_burn_count;
  }

  return true;
}

bool Server::LoadConfig(void) {
  DBQuery *q = db->query("SELECT * FROM config");
  
  while (q->fetch()) {
    const char *n = (const char *)(*q)[DBF_CONFIG_NAME];
    const char *v = (const char *)(*q)[DBF_CONFIG_VALUE];
    unsigned vu = (unsigned)(*q)[DBF_CONFIG_VALUE];
    bool vb = !!vu;

    if (!strcasecmp(n, "max_search_results"))			MaxSearchResults = vu;
    else if (!strcasecmp(n, "max_search_duplicates"))		MaxSearchDuplicates = vu;
    else if (!strcasecmp(n, "max_hash_results"))		MaxHashResults = vu;
    else if (!strcasecmp(n, "max_remote_drops"))		MaxRemoteDrops = vu;
    else if (!strcasecmp(n, "max_searches"))			MaxSearches = vu;
    else if (!strcasecmp(n, "max_search_time"))			MaxSearchTime = vu;
    else if (!strcasecmp(n, "max_search_servers"))		MaxSearchServers = vu;
    else if (!strcasecmp(n, "max_matches"))			MaxMatches = vu;
    else if (!strcasecmp(n, "max_user_channels"))		MaxUserChannels = vu;
    else if (!strcasecmp(n, "max_notifies"))			MaxNotifies = vu;
    else if (!strcasecmp(n, "drop_timeout"))			DropTimeout = vu;
    else if (!strcasecmp(n, "sweep_blocked_files"))		SweepBlockedFiles = vu;
    else if (!strcasecmp(n, "packetq_timeout"))			PacketQTimeout = vu;
    else if (!strcasecmp(n, "throttle_timeout"))		ThrottleTimeout = vu;
    else if (!strcasecmp(n, "allow_failed_logins"))		AllowFailedLogins = vb;
    else if (!strcasecmp(n, "allow_db_writes"))			AllowDBWrites = vb;
    else if (!strcasecmp(n, "allow_login_write"))		AllowLoginWrite = vb;
    else if (!strcasecmp(n, "allow_peer_browsing"))		AllowPeerBrowsing = vb;
    else if (!strcasecmp(n, "link_allow_searches"))		LinkAllowSearches = vb;
    else if (!strcasecmp(n, "link_allow_browsing"))		LinkAllowBrowsing = vb;
    else if (!strcasecmp(n, "link_allow_im"))			LinkAllowIM = vb;
    else if (!strcasecmp(n, "link_allow_downloads"))		LinkAllowDownloads = vb;
    else if (!strcasecmp(n, "allow_new_channels"))		AllowNewChannels = vb;
    else if (!strcasecmp(n, "allow_badword_filter"))		AllowBadWordFilter = vb;
    else if (!strcasecmp(n, "allow_downloading"))		AllowDownloading = vb;
    else if (!strcasecmp(n, "allow_searching"))			AllowSearching = vb;
    else if (!strcasecmp(n, "ban_timeout"))			BanTimeout = vb;
    else if (!strcasecmp(n, "log_level"))			log.level(vu);
    else if (!strcasecmp(n, "forward_searches"))		ForwardSearches = vb;
    else if (!strcasecmp(n, "allow_virtual"))			AllowVirtual = vb;
    else if (!strcasecmp(n, "blockoldvernewuser"))		BlockOldVerNewUser = vb;

    else if (!strcasecmp(n, "badwords")) {
      if (!BadWords.empty()) {
        foreach (i, BadWords) delete[] *i;
        BadWords.clear();
      }
       
      // make temp copy so we can tokenize it
      char *db_badwords_base = my_strdup(v);
      char *db_badwords = db_badwords_base;
      char *badword;

      // we need a pointer to manipulate - otherwise free() will be passed an invalid pointer
      char *db_badwords_tok = db_badwords;
      while (db_badwords_tok != '\0' &&
             (badword = strsep(&db_badwords_tok, " ,")))
        if (strlen(badword))
          BadWords.push_front(my_strdup(badword));

      delete[] db_badwords_base;

    } else if (!strcasecmp(n, "file_log")) {
      if (vu == 1 && !filelog) {
	char filename[32];
	sprintf(filename, "file.log.%s.%d", LocalIP.c_str(), ServerPort);
	filelog = fopen(filename, "a");
	
	log.notice("Server::LoadConfig: turned on filelog from config");
      } else if (vu == 0 && filelog) {
	fclose(filelog);
	filelog = NULL;
      }

    } else if (!strcasecmp(n, "xfer_log")) {

      if (vu == 1 && !xferlog) {
	char filename[32];
	sprintf(filename, "xfer.log.%s.%d", LocalIP.c_str(), ServerPort);
	xferlog = fopen(filename, "a");
	  
	log.notice("Server::LoadConfig: turned on xferlog from config");
      } else if (vu == 0 && xferlog) {
	fclose(xferlog);
	xferlog = NULL;
      }
    }
  } 
  return true;
}

char *Server::GetDB(void) {
  DBQuery *q = db->query("SELECT db FROM servers WHERE ip = :ip");
  
  q->bind(":ip", LocalIP.c_str());
  if (!q->fetch())
    return NULL;

  return my_strdup((const char *)(*q)[DBF_SERVERS_DB]);
}

// purpose is to find myself in the table, and return true if I'm
// supposed to link to anyone.  if a db string is requested of me,
// return it.
bool Server::CheckLinking(void) {
  DBQuery *q = db->query("SELECT * FROM servers WHERE ip = :ip");
  
  q->bind(":ip", LocalIP.c_str());
  bool found = false;
  signed old_cluster = r_cluster;

  if ((q->fetch())) {  // only one potential entry
    char *db_ports_base = my_strdup((const char*)(*q)[DBF_SERVERS_PORTS]);
    char *db_ports = db_ports_base;
    char *portstr;
      
    char **db_ports_tok = &db_ports;
    while ((portstr = my_strsep(db_ports_tok, ","))) {
      log.debug("Server::CheckLinking: examining %s:%s for self", LocalIP.c_str(), portstr);

      if (atoi(portstr) == ServerPort) {	  // ME!
	r_cluster = (unsigned)(*q)[DBF_SERVERS_CLUSTER];

	delete[] r_name;
        r_name = my_strdup((const char *)(*q)[DBF_SERVERS_NAME]);

	delete[] r_key;
        r_key = my_strdup((const char *)(*q)[DBF_SERVERS_KEY]);
	
	log.debug("Server::CheckLinking: found myself: (%d:%s) %s:%d [%s]", 
		  r_cluster, r_name, LocalIP.c_str(), ServerPort, r_key);

	found = true;
      }	    
    }

    delete[] db_ports_base;
  }

  if (r_cluster != old_cluster) { // if our cluster changed
    log.debug("Server::CheckLinking: my cluster changed %d -> %d, clearing links",
	      old_cluster, r_cluster);
    // close all links, can't delete because that will invalidate iterators    
    foreach (q, RemoteServers) {
      (*q)->Say(LINK_SHUTDOWN, "cluster changed %d -> %d, delinking", old_cluster, r_cluster);
      (*q)->Status(OFFLINE);
    }

    foreach (q, ReconnectQueue)
      delete *q;

    ReconnectQueue.clear();
  }

  // should we relink?
  if (r_cluster == 0) {
    log.debug("Server::CheckLinking: db told me my cluster is 0, setting -1"); 
    r_cluster = -1;
  }

  if (!found) {
    log.debug("Server::CheckLinking: didn't find myself in the db, setting -1");
    r_cluster = -1;
  }

  return (r_cluster == -1)?false:true;
}

bool Server::LoadRemotes(void) {
  // initial condition, CheckLinking wasn't called or something else is borked
  if (r_cluster < 1) {
    log.error("Server::LoadRemotes: cluster == %d, someone doesn't want me to link", r_cluster);
    return false;
  }

  DBQuery *q = db->query("SELECT * FROM servers");
  unsigned total_servers = 0;

  while ((q->fetch())) {
    total_servers++;

    unsigned cluster = (unsigned)(*q)[DBF_SERVERS_CLUSTER];
    if ((int)cluster != r_cluster) // if not in my cluster id
      continue;

    char *db_ports_base = my_strdup((const char*)(*q)[DBF_SERVERS_PORTS]);
    char *db_ports = db_ports_base;
    char *portstr;

    char **db_ports_tok = &db_ports;
    while ((portstr = my_strsep(db_ports_tok, " ,"))) {
      if (LocalIP == (const char *)(*q)[DBF_SERVERS_IP] && ServerPort == atoi(portstr))
	continue;  // can't res.erase(i), so I just have to detect it

      char *name = my_strdup((const char*)(*q)[DBF_SERVERS_NAME]);
      char *key  = my_strdup((const char*)(*q)[DBF_SERVERS_KEY]);
      char *host = my_strdup((const char*)(*q)[DBF_SERVERS_IP]);
      unsigned port = atoi(portstr) - 1;
      
      // else add as a remote server to the list

      Connection *con = new Connection();
      RemoteServer *rs = new RemoteServer(con);

      rs->Key(key);
      rs->Name(name);

      rs->Connect(host, port);
      RemoteServers.push_back(rs);

      delete[] name;
      delete[] key;
      delete[] host;
    }

    delete[] db_ports_base;
  }

  log.debug("Server::LoadRemotes: examined %d server entries", total_servers);
  log.debug("Server::LoadRemotes: linked to %d servers", RemoteServers.size());

  return true;
}

bool Server::ReloadRemotes(Client *caller) {
  if (r_cluster < 1)
    return false;

  DBQuery *q = db->query("SELECT * FROM servers WHERE clusterid = :clusterid");

  q->bind(":clusterid", r_cluster);
  unsigned new_servers = 0, total_servers = 0;

  while (( q->fetch())) {
    total_servers++;

    char *db_ports_base = my_strdup((const char*)(*q)[DBF_SERVERS_PORTS]);
    char *db_ports = db_ports_base;
    char **db_ports_tok = &db_ports;
    char *portstr;

    while ((portstr = my_strsep(db_ports_tok, " ,"))) {
      if (LocalIP == (const char *)(*q)[DBF_SERVERS_IP] && ServerPort == atoi(portstr))
	continue;  // found myself

      char *name = my_strdup((const char*)(*q)[DBF_SERVERS_NAME]); 
      char *key  = my_strdup((const char*)(*q)[DBF_SERVERS_KEY]);
      char *host = my_strdup((const char*)(*q)[DBF_SERVERS_IP]);
      unsigned port = atoi(portstr) - 1;

      bool found = false;

      // we have an entry that is not us, and in our cluster.  check
      // our server listing and reconnect queue
      vector<RemoteServer*>::iterator q = RemoteServers.begin();
      while (q != RemoteServers.end() && !found) {
	// is it someone we know about?
	if (!strcmp((*q)->Address(), host) && (*q)->Port() == port) {
	  found = true;

	  // some one the sly updating of keys
	  if ((*q)->Key() && strcmp((*q)->Key(), key))
	    if (caller)
	      caller->Say(ERROR_MESSAGE, "Server::ReloadRemotes: "
			  "%s:%d's key (%s) is different from what we thought it was (%s)", 
			  host, port, key, (*q)->Key());
	}
	q++;
      }

      if (!found) {
	q = ReconnectQueue.begin();
	while (q != ReconnectQueue.end() && !found) {
	  if (!strcmp((*q)->Address(), host) && (*q)->Port() == port) {
	    found = true;

	    if ((*q)->Key() && strcmp((*q)->Key(), key)) {
	      if (caller)
		caller->Say(ERROR_MESSAGE, "Server::ReloadRemotes: "
			    "%s:%d is in the reconnect queue and their key is different, setting to %s", 
			    host, port, key);
	      (*q)->Key(key);
	    }
	  }
	  q++;
	}
      }

      // if still not found, then we have a new entry
      if (!found) {
	new_servers++;
	if (caller)
	  caller->Say(SYSTEM_MESSAGE, "Server::ReloadRemotes: discovered new host %s:%d", 
		      host, port);
	RemoteServer *rs = new RemoteServer(new Connection());
	rs->Key(key);
        rs->Name(name);

	rs->Connect(host, port);
	RemoteServers.push_back(rs);
      }
    
      delete[] name;
      delete[] key;
      delete[] host;
    }

    delete[] db_ports_base;
  }

  log.debug("Server::ReloadRemotes: examined %d server entries; %d are new", 
	    total_servers, new_servers);

  return (bool)new_servers;
}


bool Server::LoadServerId(void)
{
       DBQuery *q;
			 
      char  pv_server_name[5], pv_server_type[5];
      long pn_error_code= 0;
			 
#define ZERO(x) memset((x), 0, sizeof(x))
          ZERO(pv_server_name); ZERO (pv_server_type);
#undef ZERO
         strcpy(pv_server_type, "NAPD");
		       
         q = db->query(
										                            "BEGIN NAPD_EMD_PKG.ACQUIRE_SERVER_ID("
                 "  :pv_server_type, :pn_server_id,"
        	 "  :pv_server_name, :pn_error_code "
		 "); END;"
		  );
							        
		BIND3(q, pv_server_type);
		BIND2(q,pn_server_id);
		BIND3(q,pv_server_name);
		BIND2(q,pn_error_code);
													        
		//TODO what are the error codes?
		 if (q->run(), db->isErr()) {
		      log.error("Server::LoadServerId: query run failed err code %d ", pn_error_code);
		      return false;
		}
                                         
		/** this proc doesn't see to be working although it doesn't 
		 throw any excpetions */
		//_serverId = pno_server_id;
		log.info( "Server Identifier obtained : %ld", pn_server_id);
		return true;
}                                                                     

bool Server::ReleaseServerId (void){
    DBQuery *q;

    long pn_error_code = 0;
    char pv_server_type[5] = {0};
    sprintf (pv_server_type, "%s", "NAPD");
    q = db->query (
	"BEGIN NAPD_EMD_PKG.RELEASE_SERVER_ID ("
	"  :pn_server_id, :pv_server_type,"
        "  :pn_error_code "
        "); END;"
       );
	
    BIND3(q, pv_server_type);
    BIND2(q,pn_server_id);
    BIND2(q,pn_error_code);        
 //TODO what are the error codes?
    if (q->run(), db->isErr()) {
        log.error("Server::ReleaseServerId: query run failed err Code %d", pn_error_code);
        return false;
     }                     
    
    log.info("Server Identifier Released : %ld", pn_server_id);
    pn_server_id = 0;
    return true;      
}

bool Server::UserExist(const char *username) {
  if (!username || !strlen(username))
    return false;

  // for efficiency's sake, let's try hitting the currently loaded
  // userlist.  if we hit, we've just saved ourselves a db access.
  if ((GetLocalUser(username)) || (GetRemoteUser(username))) 
    return true;

  unsigned id = 0;
  DBQuery *q = db->query("BEGIN "
    "SELECT id INTO :id FROM users WHERE ucasename = UPPER(:username);"
    "END;"
  );

  q->bind(":id", id);
  q->bind(":username", username);

  if (!q->run())
    return false;

  return (bool)id;
}

#endif
