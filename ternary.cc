/*
 * $Id: ternary.cc,v 1.4.16.3 2001/08/23 18:29:28 dbrumleve Exp $
 */

#define _NAPD_TERNARY_CC

#include <iostream>

#include "ternary.hh"
#include "defines.hh"

void TernaryTree<int>::test() {
  LOG(INFO);

  TernaryTree<int> t;
  set<int> *i;
  set<int>::iterator ip;

  t.insert("Dan", 23);
  t.insert("Dan", 24);

  i = t.find("Dan");		assert(i->size() == 2);
  ip = i->begin();		assert(*ip++ == 23);
				assert(*ip++ == 24);
				assert(ip == i->end());
  t.erase("Dan", 24);
  i = t.find("Dan");		assert(i->size() == 1);
  ip = i->begin();		assert(*ip++ == 23);
				assert(ip == i->end());
  t.insert("Foo", -1);
  i = t.find("Foo");		assert(i->size() == 1);
  ip = i->begin();		assert(*ip++ == -1);
				assert(ip == i->end());
  t.erase("Dan", 23);		assert(t.root);
  t.erase("Foo", -1);		assert(t.root == NULL);
}
