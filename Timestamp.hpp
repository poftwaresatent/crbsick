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


#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP


#include <iosfwd>
#include <limits>
#include <sys/time.h>


class Timestamp
{
public:
  /**
     Default Timestamp will all zeros.
  */
  Timestamp() {
    _stamp.tv_sec = 0;
    _stamp.tv_usec = 0;
  }

  /**
     Timestamp from seconds and microseconds
  */
  Timestamp(long sec, long usec) {
    _stamp.tv_sec = sec;
    _stamp.tv_usec = usec;
  }
  
  /**
     Converts a HAL timestamp into a Timestamp instance. If you need
     a Timestamp of "unspecified" time, use Last() or First().
  */
  Timestamp(const struct timeval & stamp): _stamp(stamp) { }

  /**
     Create a Timestamp by specifying it's length in seconds,
     especially useful for timeouts.
  */
  Timestamp(double seconds);

  /**
     Create a Timestamp from the current system time.
  */
  static const Timestamp Now();
  
  /**
     \return A (static) Timestamp of the last representable
     moment. This is useful for initializing a Timestamp before
     finding a minimum.
  */
  static inline const Timestamp & Last();
  
  /**
     \return A (static) Timestamp of the first representable
     moment. This is useful for initializing a Timestamp before
     finding a maximum.
  */
  static inline const Timestamp & First();
  
  /**
     Assignement operator for easy handling of timestamp updates.
  */
  const Timestamp & operator=(const Timestamp & original) {
    _stamp = original._stamp;
    return * this;
  }
  
  const Timestamp & operator=(const struct timeval & stamp) {
    _stamp = stamp;
    return * this;
  }
  
  /**
     You can use a Timestamp instance instead of a struct timespec
     by using this dereference operator. This means that Timestamp
     behaves like a pointer to a struct timespec in certain
     circumstances.
  */
  const struct timeval operator*() const { return _stamp; }
  
  /**
     Ouput operator for human-readable messages, prints the
     Timestamp as a floating point number in seconds.
  */
  friend std::ostream & operator<<(std::ostream & os, const Timestamp & t);
  
  /**
     Basic comparison operator so that you can write code like this:
     
     \code
     Timestamp t0(some_data.GetTimestamp());
     Timestamp t1(other_data.GetTimestamp());
     if(t0 < t1)
     remove_old_data(some_data);
     else
     remove_old_data(other_data);
     \endcode
     
     It is also important for creating chronological maps of data,
     for example in order to implement a sensor history using STL:
     
     \code
     typedef std::multimap<Timestamp, double> distance_history_t;
     distance_history_t disthist;
     while(Whatever())
     disthist.insert(make_pair(Timestamp::Now(), ReadDistanceSensor()));
     // print readings in reverse:
     for(distance_history_t::reverse_iterator id(disthist.rbegin());
     id != disthist.rend();
     ++id)
     cerr << id->first << ": " << id->second << endl;
     \endcode
  */
  friend bool operator<(const Timestamp & left, const Timestamp & right) {
    return
      (   left._stamp.tv_sec  <  right._stamp.tv_sec ) ||
      ( ( left._stamp.tv_sec  == right._stamp.tv_sec ) &&
	( left._stamp.tv_usec <  right._stamp.tv_usec )   );
  }
  
  /**
     The opposite of Timestamp::operator<().
  */
  friend bool operator>(const Timestamp & left, const Timestamp & right) {
    return
      (   left._stamp.tv_sec  >  right._stamp.tv_sec ) ||
      ( ( left._stamp.tv_sec  == right._stamp.tv_sec ) &&
	( left._stamp.tv_usec >  right._stamp.tv_usec )   );
  }
  
  /**
     Equality operator.
  */
  friend bool operator==(const Timestamp & left, const Timestamp & right) {
    return
      ( left._stamp.tv_sec  == right._stamp.tv_sec ) &&
      ( left._stamp.tv_usec == right._stamp.tv_usec );
  }
  
  friend bool operator!=(const Timestamp & left, const Timestamp & right) {
    return ! (left == right);
  }
  
  friend bool operator>=(const Timestamp & left, const Timestamp & right) {
    return (left > right) || (left == right);
  }
  
  friend bool operator<=(const Timestamp & left, const Timestamp & right) {
    return (left < right) || (left == right);
  }
  
  /**
     Decrement operator.
  */
  const Timestamp & operator-=(const Timestamp & other) {
    if(other._stamp.tv_usec > _stamp.tv_usec){
      --_stamp.tv_sec;
      _stamp.tv_usec += 1000000L;
    }
    _stamp.tv_sec  -= other._stamp.tv_sec;
    _stamp.tv_usec -= other._stamp.tv_usec;
    return * this;
  }
  
  friend
  const Timestamp operator-(const Timestamp & left, const Timestamp & right) {
    Timestamp tmp(left);
    tmp -= right;
    return tmp;
  }
  
  const Timestamp & operator+=(const Timestamp & other) {
    _stamp.tv_usec += other._stamp.tv_usec;
    _stamp.tv_sec  += _stamp.tv_usec / 1000000L;
    _stamp.tv_usec %= 1000000L;
    _stamp.tv_sec  += other._stamp.tv_sec;
    return * this;
  }
  
  friend
  const Timestamp operator+(const Timestamp & left, const Timestamp & right) {
    Timestamp tmp(left);
    tmp += right;
    return tmp;
  }

  /** \return the timestamp as a double, unit = seconds */
  const double ConvertToSeconds() const;
  
  /**
     This is a functor for using as sort key in STL containers. For
     example, you can create a set of timestamps that is
     automatically sorted in ascending chronological order like
     this:
     
     \code
     typedef std::set<Timestamp, Timestamp::less> chronology_t;
     chronology_t chrono;
     while(something())
     chrono.insert(RandomTimestamp());
     // print the random timestamps in ascending order:
     for(chronology_t::iterator ic(chrono.begin(); ic != chrono.end(); ++ic)
     cerr << * ic << endl;
     \endcode
  */
  class less {
  public:
    /** for references */
    bool operator()(const Timestamp & left, const Timestamp & right) const {
      return left < right; }
    
    /** for pointers */
    bool operator()(const Timestamp * left, const Timestamp * right) const {
      return (*left) < (*right); }
  };
  
  class greater {
  public:
    /** for references */
    bool operator()(const Timestamp & left, const Timestamp & right) const {
      return left > right; }
    
    /** for pointers */
    bool operator()(const Timestamp * left, const Timestamp * right) const {
      return (*left) > (*right); }
  };
  
private:
  friend class Timeout;		// dbg
  
  struct timeval _stamp;
};


const Timestamp & Timestamp::
Last()
{
  static Timestamp last(std::numeric_limits<long>::max(),
			std::numeric_limits<long>::max());
  return last;
}


const Timestamp & Timestamp::
First()
{
  static Timestamp first(std::numeric_limits<long>::min(),
			 std::numeric_limits<long>::min());
  return first;
}

#endif // TIMESTAMP_HPP
