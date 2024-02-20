/*
 * $Id: ignore.cc,v 1.14.8.2.2.9 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_IGNORE_CC

#include <slist>

#include "defines.hh"
#include "functions.hh"
#include "db.hh"
#include "client.hh"
#include "ignore.hh"
#include "server.hh"

IgnoreList::IgnoreList(Client *_ignorer = NULL) {
  ignorer = _ignorer;
}
  
IgnoreList::~IgnoreList(void) {
  update();
  clear();
}

bool IgnoreList::load(Client *i = NULL) {
#if 0
  if (i)
    ignorer = i;
  else if (!ignorer)
    return false;
  return LoadIgnores(this);
#else
  return true;
#endif
}

bool IgnoreList::update(void) {
#if 0
  return UpdateIgnores(&todo, ignorer);
#else
  todo.clear();
  return true;
#endif
}

void IgnoreList::insert(const char *_s) {
  insert(end(), _s);
}

void IgnoreList::insert(iterator i, const char *_s) {
  if (exists(_s))
    return;
  char *s = strdup(_s);
  todo.push_front(IgnoreDelta(s, IgnoreDelta::ADD));
  list<char*>::insert(i, s);
}

IgnoreList::iterator IgnoreList::erase(iterator i) {
  todo.push_front(IgnoreDelta(*i, IgnoreDelta::REMOVE));
  return list<char*>::erase(i);
}

void IgnoreList::erase(const char *s) {
  iterator i = find(s);
  if (i != end())
    erase(i);
}

IgnoreList::iterator IgnoreList::find(const char *s) {
  foreach (i, *this)
    if (!strcasecmp(*i, s))
      return i;
  return end();
}

bool IgnoreList::exists(const char *s) {
  return (find(s) != end());
}

void IgnoreList::clear(void) {
  foreach_erase (i, *this);
}

void IgnoreList::test() {
  LOG(INFO);

  IgnoreList i;		assert(i.size() == 0);
  i.insert("A");	assert(i.exists("A"));
  i.insert("B");	assert(i.exists("B"));
			assert(i.todo.size() == 2);
  i.erase("A");		assert(i.size() == 1);
  i.clear();		assert(i.size() == 0);
			assert(i.todo.size() == 4);
  i.update();		assert(i.todo.size() == 0);
}


#if 0
void Server::ListIgnores(Client *caller) {
  foreach (i, caller->ignores)
    SAY(caller, IGNORE_ENTRY, *i);
  SAY(caller, IGNORE_LIST, caller->ignores.size());
}

void Server::AddIgnore(Client *caller, Command *com) {
  char *ignoree;
  UNPACK_COMMAND(com, &ignoree);

  if (
    caller->ignores.exists(ignoree) ||
    !strcasecmp(caller->name, ignoree)
  ) {
    SAY(caller, IGNORE_UNKNOWN, ignoree);
    return;
  } 

  caller->ignores.insert(ignoree);
  caller->ignores.update();

  SAY(caller, IGNORE_ADD, ignoree);
}

void Server::RemoveIgnore(Client *caller, Command *com) {
  char *ignoree;
  UNPACK_COMMAND(com, &ignoree);

  if (!caller->ignores.exists(ignoree)) {
    SAY(caller, IGNORE_UNKNOWN, ignoree);
    return;
  }

  caller->ignores.erase(ignoree);
  caller->ignores.update();
  SAY(caller, IGNORE_REMOVE, ignoree);
}

void Server::ClearIgnores(Client *caller) {
  unsigned size = caller->ignores.size();
  caller->ignores.clear();
  caller->ignores.update();
  SAY(caller, IGNORE_CLEAR, size);
}
#endif
