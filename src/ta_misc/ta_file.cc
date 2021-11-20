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

// ta_file.cc

#include <ta_misc/ta_file.h>
#include <ta/taiv_data.h>


//////////////////////////////////////////////////////////
// 			taFile				//
//////////////////////////////////////////////////////////

void taFile::Initialize() {
  mode = NO_AUTO;
  cmp = false;
  file_selected = false;
  gf = NULL;
}

void taFile::Destroy() {
  if(gf != NULL) 
    taRefN::unRefDone(gf);
  gf = NULL;
}

void taFile::Copy_(const taFile& cp) {
  fname = cp.fname;
  mode = cp.mode;
  cmp = cp.cmp;
}

void taFile::GetGetFile() {
  if(gf == NULL) {
    gf = new taivGetFile();
    taRefN::Ref(gf);
  }
}

void taFile::UpdateGF() {
  if(gf == NULL) return;
  gf->compress = cmp;
  gf->fname = fname;
  gf->file_selected = false;
}

bool taFile::IsOpen() {
  if((gf != NULL) && (gf->open_file))
    return true;
  return false;
}

void taFile::UpdateMe() {
  if(gf == NULL) return;
  cmp = gf->compress;
  fname = gf->fname;
  file_selected = gf->file_selected;
  gf->file_selected = false;
}

void taFile::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  if(gf == NULL) return;
  if(gf->file_selected)
    UpdateMe();
  else
    UpdateGF();
}

istream* taFile::OpenFile(const char* nm, bool no_dlg) {
  GetGetFile();
  istream* strm = gf->Open(nm, no_dlg);
  UpdateMe();
  return strm;
}

ostream* taFile::SaveFile(const char* nm, bool no_dlg) {
  GetGetFile();
  ostream* strm = gf->Save(nm, no_dlg);
  UpdateMe();
  return strm;
}
ostream* taFile::SaveAsFile(const char* nm, bool no_dlg) {
  GetGetFile();
  ostream* strm = gf->SaveAs(nm, no_dlg);
  UpdateMe();
  return strm;
}
ostream* taFile::AppendFile(const char* nm, bool no_dlg) {
  GetGetFile();
  ostream* strm = gf->Append(nm, no_dlg);
  UpdateMe();
  return strm;
}
void taFile::CloseFile() {
  GetGetFile();
  gf->Close();
  file_selected = gf->file_selected;
  gf->file_selected = false;
}

void taFile::AutoOpen() {
  if(gf == NULL) return;
  if(gf->open_file)
    return;
  if(mode == NO_AUTO)
    return;
  if(mode == READ)
    OpenFile(fname);
  if(mode == WRITE)
    SaveFile(fname);
  if(mode == APPEND)
    AppendFile(fname);
}

