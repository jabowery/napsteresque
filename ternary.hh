/* 
 * $Id: ternary.hh,v 1.5.16.2 2001/08/23 18:29:28 dbrumleve Exp $
 */

#ifndef _NAPD_TERNARY_HH
#define _NAPD_TERNARY_HH

#include <set>

template <class Value> struct TernaryTree {
  typedef const char *Key;
  typedef set<Value> ValueSet;

  struct Node {
    Node() : c(' '), lo(NULL), eq(NULL), hi(NULL) { }
    ~Node() { };

    char c;
    Node *lo, *eq, *hi;
    ValueSet values;
  } *root;

  TernaryTree(Node *r = NULL) : root(r) { }
  ~TernaryTree() { };

  void erase(Key key, Value value)	{ erase(key, &value); }
  void erase(Key key)			{ erase(key, NULL); }

  void erase(Key key, Value *valuep) {
    register Node *p = root, *pp = NULL;
    Node *lb = NULL, *lbp = NULL, *lbr = NULL, *lbdc = NULL, *dt;
  
    // The first thing we do is traverse the tree and record the position
    // of the last node which had either a lokid or a hikid as well as 
    // it's parent node for rebalancing purposes. This will become the point
    // we delete up to
  
    while (p) {
      // Compare the next key character to the node/character
      int compare = *key - p->c;
  
      // If the characters match  ...
      if (!compare) {
        // We've found a branch node, record it
        if (p->lo || p->hi) {
          lb = p;
          lbp = pp;
        }
  
        if (!*key)
          break;
  
        pp = p;
        p = p->eq;
        key++;
      }
  
      // Otherwise, if the character is lesser ...
      else if (compare < 0) {
        pp = lbp = p;
        p = lb = p->lo;
      } 
      
      // Otherwise ...
      else {
        pp = lbp = p;
        p = lb = p->hi;
      }
    }
  
    if (!p)
      return;
  
    if (valuep) {
      p->values.erase(*valuep);
  
      // We don't delete a node if it's currently being used
      if (p->values.size())
        return;
    } else {
      p->values.clear();
    }
  
    // If there wasn't a branch recorded, it means we only have eqkids
    // and we will be deleting all the way up to the root node
    if (!lb) {
      dt = root;
      root = NULL;
    } 
  
    // If we recorded a last branch, but it doesn't have a lokid or hikid,
    // it means that it was a lokid or hikid for another branch. We will
    // be deleting up to and including the last branch so we NULL out
    // it's link from it's parent node
  
    else if (!lb->lo && !lb->hi) {
      if (lbp->lo == lb)
        lbp->lo = NULL;
      else
        lbp->hi = NULL;
  
      dt = lb;
    } 
  
    // Otherwise, we recorded a branch which has either a lokid or a hikid
    // and we must relink the tree because it's eqkid won't exist as soon
    // as we do the deletion.
    else {
      // First record which node, lokid or hikid will be replacing the eqkid
      // spot on the last branched node
  
      if (lb->lo && lb->hi) {
        lbr = lb->hi;
        lbdc = lb->lo;
      } else if (lb->hi) {
        lbr = lb->hi;
        lbdc = NULL;
      } else {
        lbr = lb->lo;
        lbdc = NULL;
      }
  
      // Special circumstances... if we are at the root node and the root
      // is our last branch, it has no parent
  
      if (!lbp)
        root = lbr;
  
      // Otherwise, relink the tree setting the lo, hi or eq
      // of the last branches parent to be the last branch's
      // lo or hi
      else {
        if (lbp->lo == lb)
          lbp->lo = lbr;
        else if (lbp->hi == lb)
          lbp->hi = lbr;
        else
          lbp->eq = lbr;
      }
  
      if (lbdc) {
        for (p = lbr; p->lo; p = p->lo);
        p->lo = lbdc;
      }
  
      dt = lb;
    }
  
    // Delete the now detached tree
    do {
      p = dt;
      dt = p->eq;
        
      delete p;
    } while (dt);
  }
  
  void insert(Key s, Value value) {
    int d;
    Node *p, **pp = &root;
  
    while ((p = *pp)) {
      if ((d = *s - p->c) == 0) {
        if (!*s++) {
          p->values.insert(value);
  	return;
        }
        pp = &p->eq;
      }
  
      else if (d < 0)
        pp = &p->lo;
      else
        pp = &p->hi;
    }
  
    do {
      p = *pp = new TernaryTree<Value>::Node;
      p->c = *s;
      pp = &p->eq;
    } while (*s++);
  
    p->values.insert(value);
  }
  
  ValueSet *TernaryTree::find(Key s) {
    Node *p = root;
    char pc;

    while (p) {
      pc = p->c;
  
      if (*s < pc) {
        p = p->lo;
      } else if (*s == pc) {
        if (!*s++) 
          return &p->values;
        p = p->eq;
      } else {
        p = p->hi;
      }
    }
  
    return NULL;
  };

  static void test();
};

#ifndef _NAPD_TERNARY_CC
extern void TernaryTree<int>::test();
#endif

#endif
