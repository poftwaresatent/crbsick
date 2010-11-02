Drivers for SICK LMS
====================

This GPL'ed code is taken from http://cerebrate.sourceforge.net/ where
it was part of the driver suite for Fernandez, an autonomous cactus
that defended its territory.

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

Running
-------

This is a C library which is intended to be used from within other
programs. It connects to a SICK LMS laser scanner via a serial
port. There also is a little console program called `csick` which you
can use for testing and setting some parameters of your scanner.

...I haven't used any of this in 5 years, so please look at the code
for more details...
