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


#include "util.h"
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


int serial_open(const char * device,
		unsigned long baud)
{
  int fd;
  struct termios tio;

  fd = open(device, O_RDWR | O_NDELAY);

  if(fd == -1){
    fprintf(stderr, "serial_open: ");
    perror(device);
    return -1;
  }

  if(memset(&tio, 0, sizeof(struct termios)) != &tio){
    perror("serial_open: memset");
    close(fd);
    return -2;
  }
  
  switch(baud){
  case 50L    : tio.c_cflag = B50;     break;
  case 75L    : tio.c_cflag = B75;     break;
  case 110L   : tio.c_cflag = B110;    break;
  case 134L   : tio.c_cflag = B134;    break;
  case 150L   : tio.c_cflag = B150;    break;
  case 200L   : tio.c_cflag = B200;    break;
  case 300L   : tio.c_cflag = B300;    break;
  case 600L   : tio.c_cflag = B600;    break;
  case 1200L  : tio.c_cflag = B1200;   break;
  case 1800L  : tio.c_cflag = B1800;   break;
  case 2400L  : tio.c_cflag = B2400;   break;
  case 4800L  : tio.c_cflag = B4800;   break;
  case 9600L  : tio.c_cflag = B9600;   break;
  case 19200L : tio.c_cflag = B19200;  break;
  case 38400L : tio.c_cflag = B38400;  break;
  case 57600L : tio.c_cflag = B57600;  break;
  case 115200L: tio.c_cflag = B115200; break;
  case 230400L: tio.c_cflag = B230400; break;
  default:
    fprintf(stderr, "serial_open: baudrate %lu not supported\n", baud);
    return -5;
  }
  tio.c_cflag |= CS8 | CLOCAL | CREAD ;
  tio.c_iflag = IGNPAR;
  
  if(tcflush(fd, TCIOFLUSH) < 0){
    perror("serial_open: tcflush");
    close(fd);
    return -3;
  }
  
  if(tcsetattr(fd, TCSANOW, &tio) < 0){
    perror("serial_open: tcsetattr");
    close(fd);
    return -4;
  }
  
  return fd;
}


int serial_close(int filedescriptor)
{
  if(tcdrain(filedescriptor) < 0){
    perror("serial_close: tcdrain");
    return -1;
  }
  
  if(tcflush(filedescriptor, TCIOFLUSH) < 0){
    perror("serial_close: tcflush");
    return -2;
  }
  
  if(close(filedescriptor) < 0){
    perror("serial_close: close");
    return -3;
  }
  
  return 0;
}


int buffer_write(int fd,
		 const uint8_t * buffer,
		 ssize_t n_bytes,
		 FILE * dbg)
{
  if(dbg != 0){
    ssize_t i;
    fprintf(dbg, "DEBUG buffer_write():\n ");
    for(i = 0; i < n_bytes; ++i){
      fprintf(dbg, " %02X", buffer[i]);
      if(i % 4 == 3)
	fprintf(dbg, "  ");
      if(i % 16 == 15)
	fprintf(dbg, "\n ");
    }
    fprintf(dbg, "\n");
  }
  
  while(n_bytes > 0){
    ssize_t n;
    while((n = write(fd, buffer, n_bytes)) == 0);
    if(n < 0){
      perror("buffer_write: write");
      return -1;
    }
    
    n_bytes -= n;
    buffer  += n;
  }
  
  return 0;
}


int buffer_read(int fd,
		uint8_t * buffer,
		ssize_t n_bytes,
		FILE * dbg)
{
  ssize_t remain = n_bytes;
  uint8_t * bp = buffer;  
  while(remain > 0){
    ssize_t n;
    while((n = read(fd, bp, remain)) == 0);
    if(n < 0){
      perror("buffer_read: read");
      return -1;
    }
    
    remain -= n;
    bp     += n;
  }
  
  if(dbg != 0){
    ssize_t i;
    fprintf(dbg, "DEBUG buffer_read():\n ");
    for(i = 0; i < n_bytes; ++i){
      fprintf(dbg, " %02X", buffer[i]);
      if(i % 4 == 3)
	fprintf(dbg, "  ");
      if(i % 16 == 15)
	fprintf(dbg, "\n ");
    }
    fprintf(dbg, "\n");
  }
  
  return 0;
}


static void handle(int signum)
{
  /* The cleanup function is called implcitly through exit(). */
  exit(EXIT_SUCCESS);
}


void set_cleanup(void (*function)())
{
  if(atexit(function)){
    perror("set_cleanup(): atexit() failed");
    exit(EXIT_FAILURE);
  }
  if(signal(SIGINT, handle) == SIG_ERR){
    perror("set_cleanup(): signal(SIGINT) failed");
    exit(EXIT_FAILURE);
  }
  if(signal(SIGHUP, handle) == SIG_ERR){
    perror("set_cleanup(): signal(SIGHUP) failed");
    exit(EXIT_FAILURE);
  }
  if(signal(SIGTERM, handle) == SIG_ERR){
    perror("set_cleanup(): signal(SIGTERM) failed");
    exit(EXIT_FAILURE);
  }
}
