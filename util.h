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


#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
  
  
  int serial_open(const char * device, unsigned long baud);
  int serial_close(int fd);

  int tcp_open(uint32_t portnum, const char * server);
  int tcp_close(int fd);

  int buffer_write(int fd, const uint8_t * buffer, ssize_t n_bytes,
		   FILE * dbg);
  int buffer_read(int fd, uint8_t * buffer, ssize_t n_bytes, FILE * dbg);
  
  /**
     Set the function for cleaning up after your program. Also sets up
     signal handlers for SIGINT, SIGHUP, and SIGTERM to call that ceanup
     function.
     
     \note If you use this function, you should put your cleanup all
     into the function passed as argument and NOT call that function
     yourself. It is automatically called upon calls to exit() or return
     from main.
  */
  void set_cleanup(void (*function)());
  
  
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // UTIL_H
