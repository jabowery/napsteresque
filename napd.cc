/*
 * $Id: main.cc,v 1.53.4.1.2.6 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_MAIN_CC

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
#include <syslog.h>
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
#include "time.hh"

Server server;
Client napnet;
Client napdirect;

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

static int test() {
  LOG(INFO, "napd %s.%s", NAP_MAJOR_VERSION, NAP_MINOR_VERSION); \
  LOG(INFO, "%s@%s %s %s", \
    NAP_COMPILE_USER, NAP_COMPILE_HOST, \
    NAP_COMPILE_TIME, NAP_COMPILE_DATE \
  );

  Address::test();
  Time::test();
  Command::test();
  Socket::test();
  User::test();
  TernaryTree<int>::test();
  IgnoreList::test();
  Data::test();
  Security::test();
  Thread::test();
  Mutex::test();
  Event::test();
  PollSet::test();
  Client::test();
  Server::test();
  return 0;
}


static void usage(int status) {
  fprintf(stderr,
    "Usage: napd [option...] [host:port]\n"
    "  -a info   oracle authentication info (user/pass@tns)\n"
    "  -u user   run as user (name or uid)\n"
    "  -d size   set the descriptor table size\n"
    "  -l level  set the log level (0..7)\n"
    "  -o file   set log file\n"
    "  -m ip     set multicast group (22[45].x.x.x)\n"
    "  -b        enable benchmarking\n"
    "  -g        dump core on segv\n"
    "  -r        chroot to cwd\n"
    "  -s        remove exec bit from stack\n"
    "  -H        show usage\n"
    "  -V        show version\n"
    "  -T        run test suite\n"
  );
  _exit(status);
}

int main(int argc, char **argv) {
  I32 dtable_size = -1;
  bool do_coredump = false;
  bool do_chroot = false, do_stackprot = false, do_benchmark = false;
  uid_t revoke_uid = 0;
  char *db_info = NULL;
  const char *group_addr_str = "224.6.6.6";
  const char *server_addr_str = ":8888";

  setbuf(stdout, NULL);

  if (char *p = strrchr(*argv, '/'))
    *argv = p + 1;
  close(2);
  dup2(1, 2);
  openlog(*argv, LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_LOCAL6);

  char c;
  while ((c = getopt(argc, argv, "m:a:u:d:o:l:gVHTsbr")) != EOF) {
    switch(c) {
    case 'm':
      group_addr_str = optarg;
      break;
    case 'a':
      db_info = Security::lock_string(optarg);
      break;
    case 'b':
      do_benchmark = true;
      break;
    case 'd': 
      dtable_size = atoi(optarg);
      break;
    case 'l': 
      _log_level = atoi(optarg);
      break;
    case 'g':
      do_coredump = true;
      if (Security::set_coredump_size(do_coredump ? -1 : 0) < 0)
        exit(1);
      break;
    case 's':
      do_stackprot = true;
      break;
    case 'r':
      do_chroot = true;
      break;
    case 'o':
      {
        int fd = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, 0600);
        if (fd < 0) {
          LOG(ERR, "open: %s: %s", optarg, ERRSTR);
          exit(1);
        }

        close(1);

        if (dup2(fd, 1) < 0) {
          LOG(ERR, "dup2: %s: %s", optarg, ERRSTR);
          exit(1);
        }
      }
      break;
    case 'u':
      if (isdigit(*optarg))
        revoke_uid = atoi(optarg);
      else {
        struct passwd *pw = getpwnam(optarg);
        if (!pw) {
          LOG(ERR, "user does not exist: %s", optarg);
          _exit(1);
        }
        revoke_uid = pw->pw_uid;
      }
      break;
    case 'V':
      LOG(INFO, "napd %s.%s", NAP_MAJOR_VERSION, NAP_MINOR_VERSION); \
      LOG(INFO, "%s@%s %s %s", \
        NAP_COMPILE_USER, NAP_COMPILE_HOST, \
        NAP_COMPILE_TIME, NAP_COMPILE_DATE \
      );
      _exit(0);
      break;
    case 'H':
      usage(0);
    case 'T':
      try {
        test();
      } CATCH {
        LOG(ERR, "%s", ERRSTR);
        exit(1); 
      }
      _exit(0);
    default:
      usage(1);
    }
  }

  if (optind < argc)
    server_addr_str = argv[optind];

  LOG(INFO, "napd %s.%s", NAP_MAJOR_VERSION, NAP_MINOR_VERSION); \
  LOG(INFO, "%s@%s %s %s", \
    NAP_COMPILE_USER, NAP_COMPILE_HOST, \
    NAP_COMPILE_TIME, NAP_COMPILE_DATE \
  );

  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, handle_sigchld);
#if 0
  signal(SIGSEGV, handle_sigsegv);
#endif

  // SIGINT needs to use sigaction so we can specify SA_RESTART because
  // Oracle rebinds it later
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; 
  sa.sa_handler = handle_sigint;
  sigaction(SIGINT, &sa, 0);

  if (!do_coredump && Security::set_coredump_size(0) < 0)
    exit(1);
  if (Security::set_dtable_size(dtable_size) < 0)
    exit(1);
  if (do_stackprot && Security::stack_protect() < 0)
    exit(1);
  if (do_chroot && Security::change_root() < 0)
    exit(1);

  if (revoke_uid && Security::change_user(revoke_uid) < 0)
    exit(1);

  Security::seed_random();

  if (db_info && db_init(db_info) < 0)
    _exit(1);

  Address group_addr(group_addr_str);
  if (!group_addr.port)
    group_addr.port = 9999;
  if (!group_addr.multicast()) {
    LOG(ERR, "%s not a multicast address", (char *)group_addr);
    exit(1);
  }

  Address server_addr(server_addr_str);
  if (!server_addr.port)
    group_addr.port = 8888;

  try {
    server.open(server_addr);

    napdirect.open(SOCK_DGRAM);
    napdirect.bind(server.addr); // same address, but udp
    napdirect.nonblock(1);

    napnet.open(SOCK_DGRAM);
    napnet.bind(group_addr);
    napnet.nonblock(1);

    LOG(INFO, "multicast group_addr: %s", (char *)napnet.addr);

    SAY_TO(&napdirect, napnet.addr, NAPNET_HELLO);
  } CATCH {
    LOG(ERR, "%s", ERRSTR);
    exit(1);
  }

  PollSet p;

  try {
    while (!do_shutdown) {
      p.watch(napdirect);
      p.watch(napnet);
      p.poll();
  
      if (p.can_read(napnet)) {
        if (Command *com = napnet.read()) {
          switch (com->code) {
          case NAPNET_HELLO:
            {
              LOG(INFO, "napnet hello: %s", (char *)com->peer);
              if (com->peer != napdirect.addr)
                SAY_TO(&napdirect, com->peer, NAPNET_HELLO);
            }
            break;
          case NAPNET_GOODBYE:
            {
              LOG(INFO, "napnet goodbye: %s", (char *)com->peer);
            }
            break;
          }
          delete com;
        }
      }

      if (p.can_read(napdirect)) {
        if (Command *com = napdirect.read()) {
          LOG(INFO, "napnet hello: %s", (char *)com->peer);
        }
      }

      server.check_incoming();
      server.check_clients();
    }
  } CATCH { LOG(INFO, "%s", ERRSTR); }

  LOG(INFO, "exiting");

  try {
    SAY_TO(&napdirect, napnet.addr, NAPNET_GOODBYE);
    napnet.flush();
  } CATCH { }

  Thread::kill_all();
  return 0;
}

