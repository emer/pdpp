# pdpp

This is the PDP++ simulator circa 2005, written in C++ using the iv (InterViews) toolkit (ttps://github.com/emer/iv), precursor to https://github.com/emer/cemer

Welcome to the PDP++ Software distribution!

This is version 2.0 of PDP++.

This software is Copyright (C) 1995-2000, Randall C. O'Reilly, Chadley K. Dawson, 
			    James L. McClelland, and Carnegie Mellon University

The manual is Copyright (C) 1995-2000, Chadley K. Dawson, Randall C. O'Reilly, 
			    James L. McClelland, and Carnegie Mellon University

Please see the CopyRight file for details on the copyright.

Bug Reports:        	pdp++bugreport@cnbc.cmu.edu
WWW Page:           	http://www.cnbc.cmu.edu/PDP++/PDP++.html
Anonymous FTP Site: 	ftp://grey.colorado.edu/pub/oreilly/pdp++ (latest) *or*
			ftp://cnbc.cmu.edu/pub/pdp++/		  *or*
			unix.hensa.ac.uk/mirrors/pdp++/

The software contains the following directories:

	bin		binaries go here

	lib		links to the libraries and stuff for interviews
			library are here.

	include		links to the include file directories are here

	config		configuration (for Makefile) and some standard
			init files are found here

	css		contains include files for commonly-used css scripts
			and some additional documentation, plus some demo
			script files

	defaults	contains default configuration files for the various
			executables (see the manual for more information).

	demo		contains demonstrations of various aspects of the
			PDP++ software.

	manual		contains the manual, which is in texinfo format
			and has been made into a .ps, emacs .info, and
			html files.

	src		contains the source code for the software.


The INSTALL file in this directory contains instructions on how to install
the software.  It is just a copy of the install chapter of the manual.

The ANNOUNCE file contains a list of software features.

The ANNOUNCE.TA_CSS file contains a description of the basic interface
system used to implement the software.  This includes the TypeAccess system
and the CSS script language.  These are also documented in the manual.

The TODO file contains a list of things we're planning on doing with the
software in the future.

The NEWS file contains a summary of changes made since previous release.

The ChangeLog file(s) contains a detailed log of changes that were
made since the last release.

------------------------
Special notes for CYGWIN users:

Middle mouse button can be emulated by either shift and left mouse
button, or ctrl and left mouse button, but in some cases only ctrl and
left mouse button works.
