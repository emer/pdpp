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

#ifndef colorbar_h
#define colorbar_h

// for bar
#include <ta_misc/colorscale.h>
#include <ta/taiv_data.h>	// for ivGlyph_List

#include <ta/enter_iv.h>
#include <OS/enter-scope.h>
#include <InterViews/input.h> 
#include <InterViews/action.h>
#include <OS/list.h>
#include <iv_misc/spiral.h>
#include <InterViews/patch.h> // for CachePatch
#include <InterViews/cursor.h> // for painter cursor
#include <InterViews/label.h> // for color_label
#include <IV-look/button.h> // for ScriptButton
#include <ta/leave_iv.h>

class ivBrush;			// #IGNORE
class ivPolyGlyph;		// #IGNORE
class ivTelltaleGroup;		// #IGNORE
class ivEvent;			// #IGNORE
class ivFieldEditor;		// #IGNORE
class ivPatch;			// #IGNORE	
class DAStepper;		// #IGNORE

class Bar : public ivGlyph { // Basic properties of a bar
public:
  int 		blocks;
  int 		height;
  int 		width;
  ColorScale* 	scale;

  Bar(ColorScale* c, int b=16, int h=16, int w=16);
  virtual void request(ivRequisition&) const {};
  virtual void allocate(ivCanvas*, const ivAllocation&, ivExtension&);
  virtual void draw(ivCanvas*, const ivAllocation&) const { };
  virtual void	SetColorScale(ColorScale* c);
  virtual ivGlyph* GetBar(){return NULL;}
  virtual ~Bar();
};


class CBar : public Bar { // ColorBlocks bar
public:
  const ivBrush*	brush;
  
  virtual ivGlyph* GetBar()	{ return this; }

  CBar(const ivBrush* br, ColorScale* c, int b=16, int h=16, int w=16);
  ~CBar();
};

class HCBar : public CBar { // Horizontal ColorBlocks bar
public:
  HCBar(const ivBrush* br, ColorScale* c, int b=16, int h=20, int w=16);
  virtual void request(ivRequisition&) const;
  virtual void draw(ivCanvas*, const ivAllocation&) const;
};

class VCBar : public CBar { // Vertical ColorBlocks Bar
public:
  VCBar(const ivBrush* br, ColorScale* c, int b=16, int h=20, int w=16);
  virtual void request(ivRequisition&) const;
  virtual void draw(ivCanvas*, const ivAllocation&) const;
};


class PBar : public Bar { // Palette Bar
public:
  ivPolyGlyph* box;
  ivPatch*	blockpatch;
  ivTelltaleGroup* 	grp;
  int	paintindex;

  virtual int  	GetSelected();
  virtual void	InsertBlocks(ivPolyGlyph* pg);
  void		SetColorScale(ColorScale* c);

  PBar(ColorScale* c, int b=16, int h=16, int w=16);
  ~PBar();
};

class VPBar : public PBar { // 
public:
  VPBar(ColorScale* c, int b=16, int h=16, int w=16);
  virtual ivGlyph* GetBar();
};

class HPBar : public PBar { //  Horizontal Palette Bar
public:
  HPBar(ColorScale* c, int b=16, int h=16, int w=16);
  virtual ivGlyph* GetBar();
};


class ScaleBar;
class DynamicLabel;

class ColorPad : public ivMonoGlyph {
  // color pads for palletes
public:
  enum BlockFill {		// fill types for grid blocks
    COLOR,			// color indicates value
    AREA,			// area indicates value
    LINEAR 			// linear size of square side indicates value
  };

  ScaleBar* 	sb;
  int 		width;
  int 		height;
  float 	padval;
  float 	oldpadval;
  BlockFill 	fill_type;
  ivPatch* 	thepatch;
  ivGlyph* 	theback;
  String 	name;
  DynamicLabel* thename;
  ivColor* fg;			// foreground color;
  ivColor* bg;			// background color;
  ivColor* tc;			// text color;

  virtual void		SetFillType(BlockFill b);
  virtual void		Set(float val);
  virtual void		Reset();
  virtual void		Redraw();
  virtual void		ReFill();
  virtual float		GetVal();
  virtual void		GetColors();
  virtual void		Toggle();
  ColorPad(ScaleBar* tsb, BlockFill s=COLOR,
	   int w=8,int h=8,char* nm=NULL);
  ~ColorPad();
};

class ScaleBar : public ivResource { //  Scalebar 
public:
  enum SpanMode {
    RANGE,			// Range mode
    MIN_MAX 			// min max
  };
  float 	min;
  float 	max;
  float		range;
  float		zero;

  SpanMode	sm;
  bool		adjustflag;	// do we have min/max/range adjuster buttons
  bool		editflag;	// is the label editable

  Bar*			bar;		// actual bar
  ivGlyph* 		scalebar;	// the whole thing returned by get look
  ivInputHandler* 	ihan;

  ivFieldEditor*  	min_frep;
  ivFieldEditor*  	max_frep;

  ivPatch*		min_label;	// minium label
  ivPatch*		max_label;	// maximum label

  DAStepper* 		min_incr;	// increment minimum button
  DAStepper* 		min_decr;	// decrement minimum button
  DAStepper* 		max_incr;	// increment maximum button
  DAStepper* 		max_decr;	// decrement maximum button
  DAStepper* 		enlarger;	// increase range button
  DAStepper* 		shrinker;	// decrease range button

  ivGlyph_List  	padlist; 	// list of color pads for palletes
  ivAction* 		adjustnotify;	// action for adjusting (steppers)


  virtual DAStepper* GenEnlarger(){return NULL;}
  virtual DAStepper* GenShrinker(){return NULL;}

  virtual ivGlyph* GetLook(){ return NULL;};
  virtual  ivGlyph* GenIH(ivGlyph* g);

  virtual void SetRange(float val);
  virtual void SetRoundRange(float val);
  virtual void ModRange(float val);
  virtual void ModRoundRange(float val);

  virtual void FixRangeZero(); // sets range and zero according to min,max
  virtual void UpdateMinMax(float mn, float mx);
  virtual void SetMinMax(float min,float max);
  virtual int GetIdx(float val);	// returns index of the value
  virtual float GetVal(int idx);	// returns value of the index
  virtual ivColor* GetColor(float val, ivColor** maincolor=NULL,
			    ivColor** contrast=NULL);
   // return color object for val
  virtual ivColor* GetColor(int idx);	// return color[idx];
  virtual ivColor* GetContrastColor(int idx);	// return color[idx];

  virtual void SetColorScale(ColorScale* c);

  virtual float GetAbsPercent(float val);

  virtual void Incr_Range(int i=1); // i = number of times
  virtual void Decr_Range(int i=1);
  virtual void Incr_Min(int i=1);
  virtual void Incr_Max(int i=1);
  virtual void Decr_Min(int i=1);
  virtual void Decr_Max(int i=1);

  virtual void UpdatePads();
  virtual void SetAdjustNotify(ivAction* a);
  virtual void Adjust();
  virtual void GetScaleValues();      // gets from the nums into glyph
  virtual void UpdateScaleValues();   // puts from nums into glyph

  virtual void editor_accept(ivFieldEditor*);
  virtual void editor_reject(ivFieldEditor*);

  virtual void Initialize();
  virtual void Destroy();

  ScaleBar(float mn, float mx,bool adj=false,bool ed=true);
  ScaleBar(float r,bool adj = false,bool ed=true);
  virtual ~ScaleBar()           { Destroy(); }
};


class HCScaleBar : public ScaleBar { //  scale bar with
					     // horizontal color blocks
public:
  virtual DAStepper* GenEnlarger();
  virtual DAStepper* GenShrinker();
  virtual ivGlyph*	GetLook();

  HCScaleBar(float min, float max,bool adj,bool ed,const ivBrush* br, ColorScale* c,
	     int b=16,int h=16,int w=16);
  HCScaleBar(float r,bool adj,bool ed, const ivBrush* br, ColorScale* c,
	     int b=16,int h=16,int w=16);
};


class VCScaleBar : public ScaleBar {//  scale bar with vertical color blocks
public:

  virtual DAStepper* GenEnlarger();
  virtual DAStepper* GenShrinker();
  virtual ivGlyph*	GetLook();

  VCScaleBar(float min, float max,bool adj, bool ed,const ivBrush* br, ColorScale* c,
	     int b=16,int h=16,int w=16);
  VCScaleBar(float r,bool adj,bool ed,const ivBrush* br, ColorScale* c,
	     int b=16,int h=16,int w=16);
};

class PadButton;

class PScaleBar : public ScaleBar {
  //  Scalebar with a pallete
public:

  PScaleBar(float mn, float mx,bool adj=false,bool ed=true);
  PScaleBar(float r,bool adj = false,bool ed=true);
  virtual ~PScaleBar()          { Destroy(); }

  int GetSelected();

  float 		tag[4];
  String 		stag[4];
  PadButton* 		pb[4];
  ivPolyGlyph* 		fbox;
  ivPolyGlyph* 		tagpads;
  
  virtual void MakePads();
  virtual void MakeTags();
  virtual void Get_Tags(ivFieldEditor*);
  virtual void Set_Tags(ivFieldEditor*);
  virtual void GetScaleValues();

  void	SetColorScale(ColorScale* c);

  virtual void Destroy();

  virtual ivGlyph* GenIH(ivGlyph* g);
  float GetSelectedVal();
};

class HPScaleBar : public PScaleBar {
  //  Horizontal Scalebar with a pallete
public:

  virtual DAStepper* GenEnlarger();
  virtual DAStepper* GenShrinker();
  virtual ivGlyph*	GetLook();

   HPScaleBar(float min, float max, bool adj, bool ed,ColorScale* c,
	     int b=16,int h=16, int w=16);
  HPScaleBar(float r, bool adj, bool ed,ColorScale* c,
	     int b=16,int h=16, int w=16);
};

class VPScaleBar : public PScaleBar {
  //  Vertical Scalebar with a pallete
public:
  virtual DAStepper* GenEnlarger();
  virtual DAStepper* GenShrinker();
  virtual ivGlyph*	GetLook();

  VPScaleBar(float min, float max, bool adj, bool ed,ColorScale* c,
	     int b=16,int h=16, int w=16);
  VPScaleBar(float r, bool adj, bool ed,ColorScale* c,
	     int b=16,int h=16, int w=16);
};


class PadButton : public ivActiveHandler {
public:
  ColorPad* 	cp;
  
  operator ColorPad*(){ return cp;};
  virtual void		Set(float val) { cp->Set(val);}
  void press(const ivEvent&);
  PadButton(ivStyle* s, PScaleBar* sb,
	    ColorPad::BlockFill sh=ColorPad::COLOR, int w=8,int h=8);
};

class PaintPad : public ivInputHandler {
public:
  ColorPad* pb;
  void* obj;			    // object to pass to change_notify
  static ivCursor* painter_cursor;

  void (*change_notify)(void*); // function to call if changed
  virtual void press(const ivEvent& e);
  virtual void release(const ivEvent& e);
  virtual void drag(const ivEvent& e);
  virtual void move(const ivEvent& e);
  PaintPad(ColorPad* c,void* o, void (*cn)(void*));
};

class DynamicBackground : public ivMonoGlyph {
public:
  DynamicBackground(ivGlyph* body, const ivColor*);
  virtual ~DynamicBackground();
  virtual void SetColor(ivColor*);
  virtual void allocate(ivCanvas*, const ivAllocation&, ivExtension&);
  virtual void draw(ivCanvas*, const ivAllocation&) const;
  virtual void print(ivPrinter*, const ivAllocation&) const;
protected:
  const ivColor* color_;
};


class CachePatch_Impl {
public:
  enum FakeLevel {
    FIRST,
    ONCE,
    ALWAYS
  };
      
  ivRequisition	oldreq;
  FakeLevel	fakeflag;
  CachePatch_Impl(){};
  ~CachePatch_Impl(){};
};

class CachePatch : public ivPatch {
public:
  void request(ivRequisition&) const;
  CachePatch_Impl *impl;

  CachePatch(ivGlyph*);
  ~CachePatch(){ delete impl;}
};

#endif // colorbar_h
