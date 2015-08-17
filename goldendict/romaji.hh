/* This file is (c) 2008-2011 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ROMAJI_HH_INCLUDED__
#define __ROMAJI_HH_INCLUDED__

#include "transliteration.hh"

/// Japanese romanization (Romaji) support.
namespace Romaji {

using std::vector;

vector< sptr< Dictionary::Class > > makeDictionaries()
  throw( std::exception );

}

#endif
