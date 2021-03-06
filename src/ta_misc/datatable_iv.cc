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

#include <ta_misc/datatable_iv.h>

// stuff to implement graphical view of datatable

#include <ta/ta_group_iv.h>

#include <iv_misc/lrScrollBox.h>
#include <iv_misc/tbScrollBox.h>
#include <iv_misc/dastepper.h>

#include <ta/enter_iv.h>
#include <InterViews/brush.h>
#include <InterViews/color.h>
#include <InterViews/tformsetter.h>
#include <InterViews/transformer.h>
#include <InterViews/action.h>
#include <InterViews/box.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <InterViews/canvas.h>
#include <InterViews/session.h>
#include <InterViews/event.h>
#include <InterViews/style.h>
#include <InterViews/telltale.h>
#include <InterViews/geometry.h>
#include <InterViews/patch.h>
#include <InterViews/background.h>
#include <InterViews/cursor.h>
#include <InterViews/hit.h>
#include <InterViews/bitmap.h>
#include <InterViews/font.h>
#include <InterViews/polyglyph.h> // for height field neighbors
#include <IV-look/kit.h>
#include <IV-look/stepper.h>
#include <IV-look/field.h>
#include <IV-look/bevel.h>
#include <OS/file.h>
#include <OS/math.h>
// for hier menu
#include <IV-look/menu.h>
#include <ta/leave_iv.h>

#include <limits.h>
#include <float.h>
#include <unistd.h>

const float DTV_INITIAL_X = 408.0f;
const float DTV_INITIAL_Y = 100.0f;
const float DTV_MARG = 3.0f;
const float DT_LINE_GAP = 2.0f;

#define DIST(x,y) sqrt((double) ((x * x) + (y*y)))

//////////////////////////////////
//	   DataTable_G		//
//////////////////////////////////

DataTable_G::DataTable_G(DataTable* d, DT_GridViewSpec* dtv) {
  dt = d;
  dtvsp = dtv;
  defbrush = new ivBrush(BRUSH_SOLID,DEFAULT_BRUSH_WIDTH);
  ivResource::ref(defbrush);

  border = new ivColor(0.0, 0.0, 0.0, 1.0);
  ivResource::ref(border);

  reinit_display = false;
  taBase::Own(selectgroup, dt); // these are conceptually owned by the DataTable..
  taBase::Own(pickgroup, dt);	  // ditto
  tx = NULL;
}


DataTable_G::~DataTable_G() {
  // Reset(); // do not call this since there is a flush in Reset();
  selectgroup.RemoveAll(); // just do these two instead
  pickgroup.RemoveAll(); 

  ivResource::unref(defbrush); defbrush = NULL;
  ivResource::unref(border);	border = NULL;
}

void DataTable_G::Reset() {
  remove_all();
  selectgroup.RemoveAll();
  pickgroup.RemoveAll();
  ivResource::flush(); // clear our all defered unrefs.
}

void DataTable_G::Build() {
  Reset();			// clear out existing stuff
  max_size.Initialize();
  dtvsp->UpdateGeom();
  max_size.x = dtvsp->full_geom.x;
  max_size.y = dtvsp->full_geom.y;

  if((max_size.x == 0) || (max_size.y == 0)) return; // nothin there!
  
  ivAllotment& ax = _a.x_allotment();
  ivAllotment& ay = _a.y_allotment();

  float eff_ln_gap = DT_LINE_GAP / ay.span();

  // set maximum block size
  float blksz = ax.span() / (float)max_size.x;
  if(blksz > 2.0f * (float)owner->block_size) {	// each "pixel" counts for 2
    // make it seem wider to scale to fit block size
    max_size.x = (((blksz / (2.0f * (float)owner->block_size))) * (float)max_size.x);
  }

  // figure out y in order to keep the block sizes roughly square
  float yxrat = ay.span() / ax.span();
  int trgy = (int)floor(yxrat * (float)max_size.x); // target size of y coords based on x & scrn rat, round down
  int nlines = (int)floor((float)trgy / (float)max_size.y);
  nlines--;			// actually requires nlines+1 display area, so subtract 1 here
  if(nlines < 1) nlines = 1;

  max_size.y *= (float)nlines;		// prospective max size

  // one grid point = 1 / mxy.  total extra line gap = nl * lg.  find out how many extra grid pts in line gaps
  int lgblocks = (int)((float)nlines * eff_ln_gap * (float)max_size.y);
  // now how many nlines is this?
  int lglns = lgblocks / (2 * dtvsp->full_geom.y);// only go half-way towards meeting it!
  nlines -= lglns;		// get rid of it!
  if(nlines < 1) nlines = 1;

  // now, figure out exact max size scaling to fit in display
  int mxypos = dtvsp->full_geom.y * (nlines + 1); // maximum y coordinate position
  // 1 = mxyp / mxy + nl*lg,  mxyp/mxy = 1-nl*lg, mxyp = mxy(1-nl*lg), mxy = mxyp /(1-nl*lg)
  max_size.y = ((float)mxypos / (1.0f - (float)nlines * eff_ln_gap));
  
  // add two border lines at top and bottom to keep frame always present
  DataLineBox_G* bx = new DataLineBox_G(NULL, this);
  append_(bx);
  bx->SetPointsLine(dtvsp->full_geom.x, dtvsp->full_geom.y);
  // don't translate -- leave at 0,0

  bx = new DataLineBox_G(NULL, this);
  append_(bx);
  bx->SetPointsLine(dtvsp->full_geom.x, dtvsp->full_geom.y);
  bx->translate(0.0f, ycscale(mxypos) + (float)nlines * eff_ln_gap);
  // set to maximum y position

  int dlines = nlines;		// nlines includes header, dlines = data lines
  if(!owner->header_on) dlines++; // one more display line for you!
  owner->disp_lines = dlines;

  // try to extend view_range.max to use up entire display, within confines of the data
  int rng = owner->view_range.Range() + 1; // actual number of lines = +1
  int mxln = dt->MaxLength()-1;
  owner->view_range.max = owner->view_range.min + dlines -1;
  owner->view_range.MaxLT(mxln); // double check
  owner->view_range.max = MAX(owner->view_range.max, -1); // don't go below -1!
  owner->view_range.min = owner->view_range.max - dlines +1;
  owner->view_range.MinGT(0);
  rng = owner->view_range.Range() + 1; // final range

  // now build it! -- goes in reverse order!
  int ypos = dtvsp->full_geom.y * nlines;
  int i;

  int st = -1;			// start at -1 == spec
  int gaplns = nlines;
  if(!owner->header_on) {
    st = 0; // skip header
    gaplns++;
  }

  for(i=st;i<rng;i++) {		
    DataLine_G* dlg;
    if(i == -1)
      dlg = new DataLine_G(i, this);
    else
      dlg = new DataLine_G(mxln - (owner->view_range.min + i), this);
    append_(dlg);
    dlg->translate(0.0f, ycscale((float)ypos) + (float)(gaplns - i - 1) * eff_ln_gap);
    dlg->Build();
    ypos -= dtvsp->full_geom.y;
  }

  ivExtension ext;
  if((owner!= NULL) && (owner->viewer != NULL) &&
     (owner->viewer->canvas() != NULL)) {
    allocate(owner->viewer->canvas(),_a,ext);
    safe_damage_me(owner->viewer->canvas());
  }
}

void DataTable_G::ScrollLines() {
  int dlines = owner->disp_lines;
  int nlines = dlines;
  if(!owner->header_on) nlines--;

  int rng = owner->view_range.Range() + 1; // final range

  if(rng > dlines) {
    Build();			// problemo!
    return;
  }

  // change to new line numbers!
  int mxln = dt->MaxLength()-1;
  int ypos = dtvsp->full_geom.y * nlines;

  int i = 0;
  int cnt;
  for(cnt=2;cnt<count_();cnt++) { // skip 1st two line 
    DataLine_G* dlg = (DataLine_G*)component_(cnt);
    if(dlg->line_no < 0) {
      ypos -= dtvsp->full_geom.y;
      continue;	// skip over spec
    }
    int nw_ln = mxln - (owner->view_range.min + i);
    dlg->ScrollLines(nw_ln);
    ypos -= dtvsp->full_geom.y;
    i++;
    if(i >= rng) break;
  }
}

void DataTable_G::AddOneLine() {
  // assume that view range has been updated and there is just one more line to display
  int dlines = owner->disp_lines;
  int nlines = dlines;
  if(!owner->header_on) nlines--;

  int rng = owner->view_range.Range() + 1; // final range

  int trg_cnt = dlines + 1;
  if(!owner->header_on) trg_cnt--;
  if((rng == dlines) && (count_() == trg_cnt + 2)) {
    // actually displaying full range!
    ScrollLines();
    update_from_state(owner->viewer->canvas());
    return;
  }
  if((rng > dlines) || (count_() > trg_cnt + 2)) {
    Build();			// problemo!
    return;
  }

  ivAllotment& ay = _a.y_allotment();
  float eff_ln_gap = DT_LINE_GAP / ay.span();

  // add one new line
  int i = rng-1;		// at the end
  int yc = i+1;
  int gaplns = nlines;
  if(!owner->header_on) {
    yc--;
    gaplns++;
  }
  int mxln = dt->MaxLength()-1;
  int ypos = dtvsp->full_geom.y * nlines - dtvsp->full_geom.y * yc;
  DataLine_G* dlg = new DataLine_G(mxln - (owner->view_range.min + i), this);
  append_(dlg);
  dlg->translate(0.0f, ycscale((float)ypos) + (float)(gaplns - i - 1) * eff_ln_gap);
  dlg->Build();
  
  ScrollLines();		// update other guys!

  dlg->damage_me(owner->viewer->canvas());
}

ivColor* DataTable_G::GetLabelColor() {
  return owner->scale->background.contrastcolor;
}

ivColor*  DataTableGGetLabelColor(void* obj){
  return ((DataTable_G *) obj)->GetLabelColor();
}

bool DataTable_G::update_from_state(ivCanvas* c) {
  bool result = GraphicMaster::update_from_state(c);
  return result;
}

bool DataTable_G::effect_select(bool set_select) {
  bool result = GraphicMaster::effect_select(set_select);
  if(reinit_display) {
    reinit_display = false;
    owner->InitDisplay();
  }
  return result;
}

bool DataTable_G::select(const ivEvent& e, Tool& tool, bool b) {
  bool result = GraphicMaster::select(e,tool,b);
  if(reinit_display) {
    reinit_display = false;
    owner->InitDisplay();
  }
  return result;
}

void DataTable_G::ScaleCenter(const ivAllocation& a){
  ivAllocation b(a);
  ivAllotment& ax = _a.x_allotment();
  ivAllotment& ay = _a.y_allotment();
  ivAllotment& bx = b.x_allotment();
  ivAllotment& by = b.y_allotment();
  translate((bx.span()-ax.span())/2.0, (by.span()-ay.span())/2.0);
  translate(bx.begin()-ax.begin(), by.begin()-ay.begin());
  //  translate(0, 3);
}

void DataTable_G::ReCenter(){
  // clear out old allocation so DataTable is rescaled/translated.
  ivAllocation a = _a; //temp for clearing
  ivAllocation b;
  _a = b; // zero's
  ScaleCenter(a);
  _a = a;
}

void DataTable_G::allocate (ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  if(tx) {
    transformer(tx);
    ivResource::unref(tx);
    tx = NULL;
    _a = a;
  }
  else {
    if (!_a.equals(a, 0.001)) {
      if(owner!= NULL){
	ScaleCenter(a);
      }
      _a = a;
    }
  }
  if (c != nil) {
    PolyGraphic::allocate(c, a, ext);
  }
}

//////////////////////////////////
//	   DataLine_G		//
//////////////////////////////////

DataLine_G::DataLine_G(int lno, DataTable_G* ng) {
  line_no = lno;
  dtg = ng;
}

DataLine_G::~DataLine_G() {
}

ivGlyph* DataLine_G::clone() const {
  return new DataLine_G(line_no, dtg); 
}


void DataLine_G::BuildTable(DT_GridViewSpec* dtv) {
  // first do the individual elements
  int i;
  for(i=0;i<dtv->size; i++) {
    DA_GridViewSpec* vs = (DA_GridViewSpec*)dtv->FastEl(i);
    if(!vs->visible) continue;
    DataArray_impl* da = vs->data_array;
    taArray_base* ar = da->AR();
    if((line_no < 0) || (vs->display_style == DA_GridViewSpec::TEXT) || 
       da->InheritsFrom(&TA_String_Data)) {
      String txt;
      if(line_no < 0)
	txt = vs->display_name;
      else {
	if((ar != NULL) && (ar->size > line_no))
	  txt = ar->El_GetStr_(ar->FastEl_(ar->size-1-line_no));
	else
	  txt = "n/a";
      }
      TextData_G* gr = new TextData_G(line_no, vs, dtg, dtg->owner->view_font.fnt,
				      dtg->border, (const char*)txt, NULL);
      append_(gr);
      gr->translate(dtg->xtscale(vs->pos.x) + 3.0f, dtg->ytscale(vs->pos.y) + 3.0f);
    }
    else {
      Block_G* gr = new Block_G(line_no, vs, dtg);
      append_(gr);
      gr->translate(dtg->xcscale(vs->pos.x), dtg->ycscale(vs->pos.y));
    }
  }

  for(i=0;i<dtv->gp.size;i++) {
    DT_GridViewSpec* subsp = (DT_GridViewSpec*)dtv->FastGp(i);
    if(!subsp->visible) continue;
    if(subsp->use_gp_name) {
      Grid_G* gr = new Grid_G(line_no, subsp, dtg);
      append_(gr);
      gr->translate(dtg->xcscale(subsp->pos.x), dtg->ycscale(subsp->pos.y));
    }
    else
      BuildTable(subsp);
  }
}

void DataLine_G::Build() {
  remove_all();
  DataLineBox_G* eb = new DataLineBox_G(this, dtg); // first element
  append_(eb);
  ivCoord x = dtg->dtvsp->full_geom.x;
  ivCoord y = dtg->dtvsp->full_geom.y;
  eb->SetPointsBox(x, y);

  BuildTable(dtg->dtvsp);
}

void DataLine_G::ScrollLines(int nw_ln) {
  line_no = nw_ln;
  int i;
  for(i=1; i < count_(); i++) {
    if(component_(i)->InheritsFrom(&TA_TextData_G))
      ((TextData_G*)component_(i))->line_no = nw_ln;
    else if(component_(i)->InheritsFrom(&TA_Block_G))
      ((Block_G*)component_(i))->line_no = nw_ln;
    else if(component_(i)->InheritsFrom(&TA_Grid_G))
      ((Grid_G*)component_(i))->line_no = nw_ln;
  }
}

bool DataLine_G::select(const ivEvent& e, Tool& tool, bool unselect) {
  return PolyGraphic::select(e, tool, unselect);
}

bool DataLine_G::effect_select(bool set_select) {
  bool temp = PolyGraphic::effect_select(set_select);
  return temp;
}

//////////////////////////////////
//	   DataLineBox_G	//
//////////////////////////////////

static ivCoord pts[4];

DataLineBox_G::DataLineBox_G(DataLine_G* eg, DataTable_G* dg) :
Polygon(dg->defbrush, dg->border, NULL, pts, pts, 4, NULL) {
  dlg = eg;
  dtg = dg;
}

void DataLineBox_G::SetPointsBox(ivCoord x, ivCoord y) {
  _x[0] = dtg->xcscale(0.0f);
  _x[1] = dtg->xcscale(x);
  _x[2] = dtg->xcscale(x);
  _x[3] = dtg->xcscale(0.0f);
  _y[0] = dtg->ycscale(0.0f);
  _y[1] = dtg->ycscale(0.0f);
  _y[2] = dtg->ycscale(y);
  _y[3] = dtg->ycscale(y);
  recompute_shape();
}

void DataLineBox_G::SetPointsLine(ivCoord x, ivCoord) {
  _x[0] = dtg->xcscale(0.0f);
  _x[1] = dtg->xcscale(x);
  _x[2] = dtg->xcscale(x);
  _x[3] = dtg->xcscale(0.0f);
  _y[0] = dtg->ycscale(0.0f);
  _y[1] = dtg->ycscale(0.0f);
  _y[2] = dtg->ycscale(.01f);
  _y[3] = dtg->ycscale(.01f);
  recompute_shape();
}

bool DataLineBox_G::select(const ivEvent& e, Tool& tool, bool unselect) {
  if((dlg == NULL) || (dlg->line_no >= 0)) return false; // never select
  int but = Graphic::GetButton(e);
  if(but == ivEvent::right)
    editb_used = true;
  else
    editb_used = false;
  return Polygon::select(e, tool, unselect);
}

bool DataLineBox_G::effect_select(bool set_select) {
  if((dlg == NULL) || (dlg->line_no >= 0)) return false;
  bool temp = Polygon::effect_select(set_select);
  if(set_select && editb_used) {
    //    dtg->dtvsp->Edit();
  }
  editb_used = false;		// don't do it again!
  return temp;
}


//////////////////////////////////
//	   TextData_G		//
//////////////////////////////////

TextData_G::TextData_G(int lno, DA_GridViewSpec* sp, DataTable_G* dg,
		       const ivFont* f, const ivColor* c, const char* ch, ivTransformer* t) :
  NoScale_Text_G(sp, dg, f, c, ch, t) {
  line_no = lno;
  daspec = sp;
  dtg = dg;
  editb_used = false;
  get_color = DataTableGGetLabelColor; // use background color of DataTable_g
}

void TextData_G::Position() {
  translate(dtg->xtscale(daspec->pos.x), dtg->ytscale(daspec->pos.y));
}

bool TextData_G::update_from_state(ivCanvas* c) {
  if(line_no < 0) return false;
  DataArray_impl* da = daspec->data_array;
  if(da->InheritsFrom(&TA_float_Data)) {
    float_RArray* ar = ((float_Data*)da)->ar;
    if((ar != NULL) && (ar->size > line_no))
      name = ar->El_GetStr_(ar->FastEl_(ar->size-1-line_no));
  }
  else {
    String_Array* ar = ((String_Data*)da)->ar;
    if((ar != NULL) && (ar->size > line_no))
      name = ar->FastEl(ar->size-1-line_no);
  }
  init();
  damage_me(c);
  return true;
}

bool TextData_G::select(const ivEvent& e, Tool& tool, bool unselect) {
  if(line_no < 0) {
    int but = Graphic::GetButton(e);
    if(but == ivEvent::right)
      editb_used = true;
    else
      editb_used = false;
    return Graphic::select(e, tool, unselect);
  }
  return false;
}

bool TextData_G::effect_select(bool set_select) {
  if(line_no >= 0) return false;
  bool temp = Graphic::effect_select(set_select);
  if(set_select && editb_used) {
    daspec->Edit();
  }
  editb_used = false;		// don't do it again!
  return temp;
}


bool TextData_G::grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord) {
  move_pos.Initialize();
  return true;
}

bool TextData_G::manip_move(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;

  if((dx == 0) && (dy == 0))  return true;

  dx = cx - ix;  dy = cy - iy;

  TDCoord ng, dg;
  ng.x = (int) (dx * (float)dtg->max_size.x);
  ng.y = (int) (dy * (float)dtg->max_size.y);

  dg = ng;  dg -= move_pos;

  if((dg.x != 0) || (dg.y != 0)) {
    TDCoord updtpos = daspec->pos + dg;
    if((updtpos.x <0) || (updtpos.y <0)) {
      return true;
    }
    move_pos = ng;
    daspec->pos.Invert();
    Position();
    daspec->pos.Invert();
    PosTDCoord og = daspec->pos;
    daspec->pos += dg;
    Position();			// put into the new one
  }
  return true;
}

bool TextData_G::effect_move(const ivEvent&,Tool&,ivCoord, ivCoord,ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0)) return true;
  dtg->reinit_display = true;
  move_pos.Initialize();
  tabMisc::NotifyEdits(daspec);
  ((DT_GridViewSpec*)daspec->owner)->customized = true;
  taivMisc::SRIAssignment(daspec,daspec->FindMember("pos"));
  return true;
}

//////////////////////////////////
//	   Block_G		//
//////////////////////////////////

Block_G::Block_G(int lno, DA_GridViewSpec* sp, DataTable_G* dg) :
Polygon(dg->defbrush, dg->border, NULL, pts, pts, 4, NULL) {
  line_no = lno;
  daspec = sp;
  dtg = dg;
  ScaleBar* cbar = dtg->owner->cbar;
  ivColor* fl;
  ivColor* txt;
  cbar->GetColor(0.0,&fl,&txt);
  fill(fl);			// so we're solid!
  Build();
}

Block_G::~Block_G() {
}

ivGlyph* Block_G::clone() const {
  return new Block_G(line_no, daspec, dtg); 
}

void Block_G::Build() {
  ivCoord px[4], py[4];
  px[0] = px[3] = dtg->xcscale((float)dtg->owner->block_border_size * .1f);
  py[0] = py[1] = dtg->ycscale((float)dtg->owner->block_border_size * .1f);
  px[1] = px[2] = dtg->xcscale(1.0f - (float)dtg->owner->block_border_size * .1f);
  py[2] = py[3] = dtg->ycscale(1.0f - (float)dtg->owner->block_border_size * .1f);
  SetPoints(px[0],py[0],px[1],py[1],px[2],py[2],px[3],py[3]);
}

void Block_G::Position() {
  translate(dtg->xcscale(daspec->pos.x), dtg->ycscale(daspec->pos.y));
}

void Block_G::SetPoints(ivCoord x0,ivCoord y0, ivCoord x1, ivCoord y1,
			ivCoord x2,ivCoord y2, ivCoord x3, ivCoord y3) {
  _x[0] = x0; _x[1] = x1; _x[2] = x2; _x[3] = x3;
  _y[0] = y0; _y[1] = y1; _y[2] = y2; _y[3] = y3;
  recompute_shape();
}

bool Block_G::update_from_state(ivCanvas* c) {
  if(line_no < 0) return false;
  damage_me(c);
  return true;
}

void Block_G::render_text(ivCanvas* c, ScaleBar* cbar, float val, String& str,
			  FloatTwoDCoord& ll, FloatTwoDCoord& ur)
{
  ivColor* fl;
  ivColor* txt;
  cbar->GetColor(val, &fl, &txt);

  ivFontBoundingBox bbox;
  float nx,ny;
  float ex,ey;
  c->transformer().transform(ll.x, ll.y,nx,ny);
  c->transformer().transform(ur.x, ur.y,ex,ey);
  c->push_transform(); // store the current
  ivTransformer nt = c->transformer();
  nt.invert();			// invert current transformer
  c->transform(nt);		// put inverse in there -- effectively null xform
  int len = str.length();
  nx += 1.0f;
  ny += 2.0f;
  float cx = nx;
  int j;
  for(j=0;j<len;j++){	// draw each character
    dtg->owner->view_font.fnt->char_bbox(str[j], bbox);
    float nxtx = cx + bbox.width();
    if(nxtx > ex) break;
    c->character((const ivFont*)(dtg->owner->view_font.fnt),str[j],8,txt,cx,ny);
    cx = nxtx;
  }

  c->pop_transform(); // remove null transform
}

void Block_G::render_color(ivCanvas* c, ScaleBar* cbar, float val,
				FloatTwoDCoord& ll, FloatTwoDCoord& ur)
{
  ivColor* fl;
  ivColor* txt;
  cbar->GetColor(val, &fl, &txt);
  c->fill_rect(ll.x, ll.y, ur.x, ur.y, fl);
}

void Block_G::render_area(ivCanvas* c, ScaleBar* cbar, float val,
			       FloatTwoDCoord& ll, FloatTwoDCoord& ur)
{
  ivColor* bg_color;
  ivColor* fg_color;
  ivColor* delete_me = NULL;
  bg_color = cbar->bar->scale->GetColor(((int) (.5f * (float)(cbar->bar->scale->chunks-1))));
  if(val > cbar->max)
    fg_color = cbar->bar->scale->maxout.color;
  else if(val < cbar->min)
    fg_color = cbar->bar->scale->minout.color;
  else {
    if(val >= cbar->zero)
      fg_color = cbar->bar->scale->GetColor(cbar->bar->scale->colors.size-1);
    else 
      fg_color = cbar->bar->scale->GetColor(0);

    if(fg_color->alpha() < 0.5f) {// ??
      ivColorIntensity r,g,b;
      cbar->bar->scale->background.color->intensities(r,g,b);
      delete_me = fg_color = new ivColor(r,g,b,1.0-fg_color->alpha());
      ivResource::ref(delete_me);
    }
  }
  //first draw the background
  c->fill_rect(ll.x, ll.y, ur.x, ur.y, bg_color);

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop;
  c->transformer().transform(ll.x, ll.y, cleft,cbottom);
  c->transformer().transform(ur.x, ur.y, cright,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = top-bottom;
  int max_area = width * height;
  float area = ((float) max_area) * cbar->GetAbsPercent(val);
  if(area > max_area) area = max_area;

  ivPixelCoord max_size = (ivPixelCoord) sqrt(fabs((double) max_area));
  ivPixelCoord box_size = (ivPixelCoord) sqrt(fabs((double) area));

  int x_size = (int) ((((float) width)/(float) max_size) * (float) box_size);
  int y_size = (int) (((float) (height)/(float) max_size) * (float) box_size);

  ivPixelCoord xboxoff = (width - x_size) / 2;
  ivPixelCoord yboxoff = (height - y_size) / 2;

  c->push_transform(); // save old xform
  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  c->fill_rect(cleft + c->to_coord(xboxoff),
	       cbottom + c->to_coord(yboxoff),
	       cleft + c->to_coord(xboxoff + x_size),
	       cbottom + c->to_coord(yboxoff + y_size),fg_color);

  if(delete_me != NULL)
    ivResource::unref(delete_me);
  c->pop_transform(); // remove null xform
}

void Block_G::render_linear(ivCanvas* c, ScaleBar* cbar, float val,
				FloatTwoDCoord& ll, FloatTwoDCoord& ur)
{
  ivColor* bg_color;
  ivColor* fg_color;
  ivColor* delete_me = NULL;
  bg_color = cbar->bar->scale->GetColor(((int) (.5f * (float)(cbar->bar->scale->chunks-1))));
  if(val > cbar->max)
    fg_color = cbar->bar->scale->maxout.color;
  else if(val < cbar->min)
    fg_color = cbar->bar->scale->minout.color;
  else {
    if(val >= cbar->zero)
      fg_color = cbar->bar->scale->GetColor(cbar->bar->scale->colors.size-1);
    else 
      fg_color = cbar->bar->scale->GetColor(0);

    if(fg_color->alpha() < 0.5f) {// ??
      ivColorIntensity r,g,b;
      cbar->bar->scale->background.color->intensities(r,g,b);
      delete_me = fg_color = new ivColor(r,g,b,1.0-fg_color->alpha());
      ivResource::ref(delete_me);
    }
  }
  //first draw the background
  c->fill_rect(ll.x, ll.y, ur.x, ur.y, bg_color);

  // then compute the foreground
  ivCoord cleft,cright,cbottom,ctop;
  c->transformer().transform(ll.x, ll.y, cleft,cbottom);
  c->transformer().transform(ur.x, ur.y, cright,ctop);

  ivPixelCoord left = c->to_pixels(cleft);
  ivPixelCoord right = c->to_pixels(cright);
  ivPixelCoord top = c->to_pixels(ctop);
  ivPixelCoord bottom = c->to_pixels(cbottom);
  
  int width = right-left;
  int height = top-bottom;

  int x_size = (int) (((float) width) * cbar->GetAbsPercent(val));
  int y_size = (int) (((float) height) * cbar->GetAbsPercent(val));

  if(y_size > height) y_size = height;
  if(x_size > width) x_size = width;

  ivPixelCoord xboxoff = (width - x_size) / 2;
  ivPixelCoord yboxoff = (height - y_size) / 2;

  c->push_transform(); // save old xform
  ivTransformer tt(c->transformer()); // create null xform
  tt.invert();
  c->transform(tt); // set null xform

  c->fill_rect(cleft + c->to_coord(xboxoff),
	       cbottom + c->to_coord(yboxoff),
	       cleft + c->to_coord(xboxoff + x_size),
	       cbottom + c->to_coord(yboxoff + y_size),fg_color);

  if(delete_me != NULL)
    ivResource::unref(delete_me);
  c->pop_transform(); // remove null xform
}

void Block_G::draw_gs (ivCanvas* c, Graphic* gs) {
  if((_ctrlpts <= 0) || (line_no < 0) || !daspec->visible)
    return;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  if(brush == nil || stroke == nil) return;

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  FloatTwoDCoord ll;		// lower left coords
  FloatTwoDCoord ur;		// upper right
  ScaleBar* cbar = dtg->owner->cbar;

  DataArray_impl* da = daspec->data_array;
  float_RArray* ar = ((float_Data*)da)->ar;
  float val = 0.0f;
  if((ar != NULL) && (ar->size > line_no))
    val = ((float_RArray*)ar)->FastEl(ar->size-1-line_no);
  ll.x = dtg->xcscale((float)dtg->owner->block_border_size * .1f);
  ll.y = dtg->ycscale((float)dtg->owner->block_border_size * .1f);
  ur.x = dtg->xcscale(1.0f - (float)dtg->owner->block_border_size * .1f);
  ur.y = dtg->ycscale(1.0f - (float)dtg->owner->block_border_size * .1f);
  switch(dtg->owner->fill_type) {
  case DT_GridViewSpec::COLOR:
    render_color(c, cbar, val, ll, ur);
    break;
  case DT_GridViewSpec::AREA:
    render_area(c, cbar, val, ll, ur);
    break;
  case DT_GridViewSpec::LINEAR:
    render_linear(c, cbar, val, ll, ur);
    break;
  }

  if(daspec->display_style == DA_GridViewSpec::TEXT_AND_BLOCK) {
    String str = (String)val;
    if(str.contains("e-")) str = "0.0";
    render_text(c, cbar, val, str, ll, ur);
  }
  
  if(dtg->owner->block_border_size > 0) {
    c->new_path();
    c->move_to(_x[0], _y[0]);
    for (int i = 1; i < _ctrlpts; ++i) {
      c->line_to(_x[i], _y[i]);
    }
    if (_closed) {
      c->close_path();
    }
    c->stroke(stroke, brush);
  }

  if (tx != nil) {
    c->pop_transform();
  }
}

bool Block_G::select(const ivEvent& e, Tool& tool, bool unselect){
  if(line_no < 0) {
    int but = Graphic::GetButton(e);
    if(but == ivEvent::right)
      editb_used = true;
    else
      editb_used = false;
    return Graphic::select(e,tool,unselect);
  }
  return false;
}

bool Block_G::effect_select(bool set_select) {
  if(line_no >= 0) return false;
  bool temp = Polygon::effect_select(set_select);
  if(set_select && editb_used) {
    daspec->Edit();
  }
  editb_used = false;		// don't do it again!
  return temp;
}

bool Block_G::grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord) {
  move_pos.Initialize();
  return true;
}

bool Block_G::manip_move(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;

  if((dx == 0) && (dy == 0))  return true;

  dx = cx - ix;  dy = cy - iy;

  TDCoord ng, dg;
  ng.x = (int) (dx * (float)dtg->max_size.x);
  ng.y = (int) (dy * (float)dtg->max_size.y);

  dg = ng;  dg -= move_pos;

  if((dg.x != 0) || (dg.y != 0)) {
    TDCoord updtpos = daspec->pos + dg;
    if((updtpos.x <0) || (updtpos.y <0)) {
      return true;
    }
    move_pos = ng;
    daspec->pos.Invert();
    Position();
    daspec->pos.Invert();
    PosTDCoord og = daspec->pos;
    daspec->pos += dg;
    Position();			// put into the new one
  }
  return true;
}

bool Block_G::effect_move(const ivEvent&,Tool&,ivCoord, ivCoord,ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0)) return true;
  dtg->reinit_display = true;
  move_pos.Initialize();
  tabMisc::NotifyEdits(daspec);
  ((DT_GridViewSpec*)daspec->owner)->customized = true;
  taivMisc::SRIAssignment(daspec,daspec->FindMember("pos"));
  return true;
}

//////////////////////////////////
//	   Grid_G		//
//////////////////////////////////

Grid_G::Grid_G(int lno, DT_GridViewSpec* sp, DataTable_G* dg) :
Block_G(lno, NULL, dg) {
  dtspec = sp;
  Build();
}

Grid_G::~Grid_G() {
}

ivGlyph* Grid_G::clone() const {
  return new Grid_G(line_no, dtspec, dtg); 
}

void Grid_G::Build() {
  ivCoord px[4], py[4];
  px[0] = py[0] = py[1] = px[3] = 0;
  px[1] = px[2] = dtg->xcscale((float)dtspec->geom.x);
  py[2] = py[3] = dtg->ycscale((float)dtspec->geom.y);
  SetPoints(px[0],py[0],px[1],py[1],px[2],py[2],px[3],py[3]);
}

void Grid_G::Position() {
  translate(dtg->xcscale(dtspec->pos.x), dtg->ycscale(dtspec->pos.y));
}

bool Grid_G::update_from_state(ivCanvas* c) {
  if(line_no < 0) return false;
  damage_me(c);
  return true;
}

void Grid_G::draw_gs (ivCanvas* c, Graphic* gs) {
  if(_ctrlpts <= 0)
    return;
  const ivBrush* brush = gs->brush();
  const ivColor* stroke = gs->stroke();
  if(brush == nil || stroke == nil) return;

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }
  FloatTwoDCoord pos;
  if(line_no < 0) {
    for(pos.x=1; pos.x < dtspec->geom.x; pos.x++){
      pos.y = 0;
      c->new_path();
      c->move_to(dtg->xcscale(pos.x), dtg->ycscale(pos.y));
      pos.y = dtspec->geom.y;
      c->line_to(dtg->xcscale(pos.x), dtg->ycscale(pos.y));
      c->stroke(stroke, brush);
    }
    for(pos.y=1; pos.y < dtspec->geom.y; pos.y++){
      pos.x = 0;
      c->new_path();
      c->move_to(dtg->xcscale(pos.x), dtg->ycscale(pos.y));
      pos.x = dtspec->geom.x;
      c->line_to(dtg->xcscale(pos.x), dtg->ycscale(pos.y));
      c->stroke(stroke, brush);
    }
    
    String str = dtspec->display_name;
    ivFontBoundingBox bbox;
    float nx,ny;
    float ex,ey;
    c->transformer().transform(0.0f, 0.0f,nx,ny);
    c->transformer().transform(dtg->xcscale(dtspec->geom.x), dtg->xcscale(dtspec->geom.y),ex,ey);
    c->push_transform(); // store the line*net transform;
    ivTransformer nt = c->transformer();
    nt.invert();			// invert current transformer
    c->transform(nt);		// put inverse in there -- effectively null xform
    int len = str.length();
    nx += 3.0f;    ny += 3.0f;
    float cx = nx;
    int j;
    for(j=0;j<len;j++){	// draw each character
      c->character((const ivFont*)(dtg->owner->view_font.fnt),str[j],8,stroke,cx,ny);
      dtg->owner->view_font.fnt->char_bbox(str[j], bbox);
      cx += bbox.width();
      if(cx > ex) break;
    }
    c->pop_transform(); // remove null transform
  }
  else {
    FloatTwoDCoord ll;		// lower left coords
    FloatTwoDCoord ur;		// upper right
    ScaleBar* cbar = dtg->owner->cbar;
    int vl;
    for(vl=0;vl<dtspec->size;vl++) {
      DA_GridViewSpec* vs = (DA_GridViewSpec*)dtspec->FastEl(vl);
      if(!vs->visible) continue;
      DataArray_impl* da = vs->data_array;
      float val = 0.0f;
      ll.x = dtg->xcscale(vs->pos.x - dtspec->pos.x + (float)dtg->owner->block_border_size * .1f);
      ll.y = dtg->ycscale(vs->pos.y - dtspec->pos.y + (float)dtg->owner->block_border_size * .1f);
      ur.x = dtg->xcscale(vs->pos.x+1.0f - dtspec->pos.x - (float)dtg->owner->block_border_size * .1f);
      ur.y = dtg->ycscale(vs->pos.y+1.0f - dtspec->pos.y - (float)dtg->owner->block_border_size * .1f);
      if(da->InheritsFrom(&TA_float_Data)) {
	float_RArray* ar = ((float_Data*)da)->ar;
	if((ar != NULL) && (ar->size > line_no))
	  val = ((float_RArray*)ar)->FastEl(ar->size-1-line_no);
	if(dtspec->display_style != DA_GridViewSpec::TEXT) {
	  switch(dtg->owner->fill_type) {
	  case DT_GridViewSpec::COLOR:
	    render_color(c, cbar, val, ll, ur);
	    break;
	  case DT_GridViewSpec::AREA:
	    render_area(c, cbar, val, ll, ur);
	    break;
	  case DT_GridViewSpec::LINEAR:
	    render_linear(c, cbar, val, ll, ur);
	    break;
	  }
	  if(dtg->owner->block_border_size > 0) {
	    c->rect(ll.x, ll.y, ur.x, ur.y, stroke, brush);
	  }
	}
	if((dtspec->display_style == DA_GridViewSpec::TEXT) ||
	   (dtspec->display_style == DA_GridViewSpec::TEXT_AND_BLOCK)) {
	  String str = (String)val;
	  if(str.contains("e-")) str = "0.0";
	  render_text(c, cbar, val, str, ll, ur);
	}
      }
      else {
	String_Array* ar = ((String_Data*)da)->ar;
	if((ar != NULL) && (ar->size > line_no)) {
	  String str = ((String_Array*)ar)->FastEl(ar->size-1-line_no);
	  render_text(c, cbar, val, str, ll, ur);
	}
      }
    }
  }
  c->new_path();
  c->move_to(_x[0], _y[0]);
  int i;
  for (i = 1; i < _ctrlpts; ++i) {
    c->line_to(_x[i], _y[i]);
  }
  if (_closed) {
    c->close_path();
  }
  c->stroke(stroke, brush);
  if (tx != nil) {
    c->pop_transform();
  }
}

bool Grid_G::select(const ivEvent& e, Tool& tool, bool unselect){
  if(line_no < 0) {
    int but = Graphic::GetButton(e);
    if(but == ivEvent::right)
      editb_used = true;
    else
      editb_used = false;
    return Graphic::select(e,tool,unselect);
  }
  return false;
}

bool Grid_G::effect_select(bool set_select) {
  if(line_no >= 0) return false;
  bool temp = Polygon::effect_select(set_select);
  if(set_select && editb_used) {
    dtspec->Edit();
  }
  editb_used = false;		// don't do it again!
  return temp;
}

bool Grid_G::grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord) {
  move_pos.Initialize();
  return true;
}

bool Grid_G::manip_move(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;

  if((dx == 0) && (dy == 0))  return true;

  dx = cx - ix;  dy = cy - iy;

  TDCoord ng, dg;
  ng.x = (int) (dx * (float)dtg->max_size.x);
  ng.y = (int) (dy * (float)dtg->max_size.y);

  dg = ng;  dg -= move_pos;

  if((dg.x != 0) || (dg.y != 0)) {
    TDCoord updtpos = dtspec->pos + dg;
    if((updtpos.x <0) || (updtpos.y <0)) {
      return true;
    }
    move_pos = ng;
    dtspec->pos.Invert();
    Position();
    dtspec->pos.Invert();
    PosTDCoord og = dtspec->pos;
    dtspec->pos += dg;
    Position();			// put into the new one
  }
  return true;
}

bool Grid_G::effect_move(const ivEvent&,Tool&,ivCoord, ivCoord,ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0)) return true;
  dtg->reinit_display = true;
  move_pos.Initialize();
  dtspec->UpdateLayout();
  ((DT_GridViewSpec*)dtspec->owner)->customized = true;
  tabMisc::NotifyEdits(dtspec);
  taivMisc::SRIAssignment(dtspec,dtspec->FindMember("pos"));
  return true;
}

bool Grid_G::grasp_stretch(const ivEvent& , Tool& ,ivCoord, ivCoord) {
  move_pos.Initialize();
  return true;
}

bool Grid_G::manip_stretch(const ivEvent& ,Tool& ,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy) {
  ivCoord dx = cx - lx;
  ivCoord dy = cy - ly;
  if((dx == 0) && (dy == 0)) return true;

  dx = (cx - ix);  dy = (cy - iy);

  TDCoord ng, dg;
  ng.x = (int) (dx * (float)dtg->max_size.x);
  ng.y = (int) (dy * (float)dtg->max_size.y);

  dg = ng;  dg -= move_pos;

  if((dg.x != 0) || (dg.y != 0)) {
    TDCoord updtgeom = dtspec->geom + dg;
    if((updtgeom.x <0) || (updtgeom.y <1) ||
       ((updtgeom.x * updtgeom.y) < dtspec->size)) {
      return true;
    }
    move_pos = ng;
    PosTDCoord og = dtspec->geom;
    dtspec->geom += dg;
    Build();
  }
  return true;
}

bool Grid_G::effect_stretch(const ivEvent&, Tool&,ivCoord, ivCoord, ivCoord, ivCoord) {
  if((move_pos.x == 0) && (move_pos.y == 0)) return true;
  dtg->reinit_display = true;
  move_pos.Initialize();
  int gsz = dtspec->geom.x * dtspec->geom.y;
  if(dtspec->size == 0) {
    dtspec->size = gsz;
    taivMisc::SRIAssignment(dtspec,dtspec->FindMember("size"));
  }
  dtspec->UpdateLayout();
  tabMisc::NotifyEdits(dtspec);
  taivMisc::SRIAssignment(dtspec,dtspec->FindMember("geom"));
  return true;
}

//////////////////////////////////
//	  DTViewer		//
//////////////////////////////////

//  static ivCursor* picker_cursor = nil;
static ivCursor* mywindow_cursor = nil;

DTViewer::DTViewer(float w, float h, const ivColor* bg)
: GlyphViewer(w,h,bg) {
};

void DTViewer::press(const ivEvent& ev) {
  // this is apparently never actually called!!
  GlyphViewer::press(ev);
}

void DTViewer::pick(ivCanvas* c, const ivAllocation& a, int depth, ivHit& h) {
  const ivEvent* e = h.event();
  int button = taivM->GetButton(*e);
  if(button == ivEvent::right) {
    cur_tool(Tool::select);	// select on rmb
    //    win->cursor(mywindow_cursor);
  }
  else if(button == ivEvent::middle) {
    cur_tool(Tool::stretch);
    //    win->cursor(mywindow_cursor);
  }
  else if(button == ivEvent::left) {
    cur_tool(Tool::move);
    //    win->cursor(mywindow_cursor);
  }
  GlyphViewer::pick(c, a, depth, h);
}

void DTViewer::release(const ivEvent& ev) {
  if(_zoom || _pan || _grab) {
    //    envv->GetEnvXform();
  }
  GlyphViewer::release(ev);
}

void DTViewer::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext) {
  GlyphViewer::allocate(c,a,ext);
  ((DataTable_G*)_root)->owner->InitDisplay(); // initialize upon resize!
}

#include <ta/enter_iv.h>
declareActionCallback(DTEditor)
implementActionCallback(DTEditor)
#include <ta/leave_iv.h>

//////////////////////////////////
//	   DTEditor		//
//////////////////////////////////

DTEditor::DTEditor(DataTable* d, DT_GridViewSpec* sp, ivWindow* w) {
  viewer = NULL;
  dtg = NULL;

  win = w;

  dt = d;
  dtvsp = sp;

  disp_lines = 0;
  fill_type = DT_GridViewSpec::COLOR;
  block_size = 8;
  block_border_size = 1;
  header_on = true;
  auto_scale = false;
  scale_range.min = -1.0f;
  scale_range.max = 1.0f;

  taBase::Own(scale_range, dt);
  taBase::Own(view_font, dt);
  taBase::Own(view_range, dt);

  body = NULL;

  vtextmenu=NULL;
  dispmdmenu=NULL;

  print_patch = NULL;
  data = NULL;

  scale = NULL;
  cbar = NULL;
}

DTEditor::~DTEditor() {
  ivResource::unref(dtg); 	dtg = NULL;
  ivResource::unref(viewer); 	viewer = NULL;
  ivResource::unref(cbar); 	cbar = NULL;
  ivResource::unref(body); 	body = NULL;
  if (scale != NULL) { taBase::unRefDone(scale); scale = NULL; }

  if(vtextmenu != NULL)     { delete vtextmenu; vtextmenu = NULL; }
  if(dispmdmenu != NULL)    { delete dispmdmenu; dispmdmenu = NULL; }
}

void DTEditor::Init() {
  dtg = new DataTable_G(dt, dtvsp);
  dtg->owner = this;
  ivResource::ref(dtg);

  scale = new ColorScale();
  taBase::Own(scale,dt);

  cbar = new HCScaleBar(0,true,true,new ivBrush(0),scale,16,8,18);
  ivResource::ref(cbar);
  cbar->SetAdjustNotify(new ActionCallback(DTEditor)
			(this,&DTEditor::CBarAdjustNotify));
  cbar->SetMinMax(scale_range.min, scale_range.max);

  viewer = new DTViewer(DTV_INITIAL_X,
			 DTV_INITIAL_Y, scale->Get_Background());
  ivResource::ref(viewer);
  viewer->root(dtg);
  if(dtg->transformer()) {
    ivTransformer* t = dtg->transformer();
    ivTransformer idnty;
    *t = idnty;			// return to identity
    dtg->scale(DTV_INITIAL_X - 2.0f * DTV_MARG,
	       DTV_INITIAL_Y - 2.0f * DTV_MARG);
    dtg->translate(-(.5f * (float)DTV_INITIAL_X),
		    -(.5f * (float)DTV_INITIAL_Y));
  }
  else {
    dtg->scale(DTV_INITIAL_X - 2.0f * DTV_MARG,
		DTV_INITIAL_Y - 2.0f * DTV_MARG,
		0.5,0.5);
  }
}

ivGlyph* DTEditor::GetLook() {
  if(!taMisc::iv_active) return NULL;

  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();

  print_patch = new ivPatch(viewer);

  //////////////////////////////////////
  // now for the event editing functions
  //////////////////////////////////////
    
  body = layout->vbox
    (layout->hcenter(wkit->inset_frame(print_patch),0),
     layout->hcenter(cbar->GetLook(),0));

  ivResource::ref(body);		// gotta ref the body..

  //  if(owner->auto_scale == true)
  cbar->UpdateScaleValues();
  SetMove();			// default is to move!
  InitDisplay();

  return body;
}

void DTEditor::UpdateDisplay(){
  UpdateMinMaxScale();
  viewer->update_from_state();
  if((win->is_mapped()) && (win->bound())) win->repair();
}

void DTEditor::update_from_state() {
  UpdateDisplay();
}

void DTEditor::InitDisplay(){
  UpdateMinMaxScale();
  dtg->Build();
  dtg->transformer(NULL);
  dtg->no_text_extent = true;
  viewer->init_graphic();
  dtg->no_text_extent = false;
  dtg->scale(viewer->viewallocation().x_allotment().span() -  2.0f * DTV_MARG,
	     viewer->viewallocation().y_allotment().span() -  2.0f * DTV_MARG);
  dtg->ReCenter();
  viewer->Update_All();
}

void DTEditor::AddOneLine() {
  UpdateMinMaxScale();
  dtg->AddOneLine();
}

void DTEditor::ScrollLines() {
  UpdateMinMaxScale();
  dtg->ScrollLines();
  UpdateDisplay();
}

void DTEditor::CBarAdjustNotify(){
  auto_scale = false;
  scale_range.min = cbar->min;
  scale_range.max = cbar->max;
  UpdateDisplay();
}

void DTEditor::UpdateMinMaxScale() {
  if(!auto_scale)  return;
  scale_range.max = 1.0f;
  scale_range.min = -1.0f;
  dtvsp->GetMinMaxScale(scale_range);
  if((scale_range.min == 0.0f) && (scale_range.max == 0.0f)) {
    scale_range.min = -1.0f;
    scale_range.max = 1.0f;
  }
  if(cbar != NULL)
    cbar->SetMinMax(scale_range.min, scale_range.max);
}

void DTEditor::SetSelect(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::select);
}
  
void DTEditor::SetMove(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::move);
}
  
void DTEditor::SetReScale(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::scale);
}

void DTEditor::SetReShape(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::stretch);
}

void DTEditor::SetRotate(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::rotate);
}

void DTEditor::SetAlter(){
  // Set select cursor
  if(win) win->cursor(mywindow_cursor);
  if(viewer) viewer->cur_tool(Tool::alter);
}

void DTEditor::EditSelections(){
  taBase_List& selectgroup = dtg->selectgroup;
  if(selectgroup.size > 1){
    selectgroup.Edit();
  }
  else if (selectgroup.size == 1) {
    selectgroup[0]->Edit();
  }
}   

