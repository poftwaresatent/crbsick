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

/* for timersub() and usleep() */
#define _BSD_SOURCE

#include "sick_poster.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


struct sick_poster_s * sick_poster_new(int fd,
				       unsigned int usec_cycle,
				       FILE * dbg)
{
  struct sick_poster_s * sp = calloc(1, sizeof(* sp));
  if(0 == sp)
    return 0;

  sp->fd = fd;
  sp->usec_cycle = usec_cycle;
  sp->dirty = 1;
  sp->dbg = dbg;

  return sp;
}


void sick_poster_delete(struct sick_poster_s * sp)
{
  if(0 != sp->running)
    sick_poster_stop(sp);
  if(0 != sp->thread)
    free(sp->thread);
  free(sp);
}


static void * sick_poster_run(struct sick_poster_s * sp)
{
  static const char * msg;
  if(0 != sp->dbg)
    fprintf(sp->dbg, "sick_poster_run(): debug stream enabled.\n");
  
  sp->tc_msec_min = -1;
  sp->tc_msec_max = -1;
  sp->tc_msec_sum = 0;
  sp->tc_count = 0;
  
  sp->running = 1;
  while(sp->running){
    msg = "no message";
    struct sick_scan_s * dirtyscan = & sp->scan[sp->dirty];
    if(0 != gettimeofday(& dirtyscan->t0, 0)){
      ++sp->error_count;
      msg = "read t0 failed";
    }
    else{
      if(0 != sick_rscan(sp->fd, dirtyscan->rho, 0 /* sp->dbg */)){
	++sp->error_count;
	msg = "rscan failed";
      }
      else{
	if(0 != gettimeofday(& dirtyscan->t1, 0)){
	  ++sp->error_count;
	  msg = "read t1 failed";
	}
	else{
	  struct timeval dt;
	  long msec;
	  timersub(& dirtyscan->t1, & dirtyscan->t0, & dt);
	  msec = dt.tv_sec * 1000 + dt.tv_usec / 1000;
	  if((msec < sp->tc_msec_min) || (sp->tc_msec_min < 0))
	    sp->tc_msec_min = msec;
	  if((msec > sp->tc_msec_max) || (sp->tc_msec_max < 0))
	    sp->tc_msec_max = msec;
	  sp->tc_msec_sum += msec;
	  ++sp->tc_count;
	  sp->error_count = 0;
	}
      }
    }
    
    if(0 != sp->dbg){
      if(0 != sp->error_count)
	fprintf(sp->dbg,
		"sick_poster_run(): error_count = %d, msg = %s\n",
		sp->error_count, msg);
      else if(0 != sp->tc_count)
	fprintf(sp->dbg,
		"sick_poster_run(): tc_msec (count/min/max/mean):"
		" %ld / %ld / %ld / %f\n",
		sp->tc_count, sp->tc_msec_min, sp->tc_msec_max,
		sp->tc_msec_sum / ((double) sp->tc_count));
    }
    
    sp->current = sp->dirty;
    sp->current_t0 = dirtyscan->t0;
    sp->dirty = (sp->dirty + 1) % 3;
    if(0 < sp->usec_cycle)
      usleep(sp->usec_cycle);
  }
  sp->running = 1;
  
  return sp;
}


int sick_poster_start(struct sick_poster_s * sp)
{
  if(0 == sp->thread)
    sp->thread = malloc(sizeof(* sp->thread));
  if(0 == sp->thread){
    if(0 != sp->dbg)
      fprintf(sp->dbg, "ERROR in sick_poster_start(): malloc() failed.\n");
    return -1;
  }
  
  if(0 != pthread_create(sp->thread, 0,
			 (void*(*)(void*)) sick_poster_run, sp)){
    if(0 != sp->dbg)
      fprintf(sp->dbg,
	      "ERROR in sick_poster_start(): pthread_create(): %s.\n",
	      strerror(errno));
    return -2;
  }
  
  sp->running = 1;
  return 0;
}


int sick_poster_stop(struct sick_poster_s * sp)
{
  sp->running = 0;
  usleep(100000);
  while(0 == sp->running){
    if(0 != sp->dbg)
      fprintf(sp->dbg, ".");
    usleep(100000);
  }
  sp->running = 0;
  
  if(0 != pthread_join( * sp->thread, 0)){
    if(0 != sp->dbg)
      fprintf(sp->dbg,
	      "ERROR in sick_poster_stop(): pthread_join(): %s.\n",
	      strerror(errno));
    return -1;
  }
  
  return 0;
}


int sick_poster_abort(struct sick_poster_s * sp)
{
  if(0 != pthread_cancel( * sp->thread)){
    if(0 != sp->dbg)
      fprintf(sp->dbg,
	      "ERROR in sick_poster_abort(): pthread_cancel(): %s.\n",
	      strerror(errno));
    return -1;
  }
  if(0 != pthread_join( * sp->thread, 0)){
    if(0 != sp->dbg)
      fprintf(sp->dbg,
	      "ERROR in sick_poster_abort(): pthread_join(): %s.\n",
	      strerror(errno));
    return -2;
  }
  sp->running = 0;
  
  return 0;
}


int sick_poster_getscan(const struct sick_poster_s * sp,
			struct sick_scan_s * scan)
{
  const struct sick_scan_s * current_scan = & sp->scan[sp->current];
  int i;
  
  if(0 == sp->running){
    if(0 != sp->dbg)
      fprintf(sp->dbg, "sick_poster_getscan(): Poster not running.\n");
    return -1;
  }
  
  if(0 != sp->error_count){
    if(0 != sp->dbg)
      fprintf(sp->dbg, "sick_poster_getscan(): error_count = %d.\n",
	      sp->error_count);
    return -2;
  }
  
  for(i = 0; i < 361; ++i)
    scan->rho[i] = current_scan->rho[i];
  scan->t0 = current_scan->t0;
  scan->t1 = current_scan->t1;
  
  return 0;
}
