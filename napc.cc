/*
 * $Id: main.cc,v 1.53.4.1.2.6 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPC_MAIN_CC

extern "C" {
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <termios.h>
#include <malloc.h>
#include <mcheck.h>
}

#include "defines.hh"
#include "functions.hh"
#include "client.hh"
#include "channel.hh"
#include "server.hh"
#include "build.hh"
#include "security.hh"
#include "db.hh"
#include "address.hh"
#include "command.hh"
#include "user.hh"
#include "ignore.hh"
#include "codes.hh"
#include "ternary.hh"
#include "socket.hh"
#include "data.hh"
#include "security.hh"
#include "thread.hh"
#include "mutex.hh"
#include "event.hh"

Client napclient;

bool do_shutdown = 0;

void handle_sigsegv(int e) {
  Thread::kill_all();
  signal(SIGSEGV, SIG_DFL);
}

// It appears that this is required as Oracle puts it's own SIGCHLD
// handler in when it starts up so our fork() call will hang forever as
// a defunct process

void handle_sigchld(int e) {
  while (waitpid(-1, NULL, WNOHANG) > 0);
  signal(SIGCHLD, handle_sigchld);
}

void handle_sigint(int e) {
  do_shutdown = 1;
}

static void usage(int status) {
  fprintf(stderr,
    "Usage: napc [option...]\n"
    "  -n addr   set napclient location (host:port)\n"
    "  -l level  set the log level (0..7)\n"
    "  -d dir    share directory\n"
    "  -H        show usage\n"
    "  -V        show version\n"
  );
  _exit(status);
}

int main(int argc, char **argv) {
  const char *share_dir;
  char *server_addr_str = "127.0.0.1:8888";

  setbuf(stdout, NULL);

  openlog(*argv, LOG_PID | LOG_NDELAY | LOG_PERROR, LOG_USER);

  char c;
  while ((c = getopt(argc, argv, "n:l:d:H")) != EOF) {
    switch(c) {
//    case 'l': 
//      log.level(atoi(optarg));
//      break;
    case 'n':
      server_addr_str = optarg;
      break;
    case 'd':
      share_dir = optarg;
      break;
    case 'H':
      usage(0);
    case 'V':
      LOG(INFO, "napc %s.%s", NAP_MAJOR_VERSION, NAP_MINOR_VERSION);
      LOG(INFO, "%s@%s %s %s",
        NAP_COMPILE_USER, NAP_COMPILE_HOST,
        NAP_COMPILE_TIME, NAP_COMPILE_DATE
      );
      exit(0);
    default:
      usage(1);
    }
  }

  LOG(INFO, "napc %s.%s", NAP_MAJOR_VERSION, NAP_MINOR_VERSION);
  LOG(INFO, "%s@%s %s %s",
    NAP_COMPILE_USER, NAP_COMPILE_HOST,
    NAP_COMPILE_TIME, NAP_COMPILE_DATE
  );

  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, handle_sigchld);

  Security::seed_random();

  try {
    Address server_addr(server_addr_str);
    napclient.open(SOCK_STREAM);
    napclient.connect(server_addr);
    LOG(INFO, "%s", (char *)napclient.peer);
  } CATCH { LOG(ERR, "%s: %s", server_addr_str, ERRSTR); exit(1); }

  Thread::kill_all();
  return 0;
}

