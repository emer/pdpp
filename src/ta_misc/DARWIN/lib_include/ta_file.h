/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the TypeAccess/C-Super-Script software package.	      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, modify, and distribute this software and its      //
// documentation for any purpose is hereby granted without fee, provided that //
// the above copyright notice and this permission notice appear in all copies //
// of the software and related documentation.                                 //
// 									      //
// Note that the PDP++ software package, which contains this package, has a   //
// more restrictive copyright, which applies only to the PDP++-specific       //
// portions of the software, which are labeled as such.			      //
//									      //
// Note that the taString class, which is derived from the GNU String class,  //
// is Copyright (C) 1988 Free Software Foundation, written by Doug Lea, and   //
// is covered by the GNU General Public License, see ta_string.h.             //
// The iv_graphic library and some iv_misc classes were derived from the      //
// InterViews morpher example and other InterViews code, which is             //
// Copyright (C) 1987, 1988, 1989, 1990, 1991 Stanford University             //
// Copyright (C) 1991 Silicon Graphics, Inc.				      //
//									      //
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,         //
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 	      //
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  	      //
// 									      //
// IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY SPECIAL,    //
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES  //
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT     //
// ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY,      //
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        //
// SOFTWARE. 								      //
==============================================================================*/

#ifndef ta_file_h
#define ta_file_h 1

#include <ta/ta_base.h>
#include <ta_misc/ta_misc_TA_type.h>

class taivGetFile;

class taFile : public taBase {
  // #NO_TOKENS #INLINE wrapper for getfile class 
public:
  enum OpenMode {
    NO_AUTO,			// don't automatically open
    READ,			// auto open in READ mode..
    WRITE,
    APPEND 
  };

  String	fname;		// #HIDDEN file name
  OpenMode	mode;		// #HIDDEN auto-open file in this mode (for auto)
  bool		cmp;		// #HIDDEN compressed mode
  bool		file_selected;	// #HIDDEN was a file selected last operation?
  taivGetFile*	gf;		// #NO_SAVE

  virtual istream*	OpenFile(const char* nm = NULL, bool no_dlg = false);
  // to get a file for reading (already exists)
  virtual ostream*	SaveFile(const char* nm = NULL, bool no_dlg = false);
  // to save to an existing file
  virtual ostream*	SaveAsFile(const char* nm = NULL, bool no_dlg = false);
  // to save with a new file
  virtual ostream*	AppendFile(const char* nm = NULL, bool no_dlg = false);
  // to save a file for appending (already exists)
  virtual void		CloseFile();
  // close the stream
  virtual void		AutoOpen();		// auto-open a file based on mode
  virtual bool		IsOpen();		// check if file is open

  virtual void		UpdateGF(); 	// transfer stuff to the getfile
  virtual void		UpdateMe();	// transfer stuff to me from getfile

  virtual void		GetGetFile(); 	// make sure we have a getfile

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy();
  void	Copy_(const taFile& cp);
  COPY_FUNS(taFile, taBase);
  TA_BASEFUNS(taFile);
};

#endif // ta_file_h
