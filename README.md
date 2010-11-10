Drivers for SICK LMS
====================

This GPL'ed code is taken from http://cerebrate.sourceforge.net/ where
it was part of the driver suite for Fernandez, an autonomous cactus
that defended its territory.

Copyrights:

 - Copyright (C) 2005 Ecole Polytechnique Federale de Lausanne, Switzerland
 - Copyright (C) 2005 Roland Philippsen <roland dot philippsen at gmx dot net>
 - various copyrights on the build system

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    USA


Compiling
=========

This package comes with two build setups, depending on whether you
want to whole shebang (use GNU Automake) or just the bare bones driver
library (use Makefile.ansi). 

GNU Automake
------------

The usual `configure`, `make`, and optional `make install` should do
the trick.

    git clone git://github.com/poftwaresatent/crbsick.git
    mkdir crbsick/build
    cd crbsick/build
    ../configure
    make 

Makefile.ansi
-------------

You can use `Makefile.ansi` to build just the pure ANSI C portions of
this package. Or copy-paste adapt it for your purposes.

    git clone git://github.com/poftwaresatent/crbsick.git
    cd crbsick
    make -f Makefile.ansi


Running
=======

This is a C library which is intended to be used from within other
programs. It connects to a SICK LMS laser scanner via a serial
port. The included programs are not exactly fancy...

When you use the full GNU Automake setup (and have a sanely recent C++
compiler), you will end up with `csick` console SICK *Configuration
Program*. With the `Makefile.ansi` setup, all you get is the `tsick`
console SICK *Test Program* (which is essentially a pared-down version
of `csick`).

Configuration Program
---------------------

After building with the GNU Automake system, you get a little console
program called `csick` which you can use for testing and setting some
parameters of your scanner. (The setting functionality is admittedly
very limited because I ran out of time back in 2005 and now I have
other things on my plate.)

    ./csick

If you have a non-standard name of the serial device or a specific
baudrate to use, pass them as arguments. For example, to use 9600 baud
on /dev/tts/17, you would say:

    ./csick /dev/tts/17 9600 

Test Program
------------

Similar to `csick` but even less functionality, you get this app when
you compile the package with the `Makefile.ansi` approach. E.g.

    ./tsick /dev/tts/17 9600 


Library
=======

Include the `sick.h` header file in your program, and link it with the
`libsick` library.

The `sick_poster.h` header file allows you to spawn an acquisition
thread that continuously reads scans into a "poster" (a term borrowed
from GenoM) and then copy the data into your own thread for
processing.

So, your usual code would look something like the following examples,
but please make sure to *use the return values for error detection*:

With sick_poster
----------------

Let's assume you're writing in C++:

    int fd(serial_open(comport, baudrate));
    struct sick_poster_s * sp(sick_poster_new(fd, 1000, stderr));
    sick_poster_start(sp);
    struct sick_scan_s scan;
    sick_poster_getscan(sp, & scan);
    for(int i(0); i < 361; ++i)
      cout << " " << i << "\t" << scan.rho[i] << "\n";
    sick_poster_stop(sp);
    sick_poster_delete(sp);
    serial_close(fd);

(But of course you are diligent enough to actually check the return
values, right?)


Without sick_poster
-------------------

For instance when writing ANSI C without pthreads, you only include
`sick.h' instead of `sick_poster.h':

    const char * comport;
    unsigned long baudrate;
    int fd;
    int result;
    uint16_t scan[361];
    comport = "/dev/tts/0";
    baudrate = 38400L;
    fd = serial_open(comport, baudrate);
    sick_dumpstatus(fd, stdout);
    result = sick_rscan(fd, scan, dbg);
    if(0 == result){
      int i;
      for(i = 0; i < 361; ++i){
        printf("  %d\t%u\n", i, scan[i]);
      }
    }    

(Again, in real apps, you should check for all return values.)
