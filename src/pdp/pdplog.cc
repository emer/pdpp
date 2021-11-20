/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the PDP++ software package.			      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted	      //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//									      //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions. 	HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
// 									      //
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

// pdplog.cc
#include <pdp/pdplog.h>
#include <pdp/process.h>
#include <pdp/sched_proc.h>
#include <ta/ta_group_iv.h>
#include <limits.h>
#include <float.h>

#include <ta/enter_iv.h>
#include <IV-look/kit.h>
#include <InterViews/style.h>
#include <ta/leave_iv.h>


////////////////////////////////////////////////////////////////////////////////
// important notes on data_range, log_lines, and view_range:
// data_range reflects log_lines if logging to file, otherwise it is in lines
//   	in the data object
// data_range.min is >= 0, but data_range.max is not the _size_ of the max
// 	but the _line number_ of the max:  thus it is typically log_lines - 1
//	or data.MaxLength() - 1
// view_range is in the same units as data_range
// to translate either view or data_range into indicies in the data table (data)
// 	it is necessary to subtract data_range.min from the view_range
// 	to take into account the possiblity that the data in the table is only
// 	a portion of the data in the actual log file.
////////////////////////////////////////////////////////////////////////////////


//////////////////////////
// 	PDPLog		//
//////////////////////////
 
void PDPLog::Initialize() {
  log_file.cmp = false;
  log_file.mode = taFile::NO_AUTO;
  log_lines = 0;

  data.el_typ = &TA_float_Data;
  data_bufsz = 10000;		// default bufsz is 5000 lines
  data_shift = .20f;		// shift 20 percent
  record_proc_name = false;	// don't by default..
  data_range.min = 0;
  data_range.max = -1;
  cur_proc = NULL;
  views.SetBaseType(&TA_LogView);
}

void PDPLog::InitLinks() {
  taBase::Own(log_proc, this);
  taBase::Own(log_file, this);
  taBase::Own(data, this);
  taBase::Own(data_range, this);
  taBase::Own(log_data, this);
  taBase::Own(display_labels, this);

  WinMgr::InitLinks();
  InitFile();
  log_file.Close();
}

void PDPLog::Destroy() {
  CutLinks();
}

// PDPLog::CutLinks() in pdpshell.cc

void PDPLog::Copy_(const PDPLog& cp) {
  log_file = cp.log_file;
  log_lines = cp.log_lines;
  log_data = cp.log_data;

  data = cp.data; 
  data_bufsz = cp.data_bufsz;
  data_shift = cp.data_shift;
  data_range = cp.data_range;
  record_proc_name = cp.record_proc_name;

  log_proc.BorrowUnique(cp.log_proc);	// this is a link group..
  cur_proc = cp.cur_proc;

  display_labels = cp.display_labels;

  InitAllViews();
}

void PDPLog::AddUpdater(SchedProcess* sp) {
  if(sp == NULL) return;
  bool was_linked = false;
  was_linked = sp->logs.LinkUnique(this);	// add this to the list on the process
  was_linked = (log_proc.LinkUnique(sp) || was_linked); // add the process to the list on this;
  if(was_linked) {
    name = sp->name + "_" + GetTypeDef()->name;
    SyncLogViewUpdaters();
    GetHeaders();
  }
}  

void PDPLog::RemoveUpdater(SchedProcess* sp) {
  if(sp == NULL) return;
  sp->logs.RemoveLeaf(this);	// remove this to the list on the process
  log_proc.RemoveLeaf(sp);     	// remove the process to the list on this;
  SyncLogViewUpdaters();
  GetHeaders();
}  

void PDPLog::RemoveAllUpdaters() {
  int i;
  for(i=0; i<log_proc.size; i++) {
    ((SchedProcess*)log_proc.FastEl(i))->logs.RemoveLeaf(this);
  }
  log_proc.RemoveAll();
  SyncLogViewUpdaters();
}

void PDPLog::SyncLogViewUpdaters() {
  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i) {
    lv->updaters.Reset();
    lv->updaters.BorrowUnique(log_proc);
  }
}

int PDPLog::Dump_Save_Value(ostream& strm, TAPtr par, int indent) {
  if(log_file.IsOpen())
    log_file.mode = taFile::APPEND; // auto-open in append mode..
  else
    log_file.mode = taFile::NO_AUTO;
  return WinMgr::Dump_Save_Value(strm, par, indent);
}

void PDPLog::UpdateAfterEdit() {
  WinMgr::UpdateAfterEdit();
  SyncLogViewUpdaters();

  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i) {
    if(lv->viewspecs.leaves != data.leaves)
      lv->NewHead();
  }

  lv = (LogView*)views.SafeEl(0);
  if(lv != NULL)
    lv->NotifyAllUpdaters();	// make sure logs are getting it from  us..
  if(log_proc.size > 1)		// always record proc name if more than one..
    record_proc_name = true;
  if(log_file.gf != NULL) {
    log_file.AutoOpen();
    log_file.mode = taFile::NO_AUTO; // prevent this from happening again..
  }
  if(log_file.IsOpen()) {
    if(log_lines == 0) {
      GetFileLength();
      if(log_file.gf->ostrm != NULL) {
	HeadToFile();
      }
    }
  }
  else {
    log_lines = 0;
    data_range.max = data.MaxLength()-1;
    data_range.min = 0;
  }
  if((log_proc.size > 0) && (name.empty() || name.contains(GetTypeDef()->name))) {
    name = ((SchedProcess*)log_proc[0])->name + "_" + GetTypeDef()->name;
  }
}

void PDPLog::InitFile() {
  if(log_file.gf == NULL) {
    if(!taMisc::iv_active)
      log_file.GetGetFile();
    else {
      log_file.gf = new taivGetFile(".","*log*",NULL,wkit->style(),false);
      taRefN::Ref(log_file.gf);
    }
  }
  log_file.fname = "";
  log_file.UpdateGF();
}

void PDPLog::SetSaveFile(const char* nm, bool no_dlg) {
  log_file.SaveAsFile(nm, no_dlg);
  GetFileLength();
  HeadToFile();
  if(taMisc::iv_active)
    SetWinName();
}

void PDPLog::SetAppendFile(const char* nm, bool no_dlg) {
  log_file.AppendFile(nm, no_dlg);
  GetFileLength();
  HeadToFile();
  if(taMisc::iv_active)
    SetWinName();
}

void PDPLog::LoadFile(const char* nm, bool no_dlg) {
  log_file.OpenFile(nm, no_dlg);
  GetFileLength();
  ReadNewLogFile();
  InitAllViews();
  if(taMisc::iv_active)
    SetWinName();
}

void PDPLog::CloseFile() {
  log_file.CloseFile();
  log_lines = 0;
  if(taMisc::iv_active)
    SetWinName();
}


//////////////////////////
//	Headers 	//
//////////////////////////

void PDPLog::NewHead(LogData& ld, SchedProcess* sproc) {
  display_labels.EnforceSize(ld.items.size);

  // null is a sign to regenerate regardless
  if((sproc != NULL) && (cur_proc == sproc) && (ld.items.size == data.leaves))
    return; 

  HeadToBuffer(ld);
  LogDataFromBuffer();		// get the log data corresponding to the buffer
  UpdateViewHeaders();		// tells views that header is updated

  if((sproc != NULL) && (sproc->network != NULL)) {	// only send to log if its real
    cur_proc = sproc;

#ifdef DMEM_COMPILE
    int netsz = 0; MPI_Comm_size(sproc->network->dmem_share_units.comm, &netsz);
    if(taMisc::dmem_proc % netsz == 0)
      HeadToFile();		// use buffer version: has more accurate state info
#else
    HeadToFile();
#endif
  }
}

void PDPLog::UpdateViewHeaders() {
  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i) {
    lv->NewHead();
  }
}

void PDPLog::SetFieldHead(DataItem* ditem, DataTable* dat, int idx) {
  DataArray_impl* da = NULL;
  if(dat->size <= idx) {
    if(ditem->is_string)
      da = dat->NewEl(1, &TA_String_Data);
    else
      da = dat->NewEl(1,&TA_float_Data);
  }
  else {
    da = dat->FastEl(idx);
    if(ditem->is_string && !da->InheritsFrom(&TA_String_Data)) {
      da = new String_Data;
      dat->Replace(idx, da);
    }
    if(!ditem->is_string && !da->InheritsFrom(&TA_float_Data)) {
      da = new float_Data;
      dat->Replace(idx, da);
    }
  }
  if(da->AR() == NULL) {
    da->NewAR();
    da->AR()->Alloc(data_bufsz);
  }
  da->name = ditem->name;
  da->disp_opts = ditem->disp_opts;
}


void PDPLog::HeadToBuffer(LogData& ld) {
  int cur_i = 0;		// current item at top level
  int subgp_gpi = 0;		// current subgroup index (of the group)
  int subgp_i = 0;		// current subgroup index (of the items in the group)
  int subgp_max = 0;		// max for current subgroup
  DataTable* subgp = NULL;	// the subgroup
    
  int ldi;
  for(ldi=0; ldi< ld.items.size; ldi++) {
    DataItem* ditem = ld.items.FastEl(ldi);
    if(ditem->vec_n > 1) {
      if(data.gp.size > subgp_gpi)
	subgp = (DataTable*)data.gp[subgp_gpi];
      else
	subgp = (DataTable*)data.NewGp(1);
      subgp_gpi++;
      subgp_max = ditem->vec_n;
      subgp_i = 0;

      subgp->name = ditem->name;	// group gets name of first element
      if(!subgp->name.empty() && (subgp->name[0] == '<'))
	subgp->name = subgp->name.after('>'); // get rid of vector notation

      if(subgp->size > subgp_max)
	subgp->EnforceSize(subgp_max); // trim excess (but don't add -- could be wrong)
      SetFieldHead(ditem, subgp, subgp_i);
    }
    else {
      if(subgp != NULL) {	// in a subgroup
	subgp_i++;		// increment the index
	if(subgp_i >= subgp_max) { // done with this group
	  subgp = NULL;
	  SetFieldHead(ditem, &data, cur_i);
	  cur_i++;
	}
	else {			// get item from this group
	  SetFieldHead(ditem, subgp, subgp_i);
	}
      }
      else {			// in top-level group
	SetFieldHead(ditem, &data, cur_i);
	cur_i++;
      }
    }
  }
  if(data.size > cur_i)		// keep it the same size
    data.EnforceSize(cur_i);
  if(data.gp.size > subgp_gpi)	// keep it the same size
    data.gp.EnforceSize(subgp_gpi);
}

void PDPLog::HeadToLogFile(LogData& ld) { // this function is deprecated in favor of HeadToFile
  if(!log_file.IsOpen() || (log_file.gf->ostrm == NULL))
    return;
  
  ostream& strm = *(log_file.gf->ostrm);

  if(record_proc_name && (cur_proc != NULL))
    strm << cur_proc->name;
  strm << "_H:\t";
  int i;
  for(i=0; i < ld.items.size; i++) {
    DataItem* it = ld.items.FastEl(i);
    String hdnm = it->name;
    if((display_labels.size > i) && !display_labels[i].empty())
      hdnm = display_labels[i];
    if(it->HasDispOption(" NARROW,"))
      LogColumn(strm, hdnm, 1); // indicates one column
    else
      LogColumn(strm, hdnm, 2); // default is 2 columns
  }
  strm << "\n";
  strm.flush();
}

DataItem* PDPLog::DataItemFromDataArray(DataArray_impl* da) {
  DataItem* it = new DataItem();
  it->name = da->name;
  it->disp_opts = da->disp_opts;
  if(da->InheritsFrom(TA_String_Data))
    it->is_string = true;
  log_data.items.Add(it);
  return it;
}

void PDPLog::LogDataFromDataTable(DataTable* dt, int st_idx) {
  int i;
  for(i=st_idx; i < dt->size; i++) {
    DataArray_impl* da = dt->FastEl(i);
    DataItemFromDataArray(da);
  }
  if(dt->gp.size > 0) {
    for(i=0; i<dt->gp.size; i++) {
      DataTable* ndt = (DataTable*)dt->FastGp(i);
      if(ndt->size > 0) {
	DataArray_impl* da = ndt->FastEl(0);
	DataItem* it = DataItemFromDataArray(da);
	it->vec_n = ndt->size;
	LogDataFromDataTable(ndt, 1); // start from 1 since 0 was already done..
      }
    }
  }
}

void PDPLog::LogDataFromBuffer() {
  log_data.Reset();
  LogDataFromDataTable(&data);
  log_data.InitBlankData();
}

//////////////////////////
//	  Data 		//
//////////////////////////


void PDPLog::NewData(LogData& ld, SchedProcess* sproc) {
  NewHead(ld, sproc);		// see if header info is new..

  // buffer always reflects latest contents (views can scroll within)
  if(log_file.IsOpen() && (data_range.max < log_lines -1))
    Buffer_FF();

  DataToBuffer(ld);

  if((sproc != NULL) && (sproc->network != NULL)) {
#ifdef DMEM_COMPILE
    // only write to log for first proc of each network processing group
    int netsz = 0; MPI_Comm_size(sproc->network->dmem_share_units.comm, &netsz);
    if(taMisc::dmem_proc % netsz == 0)
      DataToLogFile(ld);
#else
    DataToLogFile(ld);
#endif
  }

  int max_lns = data.MaxLength() -1; // get new size
  if(max_lns < data_range.Range()) { // must have shifted buffers..
    if(!log_file.IsOpen()) {
      data_range.min = 0;
      data_range.max = max_lns;
    }
    else {
      data_range.max++;
      data_range.min = data_range.max - max_lns;
      data_range.MinGT(0);
    }
    if(taMisc::iv_active) {
      LogView* lv;
      taLeafItr i;
      FOR_ITR_EL(LogView, lv, views., i) {
	lv->View_FF();		// catch the views up with the new range
      }
    }
  }
  else {
    data_range.max++;
    LogView* lv;
    taLeafItr i;
    FOR_ITR_EL(LogView, lv, views., i) {
      lv->NewData();		// just update new line
    }
  }
}

void PDPLog::SetFieldData(LogData& ld, int ldi, DataItem* ditem, DataTable* dat, int idx) {
  if(dat->size <= idx) {
    return;			// not good!
  }
  DataArray_impl* da = dat->FastEl(idx);
  if(da->AR() == NULL) {
    da->NewAR();
  }
  if(da->AR()->size >= data_bufsz) { // adding will 'overflow' buffer
    da->AR()->ShiftLeftPct(data_shift);
  }

  if(ditem->is_string) {
    if(da->InheritsFrom(&TA_String_Data))
      ((String_Array*)da->AR())->Add(ld.GetString(ldi));
  }
  else {
    if(da->InheritsFrom(&TA_float_Data))
      ((float_Array*)da->AR())->Add(ld.GetFloat(ldi));
  }
}


void PDPLog::DataToBuffer(LogData& ld) {
  int cur_i = 0;		// current item at top level
  int subgp_gpi = 0;		// current subgroup index (of the group)
  int subgp_i = 0;		// current subgroup index (of the items in the group)
  int subgp_max = 0;		// max for current subgroup
  DataTable* subgp = NULL;	// the subgroup

  int ldi;
  for(ldi=0; ldi < ld.items.size; ldi++) {
    DataItem* ditem = ld.items.FastEl(ldi);
    if(ld.IsVec(ldi)) {
      if(data.gp.size > subgp_gpi)
	subgp = (DataTable*)data.gp[subgp_gpi];
      else {
	return;			// should not happen!
      }
      subgp_gpi++;
      subgp_max = ld.GetVecN(ldi);
      if(subgp_max <= 0)
	subgp_max = 1;		// need at least one in group (this one!)
      subgp_i = 0;

      SetFieldData(ld, ldi, ditem, subgp, subgp_i);
    }
    else {
      if(subgp != NULL) {	// in a subgroup
	subgp_i++;		// increment the index
	if(subgp_i >= subgp_max) { // done with this group
	  subgp = NULL;
	  SetFieldData(ld, ldi, ditem, &data, cur_i);
	  cur_i++;
	}
	else {			// get item from this group
	  SetFieldData(ld, ldi, ditem, subgp, subgp_i);
	}
      }
      else {			// in top-level group
	SetFieldData(ld, ldi, ditem, &data, cur_i);
	cur_i++;
      }
    }
  }
}

void PDPLog::DataToLogFile(LogData&) {
  if(!log_file.IsOpen() || (log_file.gf->ostrm == NULL))
    return;

  ostream& strm = *(log_file.gf->ostrm);

  if(record_proc_name && (cur_proc != NULL))
    strm << cur_proc->name;
  strm << "_D:\t";

  taLeafItr i;
  DataArray_impl* da;
  FOR_ITR_EL(DataArray_impl, da, data., i) {
    if(!da->save_to_file) continue;
    String el;
    taArray_base* ar = da->AR();
    if((ar != NULL) && (ar->size > 0))
      el = ar->El_GetStr_(ar->FastEl_(ar->size-1));
    else
      el = "n/a";

    if(da->HasDispOption(" NARROW,"))
      LogColumn(strm, el, 1);
    else
      LogColumn(strm, el, 2);
  }
  strm << "\n";
  strm.flush();
  log_lines++;
}

void PDPLog::GetHeaders(bool keep_display_settings) {
  display_labels.Reset();	// these get re-generated from the viewspecs..
  data.Reset();		// clear out old data...

  LogView* lv;
  taLeafItr vi;
  if(!keep_display_settings) {
    FOR_ITR_EL(LogView, lv, views., vi) {
      if(lv->viewspec != NULL)
	lv->viewspec->Reset();	// clear all viewspecs too
      lv->view_range.min =0; lv->view_range.max = -1;
      lv->InitDisplay();
    }
  }

  taLeafItr i;
  SchedProcess* sproc;
  FOR_ITR_EL(SchedProcess, sproc, log_proc., i) {
    sproc->GenLogData();
    NewHead(sproc->log_data, NULL); // this is a "fake" head, not from a log directly
    HeadToFile();
  }

  // might need to manually rebuild if sizes did not change..
  if(keep_display_settings) {
    FOR_ITR_EL(LogView, lv, views., vi) {
      if(lv->viewspec == NULL) continue;
      if((lv->viewspec->size == data.size) && (lv->viewspec->leaves == data.leaves)
	 && (lv->viewspec->gp.size == data.gp.size)) {
	lv->viewspec->ReBuildFromDataTable();
	lv->InitDisplay();
      }
    }
  }
}

ostream& PDPLog::LogColumn(ostream& strm, String& str, int tabs) {
  strm << str;
  if(tabs == 2) {		// only 2 options (1 or 2)
    if(str.length() < 8)
      strm << "\t\t";
    else if(str.length() < 16)
      strm << "\t";
    else
      strm << "\t";		// always tab delimited..
  }
  else {
    if(str.length() < 8)
      strm << "\t";
    else
      strm << "\t";
  }
  return strm;
}
  
 

//////////////////////////
// 	Buffers		//
//////////////////////////
 
// checks for comments and headers,
// 0 = comment, 1 = header, 2 = data

int PDPLog::LogLineType(char* lnc) {
  String ln = String(lnc);

  // skip leading spaces if any
  while(ln.firstchar() == ' ') {
    ln = ln.after(' ');
  }
  if(ln.empty())  return 0;

  if((ln.firstchar() == '#') || (ln.before(2) == "//")) {
    return 0;
  }
  
  String id = ln.before('\t');
  if(id .empty()) return 0;

  String id_sufx = id.after('_', -1); // find after last underbar
  if(id_sufx == "H:") return 1;
  if(id_sufx == "D:") return 2;
  return 2;			// assume its data
}

// reads the file up to just before the dataline, strm must be just opened
// returns the maximum dataline number it could read

int PDPLog::FileScanTo(istream& strm, int log_ln) {
  if(log_ln <= 0) return 0;	// ready to read line 0

  int count = 0;
  String templine;
  
  while(readline(strm,templine)) {
    if(LogLineType(templine) != 2) // only count data lines
      continue;
    count++;
    if (count >= log_ln) break;
  }
  return count;		// the number of the line were ready to read
}  


int PDPLog::GetFileLength() {
  if(!log_file.IsOpen()) return 0;

  taivGetFile gf;		// use a getfile for compressed reads..
  gf.fname = log_file.fname;
  istream* strm = gf.open_read();
  if((strm == NULL) || strm->bad())
    return 0;

  int act_lines = FileScanTo(*strm, INT_MAX); // read as far as we can..
  gf.Close();

  log_lines = act_lines;
  data_range.MaxLT(log_lines -1);
  return act_lines;
}

void PDPLog::HeadFromLogFile(String& hdln) { // argument is mutable..
  int itm_cnt = 0;
  if(hdln.lastchar() != '\t') hdln += "\t";
  while(hdln.contains('\t')) {
    String f = hdln.before('\t');
    if((!f.empty()) && !f.contains("_H:")) {
      DataItem* it;
      if(itm_cnt >= log_data.items.size) {
	it = new DataItem();
	log_data.items.Add(it);
      }
      else
	it = log_data.items.FastEl(itm_cnt);
      if(f != it->name) {	// names don't match
	it->Initialize();	// reset info on 
	it->name = f;
	if(f[0] == '$') {
	  it->is_string = true;
	  if(f[1] == '<') {
	    String vec_n = f.after('<');
	    vec_n = vec_n.before('>');
	    it->vec_n = (int)vec_n;
	  }
	}
	else if(f[0] == '|') {
	  it->AddDispOption("NARROW");
	}
	else if(f[0] == '<') {
	  String vec_n = f.after('<');
	  vec_n = vec_n.before('>');
	  it->vec_n = (int)vec_n;
	}
      }
      itm_cnt++;
    }
    hdln=hdln.after('\t');
  }
  if(log_data.index.size != log_data.items.size)
    log_data.InitBlankData();
  NewHead(log_data, NULL);
}


void PDPLog::DataFromLogFile(String& dtln) {
  // log data heads are assumed to be current..
  int cnt = 0;
  if(dtln.lastchar() != '\t') dtln += "\t";
  if(log_data.index.size != log_data.items.size) { // make sure these line up..
    log_data.InitBlankData();			   // if not, reset data
  }
  while(dtln.contains('\t') && (cnt < log_data.items.size)) {
    String f = dtln.before('\t');
    if((!f.empty()) && !f.contains("_D:")) {
      if(log_data.IsString(cnt))
	log_data.GetString(cnt) = f; // these don't existe..
      else
	log_data.GetFloat(cnt) = atof(f);
      cnt++;
    }
    dtln=dtln.after('\t');
  }
  DataToBuffer(log_data);
}


void PDPLog::LogFileToBuffer() {
  if(!log_file.IsOpen()) return;

  data.ResetData();		// clear out the arrays for new data
  data_range.MinGT(0);		// min must be greater than 0

  taivGetFile gf;		// use a getfile for compressed reads..
  gf.fname = log_file.fname;
  istream* strm = gf.open_read();

  if((strm == NULL) || strm->bad())
    return;

  int act_lines = FileScanTo(*strm, (int)data_range.min);
  if(act_lines != data_range.min) {
    log_lines = act_lines;	// we adjust to the actual size of the file..
    data_range.MaxLT(log_lines-1);
    data_range.min = data_range.max - data_bufsz + 1;
    data_range.MinGT(0);
    if(log_lines == 0) {
      gf.Close();
      return;
    }
  }

  int lno = data_range.min;
  String templn;
  int rval;
  while((rval = readline(*strm,templn)) && (lno <= data_range.max)) {
    int ltyp = LogLineType(templn);
    if(ltyp == 0)
      continue;
    if(ltyp == 1) {
      HeadFromLogFile(templn);
    }
    if(ltyp == 2) {
      DataFromLogFile(templn);
      lno++;
    }
  }
  if(rval == 0) {
    log_lines = lno;
    data_range.MaxLT(lno -1 );	// reset max based on what we actually got
  }
  gf.Close();
}

void PDPLog::HeadToFile() {	// based on buffer
  if(!log_file.IsOpen() || (log_file.gf->ostrm == NULL))
    return;

  ostream& strm = *(log_file.gf->ostrm);

  if(record_proc_name && (cur_proc != NULL))
    strm << cur_proc->name;
  strm << "_H:\t";
  int i;
  for(i=0; i < data.leaves; i++) {
    DataArray_impl* da = data.Leaf(i);
    if(!da->save_to_file) continue;
    String hdnm = da->name;
    if((display_labels.size > i) && !display_labels[i].empty())
      hdnm = display_labels[i];
    int wdth = 2;
    if(da->HasDispOption(" NARROW,"))
      wdth = 1;
    LogColumn(strm, hdnm, wdth);
  }
  strm << "\n";
  strm.flush();
}

void PDPLog::BufferToFile(const char* nm, bool no_dlg) {
  bool new_open = false;
  if(!log_file.IsOpen() || (log_file.gf->ostrm == NULL) || (nm != NULL)) {
    SetSaveFile(nm, no_dlg);
    if(log_file.gf->ostrm == NULL) return;
    new_open = true;
  }

  ostream& strm = *(log_file.gf->ostrm);

  int max_lns = data.MaxLength()-1;
  int ln;
  for(ln=0; ln <= max_lns; ln++) {
    if(record_proc_name && (cur_proc != NULL))
      strm << cur_proc->name;
    strm << "_D:\t";
    int from_end = max_lns - ln ;
    taLeafItr i;
    DataArray_impl* da;
    FOR_ITR_EL(DataArray_impl, da, data., i) {
      if(!da->save_to_file) continue;
      String el;
      taArray_base* ar = da->AR();
      if((ar != NULL) && (ar->size > from_end) && (from_end >= 0))
	el = ar->El_GetStr_(ar->FastEl_(ar->size-1-from_end));
      else
	el = "n/a";

      if(da->HasDispOption(" NARROW,"))
	LogColumn(strm, el, 1);
      else
	LogColumn(strm, el, 2);
    }
    strm << "\n";
  }
  strm.flush();
  if(new_open) {
    CloseFile();
  }
}

void PDPLog::Buffer_F() {
  if(!log_file.IsOpen()) return;
  int shft = (int)((float)data_bufsz * data_shift);
  data_range.max += shft;
  data_range.MaxLT(log_lines - 1);
  data_range.min = data_range.max - data_bufsz + 1;
  data_range.MinGT(0);
  LogFileToBuffer();
}

void PDPLog::Buffer_FF() {
  if(!log_file.IsOpen()) return;
  data_range.max = log_lines - 1;
  data_range.min = data_range.max - data_bufsz + 1;
  data_range.MinGT(0);
  LogFileToBuffer();
}

void PDPLog::Buffer_R() {
  if(!log_file.IsOpen()) return;
  int shft = (int)((float)data_bufsz * data_shift);
  data_range.min -= shft;
  data_range.MinGT(0);
  data_range.max = data_range.min + data_bufsz - 1;
  data_range.MaxLT(log_lines - 1);
  LogFileToBuffer();
}

void PDPLog::Buffer_FR() {
  if(!log_file.IsOpen()) return;
  data_range.min = 0;
  data_range.max = data_range.min + data_bufsz -1;
  data_range.MaxLT(log_lines - 1);
  LogFileToBuffer();
}

void PDPLog::ReadNewLogFile() {
  Buffer_FF();		// go to end
  if(!taMisc::iv_active) return;
  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i) {
    lv->View_FF();		// same for all the views
  }
}

void PDPLog::Clear() {
  data.ResetData();
  data_range.min = 0; data_range.max = -1;
  if(!taMisc::iv_active) return;
  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i)
    lv->Clear_impl();
}

void PDPLog::ViewAllData() {
  if(data.leaves == 0) return;
  DataArray_impl* da = (DataArray_impl*)data.Leaf(0);
  data_range.min = 0;
  data_range.max = da->AR()->size - 1;
  LogView* lv;
  taLeafItr i;
  FOR_ITR_EL(LogView, lv, views., i) {
    lv->viewspec->Reset();
    lv->NewHead();
    lv->View_FR();
  }
}

void NetLog::SetNetwork(Network* net) {
  NetLogView* lv;
  taLeafItr i;
  FOR_ITR_EL(NetLogView, lv, views., i)
    lv->SetNetwork(net);
}

//////////////////////////////////
// 	Analysis Routines 	//
//////////////////////////////////

void LogView::CopyToEnv(taBase* data, taBase* labels, Environment* env) {
  if((data == NULL) || (labels == NULL)) return;
  if(!data->InheritsFrom(&TA_DT_ViewSpec)){
    taMisc::Error("CopyToEnv: copying of single column of data not supported -- please select a group of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)){
    taMisc::Error("CopyToEnv: labels must be a single column in the log, not a group");
    return;
  }
  if(env == NULL) {
    env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
    if(env == NULL) return;
  }
  env->events.Reset();
  DT_ViewSpec* dtvs = (DT_ViewSpec*)data;
  DataTable* tabl = dtvs->data_table;

  DA_ViewSpec* davs = (DA_ViewSpec*)labels;
  taArray_base* labsbs = davs->data_array->AR();
  float_RArray* rlabs = NULL;
  String_Array* slabs = NULL;
  if(labsbs->InheritsFrom(TA_float_RArray))
    rlabs = (float_RArray*)labsbs;
  else if(labsbs->InheritsFrom(TA_String_Array))
    slabs = (String_Array*)labsbs;

  EventSpec* es = env->GetAnEventSpec();
  es->UpdateAfterEdit();	// make sure its all done with internals..
  es->patterns.EnforceSize(1);

  PatternSpec* ps = (PatternSpec*)es->patterns[0];
  ps->n_vals = tabl->leaves;
  if(dtvs->InheritsFrom(TA_DT_GridViewSpec))
    ps->geom = ((DT_GridViewSpec*)dtvs)->geom;
  ps->UpdateAfterEdit();	// this will sort out cases where nvals > geom
  es->UpdateAllEvents();	// get them all straightened out

  int len = tabl->MinLength();
  int i;
  for(i=0;i<len;i++) {
    Event* ev = (Event*)env->events.NewEl(1);
    if(rlabs != NULL)
      ev->name = (String)rlabs->SafeEl(i);
    else if(slabs != NULL)
      ev->name = (String)slabs->SafeEl(i);
    else
      ev->name = "Row_" + String(i);
    Pattern* pat = (Pattern*)ev->patterns.FastEl(0);
    pat->value.Reset();
    tabl->AddRowToArray(pat->value, i);
  }
}

void LogView::DistMatrixGrid(taBase* data, taBase* labels, GridLog* disp_log,
			     float_RArray::DistMetric metric, bool norm, float tol) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)){
    taMisc::Error("DistMatrixGrid: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)){
    taMisc::Error("DistMatrixGrid: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->DistMatrixGrid(disp_log, 0, metric, norm, tol);
  tabMisc::Close_Obj(env);
}

void LogView::ClusterPlot(taBase* data, taBase* labels, GraphLog* disp_log,
			  float_RArray::DistMetric metric, bool norm, float tol) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)) {
    taMisc::Error("ClusterPlot: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)) {
    taMisc::Error("ClusterPlot: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->ClusterPlot(disp_log, 0, metric, norm, tol);
  tabMisc::Close_Obj(env);
}

void LogView::CorrelMatrixGrid(taBase* data, taBase* labels, GridLog* disp_log) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)) {
    taMisc::Error("CorrelMatrixGrid: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)) {
    taMisc::Error("CorrelMatrixGrid: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->CorrelMatrixGrid(disp_log, 0);
  tabMisc::Close_Obj(env);
}

void LogView::PCAEigenGrid(taBase* data, taBase* labels, GridLog* disp_log) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)){
    taMisc::Error("PCAEigenGrid: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)){
    taMisc::Error("PCAEigenGrid: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->PCAEigenGrid(disp_log, 0);
  tabMisc::Close_Obj(env);
}

void LogView::PCAPrjnPlot(taBase* data, taBase* labels, GraphLog* disp_log, int x_axis_component, int y_axis_component) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)){
    taMisc::Error("PCAPrjnPlot: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)){
    taMisc::Error("PCAPrjnPlot: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->PCAPrjnPlot(disp_log, 0, x_axis_component, y_axis_component);
  tabMisc::Close_Obj(env);
}

void LogView::MDSPrjnPlot(taBase* data, taBase* labels, GraphLog* disp_log, int x_axis_component, int y_axis_component) {
  if(!data->InheritsFrom(&TA_DT_ViewSpec)){
    taMisc::Error("MDSPrjnPlot: use of single data of data not supported -- please select a vector (group) of data");
    return;
  }
  if(!labels->InheritsFrom(&TA_DA_ViewSpec)){
    taMisc::Error("MDSPrjnPlot: labels must be a single column in the log, not a group");
    return;
  }
  Environment* env = pdpMisc::GetNewEnv(GET_MY_OWNER(Project), &TA_Environment);
  if(env == NULL) return;
  CopyToEnv(data, labels, env);
  env->MDSPrjnPlot(disp_log, 0, x_axis_component, y_axis_component);
  tabMisc::Close_Obj(env);
}

////////////////////////////////////////

bool PDPLog_MGroup::nw_itm_def_arg = false;

PDPLog* PDPLog_MGroup::FindMakeLog(const char* nm, TypeDef* td, bool& nw_itm) {
  PDPLog* gp = NULL;
  nw_itm = false;
  if(nm != NULL)
    gp = (PDPLog*)FindLeafName(nm);
  else
    gp = (PDPLog*)FindLeafType(td);
  if(gp == NULL) {
    gp = (PDPLog*)NewEl(1, td);
    if(nm != NULL)
      gp->name = nm;
    nw_itm = true;
  }
  return gp;
}
