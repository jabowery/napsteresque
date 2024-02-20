/*
 * $Id: ignore.hh,v 1.9.18.3 2001/08/23 18:29:26 dbrumleve Exp $
 */

#ifndef _NAPD_IGNORE_HH
#define _NAPD_IGNORE_HH

#include <list>
#include <slist>

#include "defines.hh"
#include "functions.hh"
#include "ignore.hh"

class Client;

struct IgnoreDelta {
  char *ignoree;

  typedef enum { ADD, REMOVE, CLEAR } Op;
  Op op;

  IgnoreDelta(char *_ignoree, Op _op) :
    ignoree(_ignoree), op(_op) { }
};

struct IgnoreList : list<char *> {
  typedef list<char *>::iterator iterator;

  Client *ignorer;
  slist<IgnoreDelta> todo;
  
  IgnoreList(Client * = NULL);
  ~IgnoreList(void);

  bool load(Client * = NULL);
  bool update(void);

  void push_back(const char *s)		{ insert(end(), s); }
  void push_front(const char *s)	{ insert(begin(), s); }
  void insert(const char *);
  void insert(iterator, const char *);
  iterator erase(iterator);
  void erase(const char *);
  bool exists(const char *);
  iterator find(const char *);
  void clear();

  static void test();
};

#endif
