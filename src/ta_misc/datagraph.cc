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

// glyphs to graph by

#include <ta_misc/datagraph.h>
#include <ta_misc/fontspec.h> // for viewlabels
#include <ta_misc/colorbar.h> // for dynamic background

#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_text.h>
#include <iv_misc/dastepper.h>
#include <iv_misc/grid.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <InterViews/action.h>
#include <InterViews/background.h>
#include <InterViews/box.h>
#include <InterViews/brush.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/label.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/telltale.h>
#include <InterViews/geometry.h>
#include <InterViews/window.h> // for setting zoom_cursor
#include <IV-look/button.h>
#include <IV-look/dialogs.h>
#include <IV-look/kit.h>
#include <ta/leave_iv.h>

#include <iv_misc/requestbox.h>
#include <iv_misc/tbScrollBox.h>
#include <iv_misc/grid.h>
#include <ta_misc/axis.h>

#define GRAPH_TOP_BORDER  5.0
#define GRAPH_BOT_BORDER 10.0
#define GRAPH_LFT_BORDER 10.0
#define GRAPH_RGT_BORDER 10.0

//////////////////////////
// 	GraphLine	//
//////////////////////////
 
GraphLine::GraphLine(GraphEditor* ge, const ivBrush* b, GraphButton* but, ivColor* c,
		     LineType tp, LineStyle st, float lw, PointStyle ps,
		     int mod_off, int mod_m, bool nd, VerticalType vt, SharedYType sy, float thr)
: Graphic(b, c, NULL, NULL, false, false, 256, NULL)
{
  editor = ge;
  button = but;
  axis = but->axis;
  x_axis = but->x_axis;

  line_type = tp;
  line_style = st;
  line_width = lw;
  point_style = ps;
  point_mod.off = mod_off; point_mod.m = mod_m;
  negative_draw = nd;
  vertical = vt;
  shared_y = sy;
  thresh = thr;

  last_coord = 0;
  init_mv_x = 0.0f;
  init_mv_y = 0.0f;
  n_traces = 0;
  n_shared = 0;
  share_i = 0;

  switch(line_style) {
  case  SOLID:
    brush(new ivBrush(BRUSH_SOLID, line_width)); break;
  case  DOT:
    brush(new ivBrush(BRUSH_DOT, line_width)); break;
  case  DASH:
    brush(new ivBrush(BRUSH_DASH, line_width)); break;
  case  DASH_DOT:
    brush(new ivBrush(BRUSH_DASH_DOT, line_width)); break;
  }
  width0_brush = new ivBrush(BRUSH_SOLID, 0.0f);
  ivResource::ref(width0_brush);
}

GraphLine::GraphLine(GraphEditor* ge, const ivBrush* b, GraphButton* but, DA_GraphViewSpec* spec)
: Graphic(b, spec->ta_color.color, NULL, NULL, false, false, 256, NULL)
{
  editor = ge;
  button = but;
  axis = but->axis;
  x_axis = but->x_axis;

  line_type = (LineType)spec->line_type;
  line_style = (LineStyle)spec->line_style;
  line_width = spec->axis_spec->line_width;
  point_style = (PointStyle)spec->point_style;
  point_mod = spec->point_mod;
  negative_draw = spec->negative_draw;
  vertical = (VerticalType)spec->axis_spec->vertical;
  shared_y = (SharedYType)spec->axis_spec->shared_y; // use the axis on this!
  trace_incr = spec->axis_spec->trace_incr; // and this!
  thresh = spec->thresh;

  last_coord = 0;
  init_mv_x = 0.0f;
  init_mv_y = 0.0f;
  n_traces = 0;
  n_shared = 0;
  share_i = 0;

  switch(line_style) {
  case SOLID:
    brush(new ivBrush(BRUSH_SOLID, line_width)); break;
  case  DOT:
    brush(new ivBrush(BRUSH_DOT, line_width)); break;
  case  DASH:
    brush(new ivBrush(BRUSH_DASH, line_width)); break;
  case  DASH_DOT:
    brush(new ivBrush(BRUSH_DASH_DOT, line_width)); break;
  }
  width0_brush = new ivBrush(BRUSH_SOLID, 0.0f);
  ivResource::ref(width0_brush);
}

GraphLine::~GraphLine() {
  data_x.Reset();  data_y.Reset();
  string_vals.Reset();
  coord_points.Reset();
  trace_vals.Reset();
  remove_all();
  ivResource::unref(width0_brush);
}

void GraphLine::Reset() {
  ctrlpts(NULL,NULL,0);	// delete the coord space
  _ctrlpts = 0;
  _buf_size = Graphic::buf_size; // this should be set in Graphic::ctrlpts()
                                 // since _x and _y get reallocated to this size there.
  data_x.Reset();  data_y.Reset();
  coord_points.Reset();
  string_vals.Reset();
  trace_vals.Reset();
  n_traces = 1;
}

const ivColor* GraphLine::GetValueColor(float val) {
  const ivColor* clr = NULL;
  if(axis->range.Range() == 0) {
    clr = editor->scale->GetColor((int)((.5 * (float)(editor->scale->chunks-1)) + .5));
  }
  else if(val > axis->range.max) {
    clr = editor->scale->maxout.color;
  }
  else if(val < axis->range.min) {
    clr = editor->scale->minout.color;
  }
  else {
    int chnk = editor->scale->chunks-1;
    float rval = axis->range.Normalize(val);
    int idx = (int) ((rval * (float)chnk) + .5);
    idx = MAX(0, idx);
    idx = MIN(chnk, idx);
    clr = editor->scale->GetColor(idx);
  }
  return clr;
}

const ivColor* GraphLine::GetTraceColor(int trace) {
  const ivColor* clr = NULL;
  if(n_traces == 0) {
    clr = editor->scale->GetColor((int)((.5 * (float)(editor->scale->chunks-1)) + .5));
  }
  else {
    int chnk = editor->scale->chunks-1;
    float rval = (float)trace / (float)(n_traces - 1);
    int idx = (int) ((rval * (float)chnk) + .5);
    idx = MAX(0, idx);
    idx = MIN(chnk, idx);
    clr = editor->scale->GetColor(idx);
  }
  return clr;
}

void GraphLine::drawclipped_gs(ivCanvas* c, ivCoord l, ivCoord b, ivCoord r, ivCoord t, Graphic* gs)
{
  ivCoord ll, bb, rr, tt;
  getbounds_gs(ll, bb, rr, tt, gs);
  BoxObj thisBox(ll, bb, rr, tt);
  BoxObj clipBox(l, b, r, t);
  if (clipBox.Intersects(thisBox)) {
    draw_gs(c, gs);
  }
}

void GraphLine::AddPoint(ivCoord x, ivCoord y) {
  if(_ctrlpts == 0) {
    add_point(x, y);
    data_x.Add(x); data_y.Add(y);
    n_traces = 1;
    trace_vals.Add(n_traces-1);
    return;
  }
  if(x < data_x[_ctrlpts-1]) {	// new trace
    add_point(x, y);
    data_x.Add(x); data_y.Add(y);
    n_traces++;
    trace_vals.Add(n_traces-1);
  }
  else {
    add_point(x, y);
    data_x.Add(x); data_y.Add(y);
    trace_vals.Add(n_traces-1);
  }
}

bool GraphLine::GetEscEoff(ivCanvas* c) {
  if(c == NULL) return false;
  ivCoord a00,a01,a10,a11,a20,a21;
  transformer()->matrix(a00,a01,a10,a11,a20,a21);
  plwd = MAX(line_width, 1.0f) * c->to_coord(1) / a00;
  plht = MAX(line_width, 1.0f) * c->to_coord(1) / a11;

  FloatTwoDCoord prv_esc = esc;
  FloatTwoDCoord prv_eoff = eoff;

  axis->eff_range = axis->range; // start with default

  eoff.x = 0.0f; eoff.y = 0.0f;
  esc.x = 1.0f; esc.y = 1.0f; 
  if((shared_y == STACK_LINES) && (n_shared > 1)) {
    esc.y = 1.0f / (float)n_shared;
    eoff.y = ((float)share_i / (float)(n_shared)) * axis->range.Range();
    axis->eff_range.max = axis->range.min + (axis->range.Range() * (float)n_shared);
  }
  if(n_traces > 1) {
    if(vertical == STACK_TRACES) {
      esc.y *= 1.0f / (float)n_traces;
      axis->eff_range.max = axis->range.min + (axis->range.Range() * (float)n_traces);
    }
    else {
      if(trace_incr.y > 0) {
	float range = axis->range.Range();
	float rng = esc.y * range;
	float extra_y = (plht * trace_incr.y) * (float)n_traces;
	float remaining = MAX(rng - extra_y, .01 * rng);
	esc.y *= remaining / rng;
	// |-------|----|  * n_shared
	// min    max  emax 
	// remaining extra
	// range / (range + x) = remaining / rng
	// (range + x) / range = rng / remaining
	// range + x = rng * range / remaining
	// x = (rng * range / remaining) - range
	float ax_extra = ((rng * range / remaining) - range);
	axis->eff_range.max = axis->range.max + ax_extra;
	if((shared_y == STACK_LINES) && (n_shared > 1)) {
	  axis->eff_range.max = axis->range.min + (axis->eff_range.Range() * (float)n_shared);
	}
      }
      else if(trace_incr.y < 0) {
	float range = axis->range.Range();
	float rng = esc.y * range;
	float extra_y = (plht * -trace_incr.y) * (float)n_traces;
	float remaining = MAX(rng - extra_y, .01 * rng);
	esc.y *= remaining / rng;
	eoff.y += extra_y;		// start off high and go down..
	float ax_extra = ((rng * range / remaining) - range);
	axis->eff_range.min = axis->range.min - ax_extra;
	if((shared_y == STACK_LINES) && (n_shared > 1)) {
	  // todo: this is wrong!
	  axis->eff_range.max = axis->eff_range.min + (axis->eff_range.Range() * (float)n_shared);
	}
      }
      if(trace_incr.x > 0) {
	float range = x_axis->range.Range();
	float rng = esc.x * range;
	float extra_x = (plwd * trace_incr.x) * (float)n_traces;
	float remaining = MAX(rng - extra_x, .01 * rng);
	esc.x *= remaining / rng;
	float ax_extra = ((rng * range / remaining) - range);
	x_axis->eff_range.max = x_axis->range.max + ax_extra;
      }
      else if(trace_incr.x < 0) {
	float range = x_axis->range.Range();
	float rng = esc.x * range;
	float extra_x = (plwd * -trace_incr.x) * (float)n_traces;
	float remaining = MAX(rng - extra_x, .01 * rng);
	esc.x *= remaining / rng;
	eoff.x += extra_x;		// start off high and go down..
	float ax_extra = ((rng * range / remaining) - range);
	x_axis->eff_range.min = x_axis->range.min - ax_extra;
      }
    }
  }
  if((esc == prv_esc) && (eoff == prv_eoff)) return false;
  return true;
}

void GraphLine::RenderText(ivCanvas* c, ivCoord xv, ivCoord yv, String& str, const ivColor* stroke, bool keep_vis) {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivFontBoundingBox bbox;
  wkit->font()->font_bbox(bbox);
  float fx = bbox.width()/2.0;
  float fy = bbox.ascent() + bbox.descent();

  float nx,ny;
  c->transformer().transform(xv, yv, nx, ny);
  // now put in inverse-transform to nullify current transform and plot in raw coord units for text
  c->push_transform(); ivTransformer nt = c->transformer();
  nt.invert();	c->transform(nt);		
  int len = str.length();
  if(keep_vis) {
    float xpc = x_axis->range.Normalize(xv); nx += fx - (xpc * (len+1) * fx);
    float ypc = axis->range.Normalize(yv); ny += 5 - (ypc * fy);
  }
  else {
    nx += 3.0f;  ny -= 3.0f;
  }
  float cx = nx;
  int j;
  for(j=0;j<len;j++){	// draw each character
    c->character(wkit->font(),str[j],8,stroke,cx,ny);
    wkit->font()->char_bbox(str[j], bbox);
    cx += bbox.width();
  }
  c->damage(nx,ny,ny-fy,nx+(j+2)*fx);
  c->pop_transform(); // remove null transform
}

void GraphLine::RenderPoint(ivCanvas* c, ivCoord xv, ivCoord yv, const ivColor* stroke, const ivBrush* b) {
  switch(point_style){
  case SMALL_DOT:
    c->fill_rect(xv-plwd, yv-plht, xv+plwd, yv+plht, stroke);
    break;
  case BIG_DOT:
    c->fill_rect(xv-plwd*2, yv-plht*2, xv+plwd*2, yv+plht*2, stroke);
    break;
  case PLUS:
    c->new_path();
    c->move_to(xv-plwd*2, yv);
    c->line_to(xv+plwd*2, yv);
    c->stroke(stroke, b);
  case TICK:
    c->new_path();
    c->move_to(xv, yv-plht*2);
    c->line_to(xv, yv+plht*2);
    c->stroke(stroke, b);
    break;
  case NONE: break;
  }
}

void GraphLine::draw_gs(ivCanvas* c, Graphic* gs) {
  if(_ctrlpts <= 0) return;

  const ivBrush* b =  brush();
  const ivColor* stroke = gs->stroke();
  if((b == NULL) || (stroke == NULL)) return;

  ivTransformer* tx = gs->transformer();
  if (tx != nil) {
    c->push_transform();
    c->transform(*tx);
  }

  float zeroval = axis->range.min;

  // first go through and set control points to reflect transformed locations!
  bool no_y_vals = vertical == NO_VERTICAL;
  for(int i=0; i < _ctrlpts; i++) {
    float cox = eoff.x + (float)(trace_vals[i])*plwd*trace_incr.x;
    float coy = eoff.y + (float)(trace_vals[i])*plht*trace_incr.y;
    if(vertical == STACK_TRACES)
      coy += (float)(trace_vals[i]) * esc.y * axis->range.Range();
    float xv = esc.x * (data_x[i] - x_axis->range.min) + x_axis->range.min + cox;
    float y = (no_y_vals ? zeroval : data_y[i]);
    float yv = esc.y * (y - axis->range.min) + axis->range.min + coy;
    _x[i] = xv;
    _y[i] = yv;
  }
  recompute_shape();
  // allow room for coord labels!
  float min_ht = plht * 8.0f / MAX(line_width, 1.0f);
  if((_ymax - _ymin) < min_ht) {	
    _ymax = _ymin + min_ht;
  }
  
  int i_st = 0;			// start index
  int i_ed = _ctrlpts-1;	// end index
  int i_in = 1;			// index increment
  if(trace_incr.y > 0) {
    i_st = _ctrlpts-1;
    i_ed = 0;
    i_in = -1;
  }

  if((n_traces > 1) && (trace_incr.x != 0.0f) || (trace_incr.y != 0.0f)) {
    float x0 = x_axis->range.min + eoff.x; float y0 = axis->range.min + eoff.y;
    float x1 = x_axis->range.min + esc.x * x_axis->range.Range() + eoff.x;
    float y1 = axis->range.min + esc.y * axis->range.Range() + eoff.y;
    float xt = x0 + (float)(n_traces-1)*plwd*trace_incr.x;
    float yt = y0 + (float)(n_traces-1)*plht*trace_incr.y;

    c->new_path();    c->move_to(x0, y0);    c->line_to(x1, y0);
    c->stroke(editor->dtvsp->bg_color.contrastcolor, width0_brush);
    c->new_path();    c->move_to(x0, y0);    c->line_to(x0, y1);
    c->stroke(editor->dtvsp->bg_color.contrastcolor, width0_brush);
    c->new_path();    c->move_to(x0, y0);    c->line_to(xt, yt);
    c->stroke(editor->dtvsp->bg_color.contrastcolor, width0_brush);
  }

  if(line_type == STRINGS) {		// plotting strings!
    int mxidx = MIN(string_vals.size, _ctrlpts);
    for(int i=0; i<mxidx; i++) {
      if((i - point_mod.off) % (point_mod.m) != 0) continue;
      String str = string_vals.FastEl(i);
      if(str.empty()) continue; // don't plot empty strings!
      RenderText(c, _x[i], _y[i], str, stroke);
    }
  }
  else {			// plotting lines!
    if((line_type == LINE) || (line_type == LINE_AND_POINTS)) {
      if(_ctrlpts == 1) {
	c->new_path();
	c->move_to(_x[0], _y[0]);
	c->line_to(_x[0] + plwd, _y[0]);
	c->stroke(stroke, b);
      }
      else {
	c->new_path();
	c->move_to(_x[i_st], _y[i_st]);
	for(int i = i_st+i_in; ((i_in > 0) ? (i <= i_ed) : (i >= i_ed)); i += i_in) {
	  if(!negative_draw &&
	     ((i_in > 0) ? (trace_vals[i] > trace_vals[i-1]) : (trace_vals[i+1] > trace_vals[i]))) {
	    c->stroke(stroke,b);
	    c->new_path();
	    c->move_to(_x[i], _y[i]);
	  }
	  else {
	    c->line_to(_x[i], _y[i]);
	  }
	}
	c->stroke(stroke, b);
      }
    }
    else if(line_type == TRACE_COLORS) {
      if(_ctrlpts == 1) {
	c->new_path();
	c->move_to(_x[0], _y[0]);
	c->line_to(_x[0] + plwd, _y[0]);
	c->stroke(GetTraceColor(trace_vals[0]), b);
      }
      else {
	c->new_path();
	c->move_to(_x[i_st], _y[i_st]);
	for(int i = i_st+i_in; ((i_in > 0) ? (i <= i_ed) : (i >= i_ed)); i += i_in) {
	  if((i_in > 0) ? (trace_vals[i] > trace_vals[i-1]) : (trace_vals[i+1] > trace_vals[i])) {
	    c->stroke(GetTraceColor(trace_vals[i-i_in]),b);
	    c->new_path();
	    c->move_to(_x[i], _y[i]);
	  }
	  else {
	    c->line_to(_x[i], _y[i]);
	  }
	}
	c->stroke(GetTraceColor(trace_vals[i_ed]), b);
      }
    }
    else if(line_type == VALUE_COLORS) {
      if(_ctrlpts == 1) {
	c->new_path();
	c->move_to(_x[0], _y[0]);
	c->line_to(_x[0] + plwd, _y[0]);
	c->stroke(GetValueColor(data_y[0]), b);
      }
      else {
	c->new_path();
	c->move_to(_x[i_st], _y[i_st]);
	for(int i = i_st+i_in; ((i_in > 0) ? (i <= i_ed) : (i >= i_ed)); i += i_in) {
	  if(!negative_draw && 
	     ((i_in > 0) ? (trace_vals[i] > trace_vals[i-1]) : (trace_vals[i+1] > trace_vals[i]))) {
	    c->new_path();
	    c->move_to(_x[i], _y[i]);
	  }
	  else {
	    c->line_to(.5 * (_x[i-i_in] + _x[i]), .5 * (_y[i-i_in] + _y[i]));
	    c->stroke(GetValueColor(data_y[i-i_in]), b);
	    c->new_path();
	    c->move_to(.5 * (_x[i-i_in] + _x[i]), .5 * (_y[i-i_in] + _y[i]));
	    c->line_to(_x[i], _y[i]);
	    c->stroke(GetValueColor(data_y[i]), b);
	    c->move_to(_x[i], _y[i]);
	  }
	}
      }
    }
    else if(line_type == THRESH_POINTS) {
      for(int i = 0; i < _ctrlpts; i++) {
	if(data_y[i] < thresh) continue;
	float xv = _x[i]; float yv = _y[i];
	c->fill_rect(xv - (.5 * plwd), yv - (.5 * plht), xv + (.5 * plwd), yv + (.5 * plht), stroke);
      }
    }
    if((line_type == POINTS) || (line_type == LINE_AND_POINTS)) {
      c->new_path();
      c->move_to(_x[0], _y[0]);
      for(int i = 0; i < _ctrlpts; i++) {
	if((i - point_mod.off) % (point_mod.m) != 0) continue;
	RenderPoint(c, _x[i], _y[i], stroke, gs->brush());
      }
    }
    if(coord_points.size > 0) {
      stroke = editor->dtvsp->bg_color.contrastcolor;
      for(int i=0;i<coord_points.size;i++) {
	int idx = coord_points.FastEl(i);
	float xv = _x[idx]; float yv = _y[idx];
	// draw little box around point
	c->rect(xv-plwd, yv-plht, xv+plwd, yv+plht, stroke, gs->brush());
	String num = String(data_x[idx], "(%g , ") + String(data_y[idx], "%g)");
	RenderText(c, xv, yv, num, stroke, true);
      }
    }
  }
  if(tx != nil) {
    c->pop_transform();
  }
}

bool GraphLine::DrawLastPt(ivCanvas* can) {
  if((can == NULL) || (_t == NULL)) return false;
  if(_ctrlpts < 2) { 
    damage_me(can);
    return true;
  }
  float zeroval = axis->range.min;
  bool no_y_vals = vertical == NO_VERTICAL;

  ivCoord x = data_x[_ctrlpts - 1]; // current
  ivCoord y = no_y_vals ? zeroval : data_y[_ctrlpts - 1];
  ivCoord px = data_x[_ctrlpts - 2]; // previous
  //  ivCoord py = no_y_vals ? zeroval : data_y[_ctrlpts - 2];

  if((x < px) &&
     ((vertical == STACK_TRACES) || (trace_incr.x != 0.0f) || (trace_incr.y != 0.0f))) {
    damage_me(can);		// just update the whole thing!
    return true;
  }
  float cox = eoff.x + (float)(trace_vals[_ctrlpts-1])*plwd*trace_incr.x;
  float coy = eoff.y + (float)(trace_vals[_ctrlpts-1])*plht*trace_incr.y;
  if(vertical == STACK_TRACES)
    coy += (float)(trace_vals[_ctrlpts-1]) * esc.y * axis->range.Range();

  float xv = esc.x * (x - x_axis->range.min) + x_axis->range.min + cox;
  float yv = esc.y * (y - axis->range.min) + axis->range.min + coy;
  _x[_ctrlpts-1] = xv; _y[_ctrlpts-1] = yv;

  _xmin = osMath::min(_xmin, _x[_ctrlpts-1]);  _xmax = osMath::max(_xmax, _x[_ctrlpts-1]);
  _ymin = osMath::min(_ymin, _y[_ctrlpts-1]);  _ymax = osMath::max(_ymax, _y[_ctrlpts-1]);

  float pxv = _x[_ctrlpts-2]; float pyv = _y[_ctrlpts-2];

  Graphic gs;
  total_gs(gs);
  corners(pxv,pyv,xv,yv,*(gs.transformer()));
  float lw = MAX(line_width, 1.0f);
  can->damage(pxv-lw,pyv-lw,xv+lw,yv+lw);
  return false;
}

bool GraphLine::grasp_move(const ivEvent&, Tool&, ivCoord ix, ivCoord iy) {
  float ax = 0;  float ay = 0;
  transformer()->inverse_transform(ix,iy,ax,ay);
  init_mv_x = ax;  init_mv_y = ay;
  ax = x_axis->range.Normalize(ax);  ay = axis->range.Normalize(ay);
  // operate on normalized coordinates!
  // button is stored from the grasp since the move event does not contain the propper button
  float xv = x_axis->range.Normalize(_x[0]);
  float yv = axis->range.Normalize(_y[0]);

  double d = sqrt((double) (((xv-ax) * (xv-ax)) + ((yv-ay) * (yv-ay))));
  int target =0;
  for (int i = 1; i < _ctrlpts; i++) {
    xv = x_axis->range.Normalize(_x[i]);  yv = axis->range.Normalize(_y[i]);
    double dist = sqrt((double) (((xv-ax) * (xv-ax)) + ((yv-ay) * (yv-ay))));
    if(dist < d) {
      d = dist; target = i;
    }
  }
  last_coord = target;
  coord_points.Add(last_coord);
  return true;
}

bool GraphLine::manip_move(const ivEvent& , Tool&, ivCoord ix, ivCoord iy,
			      ivCoord, ivCoord, ivCoord cx, ivCoord cy) {
  float nx, ny;
  transformer()->inverse_transform(cx,cy,nx,ny);
  float ax, ay;
  transformer()->inverse_transform(ix,iy,ax,ay);

  // compensate for moving of display in case that happened
  nx -= ax - init_mv_x;
  ny -= ay - init_mv_y;

  // scan outward from the last point
  float xv = _x[last_coord];
  float d = fabs(xv-nx);
  int target = last_coord;

  int mx = MAX(_ctrlpts - last_coord, last_coord);
  for (int i = 0; i < mx; i++) {
    if((last_coord + i) < _ctrlpts) {
      xv = _x[last_coord+i];
      if(fabs(xv-nx) < d) {
	d = fabs(xv-nx);
	target = last_coord + i;
      }
    }
    if((last_coord - i) >= 0) {
      xv = _x[last_coord-i];
      if(fabs(xv-nx) < d) {
	d = fabs(xv-nx);
	target = last_coord - i;
      }
    }
  }
  if(target < 0) target = 0;
  if(target >= _ctrlpts) target = _ctrlpts-1;

  last_coord = target;
  coord_points.Peek() = target;
  return true;
}

bool GraphLine::effect_move(const ivEvent&, Tool& ,ivCoord , ivCoord ,
				 ivCoord  , ivCoord ) {
  coord_points.Pop();
  coord_points.AddUnique(last_coord);
  return true;
}

bool GraphLine::effect_border() {
  return true;
}

void GraphLine::Rescale() {
  // this sets a transform on the line that makes it just fit within the display
  // based on axis min/max values
  
  ivCoord l, b, r, t;
  l= x_axis->range.min; r= x_axis->range.max;
  b= axis->range.min;  	t= axis->range.max;

  if(_t) corners(l,b,r,t, *_t);

  // Get the (the graph_g's) size, so we know how to adjust ourselves.

  ivDisplay* disp = ivSession::instance()->default_display();
  ivCoord one_pixel = disp->to_coord(1);

  ivCoord extra_space = 2.0f * MAX(line_width, 1.0f) * one_pixel;

  ivAllocation* a = button->graph->graphg->GetAllocation();
  ivAllotment ax = a->allotment(Dimension_X);
  ivAllotment ay = a->allotment(Dimension_Y);
  ivCoord width = (ax.end() - ax.begin()) - extra_space;
  ivCoord height = (ay.end() - ay.begin()) - extra_space;

  ivCoord rngwidth = r-l;
  ivCoord rngheight = t-b;
  // numerical overflow results if things get smaller than this!
  if(rngwidth < 1.0e-6) {
    rngwidth = 1.0e-6;
  }
  if(rngheight < 1.0e-6) {
    rngheight = 1.0e-6;
  }

  ivCoord hscale = width/rngwidth;	// mulitpliers to fit range inside of width
  ivCoord vscale = height/rngheight;

  // rvscale is the real vertical scale we would multiply by the
  // Identity matrix to scale correctly.
  ivCoord rvscale = height/(axis->range.Range());
  ivCoord rhscale = width/(x_axis->range.Range());
    
  float b00,b01,b10,b11,b20,b21;
  if(_t == NULL){
    transformer(new ivTransformer);
  }
  else { 
    _t->matrix(b00,b01,b10,b11,b20,b21);
    _t->translate(-b20,-b21);	// un-translate current xform (scale w/out translation)
  }

  // now adjust the scale
  _t->scale(hscale,vscale);
    
  // and translate the correct amount
  _t->translate(-(x_axis->range.MidPoint()* rhscale), -(axis->range.MidPoint() * rvscale));
}

//////////////////////////
// 	Grapher_G	//
//////////////////////////
 
Grapher_G::Grapher_G(GraphEditor* ge, GraphGraph* gg) {
  editor = ge;
  graph = gg;
  brush(new ivBrush((ivCoord)0.0));
  ivTransformer* tx = new ivTransformer;
  transformer(tx);
  ivResource::unref(tx);
}

Grapher_G::~Grapher_G() {
  remove_all();
}

void Grapher_G::AddLine(GraphLine* gl) {
  gl->translate(-(((_a.x_allotment().span()) -10.0f)/2.0f),0);
  append_(gl);
}

void Grapher_G::Reset() {
  remove_all();
  no_text_extent = true;
  viewer()->root(this);
  no_text_extent = false;
  background(editor->dtvsp->bg_color.color);
  if(editor->boxpatch != NULL) {
    DynamicBackground* bg = (DynamicBackground*) editor->boxpatch->body();
    bg->SetColor(editor->dtvsp->bg_color.color);
  }
}

ivColor* Grapher_G::GetTextColor() {
  return editor->dtvsp->bg_color.contrastcolor;
}

ivColor* GraphGetTextColor(void* obj) {
  return ((Grapher_G*)obj)->GetTextColor();
}


//////////////////////////
// 	GraphViewer	//
//////////////////////////

GraphViewer::GraphViewer(GraphEditor* e, GraphGraph* gg, float w, float h, const ivColor* bg)
  : GlyphViewer(w,h,bg)
{
  editor = e;
  graph = gg;
  init_xform = NULL;
}

GraphViewer::~GraphViewer() {
  if (init_xform) {
    ivResource::unref(init_xform);
    init_xform = NULL;
  }
}

void GraphViewer::root(GraphicMaster* g) {
  GlyphViewer::root(g);
  if(init_xform != NULL) { ivResource::unref(init_xform); init_xform = NULL; }
  ivTransformer* rootr = _root->transformer();
  if(rootr != NULL)
    init_xform = new ivTransformer(*rootr);
  else
    init_xform = new ivTransformer();
  ReScaleAxes();
}

void GraphViewer::allocate(ivCanvas* c, const ivAllocation& a, ivExtension& e){
  GlyphViewer::allocate(c,a,e);
  editor->UpdateCanvases(c);
  editor->UpdateDisplay();
  if(init_xform != NULL) { ivResource::unref(init_xform); init_xform = NULL; }
  ivTransformer* rootr = _root->transformer();
  if(rootr != NULL)
    init_xform = new ivTransformer(*rootr);
  else
    init_xform = new ivTransformer();
}

void GraphViewer::SetCanvas(ivCanvas* c) {
  _canvas = c;
}

void GraphViewer::ClearCachedAllocation() {
  ivAllocation a;
  _a = a;
}

void GraphViewer::ReScaleAxes(bool redraw) {
  if(init_xform==NULL) return;
  if(_root->transformer() == NULL) return;

  if(graph->x_axis != NULL) {
    Axis* a = graph->x_axis;
    float vmax = 0;    float vmin = 0;
    float nvmin =0;    float nvmax =0;
    float bogus_y=0; // use vmax,vmin as temps at first;
    init_xform->inverse_transform(_a.left(),0,vmin,bogus_y);
    init_xform->inverse_transform(_a.right(),0,vmax,bogus_y);
    _root->transformer()->transform(vmin,0,nvmin,bogus_y);
    _root->transformer()->transform(vmax,0,nvmax,bogus_y);
    vmin = _a.left(); vmax = _a.right();
    float vrange = vmax - vmin;
    float nvrange = (nvmax - nvmin);
    a->min_percent = (nvmin - vmin)/(MAX(vrange,nvrange));
    a->max_percent = (nvmax - vmax)/(MAX(vrange,nvrange));
    a->range_scale = (vrange / nvrange);
    a->UpdateAxis();
    a->reallocate();
    if(redraw) a->redraw();
  }

  // scaling the axes
  for(int i=0;i<graph->y_axis.size;i++){
    Axis* a = (Axis*)graph->y_axis[i];
    float vmax = 0;    float vmin = 0;
    float nvmin =0;    float nvmax =0;
    float bogus_x=0;
    init_xform->inverse_transform(0,_a.bottom(),bogus_x,vmin);
    init_xform->inverse_transform(0,_a.top(),bogus_x,vmax);
    _root->transformer()->transform(0,vmin,bogus_x,nvmin);
    _root->transformer()->transform(0,vmax,bogus_x,nvmax);
    vmin = _a.bottom(); vmax = _a.top();
    float vrange = vmax - vmin;
    float nvrange = (nvmax - nvmin);
    a->min_percent = (nvmin - vmin)/(MAX(vrange,nvrange));
    a->max_percent = (nvmax - vmax)/(MAX(vrange,nvrange));
    a->range_scale = (vrange / nvrange);
    a->UpdateAxis();
    a->reallocate();
    if(redraw) a->redraw();
  }
}

void GraphViewer::rate_zoom() {
  GlyphViewer::rate_zoom();
  ReScaleAxes();
}

void GraphViewer::grab_scroll() {
  GlyphViewer::grab_scroll();
  ReScaleAxes(false);
}

//////////////////////////
// 	GraphButton	//
//////////////////////////
 
GraphButton::GraphButton(ivGlyph* g, ivStyle* s, ivTelltaleState* t, GraphEditor* o,
			 DA_GraphViewSpec* gvs, int f)
: ivButton(g,s,t,NULL)
{
  editor = o;
  field = f;
  spec = gvs;
  is_string = false;

  InitData();

  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();

  state()->set(ivTelltaleState::is_choosable | 
	       ivTelltaleState::is_toggle |
	       ivTelltaleState::is_enabled | 
	       ivTelltaleState::is_chosen, true);

  ivGlyph* lgnd = NULL;
  DataArray_impl* da = spec->data_array;
  if((da != NULL) && da->InheritsFrom(TA_String_Data)) {
    is_string = true;
    lgnd = layout->hfixed(layout->hflexible(wkit->label("-S-")),16);
  }
  else {
    lgnd = GetLegend();
  }
  
  // reparent the glyph in the surrounding goo
  body(layout->hbox(layout->margin(lgnd,2), 
                    taivM->big_button(g)));
  SetAxis();
}
 
GraphButton::~GraphButton() {
  InitData();
  spec = NULL;
  editor = NULL;
  field = -1;
}

void GraphButton::InitData() {
  graph = NULL;
  axis_but = NULL;
  x_axis = NULL;
  axis = NULL;
  line = NULL;
  n_shared = 0;
  n_shared_tot = 0;
  share_i = 0;
  is_shared_axis = false;
  press_button = 0;
}

void GraphButton::SetAxis() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivGlyph* axis_rep;
  if(editor->plot_rows) {
    DT_GraphViewSpec* spo = (DT_GraphViewSpec*)spec->GetOwner();
    if(spo->plot_rows) {
      int idx = spo->Find(spec);
      if(idx == 0) {
	axis_rep = layout->margin(GetLegend(spec->axis_spec),2);
	state()->set(ivTelltaleState::is_chosen, true);
	state()->set(ivTelltaleState::is_active, false);
      }
      else {
	axis_rep = layout->hfixed(layout->hflexible(wkit->label("-R-")),18);
	state()->set(ivTelltaleState::is_chosen, true);
	state()->set(ivTelltaleState::is_active, false);
      }
    }
    else {
      axis_rep = layout->margin(GetLegend(spec->axis_spec),2);
      state()->set(ivTelltaleState::is_chosen, false);
      state()->set(ivTelltaleState::is_active, false);
    }
  }
  else {
    if(editor->x_axis_index == field) { // x-axis
      axis_rep = layout->hfixed(layout->hflexible(wkit->label("-X-")),18);
      state()->set(ivTelltaleState::is_chosen, false);
      state()->set(ivTelltaleState::is_active, true);
      spec->visible = false;
    }
    else {
      axis_rep = layout->margin(GetLegend(spec->axis_spec),2);
    }
  }

  if(body()->count() == 3)
    body()->replace(2,axis_rep);
  else
    body()->append(axis_rep); // first time only
}

// legend is 16 coords wide
ivGlyph* GraphButton::GetLegend(DA_GraphViewSpec* vspec) {
  static const ivCoord thick = 2.0;
  ivLayoutKit* layout = ivLayoutKit::instance();
  if(vspec==NULL) vspec = spec;
  ivGlyph* line = NULL;
  ivGlyph* points = NULL;
  if(vspec->line_type != DA_GraphViewSpec::POINTS) {
    switch(vspec->line_style){
    case DA_GraphViewSpec::SOLID:
      line = layout->vmargin
	(layout->fixed(new ivBackground(NULL,vspec->ta_color.color),16,thick),2);
      break;
    case DA_GraphViewSpec::DOT:
      line = layout->hbox
	(layout->hfixed(layout->hmargin(NULL,1),1),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),1,thick),1,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),1,thick),1,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),1,thick),1,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),1,thick),1,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),1,thick),1,2));
      break;
    case DA_GraphViewSpec::DASH:
      line = layout->hbox
	(layout->margin
	  (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),4,thick),2,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),4,thick),2,2));
      break;
    case DA_GraphViewSpec::DASH_DOT:
      line = layout->hbox
	(layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),4,thick),2,2),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),2,thick),3,2));
      break;
    }
  }
  
  if((vspec->line_type == DA_GraphViewSpec::POINTS) ||
     (vspec->line_type == DA_GraphViewSpec::LINE_AND_POINTS)) {
    switch(vspec->point_style){
    case DA_GraphViewSpec::SMALL_DOT:
      points = layout->hbox
	(layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),2,thick+1),1,1),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),2,thick+1),1,1),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),2,thick+1),1,1),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),2,thick+1),1,1));
      break;
    case DA_GraphViewSpec::BIG_DOT:
      points = layout->hbox
	(layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),3,thick+2),
	  2,3,1,1),
	 layout->margin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),3,thick+2),
	  2,3,1,1));
      break;
    case DA_GraphViewSpec::TICK:
      points = layout->hbox
	(layout->hmargin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),thick,6),3),
	 layout->hmargin
	 (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),thick,6),3));
      break;
    case DA_GraphViewSpec::PLUS:
      points = layout->overlay
	(layout->hbox
	 (layout->hmargin
	  (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),thick,6),3),
	  layout->hmargin
	  (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),thick,6),3)),
	 layout->hbox
	 (layout->margin
	  (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),6,thick),1,1),
	  layout->margin
	  (layout->fixed(new ivBackground(NULL,vspec->ta_color.color),6,thick),1,2)));
      break;
    case DA_GraphViewSpec::NONE: break;
    }
  }
  return layout->overlay(line, points);
}

GraphButton* GraphButton::GetButton(GraphEditor* o, DA_GraphViewSpec* gvs, int f){
  ivWidgetKit* wkit = ivWidgetKit::instance();

  ivTelltaleState* ts = new ivTelltaleState;
  ivGlyph* look = wkit->palette_button_look(wkit->label(gvs->display_name), ts);
  return new GraphButton(look, wkit->style(), ts, o, gvs, f);
};


bool GraphButton::IsChosen() {
  return state()->test(ivTelltaleState::is_chosen);
}

void GraphButton::press(const ivEvent& e) {
  press_button = taivM->GetButton(e);
  ivButton::press(e);
}

void GraphButton::release(const ivEvent& e) {
  if(press_button == ivEvent::middle) {
    SetXAxis();
    tabMisc::NotifyEdits(spec);
    tabMisc::NotifyEdits(editor->owner); // the graphlogview
    return;
  }
  if(press_button == ivEvent::right) {
    osboolean chosen = state()->test(ivTelltaleState::is_chosen);
    ivButton::release(e);  
    if(editor->x_axis_index == field) {
      state()->set(ivTelltaleState::is_active, true);
    }
    state()->set(ivTelltaleState::is_chosen, chosen);
    spec->Edit();
    return;
  }
  if(editor->x_axis_index != field) { // don't do x-axis
    ivButton::release(e); 
    ToggleLine();
    tabMisc::NotifyEdits(spec);
  }
}

void GraphButton::SetXAxis() {
  if(editor==NULL) return;
  if(editor->plot_rows) {
    DT_GraphViewSpec* speco = GET_OWNER(spec, DT_GraphViewSpec);
    if(speco != NULL) {
      speco->plot_rows = !speco->plot_rows;
    }
    editor->last_row_cnt = -2;		// cause a replot..
    tabMisc::DelayedUpdateAfterEdit(editor->owner);
  }
  else {
    if(field == editor->x_axis_index) {
      // already the X axis -- try to do Rows!
      if(spec->owner != editor->dtvsp) { // only if a subgroup
	DT_GraphViewSpec* speco = GET_OWNER(spec, DT_GraphViewSpec);
	if(speco != NULL) {
	  speco->plot_rows = !speco->plot_rows;
	  if(field == editor->x_axis_index) {
	    spec->visible = true;	// reset this
	    editor->x_axis_index = 0;
	  }
	}
	editor->last_row_cnt = -2;		// cause a replot..
	tabMisc::DelayedUpdateAfterEdit(editor->owner);
      }
    }
    else {
      editor->SetXAxis(field);
    }
  }
}
  
void GraphButton::ToggleLine() {
  if(editor==NULL) return;
  editor->ToggleLine(field);
}

float_Data* GraphButton::GetYData(String_Data*& str_ar) {
  float_Data* y_ar = NULL;
  str_ar = NULL;
  if((spec->data_array != NULL) && spec->data_array->InheritsFrom(TA_float_Data))
    y_ar = (float_Data*)spec->data_array;
  else {
    if(spec->string_coords != NULL) {
      str_ar = (String_Data*)spec->data_array;
      y_ar = (float_Data*)spec->string_coords->data_array;
    }
  }
  return y_ar;
}


//////////////////////////
// 	GrapherCallback	//
//////////////////////////

typedef void (GlyphViewer::*ToolSetter) (unsigned int);

class GrapherCallback : public ivAction {
public:
  GrapherCallback(GraphEditor*, ToolSetter, unsigned int);
  virtual void execute();
private:
  GraphEditor* _ge;
  ToolSetter _tool;
  unsigned _type;
};

GrapherCallback::GrapherCallback(GraphEditor* ge, ToolSetter tool, unsigned int t) {
  _ge = ge;
  _tool = tool;
  _type = t;
}

void GrapherCallback::execute() {
  GraphEditor* ge = _ge;
  ToolSetter tool = _tool;
  unsigned int type = _type;
  
  for(int i=0;i<ge->graphs.size;i++) {
    GraphGraph* gg = (GraphGraph*)ge->graphs[i];
    (gg->viewer->*tool)(type);
  }
}

//////////////////////////
// 	GraphGraph	//
//////////////////////////

GraphGraph::GraphGraph(GraphEditor* ge) : ivPatch(taivM->hsep) {
  editor = ge;
  dtvsp = editor->dtvsp;		// the display spec for the data

  viewer = NULL;
  graphg = NULL;

  x_axis = NULL;
  x_axis_patch = NULL;
  nodisp_x_axis = false;

  y_axis_box = NULL;
  y_axis_patch = NULL;

  linespace = NULL;
}

void GraphGraph::Init() {
  ivLayoutKit* layout = ivLayoutKit::instance();

  graphg = new Grapher_G(editor, this);
  ivResource::ref(graphg);

  float width = 400.0f / editor->eff_layout.x;
  float height = 200.0f / editor->eff_layout.y;

  viewer = new GraphViewer(editor,this, width, height, dtvsp->bg_color.color);
  ivResource::ref(viewer);
  viewer->root(graphg);

  linespace = new ivPatch(viewer);
  ivResource::ref(linespace);

  x_axis = new XAxis(0.0f, 0.0f, dtvsp->bg_color.contrastcolor);
  x_axis_patch = new ivPatch(x_axis);
  ivResource::ref(x_axis_patch);

  y_axis_box = layout->hbox();
  y_axis_patch = new ivPatch(y_axis_box);
  ivResource::ref(y_axis_patch);
  viewer->cur_tool(Tool::move);
}

void GraphGraph::Reset() {
  graphg->Reset();
  y_axis.Reset();
  while(y_axis_box->count() > 0) {
    y_axis_box->remove(y_axis_box->count()-1);
  }
  y_axis_patch->reallocate();
}

GraphGraph::~GraphGraph() {
  ivResource::unref(graphg);	graphg = NULL;
  ivResource::unref(viewer);	viewer = NULL;
  ivResource::unref(linespace);	linespace = NULL;

  ivResource::unref(x_axis_patch); x_axis_patch = NULL;

  ivResource::unref(y_axis_patch); y_axis_patch = NULL;
  y_axis.Reset();
}

void GraphGraph::SetXAxis(GraphButton* xbut) {
  if(xbut == NULL) return;
  editor->SetAxisFromSpec(x_axis, xbut);
}

void GraphGraph::AddYAxis(GraphButton* but) {
  ivLayoutKit* layout = ivLayoutKit::instance();
  y_axis.Add(but->axis);
  // the 35  vvv is to create space for the text numbers
  ivGlyph* axrep = layout->margin(but->axis,35,0,0,0);
  y_axis_box->append(axrep);
}

void GraphGraph::GetLook() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  
  // margin = lrbt

  ivGlyph* whole_box;
  if(y_axis_box->count() > 0) {
    ivGlyph* ybox = 
      layout->vbox
      (layout->margin(y_axis_patch,  5.0,  0.0,
		      GRAPH_BOT_BORDER, GRAPH_TOP_BORDER),
       layout->shape_of_xy
       (layout->margin(y_axis_patch, 5.0,  0.0,
		       GRAPH_BOT_BORDER, GRAPH_TOP_BORDER),
	layout->margin(x_axis_patch, GRAPH_LFT_BORDER, GRAPH_RGT_BORDER,
		       20.0, 0.0)));

    ivPolyGlyph* line_xax_box = 
      layout->vbox
      (layout->hcenter(layout->margin(layout->flexible(linespace),
				      GRAPH_LFT_BORDER,GRAPH_RGT_BORDER,
				      GRAPH_BOT_BORDER,GRAPH_TOP_BORDER),0));

    if(!nodisp_x_axis)
      line_xax_box->append
	(layout->hcenter(layout->margin(x_axis_patch,
					GRAPH_LFT_BORDER,GRAPH_RGT_BORDER,
					20.0,0.0),0));

    whole_box = layout->hbox(ybox, line_xax_box);
  }
  else {
    whole_box = layout->hbox(taivM->hsep);
  }

  float width = 400.0f / editor->eff_layout.x;
  float height = 200.0f / editor->eff_layout.y;

  ivGlyph* body_hbox = 
    layout->natural(whole_box, width, height);

  body(body_hbox);
  ivResource::flush();
}

//////////////////////////
// 	GraphEditor	//
//////////////////////////

GraphEditor::GraphEditor(TAPtr own, DataTable* d, DT_GraphViewSpec* dtvs, ivWindow* w) {
  owner = own;
  data = d;
  dtvsp = dtvs;
  win = w;

  view_bufsz = 10000;
  view_range.max = 10000;
  last_pt_offset = 0;

  scale = NULL;
  cbar = NULL;
  use_cbar = false;
  cbar_deck = NULL;
  separate_graphs = false;
  graph_layout.x = 1;
  graph_layout.y = 10;
  n_graphs = 0;

  first_graph = NULL;
  graph_grid = NULL;
  graph_patch = NULL;
  boxpatch = NULL;

  x_axis_index = 0;
  plot_rows = false;

  x_axis_ar = NULL;

  tool_gp = NULL;

  fieldbox = NULL;
  fieldframe = NULL;
  fieldpatch = NULL;
  field_rep = NULL;

  last_col_cnt = 0;
  last_row_cnt = -2;

  SetOwnerXAxis = NULL;
}
 
GraphEditor::~GraphEditor() {
  buttons.Reset();
  ivResource::unref(first_graph);	first_graph = NULL;
  ivResource::unref(graph_grid);	graph_grid = NULL;
  ivResource::unref(graph_patch);	graph_patch = NULL;
  ivResource::unref(boxpatch);		boxpatch = NULL;
  ivResource::unref(field_rep);		field_rep = NULL;
  ivResource::unref(fieldpatch);	fieldpatch = NULL;
  fieldbox = NULL;  fieldframe = NULL;
  if (scale != NULL) { taBase::unRefDone(scale); scale = NULL; }
  ivResource::unref(cbar); 	cbar = NULL;
  rows_x_axis.Reset();
}

void GraphEditor::Init() {
  field_rep = new tbScrollBox();
  ((tbScrollBox*) field_rep)->naturalnum = 5;
  ivResource::ref(field_rep);

  scale = new ColorScale();
  taBase::Own(scale,owner);

  cbar = new HCBar(new ivBrush(0),scale,16,8,18);
  ivResource::ref(cbar);
}

void GraphEditor::UpdateCanvases(ivCanvas* c) {
  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->viewer->SetCanvas(c);
  }
}  

ivButton* GraphEditor::MEButton(ivTelltaleGroup* gp, char* txt, ivAction* a) {
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivTelltaleState* t = new Teller((TelltaleFlags)0, (ivAction*)NULL);
  ivButton* rval = new NoUnSelectButton
    (wkit->palette_button_look(wkit->label(txt), t), wkit->style(),t,a);
  rval->state()->set(ivTelltaleState::is_choosable | ivTelltaleState::is_toggle |
	ivTelltaleState::is_enabled, true);
  rval->state()->join(gp);
  return rval;
}

#include <ta/enter_iv.h>
declareActionCallback(GraphEditor)
implementActionCallback(GraphEditor)
#include <ta/leave_iv.h>

ivGlyph* GraphEditor::GetTools() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();

  tool_gp = new ivTelltaleGroup;

  ivButton* msbutton = 
    MEButton(tool_gp,"Mv/Scan",
	     new GrapherCallback(this, &GlyphViewer::cur_tool,Tool::move));
  
  ((NoUnSelectButton *) msbutton)->chooseme();

  return layout->vbox
    (layout->hbox
     (layout->hnatural
       (MEButton(tool_gp,"Select",
		 new GrapherCallback(this,
				     &GlyphViewer::cur_tool,Tool::select)), 60),
      layout->hglue(),
      layout->hnatural(msbutton,60),
      layout->hglue(),
      layout->hnatural
       (MEButton(tool_gp,"ReScale",
		 new GrapherCallback(this,
				     &GlyphViewer::cur_tool,Tool::scale)), 60)),
     taivM->vfsep,
     wkit->menu_item_separator_look(),
     layout->hbox(layout->hglue(),wkit->fancy_label("Graph Labels"),layout->hglue()),
     taivM->vfsep,
     layout->hbox
     (layout->hflexible
      (layout->hnatural
       (wkit->push_button("New", new ActionCallback(GraphEditor)
			  (this, &GraphEditor::Create_Default_Label)),50)),
      layout->hglue(),
      layout->hflexible
      (layout->hnatural
       (wkit->push_button("Delete", new ActionCallback(GraphEditor)
			  (this, &GraphEditor::Delete_Labels)),50)),
      layout->hglue(),
      layout->hflexible
      (layout->hnatural
       (wkit->push_button("Edit", new ActionCallback(GraphEditor)
			  (this, &GraphEditor::Edit_Labels)),50))));
}

ivGlyph* GraphEditor::GetFields(ivScrollBox* sb) {
  ivGlyphIndex c;
  for(c=sb->count()-1; c>= 0; c--) // clear out any existing
    sb->remove(c);
  buttons.Reset();
  ivResource::flush();
  if(data->leaves <= 0)
    return (ivGlyph*)sb;

  int cnt;
  taLeafItr vi;
  DA_GraphViewSpec* spec;
  for(cnt = 0, spec=(DA_GraphViewSpec*)dtvsp->FirstEl(vi); (spec != NULL);
      spec=(DA_GraphViewSpec*)dtvsp->NextEl(vi), cnt++)
    {
    DataArray_impl* da = spec->data_array;
    GraphButton* but = GraphButton::GetButton(this, spec, cnt);
    buttons.Add(but);
    if(da == NULL) 
      continue;
    sb->append(but);
    if(!spec->visible) {
      ((GraphButton*)buttons[cnt])->state()->set(ivTelltaleState::is_chosen, false);
    }
  }
  return (ivGlyph *) sb;
}

void GraphEditor::SetAxisFromSpec(Axis* ax, GraphButton* but) {
  String_Data* str_ar = NULL;
  float_Data* y_ar = but->GetYData(str_ar);

  ax->n_ticks = but->spec->n_ticks;
  ax->fixed_range = but->spec->range;
  if((y_ar != NULL) && (y_ar->ar != NULL))
    ax->InitRange(y_ar->ar->range.min, y_ar->ar->range.max);
  else
    ax->InitRange(0.0f, 0.0f);
}

void GraphEditor::GetGraphs() {
  last_col_cnt = data->leaves;

  ivResource::unref(graph_grid);
  graphs.Reset();
  ivResource::flush();

  if(buttons.size < data->leaves) return;

  // check for plot-rows format, and set all plot-rows to share first axis in group
  plot_rows = false;
  plot_rows_subgps.Reset();
  for(int g=0;g<dtvsp->gp.size;g++) {
    DT_GraphViewSpec* gpvs = (DT_GraphViewSpec*)dtvsp->FastGp(g);
    if(gpvs->plot_rows) {
      plot_rows_subgps.Add(g);
      plot_rows = true;
      if(gpvs->size > 0) {
	DA_GraphViewSpec* st_axis = (DA_GraphViewSpec*)gpvs->FastEl(0);
	for(int i=0;i<gpvs->size;i++) {
	  DA_GraphViewSpec* davs = (DA_GraphViewSpec*)gpvs->FastEl(i);
	  davs->SetAxis(st_axis);
	  davs->visible = true;	// all sub-guys must be visible!
	}
      }
    }
  }

  // get a good X axis
  GraphButton* xbut = NULL;
  if(buttons.size > 0) {
    if((x_axis_index < 0) || (x_axis_index >= buttons.size)) {
      x_axis_index = 0;
      if(SetOwnerXAxis != NULL) SetOwnerXAxis(owner,x_axis_index);
    }
    xbut = (GraphButton*)buttons[x_axis_index];
    x_axis_ar = (float_Data*)xbut->spec->data_array;
    int ctr = 0;
    while((x_axis_ar != NULL) &&
	  (x_axis_ar->InheritsFrom(TA_String_Data) || (x_axis_ar->ar == NULL))
	  && (ctr < buttons.size)) {
      x_axis_index++;  x_axis_index = x_axis_index % buttons.size;
      if(SetOwnerXAxis != NULL) SetOwnerXAxis(owner,x_axis_index);
      xbut = (GraphButton*)buttons[x_axis_index];
      x_axis_ar = (float_Data*)xbut->spec->data_array;
      ctr++;
    }
    xbut->spec->visible = false;
    xbut->spec->SetAxis(xbut->spec);
  }

  // reset everything and make sure everyone has an axis spec
  use_cbar = false;
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    but->InitData();
    if(but->spec->axis_spec == NULL)
      but->spec->SetAxis(but->spec);
    if((but->spec->line_type == DA_GraphViewSpec::VALUE_COLORS) || 
       (but->spec->line_type == DA_GraphViewSpec::TRACE_COLORS))
      use_cbar = true;
  }
  // make sure x_axis is not a shared axis (move to adjacent guy if so)
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(but == xbut) continue;
    if(but->spec->axis_spec == xbut->spec) {
      int ai = x_axis_index;
      if(ai < buttons.size-1) ai++;
      else ai--;
      GraphButton* new_ax = (GraphButton*)buttons[ai];
      but->spec->SetAxis(new_ax->spec);
    }
  }
  // make sure axis_specs use themselves as axes, otherwise nothing works
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(but->spec->axis_spec->axis_spec != but->spec->axis_spec)
      but->spec->axis_spec->SetAxis(but->spec->axis_spec);
  }
  // set axis_but's
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(but->spec->axis_spec == but->spec) {
      but->axis_but = but;
    }
    else {
      int ai = dtvsp->FindLeaf(but->spec->axis_spec);
      but->axis_but = (GraphButton*)buttons[ai];
    }
  }
  // now get all the axes straight in terms of shared/non-shared, n_shared, and visible stuff
  // (shared axis that is not visible still needs to create an axis!)
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    but->axis_but->n_shared_tot++;
    if(!but->spec->visible) continue;
    but->share_i = but->axis_but->n_shared++;
  }
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    but->n_shared = but->axis_but->n_shared;
    if((but->axis_but == but) && (but->n_shared_tot > 1) && (but->n_shared > 0)) {
      but->is_shared_axis = true;
    }
  }

  // now actually get the graphs
  if(separate_graphs) {
    n_graphs = 0;
    for(int i=0;i<buttons.size;i++) {
      GraphButton* but = (GraphButton*)buttons[i];
      if(!but->spec->visible && !but->is_shared_axis) continue;
      if(but->spec->axis_spec == but->spec) {
	GraphGraph* gg;
	if(n_graphs == 0) {
	  gg = first_graph;
	  gg->Reset();
	}
	else {
	  gg = new GraphGraph(this);
	  gg->Init();
	}
	graphs.Add(gg);
	but->graph = gg;
	n_graphs++;
      }
    }
    // propagate graphs for separate graphs, layout the graphs in a grid
    for(int i=0;i<buttons.size;i++) {
      GraphButton* but = (GraphButton*)buttons[i];
      if(!but->spec->visible && !but->is_shared_axis) continue;
      but->graph = but->axis_but->graph;
    }
    if(n_graphs <= 0) n_graphs = 1;
    eff_layout = graph_layout;
    eff_layout.FitN(n_graphs);
    while(eff_layout.x * eff_layout.y > n_graphs) {
      if(eff_layout.x >= eff_layout.y)
	eff_layout.x--;
      else
	eff_layout.y--;
      eff_layout.x = MAX(eff_layout.x, 1);
      eff_layout.y = MAX(eff_layout.y, 1);
    }
    if(eff_layout.x * eff_layout.y < n_graphs) {
      if(eff_layout.x >= eff_layout.y)
	eff_layout.x++;
      else
	eff_layout.y++;
    }
    eff_layout.FitN(n_graphs);	// double check!
    graph_grid = new Grid(eff_layout.y, eff_layout.x);
    int xc = 0; int yc = eff_layout.y-1;
    for(int i=0;i<graphs.size;i++) {
      GraphGraph* gg = (GraphGraph*)graphs[i];
      gg->SetXAxis(xbut);
//       if(yc > 0) {  // everyone gets an x axis -- for rescaling, etc
// 	gg->nodisp_x_axis = true;
//       }
      xc++;
      if(xc >= eff_layout.x) {
	xc = 0;	yc--;
      }
    }
  }
  else {
    eff_layout.x = 1; eff_layout.y = 1;
    GraphGraph* gg = first_graph;
    graphs.Add(gg);
    gg->Reset();
    gg->SetXAxis(xbut);
    n_graphs = 1;
    for(int i=0;i<buttons.size;i++) {
      GraphButton* but = (GraphButton*)buttons[i];
      if(!but->spec->visible && !but->is_shared_axis) continue;
      but->graph = gg;
    }
    graph_grid = new Grid(1, 1);
  }
  ivResource::ref(graph_grid);

  // now get the y axes
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(!but->spec->visible && !but->is_shared_axis) continue;
    if((but == xbut) || (but->axis_but != but)) continue;
    DataArray_impl* da = but->spec->data_array;
    if(da == NULL) continue;
    float amin = 0.0f; float amax = 0.0f;
    if(!da->InheritsFrom(TA_String_Data) && (((float_Data *)da)->ar != NULL)) {
      float_Data* fd = (float_Data*)da;
      amin = fd->ar->range.min; amax = fd->ar->range.max;
    }
    YAxis* yax = new YAxis(amin,amax,but->spec->ta_color.color);
    but->axis = yax;
    but->x_axis = but->graph->x_axis;

    SetAxisFromSpec(yax, but);
    but->graph->AddYAxis(but);
  }

  // now share the y axes with all columns
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(!but->spec->visible && !but->is_shared_axis) continue;
    if((but == xbut) || (but->axis_but == but)) continue;
    but->axis = but->axis_but->axis;
    but->x_axis = but->graph->x_axis;
  }

  // do this part at end after graphs are rendered
  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->GetLook();
    graph_grid->replace(i, gg);
  }

  graph_patch->body(graph_grid); // this should finally get rid of previous graph grid

  if(use_cbar)
    cbar_deck->flip_to(0);
  else
    cbar_deck->flip_to(1);
}

ivGlyph* GraphEditor::GetLook() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  
  GetFields(field_rep);

  fieldframe = wkit->inset_frame
    (new ivBackground
      (layout->hbox
       (wkit->vscroll_bar(field_rep),
	taivM->hfsep,
	layout->hfixed(field_rep, taivM->big_button_width + ((16 + 2 + 2)* 2 ))),
       dtvsp->bg_color.color));

  fieldbox = layout->vbox(GetTools(),
			  taivM->vfsep,
			  wkit->menu_item_separator_look(),
			  taivM->vfsep,
			  fieldframe);
  
  fieldpatch = new ivPatch(fieldbox);
  ivResource::ref(fieldpatch);

  graph_patch = new ivPatch(taivM->hsep);
  ivResource::ref(graph_patch);

  cbar_deck = new ivDeck(2);
  cbar_deck->append(layout->hcenter(cbar, 0));
  cbar_deck->append(taivM->hsep);

  eff_layout.x = 1; eff_layout.y = 1;
  first_graph = new GraphGraph(this);
  ivResource::ref(first_graph);
  first_graph->Init();

  GetGraphs();			// sets graph_patch

  UpdateButtons();		// get buttons updated after graphs!

  boxpatch = new ivPatch(new DynamicBackground 
			 (layout->vbox(graph_patch, cbar_deck),
			  dtvsp->bg_color.color));
  ivResource::ref(boxpatch);

  const ivColor* bg_color = owner->GetEditColorInherit();
  if(bg_color == NULL) bg_color = wkit->background();

  return 
    (wkit->inset_frame
     (layout->hbox
      (wkit->outset_frame
       (new ivBackground(fieldpatch, bg_color)),
       taivM->hfsep,
       boxpatch)));
}

///////////////////////////////////////////////////////////////
// 		Drawing the Lines
///////////////////////////////////////////////////////////////

void GraphEditor::Reset() {
  last_row_cnt = -2;
  fieldpatch->redraw();
  GetFields(field_rep);
  fieldbox->change(0);
  fieldframe->change(0);
  fieldpatch->reallocate();
  GetGraphs();
  //  graph_patch->redraw();
}

void GraphEditor::UpdateDisplay() {
  if(last_col_cnt != data->leaves)
    Reset();

  // check to see if everything is hunkey dory
  bool ok = true;
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if(!but->spec->visible && !but->is_shared_axis) continue;
    if((but->graph == NULL) || (but->axis_but->axis == NULL)) {
      ok = false;
      break;
    }
  }
  if(!ok || (buttons.size == 0)) {
    Reset();
  }

  if(buttons.size == 0) {
    return;
  }

  GraphButton* xbut = (GraphButton*)buttons[x_axis_index];
  x_axis_ar = (float_Data*)xbut->spec->data_array;
  if((x_axis_ar == NULL) || (x_axis_ar->ar == NULL))
    return;

  if(plot_rows)
    PlotRows();
  else
    PlotCols();
  fieldpatch->reallocate();
  fieldpatch->redraw();
}

void GraphEditor::InitDisplay() {
  Reset();
  UpdateDisplay();
  UpdateButtons();
}

void GraphEditor::PlotCols() {
  if(x_axis_ar == NULL) return;

  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->graphg->remove_all();
    gg->x_axis->UpdateRange(x_axis_ar->ar->range.min, x_axis_ar->ar->range.max);
  }

  int dt_mx = data->MaxLength();
  int max_n = MIN(dt_mx-1, view_range.min + view_bufsz);
  int st = MIN(max_n, view_range.min);
  st = MAX(0, st);
    
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if((but->axis == NULL) || !but->spec->visible) continue;
    if(i == x_axis_index) continue;
    GraphLine* gl = new GraphLine(this, but->graph->graphg->brush(), but, but->spec);
    but->line = gl;
    gl->n_shared = but->n_shared;
    gl->share_i = but->share_i;

    String_Data* str_ar = NULL;
    float_Data* y_ar = but->GetYData(str_ar);
    if(y_ar == NULL) continue;

    if(y_ar->ar->size == 1)
      but->axis->InitRange(y_ar->ar->range.min, y_ar->ar->range.max);
    but->axis->UpdateRange(y_ar->ar->range.min, y_ar->ar->range.max);

    int xa_i = x_axis_ar->ar->size - (dt_mx - st);
    int ar_i = y_ar->ar->size - (dt_mx - st);
    if(max_n < st)
      gl->AddPoint(0,0);		// todo: if nothing, blank
    else {
      for(int j=st; j <= max_n; j++, xa_i++, ar_i++) {
	ivCoord x = 0.0f;
	if((xa_i >= 0) && (xa_i < x_axis_ar->ar->size)) x = x_axis_ar->ar->FastEl(xa_i);
	ivCoord y = 0.0f;
	if((ar_i >= 0) && (ar_i < y_ar->ar->size)) y = y_ar->ar->FastEl(ar_i);
	gl->AddPoint(x,y);
	if(str_ar != NULL)
	  gl->string_vals.Add(str_ar->ar->FastEl(ar_i));
      }
    }
    but->graph->graphg->AddLine(gl);
  }
  last_row_cnt = x_axis_ar->ar->size;
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if((but->axis == NULL) || !but->spec->visible) continue;
    but->line->Rescale();
    ivCanvas* can = but->graph->viewer->canvas();
    if(can != NULL) {
      but->line->GetEscEoff(can);
      but->line->damage_me(can);
      but->axis->UpdateAxis();
      but->axis->reallocate();
    }
  }
  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->x_axis->UpdateAxis();    gg->x_axis->reallocate();
  }

  UpdateLabels();
  boxpatch->redraw();
}

void GraphEditor::PlotRows() {
  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->graphg->remove_all();
    gg->x_axis->InitRange(0, 0);
  }

  // outer loop is the rows!
  int dt_mx = data->MaxLength();
  int max_n = MIN(dt_mx-1, view_range.min + view_bufsz);
  int st = MIN(max_n, view_range.min);
  st = MAX(0, st);

  int g;
  for(g=0;g<plot_rows_subgps.size;g++) {
    DT_GraphViewSpec* gpvs = (DT_GraphViewSpec*)dtvsp->FastGp(plot_rows_subgps[g]); 
    if(gpvs->size <= 0) continue;
    DA_GraphViewSpec* spec = (DA_GraphViewSpec*)gpvs->FastEl(0);
    int st_idx = dtvsp->FindLeaf(spec);
    GraphButton* st_but = (GraphButton*)buttons[st_idx];
    st_but->line = NULL;
    if((st_but->axis == NULL)) continue;
    GraphLine* gl = new GraphLine(this, st_but->graph->graphg->brush(), st_but, spec);
    st_but->line = gl;

    st_but->graph->x_axis->UpdateRange(0, gpvs->size-1);
    for(int j=st; j <= max_n; j++) {
      for(int i=0;i<gpvs->size;i++) {
	GraphButton* but = (GraphButton*)buttons.SafeEl(st_idx + i);
	if((but == NULL) || (but->spec == NULL))  continue;
	if(!but->spec->visible) continue;

	String_Data* str_ar = NULL;
	float_Data* y_ar = but->GetYData(str_ar);
	if(y_ar == NULL) continue;

	if((j == st) && (i == 0))
	  st_but->axis->InitRange(y_ar->ar->range.min, y_ar->ar->range.max);
	st_but->axis->UpdateRange(y_ar->ar->range.min, y_ar->ar->range.max);

	int ar_i = y_ar->ar->size - (dt_mx - j);
	ivCoord x = (float)i;
	ivCoord y = 0.0f;
	if((ar_i >= 0) && (ar_i < y_ar->ar->size)) y = y_ar->ar->FastEl(ar_i);
	gl->AddPoint(x,y);
	if(str_ar != NULL)
	  gl->string_vals.Add(str_ar->ar->FastEl(ar_i));
      }
    }
    st_but->graph->graphg->AddLine(gl);
    st_but->line->Rescale();
    ivCanvas* can = st_but->graph->viewer->canvas();
    if(can != NULL) {
      gl->damage_me(can);
      st_but->line->GetEscEoff(can);
    }
    st_but->axis->UpdateAxis();
    st_but->axis->reallocate();
  }
  last_row_cnt = x_axis_ar->ar->size;

  for(int i=0;i<n_graphs;i++) {
    GraphGraph* gg = (GraphGraph*)graphs[i];
    gg->x_axis->UpdateAxis();    gg->x_axis->reallocate();
  }

  UpdateLabels();
  boxpatch->redraw();
}

void GraphEditor::AddLastPoint() {
  if(plot_rows) {
    if((x_axis_ar->ar->size != last_row_cnt + 1) || (x_axis_ar->ar->size < 3)) {
      PlotRows();
    }
    else {
      AddLastPoint_Rows();
    }
  }
  else {
    if(x_axis_ar == NULL) return;
    if((x_axis_ar->ar->size != last_row_cnt + 1) || (x_axis_ar->ar->size < 3)) {
      PlotCols();
    }
    else {
      AddLastPoint_Cols();
    }
  }
}

void GraphEditor::AddLastPoint_Cols() {
  if(x_axis_ar == NULL) return;

  bool xrescale = false; 
  bool fixyaxis = false;
  taPtrList_impl rescale_list;

  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if((but->axis == NULL) || !but->spec->visible) continue;
    if(i == x_axis_index) continue;

    if(but->x_axis->UpdateRange(x_axis_ar->ar->range.min, x_axis_ar->ar->range.max))
      xrescale = true;

    ivCanvas* can = but->graph->viewer->canvas();
    if(can == NULL) continue;

    String_Data* str_ar = NULL;
    float_Data* y_ar = but->GetYData(str_ar);
    if(y_ar == NULL) continue;

    ivCoord x = x_axis_ar->ar->RevEl(last_pt_offset);
    ivCoord y = y_ar->ar->RevEl(last_pt_offset);

    if(y_ar->ar->size == 1) // initialize range for first data point
      but->axis->InitRange(y_ar->ar->range.min, y_ar->ar->range.max);
    but->line->AddPoint(x,y);
    if(str_ar != NULL)
      but->line->string_vals.Add(str_ar->ar->RevEl(last_pt_offset));
    if(but->axis->UpdateRange(y_ar->ar->range.min, y_ar->ar->range.max)) {
      fixyaxis = true;
      if(!xrescale)		// not necessary unless x is not being rescaled
	rescale_list.AddUnique_((void*)but->spec->axis_spec);
    }
    else if(but->line->DrawLastPt(can)) { // only add last point if not new range
      fixyaxis = true;
      if(!xrescale)
	rescale_list.AddUnique_((void*)but->spec->axis_spec);
    }
  }
  for(int i=0;i<buttons.size;i++) {
    GraphButton* but = (GraphButton*)buttons[i];
    if((but->axis == NULL) || !but->spec->visible) continue;
    ivCanvas* can = but->graph->viewer->canvas();
    if(can == NULL) continue;
    // if my spec was on the list, then rescale me
    if(xrescale || (rescale_list.Find_((void*)but->spec->axis_spec) >= 0) || but->line->GetEscEoff(can)) {
      fixyaxis = true;
      but->line->Rescale();
      but->line->GetEscEoff(can);
      but->axis->UpdateAxis();
      but->axis->reallocate();
    }
  }
  last_row_cnt = x_axis_ar->ar->size;

  rescale_list.RemoveAll();
  if(fixyaxis || xrescale) {
    for(int i=0;i<n_graphs;i++) {
      GraphGraph* gg = (GraphGraph*)graphs[i];
      gg->linespace->redraw();
      gg->x_axis->UpdateAxis();    gg->x_axis->reallocate();
      if(gg->x_axis_patch != NULL)
	gg->x_axis_patch->redraw();
      if(fixyaxis) 
	gg->y_axis_patch->redraw();
    }
  }
}

void GraphEditor::AddLastPoint_Rows() {
  bool fixyaxis = false;
  bool xrescale = false;
  int g;
  for(g=0;g<plot_rows_subgps.size;g++) {
    DT_GraphViewSpec* gpvs = (DT_GraphViewSpec*)dtvsp->FastGp(plot_rows_subgps[g]); 
    if(gpvs->size <= 0) continue;

    DA_GraphViewSpec* spec = (DA_GraphViewSpec*)gpvs->FastEl(0);
    int st_idx = dtvsp->FindLeaf(spec);
    GraphButton* st_but = (GraphButton*)buttons.SafeEl(st_idx);
    if((st_but == NULL) || (st_but->axis == NULL) || (st_but->line == NULL))  return;
    ivCanvas* can = st_but->graph->viewer->canvas();
    if(can == NULL) continue;
    if(st_but->x_axis->UpdateRange(0, gpvs->size-1)) {
      xrescale = true;
    }

    bool my_fixyaxis = false;
    for(int i=0;i<gpvs->size;i++) {
      GraphButton* but = (GraphButton*)buttons.SafeEl(st_idx + i);
      if((but == NULL) || (but->spec == NULL))  continue;
      if(!but->spec->visible) continue;

      String_Data* str_ar = NULL;
      float_Data* y_ar = but->GetYData(str_ar);
      if(y_ar == NULL) continue;

      if(st_but->axis->UpdateRange(y_ar->ar->range.min, y_ar->ar->range.max)) {
	my_fixyaxis = true;
      }

      ivCoord x = (float)i;
      ivCoord y = y_ar->ar->RevEl(last_pt_offset);
      st_but->line->AddPoint(x,y);
      if(str_ar != NULL)
	st_but->line->string_vals.Add(str_ar->ar->RevEl(last_pt_offset));
    }
    if(st_but->line->GetEscEoff(can))
      my_fixyaxis = true;
    st_but->line->damage_me(can);
    if(my_fixyaxis) {
      fixyaxis = true;
      st_but->line->Rescale();
      st_but->line->GetEscEoff(can); // do again justin case rescale did something..
      st_but->axis->UpdateAxis();
      st_but->axis->reallocate();
    }
  }

  last_row_cnt = x_axis_ar->ar->size;

  if(xrescale || fixyaxis) {
    for(int i=0;i<n_graphs;i++) {
      GraphGraph* gg = (GraphGraph*)graphs[i];
      gg->linespace->redraw();
      gg->x_axis->UpdateAxis();    gg->x_axis->reallocate();
      if(gg->x_axis_patch != NULL)
	gg->x_axis_patch->redraw();
      if(fixyaxis) 
	gg->y_axis_patch->redraw();
    }
  }
}


///////////////////////////////////////////////////////////////
// 		Changing the Display
///////////////////////////////////////////////////////////////

void GraphEditor::UpdateButtons() {	
  int i;
  for(i=0;i<buttons.size;i++) {
    GraphButton* button = (GraphButton *)buttons[i];
    button->SetAxis();
  }
  fieldpatch->reallocate();
  fieldpatch->redraw();
}

void GraphEditor::SetXAxis(int idx) {
  x_axis_index = idx;
  if(SetOwnerXAxis != NULL) SetOwnerXAxis(owner, x_axis_index);
  last_row_cnt = -2;		// cause a replot..
  GetGraphs(); 			// this recreates all the axes
  UpdateDisplay();
  UpdateButtons();
}

void GraphEditor::ToggleLine(int idx) {
  if(idx >= data->leaves)
    return;
  DA_ViewSpec* spec = dtvsp->Leaf(idx);
  spec->visible = !(spec->visible);
  GetGraphs();
  UpdateDisplay();
}

void GraphEditor::InitColors() {
  dtvsp->UpdateLineFeatures();
}


///////////////////////////////////////////////////////////////
// 		Labels
///////////////////////////////////////////////////////////////

void GraphEditor::UpdateLabels() {
  ViewLabel_List* vlg = NULL;
  owner->FindMembeR(&TA_ViewLabel_List,((void *&) vlg));
  if(vlg ==  NULL) return;
  int v;
  for(v=0; v<vlg->size; v++) {
    ViewLabel* vl = vlg->FastEl(v);
    // reset non saved function pointers
    vl->get_color = GraphGetTextColor;
    vl->MakeText();
  }
}

ViewLabel* GraphEditor::Create_Label(char* label_name) {
  if(owner == NULL) return NULL;
  ViewLabel_List* vlg = NULL;
  owner->FindMembeR(&TA_ViewLabel_List,((void *&) vlg));
  if (vlg ==  NULL) return NULL;
  ViewLabel *vl = (ViewLabel *) vlg->New(1);
  vl->get_color = GraphGetTextColor;
  if(label_name != NULL) {
    vl->name = label_name;
  }
  else{
    vl->name = String("GraphView Label") + String(vlg->size);
  }
  vl->MakeText();
  InitDisplay();
  taivMisc::RecordScript(vl->GetPath() +".New(1);\n");
  taivMisc::RecordScript(vl->GetPath() + String(".SafeEl(") + String(vlg->size-1)
			  + String(").name = \"") + vl->name +
			  String("\";\n"));
  return vl;
}

void GraphEditor::Delete_Labels() {
  if((owner == NULL) || (first_graph == NULL)) return;
  taBase_List* labelgroup = &(first_graph->graphg->selectgroup);
  taBase_List* viewlabelgroup;
  owner->FindMembeR(&TA_ViewLabel_List,((void *&) viewlabelgroup));
  if(labelgroup->size == 0){
    if(taMisc::Choice("Delete All Labels?","Ok","Cancel") == 0)
      labelgroup = viewlabelgroup;
    else labelgroup = NULL;
  }
  if(labelgroup == NULL) return;
  TAPtr o;
  int i;
  for(i=labelgroup->size-1;i>=0;i--){
    o = labelgroup->FastEl(i);
    if(viewlabelgroup != NULL) {
      String recordstring = viewlabelgroup->GetPath() + ".RemoveEl("
	+ o->GetPath() + ");";
      DMEM_GUI_RUN_IF {
	viewlabelgroup->Remove(o);
      }
      taivMisc::RecordScript(recordstring);
    }
    if(labelgroup != viewlabelgroup)     labelgroup->Remove(i); // select group
  }
}

void GraphEditor::Edit_Labels() {
  if((owner == NULL) || (first_graph == NULL)) return;
  Grapher_G* graphg = first_graph->graphg;
  if(graphg->selectgroup.size > 1){
    taivEdit* gc = (taivEdit*)graphg->selectgroup.GetTypeDef()->ive;
    if(gc != NULL) 
      gc->Edit( (void*) &(graphg->selectgroup),win);
  }
  else if (graphg->selectgroup.size == 1){
    taivEdit* gc = (taivEdit*)graphg->selectgroup[0]->GetTypeDef()->ive;
    if(gc != NULL) 
      gc->Edit( (void*)(graphg->selectgroup[0]),win);
  }
}

