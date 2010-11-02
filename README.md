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
---------

The usual `configure`, `make`, and optional `make install` should do
the trick.

    git clone git://github.com/poftwaresatent/crbsick.git
    mkdir crbsick/build
    cd crbsick/build
    ../configure
    make 

Running
-------

This is a C library which is intended to be used from within other
programs. It connects to a SICK LMS laser scanner via a serial
port. There also is a little console program called `csick` which you
can use for testing and setting some parameters of your scanner.

    ./csick

If you have a non-standard name of the serial device or a specific
baudrate to use, pass them as arguments. For example, to use 9600 baud
on /dev/tts/17, you would say:

    ./csick /dev/tts/17 9600 

Please look at the code for more details...

Library
-------

Include the `sick.h` header file in your program, and link it with the
`libsick` library. The idea is to spawn an acquisition thread that
continuously reads scans into a "poster" (a term borrowed from GenoM)
and then copy the data into your own thread for processing. So, your
usual code would look something like this, but please make sure to
*use the return values for error detection*:

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
