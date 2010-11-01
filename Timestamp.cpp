/* 
 * Copyright (C) 2005 Roland Philippsen <roland dot philippsen at gmx dot net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include "Timestamp.hpp"
#include <sstream>
#include <iomanip>


using namespace std;


Timestamp::
Timestamp(double seconds)
{
  _stamp.tv_sec = static_cast<long>(seconds);
  _stamp.tv_usec = static_cast<long>((seconds - _stamp.tv_sec) * 1e6);
}


const Timestamp Timestamp::
Now()
{
  struct timeval tv;
  int res(gettimeofday( & tv, 0));
  if(res != 0)
    return First();
  return Timestamp(tv);
}


ostream & operator<<(ostream & os,
		     const Timestamp & t)
{
  char oldfill(os.fill('0'));
  os << t._stamp.tv_sec << "." << setw(6) << t._stamp.tv_usec;
  os.fill(oldfill);
  return os;
}


const double Timestamp::
ConvertToSeconds() const
{
  return _stamp.tv_sec + _stamp.tv_usec / 1000000.0;
}
