/* 
 *  $Id: codes.cc,v 1.3.34.4 2001/08/23 18:29:28 dbrumleve Exp $
 */

#define _NAPD_CODES_CC

#include "codes.hh"

/*
  The payload_relevant portion is for whether we care about the
  payload of the command in triggering the handler for that particular
  class of command.  This is to say that multiple channel_messages
  should be throttled regardless of what the data is, but we can
  expect multiple add files to be flooded so the payload must be
  relevant for judging whether or not to drop a packet.

  For the time being, payload_relevant is not regarded for any
  throttle class command.
 */

//  COMMAND_CODE           command_name             flood-class   payload_relevant

struct _cmd_tab cmd_tab[] = {

  // offline 
  { LOGIN_REQUEST,         "login_request",         THROTTLE,     false },
  { CHECK_NAME,            "check_name",            THROTTLE,     false },

  // online
  { JOIN_CHAN,             "join_chan",             DROP,         true },
  { PART_CHAN,             "part_chan",             THROTTLE,     true },
  { CHANNEL_MESSAGE,       "channel_message",       THROTTLE,     false },
  { CHANNEL_EMOTE,         "channel_emote",         THROTTLE,     false },
  { CHANNEL_TOPIC,         "channel_topic",         DROP,         false },
  { CHANNEL_LIST,          "channel_list",          THROTTLE,     false },
  { GET_LINESPEED,         "get_linespeed",         THROTTLE,     false },  // *
  { COMP_MESSAGE,          "comp_message",          THROTTLE,     false },

  { ADD_FILE,              "add_file",              DROP,         true },
  { ADD_PATH_BLOCK,        "add_path_block",        DROP,         true },
  { REMOVE_FILE,           "remove_file",           DROP,         true }, 

  { ADD_NOTIFY,            "add_notify",            DROP,         true },
  { REMOVE_NOTIFY,         "remove_notify",         DROP,         true },
  { CHECK_ONLINE,          "check_online",          PASS,         true }, // *
  { NOTIFY_LIST,           "notify_list",           THROTTLE,     false },

  { IGNORE_ADD,            "ignore_add",            DROP,         true },
  { IGNORE_REMOVE,         "ignore_remove",         DROP,         true },
  { IGNORE_CLEAR,          "ignore_clear",          DROP,         true },
  { IGNORE_LIST,           "ignore_list",           THROTTLE,     false }, 

  { SET_EMAIL,             "set_email",             DROP,         false },
  { SET_EMAILADDRESS,      "set_emailaddress",      DROP,         false },
  { SET_DATAPORT,          "set_dataport",          DROP,         false },
  { SET_USER_DATAPORT,     "set_user_dataport",     DROP,         true },
  { SET_LINESPEED,         "set_linespeed",         DROP,         false },
  { SET_USER_LINESPEED,    "set_user_linespeed",    DROP,         true },
  { SET_USER_LEVEL,        "set_user_level",        DROP,         true },
  { SET_PASSWORD,          "set_password",          THROTTLE,     false },
  { SET_USER_PASSWORD,     "set_user_password",     DROP,         true },

  { BCAST_MODERATOR,       "bcast_moderator",       DROP,         true },
  { ANNOUNCE_LOCAL,        "announce_local",        DROP,         false },

  //{ SURVEY_INFO,           "survey_info",           DROP,         false },

  { VIEW_FILES,            "view_files",            THROTTLE,     false },
  { USER_INFO,             "user_info",             THROTTLE,     false }, // *

  { SHARE_STATS,           "share_stats",           DROP,         false },

  { NEW_DOWNLOAD,          "new_download",          PASS,         false },
  { DOWNLOAD_COMPLETE,     "download_complete",     PASS,         false },
  { NEW_UPLOAD,            "new_upload",            PASS,         false },
  { UPLOAD_COMPLETE,       "upload_complete",       PASS,         false },

  { CONFIG_ERROR,          "config_error",          PASS,         false },  // *

  { SEARCH_REQUEST,        "search_request",        PASS,         false },

  { DOWNLOAD_REQUEST,      "download_request",      THROTTLE,     false },
  { DOWNLOAD_ACCEPTED,     "download_accepted",     THROTTLE,     false },
  { DELAY_DOWNLOAD,        "delay_download",        THROTTLE,     false },
  { FIREWALL_TRANSFER,     "firewall_transfer",     THROTTLE,     false }, 

  { KILL_USER,             "kill_user",             DROP,         false },
  { BAN_ADD,               "ban_add",               DROP,         false },
  { BAN_REMOVE,            "ban_remove",            DROP,         false },
  { BAN_LIST,              "ban_list",              DROP,         false },

  { CHANNEL_CLEAR,         "channel_clear",         DROP,         false }, 
  { CHANNEL_SETLEVEL,      "channel_setlevel",      DROP,         true },
  { CHANNEL_LIST2,         "channel_list2",         THROTTLE,     false }, 
  { CHANNEL_USERLIST,      "channel_userlist",      THROTTLE,     false }, // *

  { SERVER_VERSION,        "server_version",        DROP,         false },
  { SERVER_RELOAD_DB,      "server_reload_db",      DROP,         false },
  { SERVER_CONFIG,         "server_config",         DROP,         false },
  { SERVER_USERLIST,       "server_userlist",       DROP,         false },
  { SERVER_SHUTDOWN,       "server_shutdown",       DROP,         false },

  { SERVER_PING,           "server_ping",           THROTTLE,     false },
  { CLIENT_PING,           "client_ping",           THROTTLE,     true },
  { CLIENT_PONG,           "client_pong",           THROTTLE,     true },

  //{ CLIENT_REDIRECT,       "client_redirect",       DROP,         true },
  //{ CLIENT_CYCLE,          "client_cycle",          DROP,         true },

  { USERFLAG_MUTE,         "userflag_mute",         DROP,         false },
  { USERFLAG_CLOAK,        "userflag_cloak",        DROP,         false }, 

  { 0, "", PASS, false }
};


