/*-------------------------------------------------------------------------
 *
 *   FILE
 *	cachedresult.cxx
 *
 *   DESCRIPTION
 *      implementation of the pqxx::cachedresult class.
 *   pqxx::cachedresult transparently fetches and caches query results on demand
 *
 * Copyright (c) 2001-2003, Jeroen T. Vermeulen <jtv@xs4all.nl>
 *
 * See COPYING for copyright license.  If you did not receive a file called
 * COPYING with this source code, please notify the distributor of this mistake,
 * or contact the author.
 *
 *-------------------------------------------------------------------------
 */
#include "pqxx/config.h"

#include <stdexcept>

#include "pqxx/cachedresult.h"

using namespace PGSTD;



void pqxx::cachedresult::init()
{
  // We can't accept granularity of 1 here, because some block number 
  // arithmetic might overflow.
  if (m_Granularity <= 1)
    throw out_of_range("Invalid cachedresult granularity");
}


pqxx::cachedresult::size_type pqxx::cachedresult::size() const
{
  if (m_Cursor.size() == Cursor::pos_unknown)
  {
    m_Cursor.Move(Cursor::BACKWARD_ALL());
    m_Cursor.Move(Cursor::ALL());
  }
  return m_Cursor.size();
}


bool pqxx::cachedresult::empty() const
{
  return (m_Cursor.size() == 0) ||
         ((m_Cursor.size() == Cursor::pos_unknown) &&
	  m_Cache.empty() &&
	  GetBlock(0).empty());
}


void pqxx::cachedresult::MoveTo(blocknum Block) const
{
  if (Block < 0)
    throw out_of_range("Negative result set index");

  const Cursor::size_type BlockStart = FirstRowOf(Block);
  m_Cursor.MoveTo(BlockStart);
  if (m_Cursor.Pos() != BlockStart)
    throw out_of_range("Tuple number out of range");
}


const pqxx::result &pqxx::cachedresult::Fetch() const
{
  size_type Pos = m_Cursor.Pos();

  const result R( m_Cursor.Fetch(m_Granularity) );

  if (!R.empty()) 
  {
    pair<const long, const result> tmp_pair(BlockFor(Pos), R);
    /* Note: can't simply return R because it'll get destroyed if it was already
     * present in our map.  Return the inserted version instead--which will be
     * the result already present in the map, if any.
     */
    return m_Cache.insert(tmp_pair).first->second;
  }

  if (!m_HaveEmpty)
  {
    m_EmptyResult = R;
    m_HaveEmpty = true;
  }

  return m_EmptyResult;
}


