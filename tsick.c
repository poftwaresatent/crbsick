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


#include "sick.h"
#include "util.h"
#include <err.h>
#include <stdlib.h>


int main(int argc,
	 char ** argv)
{
  FILE * dbg = stderr;
  const char * comport = "/dev/tts/0";
  unsigned long baudrate = 38400L;
  int fd = -1;
  char eingabe = 'h';
  int result;
  uint16_t scan[361];
  
  if(argc > 1){
    comport = argv[1];
  }  
  
  if(argc > 2){
    if (1 != sscanf(argv[2], "%lu", &baudrate)) {
      err(EXIT_FAILURE, "sscanf(`%s'...)", argv[2]);
    }
  }
  
  fd = serial_open(comport, baudrate);
  if(fd < 0){
    errx(EXIT_FAILURE, "serial_open() returned %d", fd);
  }
  
  printf("INFO: opened serial device `%s' at %lu baud.\n", comport, baudrate);
  
  while(eingabe != 'e'){
    switch (eingabe){
      
    case '9':
      baudrate = 9600L;
      if(serial_close(fd) != 0){
	printf("WARNING: serial_close() failed.\n");
      }
      fd = serial_open(comport, baudrate);
      if(fd < 0){
	errx(EXIT_FAILURE, "ERROR: serial_open() returned %d", fd);
      }
      break;
      
    case '3':
      baudrate = 38400L;
      if(serial_close(fd) != 0){
	printf("WARNING: serial_close() failed.\n");
      }
      fd = serial_open(comport, baudrate);
      if(fd < 0){
	errx(EXIT_FAILURE, "ERROR: serial_open() returned %d", fd);
      }
      break;
      
    case 's':
      result = sick_dumpstatus(fd, stdout);
      if(result != 0){
	printf("WARNING: sick_dumpstatus() returned %d", result);
      }
      break;
      
    case 'm':
      result = sick_rscan(fd, scan, dbg);
      if(result != 0){
	printf("WARNING: sick_rscan() returned %d", result);
      }
      else{
	int i;
	for(i = 0; i < 361; ++i){
	  printf("  %d\t%u\n", i, scan[i]);
	}
      }
      break;
      
    case 'h':
    default:
      printf("\n\nSICK Test Program\n"
	     "--------------------------\n"
	     "  e     exit\n"
	     "  h     help (this menu)\n\n"
	     "  9     change baudrate to 9600\n"
	     "  3     change baudrate to 38400\n"
	     "  s     check status\n"
	     "  m     make single scan\n");
    }
    
    printf("[%s:%lu]> ", comport, baudrate);
    eingabe = getchar();
    if (EOF == eingabe) {
      eingabe = 'e';
    }
  }
  
  if(serial_close(fd) != 0){
    printf("WARNING: serial_close() failed.\n");
  }
  
  return EXIT_SUCCESS;
}
