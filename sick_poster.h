/* 
 * Copyright (C) 2005 EPFL
 * Ecole Polytechnique Federale de Lausanne, Switzerland
 *
 * Author: Jan Weingarten <jan.weingarten@epfl.ch>
 *         Developed at the Autonomous Systems Lab <http://asl.epfl.ch>
 *
 * Adapted for POSIX: Roland Philippsen <roland dot philippsen at gmx dot net>
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


#ifndef SICK_POSTER_H
#define SICK_POSTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
  

  struct sick_scan_s {
    struct timeval t0, t1;
    uint16_t rho[361];
  };
  
  struct sick_poster_s {
    int fd;
    unsigned int usec_cycle;
    struct sick_scan_s scan[3];
    int current, dirty, running;
    unsigned int error_count;
    pthread_t * thread;
    FILE * dbg;
    long tc_msec_min, tc_msec_max, tc_msec_sum, tc_count;
    struct timeval current_t0;
  };


  struct sick_poster_s * sick_poster_new(int fd, unsigned int usec_cycle,
					 FILE * dbg);
  void sick_poster_delete(struct sick_poster_s * sp);
  int sick_poster_start(struct sick_poster_s * sp);
  int sick_poster_stop(struct sick_poster_s * sp);
  int sick_poster_abort(struct sick_poster_s * sp);
  int sick_poster_getscan(const struct sick_poster_s * sp,
			  struct sick_scan_s * scan);
  
#ifdef __cplusplus
}
#endif

#endif
