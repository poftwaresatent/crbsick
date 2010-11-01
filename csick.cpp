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


#include <drivers/sick.h>
#include <drivers/util.h>
#include <util/Timestamp.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>


using namespace std;


int main(int argc,
	 char ** argv)
{
  FILE * dbg = stderr;
  
  string comport("/dev/tts/0");
  if(argc > 1){
    istringstream is(argv[1]);
    if( ! (is >> comport)){
      cerr << "ERROR: couldn't read serial device path argument.\n";
      return 1;
    }
  }  

  unsigned long baudrate(38400L);
  if(argc > 2){
    istringstream is(argv[2]);
    if( ! (is >> baudrate)){
      cerr << "ERROR: couldn't read baudrate argument.\n";
      return 1;
    }
  }
  
  struct sick_poster_s * sp(0);
  int fd(serial_open(comport.c_str(), baudrate));
  if(fd < 0){
    cerr << "ERROR: serial_open() returned " << fd << "\n";
    return 1;
  }
  
  cout << "INFO: opened serial device \"" << comport
       << "\" at " << baudrate << " baud.\n";
  
  char eingabe('h');
  int result;
  uint16_t scan[361];
  while(eingabe != 'e'){
    switch (eingabe){
      
    case 'b':
      cout << "input new baudrate (9600 / 38400 / 500000): ";
      if( ! (cin >> baudrate)){
	cerr << "ERROR: couldn't read baudrate.\n";
	break;
      }
      if( ! ((baudrate == 9600L)
	     || (baudrate == 38400L)
	     || (baudrate == 500000L))){
	cout << "wrong baudrate, you stupid! setting baudrate to 9600...\n";
	baudrate = 9600L;
      }
      if(baudrate == 500000L)
	baudrate = 921600L;
      if(serial_close(fd) != 0)
	cout << "WARNING: serial_close() failed.\n";
      fd = serial_open(comport.c_str(), baudrate);
      if(fd < 0){
	cerr << "ERROR: serial_open() returned " << fd << "\n";
	return 1;
      }
      break;
      
    case 'c':
      cout << "input serial device path: ";
      if( ! (cin >> comport)){
	cerr << "ERROR: couldn't read serial device path.\n";
	break;
      }
      if(serial_close(fd) != 0)
	cout << "WARNING: serial_close() failed.\n";
      fd = serial_open(comport.c_str(), baudrate);
      if(fd < 0){
	cerr << "ERROR: serial_open() returned " << fd << "\n";
	return 1;
      }
      break;
      
    case 's':
      result = sick_dumpstatus(fd, stdout);
      if(result != 0)
	cout << "WARNING: sick_dumpstatus() returned " << result << "\n";
      break;
      
    case 'm':
      {
	string fname;
	auto_ptr<ofstream> os;
	if( ! (cin >> fname))
	  cout << "error reading filename, won't dump\n";
	else{
	  os = auto_ptr<ofstream>(new ofstream(fname.c_str()));
	  if(0 == os.get())
	    cout << "error opening \"" << fname << "\", won't dump.\n";
	}
	
	result = sick_rscan(fd, scan, dbg);
	if(result != 0)
	  cout << "WARNING: sick_rscan() returned " << result << "\n";
	else
	  for(int i(0); i < 361; ++i){
	    cout << "  " << i << "\t" << scan[i] << "\n";
	    if(0 != os.get())
	      (* os) << scan[i] << "\n";
	  }
      }
      break;
      
    case 'p':
      if(0 == sp){
	sp = sick_poster_new(fd, 1000, stderr);
	if(0 == sp){
	  cout << "ERROR: sick_poster_new() failed.\n";
	  break;
	}
	if(0 != sick_poster_start(sp)){
	  cout << "ERROR: sick_poster_start() failed.\n";
	  break;
	}
      }
      struct sick_scan_s scan;
      if(0 != sick_poster_getscan(sp, & scan)){
	cout << "ERROR: sick_poster_getscan() failed.\n";
	break;
      }
      cout << "stamps: (" << Timestamp(scan.t0)
	   << ", " << Timestamp(scan.t1) << ")";
      for(int i(0); i < 361; ++i)
	cout << " " << i << "\t" << scan.rho[i] << "\n";
      break;
      
    case 'h':
      cout << "\n\nSICK Configuration Program\n"
	   << "--------------------------\n"
	   << "  e            exit\n"
	   << "  h            help (this menu)\n\n"
	   << "  b            change baudrate\n"
	   << "  c            change comport\n"
	   << "  s            check sick status\n"
	   << "  m <filename> make single scan\n"
	   << "  p            poster baby!\n";
    }
    
    cout << "[" << comport << ":" << baudrate << "baud]> ";
    cin	>> eingabe;
  }
  
  if(0 != sp){
    if(0 != sick_poster_stop(sp))
      cout << "WARNING: sick_poster_stop() failed.\n";
    sick_poster_delete(sp);
  }
  
  if(serial_close(fd) != 0)
    cout << "WARNING: serial_close() failed.\n";
  
  return 0;
}
