/*
 * $Id: db.hh,v 1.11.8.3.2.6 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_DB_HH
#define _NAPD_DB_HH

#include <list>
#include <slist>

#define DEFAULT_PORT			8888

#define DB_USER				"napd"
#define DB_PASS				"napd"
#define DB_TNS				"fdev"

#define BIND2(q, x)			(q)->bind(":" #x, &(x))				//jdb
#define BIND3(q, x)			(q)->bind(":" #x, (x), sizeof(x))		//jdb
#define BIND4(q, x)			(q)->bind(":" #x, (x), sizeof(x), SQLT_BIN)	//jdb
#define BINDT(q, x) 			(q)->bindTime(":" #x, &(x))			//vn

// tables

#define DBT_USERS                 "users"
#define DBT_NOTIFIES              "notifies"
#define DBT_IGNORES               "ignores"

#define DBT_BLOCKS                "hostblocks"
#define DBT_USERBANS              "userbans"
#define DBT_HOSTBANS              "hostbans"

#define DBT_ABILENE               "abilene"
#define DBT_CONFIG                "config"
#define DBT_CHANNELS              "channels"
#define DBT_SERVERS               "servers"


#define DBT_SURVEY                "survey"
#define DBT_EMAILS                "emails"


// DataBase Fields
#define DBF_USERS_ID              "id"
#define DBF_USERS_NAME            "username"
#define DBF_USERS_UNAME           "ucasename"
#define DBF_USERS_PASSWORD        "userpassword"
#define DBF_USERS_LASTPORT        "port"
#define DBF_USERS_LEVEL           "userlevel"
#define DBF_USERS_CREATION        "creation"
#define DBF_USERS_FLAG_MUTED      "muted"
#define DBF_USERS_FLAG_MUZZLED    "muzzled"
#define DBF_USERS_FLAG_CLOAKED    "cloaked"

#define DBF_LASTLOGIN_ID          "id"
#define DBF_LASTLOGIN_TIME        "login"

#define DBF_EMAILS_ENTRY          "email"

#define DBF_CONFIG_NAME           "name"
#define DBF_CONFIG_VALUE          "value"

#define DBF_NOTIFIES_ID           "id"
#define DBF_NOTIFIES_WHOTONOTIFY  "idtonotify"

#define DBF_IGNORES_ID            "id"
#define DBF_IGNORES_WHOTOIGNORE   "whotoignore"

#define DBF_USERBANS_USERID       "id"
#define DBF_USERBANS_BANNER       "banner"
#define DBF_USERBANS_REASON       "reason"
#define DBF_USERBANS_TIME         "time"
#define DBF_USERBANS_FLAGS        "flags"

#define DBF_HOSTBANS_IP           "ip"
#define DBF_HOSTBANS_BANNER       "banner"
#define DBF_HOSTBANS_REASON       "reason"
#define DBF_HOSTBANS_TIME         "time"
#define DBF_HOSTBANS_FLAGS        "flags"

#define DBF_BLOCKS_ENTRY          "ip"
#define DBF_BLOCKS_BLOCKER        "blocker"
#define DBF_BLOCKS_REASON         "reason"
#define DBF_BLOCKS_TIME           "time"
#define DBF_BLOCKS_FLAGS          "flags"

#define DBF_CHANNELS_NAME         "channelname"
#define DBF_CHANNELS_LEVEL        "channellevel"
#define DBF_CHANNELS_LIMIT        "userlimit"
#define DBF_CHANNELS_MOTD         "motd"
#define DBF_CHANNELS_TOPIC        "topic"

#define DBF_SURVEY_NAME           "realname"
#define DBF_SURVEY_ADDRESS        "address"
#define DBF_SURVEY_CITY           "city"
#define DBF_SURVEY_STATE          "state"
#define DBF_SURVEY_COUNTRY        "country"
#define DBF_SURVEY_PHONE          "phone"
#define DBF_SURVEY_AGE            "age"
#define DBF_SURVEY_INCOME         "income"
#define DBF_SURVEY_EDUCATION      "education"

#define DBF_ABILENE_IP            "ip"
#define DBF_ABILENE_MASK          "mask"
#define DBF_ABILENE_COMMUNITY     "community"

#define DBF_SERVERS_CLUSTER       "clusterid"
#define DBF_SERVERS_NAME          "name"
#define DBF_SERVERS_IP            "ip"
#define DBF_SERVERS_PORTS         "ports"
#define DBF_SERVERS_KEY           "password"
#define DBF_SERVERS_DB            "db"

#define DBF_LOCKS_ID              "lock_id"
#define DBF_LOCKS_USERNAME        "username"
#define DBF_LOCKS_REASON          "reason"

#define DBF_VERSIONS_OLD          "oldversion"
#define DBF_VERSIONS_CURRENT      "curversion"
#define DBF_VERSIONS_LOCATION     "location"
#define DBF_VERSIONS_BANONLY      "banonly"

bool LoadBans(slist<class Ban*>*);
bool UpdateBans(slist<class BanDelta>*);
bool LoadIgnores(class IgnoreList *);
bool UpdateIgnores(slist<class IgnoreDelta>*, class Client *);
bool LoadChannels(class ChannelList *);
int db_init(char *db_info);

#endif
