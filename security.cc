/*
 * $Id: security.cc,v 1.11.8.3.2.6 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_SECURITY_CC

extern "C" {
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
}

#include "defines.hh"
#include "security.hh"

static int stack_base(void **addr, U32 *len) {
#ifndef LINUX
  // i think this is possible to do on solaris as well
  LOG(WARNING, "unimplemented");
  return -1;
#endif

  // i assume the stack is the last line of this file (!)
  // it typically grows down from 0xC
  const char *maps = "/proc/self/maps";
  FILE *fp = fopen(maps, "r");

  if (!fp) {
    LOG(WARNING, "%s: %s", maps, ERRSTR);
    return -errno;
  }

  I32 s, e;
  while (!feof(fp)) {
    if (fscanf(fp, "%08x-%08x", &s, &e) != 2)
      break;
    while (getc(fp) != '\n');
  }
  fclose(fp);

  if (!s)
    return -1;

  *addr = (void *)s;
  *len = e - s;
  return 0;
}

int Security::stack_protect(int prot = PROT_READ | PROT_WRITE) {
  void *s;
  U32 len;

  if (int ret = stack_base(&s, &len)) {
    LOG(WARNING, "failed to get stack base");
    return ret;
  }

  // when the stack grows downwards it inherits the prot value
  // so we only have to do this once
  if (int ret = mprotect(s, len, prot)) {
    LOG(WARNING, "mprotect %08x: %s", s, ERRSTR);
    return ret;
  }

  LOG(INFO, "%08x %cexec", s, prot & PROT_EXEC ? '+' : '-');
  return 0;
}

int Security::stack_unprotect() {
  return stack_protect(PROT_READ | PROT_WRITE | PROT_EXEC);
}

int Security::change_root() {
  char cwd[256];
  if (!getcwd(cwd, sizeof(cwd))) {
    LOG(WARNING, ERRSTR);
    return -1;
  }
    
  if (int ret = chroot(cwd)) {
    LOG(WARNING, ERRSTR);
    return -errno;
  }

  LOG(INFO, "%s", cwd);
  return 0;
}

char *Security::lock_string(char *s) {
  I32 len = strlen(s);
  char *t = new char[len + 1];

  // mlock before strcpy
  if (mlock(t, len + 1) < 0)
    LOG(WARNING, ERRSTR);
  strcpy(t, s);
  memset(s, 0, len);

  return t;
}

void Security::unlock_string(char *t) {
  I32 len = strlen(t);

  // memset before munlock
  memset(t, 0, len);

  if (munlock(t, len + 1) < 0)
    LOG(WARNING, ERRSTR);
  delete[] t;
}

int Security::change_user(uid_t uid) {
  LOG(INFO, "%d", uid);

  if (setreuid(uid, uid) < 0) {
    LOG(WARNING, "setreuid: %s", ERRSTR);
    return -errno;
  }

  return 0;
}

U32 Security::seed_random() {
  U32 seed;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  seed = getpid() * (U32)tv.tv_usec;

#ifdef LINUX
  seed *= rdtsc();
#endif
  
  LOG(INFO, "%lu", seed);
  srand(seed);
  return seed;
}

static int _set_rlimit(enum __rlimit_resource r, int size) {
  struct rlimit rl;
  rlim_t value;

  memset(&rl, 0, sizeof(rl));
  if (getrlimit(r, &rl) < 0)
    return -1;

  value = size < 0 ? rl.rlim_max : size;
  rl.rlim_cur = value;
  if (setrlimit(r, &rl) < 0)
    return -1;

  if (getrlimit(r, &rl) < 0)
    return -1;
  if (rl.rlim_cur != value)
    return -1;

  return value;
}

int Security::set_dtable_size(I32 size) {
  int value = _set_rlimit(RLIMIT_NOFILE, size);
  if (value < 0) {
    LOG(ERR, "unable to set dtable size to %d", size);
    return -1;
  }
  LOG(INFO, "%d", value);
  return 0;
}

int Security::set_coredump_size(I32 size) {
  int value = _set_rlimit(RLIMIT_CORE, size);
  if (value < 0) {
    LOG(ERR, "unable to set coredump size to %d", size);
    return -1;
  }
  LOG(INFO, "%d", value);
  return 0;
}

void Security::test() {
  LOG(INFO);

  if (getuid()) {
    LOG(WARNING, "not root");
    return;
  }

  seed_random();
  I32 r = rand();
  seed_random();		assert(r != rand());

  char *s = strdup("foo");	assert(!stack_protect());
  char *t = lock_string(s);	assert(t);
				assert(!strcmp(t, "foo"));
  unlock_string(t);		assert(*(I32 *)s == 0);
  delete[] s;			assert(!stack_unprotect());

  // don't mess with main pid
  if (pid_t pid = ::fork()) {
    int status = 0;		assert(pid == ::waitpid(pid, &status, 0));
				assert(!status);
  } else {
				assert(!set_dtable_size(9));
				assert(!set_coredump_size(666));
    				assert(!change_root());
    				assert(!change_user(19));
				assert(getuid() == 19);
    ::exit(0);
  }
}
