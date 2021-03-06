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


#ifndef SICK_H
#define SICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
  
  void sick_crc(uint8_t * tgram, int tlen, uint8_t crc[2], FILE * dbg);
  int sick_chkcrc(uint8_t * tgram, int tlen, uint8_t crc[2], FILE * dbg);
  
  int sick_send(int fd, uint8_t * op_and_data, int odlen, FILE * dbg);
  int sick_rack(int fd, FILE * dbg);
  int sick_recv(int fd, uint8_t * tgram, int * tlen, FILE * dbg);
  
  int sick_dumpstatus(int fd, FILE * out);
  int sick_rscan(int fd, uint16_t scan[361], FILE * dbg);

  
#ifdef __cplusplus
}
#endif

#endif
