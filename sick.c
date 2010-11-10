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

#include "sick.h"
#include "util.h"
#include <string.h>


#define GetBytesAsInt(a,b) (256*(b) + (a))


#define STX               0x02
#define CMD_CHANGE_OPMODE 0x20
#define OPMODE_38400      0x40
#define OPMODE_19200      0x41
#define OPMODE_9600       0x42
#define OPMODE_500000     0x48


void sick_crc(uint8_t * tgram,
	      int tlen,
	      uint8_t crc[2],
	      FILE * dbg)
{
  uint16_t uCrc16;
  
  uCrc16 = 0;
  crc[0] = 0;
  if(0 == dbg)
    while(tlen--){
      crc[1] = crc[0];
      crc[0] = * tgram++;
      if(uCrc16 & 0x8000){
	uCrc16 = (uCrc16 & 0x7FFF) << 1;
	uCrc16 ^= 0x8005;
      }
      else
	uCrc16 <<= 1;
      uCrc16 ^= ((uint16_t) crc[0]) | ((uint16_t) crc[1] << 8);
    }
  else{
    int counter = 0;
    fprintf(dbg, "DEBUG sick_crc():\n ");
    while(tlen--){
      crc[1] = crc[0];
      crc[0] = * tgram++;
      fprintf(dbg, " %02X", crc[0]);
      if(counter % 4 == 3)
	fprintf(dbg, "  ");
      if(counter % 16 == 15)
	fprintf(dbg, "\n ");
      counter++;
      if(uCrc16 & 0x8000){
	uCrc16 = (uCrc16 & 0x7FFF) << 1;
	uCrc16 ^= 0x8005;
      }
      else
	uCrc16 <<= 1;
      uCrc16 ^= ((uint16_t) crc[0]) | ((uint16_t) crc[1] << 8);
    }
    fprintf(dbg, "\n");
  }
  
  crc[0] = uCrc16 & 0xFF;
  crc[1] = (uCrc16 >> 8) & 0xFF;
}


int sick_chkcrc(uint8_t * tgram,
		int tlen,
		uint8_t crc[2],
		FILE * dbg)
{
  uint8_t want[2];
  sick_crc(tgram, tlen, want, dbg);
  if((crc[0] != want[0]) || (crc[1] != want[1])){
    if(dbg != 0)
      fprintf(dbg,
	      "DEBUG sick_chkcrc(): CRC mismatch.\n"
	      "  want: 0x%02X%02X\n"
	      "  have: 0x%02X%02X\n",
	      want[1], want[0], crc[1], crc[0]);
    return -1;
  }
  return 0;
}


int sick_send(int fd,
	      uint8_t * op_and_data,
	      int odlen,
	      FILE * dbg)
{
  int result;
  uint8_t tgram[odlen + 6];
  tgram[0] = 0x02;
  tgram[1] = 0x00;
  tgram[2] = odlen & 0xFF;
  tgram[3] = (odlen >> 8) & 0xFF;
  memcpy(tgram + 4, op_and_data, odlen);
  sick_crc(tgram, odlen + 4, tgram + odlen + 4, dbg);
  
  result = buffer_write(fd, tgram, odlen + 6, dbg);
  if(result != 0){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_send(): buffer_write() returned %d.\n", result);
    return -1;
  }
  
  return 0;
}


int sick_rack(int fd,
	      FILE * dbg)
{
  int result;
  uint8_t ack;

  result = buffer_read(fd, & ack, 1, dbg);
  if(result != 0){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_rack(): buffer_read() returned %d.\n", result);
    return -1;
  }

  if(0x06 != ack){
    if(0x15 == ack){
      if(0 != dbg)
	fprintf(dbg, "DEBUG sick_rack(): NACK (0x15).\n");
      return -1;
    }
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_rack(): Expected ACK (0x06) but got 0x%02X.\n",
	      ack);
    return -2;
  }
  
  return 0;
}


int sick_recv(int fd,
	      uint8_t * tgram,
	      int * tlen,
	      FILE * dbg)
{
  int result;
  int length;
  
  if(7 > * tlen){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): tlen too short (%d < 7).\n", * tlen);
    return -1;
  }
  
  /* header */
  result = buffer_read(fd, tgram, 4, dbg);
  if(result != 0){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): buffer_read() returned %d.\n", result);
    return -2;
  }
  if(0x02 != tgram[0]){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): Expected STX (0x02) but got 0x%02X.\n",
	      tgram[0]);
    return -3;
  }
  if(0x80 != tgram[1]){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): Expected HADR (0x80) but got 0x%02X.\n",
	      tgram[1]);
    return -4;
  }
  length = 6 + tgram[2] + (((int) tgram[3]) << 8);
  if(length > * tlen){
    if(0 != dbg)
      fprintf(dbg,
	      "DEBUG sick_recv(): tgram buffer too small.\n"
	      "  need: %d\n"
	      "  have: %d\n",
	      length, * tlen);
    return -5;
  }
  * tlen = length;
  
  /* answer, data, and crc */
  result = buffer_read(fd, tgram + 4, length - 4, dbg);
  if(result != 0){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): buffer_read() returned %d.\n", result);
    return -6;
  }
  
  if(sick_chkcrc(tgram, length - 2, tgram + length - 2, dbg) != 0){
    if(0 != dbg)
      fprintf(dbg, "DEBUG sick_recv(): sick_chkcrc() failed.\n");
    return -7;
  }
  
  return 0;
}


int sick_dumpstatus(int fd,
		    FILE * out)
{
  int i, result, foo;
  
  int tlen = 160;
  uint8_t tgram[160] = { 0x31 };
  
  result = sick_send(fd, tgram, 1, out);
  if(result != 0)
    return -1;
  
  if(sick_rack(fd, out) != 0)
    return -2;
  
  result = sick_recv(fd, tgram, & tlen, out);
  if(result != 0)
    return -3;
  if(0xB1 != tgram[4]){
    fprintf(out,
	    "ERROR in sick_dumpstatus(): Reply mismatch.\n"
	    "  want: 0xB1\n"
	    "  have: 0x%02X\n",
	    tgram[4]);
    return -4;
  }
  
  fprintf(out,
	  "sick_getstatus():\n"
	  "  betriebsmodus:  ");
  switch(tgram[12]){
  case 0x00:
    fprintf(out, "einrichtmodus zur konfiguration\n");
    break;
  case 0x10:
    fprintf(out, "diagnosemodus\n");
    break;
  case 0x20:
    fprintf(out,
	    "arbeitsmodus: kontinuierliche minimale messwerte pro segment\n");
    break;
  case 0x24:
    fprintf(out, "arbeitsmodus: kontinuierlich ausgabe der messwerte\n");
    break;
  case 0x25:
    fprintf(out, "messwerte nur auf anforderung\n");
    break;
  default:
    fprintf(out, "0x%02X, siehe handbuch!\n", tgram[12]);
  }
  
  foo = GetBytesAsInt(tgram[70],tgram[71]);
  fprintf(out, "  rot. time:      %d microseconds = %f times per second.\n",
	  foo, 1.0/((0.000001) * foo));
  fprintf(out, "  scanning angle: %d degrees.\n",
	  GetBytesAsInt(tgram[111], tgram[112]));
  fprintf(out, "  angular res.:   %f degrees.\n",
	  GetBytesAsInt(tgram[113], tgram[114]) / 100.0);
  
  fprintf(out, "  messmodus:      ");
  switch(tgram[64]){
  case 0x00:
    fprintf(out,
	    "messbereich 8m/80m, feld a+b, blendung (grundeinstellung)\n");
    break;
  case 0x01:
    fprintf(out, "messbereich 8m/80m, reflektorbits in 8 stufen\n");
    break;
  case 0x02:
    fprintf(out, "messbereich 8m/80m, feld a+b+c\n");
    break;
  default:
    fprintf(out, "0x%02X\n", tgram[64]);
  }
  
  fprintf(out, "  baudrate:       ");
  switch(tgram[120]){
  case 0x01: fprintf(out, "500000 baud\n"); break;
  case 0x19: fprintf(out, "38400 baud\n"); break;
  case 0x33: fprintf(out, "19200 baud\n"); break;
  case 0x67: fprintf(out, "9600 baud\n"); break;
  default: fprintf(out, "0x%02X\n", tgram[120]);
  }
  
  fprintf(out, "  perm. baudrate: ");
  switch(tgram[123]){ 
  case 0x00:
    fprintf(out, "9600 bei power-on\n");
    break;
  case 0x01:
    fprintf(out, "vorher konfigurierte wird bei power-on beibehalten\n");
    break;
  default:
    fprintf(out, "0x%02X\n", tgram[123]);
  }
  
  fprintf(out, "  einheit:        ");
  switch(tgram[126]){
  case 0x00: fprintf(out, "cm\n"); break;
  case 0x01: fprintf(out, "mm\n"); break;
  case 0x02: fprintf(out, "reserviert\n"); break;
  default: fprintf(out, "0x%02X\n", tgram[126]);
  }
  
  fprintf(out, "  software:       ");
  for(i = 128; i < 134; i++)
    fprintf(out, "%c", (char) tgram[i]);
  fprintf(out, "\n");

  return 0;
}


int sick_rscan(int fd,
	       uint16_t scan[361],
	       FILE * dbg)
{
  int result, foo, bar;

  int tlen = 1024;
  uint8_t tgram[1024] = { 0x30, 0x01 };
  
  result = sick_send(fd, tgram, 2, dbg);
  if(result != 0)
    return -1;
  
  if(sick_rack(fd, dbg) != 0)
    return -2;
  
  result = sick_recv(fd, tgram, & tlen, dbg);
  if(result != 0)
    return -3;
  if(0xB0 != tgram[4]){
    if(0 != dbg)
      fprintf(dbg,
	      "ERROR in sick_rscan(): Reply mismatch.\n"
	      "  want: 0xB0\n"
	      "  have: 0x%02X\n",
	      tgram[4]);
    return -4;
  }
  
  if(dbg != 0)
    fprintf(dbg, "scan:");
  for(foo = 7, bar = 360; foo < 729; foo += 2, --bar){
    scan[bar] = GetBytesAsInt(tgram[foo], tgram[foo + 1]) & 0x1fff;
    if(dbg != 0)
      fprintf(dbg, " %d", scan[bar]);
  }
  if(dbg != 0)
    fprintf(dbg, "\n");
  
  return 0;
}
