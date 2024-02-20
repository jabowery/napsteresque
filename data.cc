/* 
 * $Id: data.cc,v 1.1.2.2 2001/08/24 02:19:17 dbrumleve Exp $
 */

#define _NAPD_DATA_CC
#include "defines.hh"
#include "data.hh"

void Data::test() {
  LOG(INFO);
  Data d("Dan", 4);		assert(!strcmp((char *)(d + 1).data, "an"));
				assert(d[3] == 0);
				assert(d.size == 4);
}
