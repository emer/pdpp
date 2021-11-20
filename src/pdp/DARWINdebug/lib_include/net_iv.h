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

#ifndef net_iv_h
#define net_iv_h

#include <pdp/base.h>
#include <pdp/netstru.h>
#include <ta_misc/colorbar.h>
#include <ta/taiv_data.h>
#include <ta_misc/minmax.h> // for height_field

#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_viewer.h>
#include <iv_graphic/graphic_text.h>

#include <ta/enter_iv.h>
#include <InterViews/patch.h>
#include <ta/leave_iv.h>

class ivColor;		// #IGNORE
class ivStyle;		// #IGNORE
class ivTransformer;	// #IGNORE
class GlyphViewer;	// #IGNORE
class ivWindow;		// #IGNORE
class ivTelltaleGroup;	// #IGNORE
class ivButton;		// #IGNORE
class ivAction;		// #IGNORE
class ivDialogKit;	// #IGNORE
class ivScrollBox;	// #IGNORE
class ivPatch;		// #IGNORE
class ivFont;		// #IGNORE
class ivPolyGlyph;	// #IGNORE
class ivDeck;		// #IGNORE

// predeclarations of all types:

class TDTransform;
class Network_G;
class NetViewGraphic_G;
class Layer_G;
class LayerNameEdit;
class LayerText_G;
class LayerBox_G;
class Unit_Group_G;
class Projection_G;
class PLine_G;
class SelfCon_PLine_G;
class Unit_G;
class UnitValue_G;
class SquareUnit_G;
class AreaUnit_G;
class LinearUnit_G;
class FillUnit_G;
class DirFillUnit_G;
class ThreeDUnit_G;
class RoundUnit_G;
class HgtFieldUnit_G;
class HgtPeakUnit_G;
class NetViewer;
class NetEditor;


class TDTransform {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS 2.5d pseudo skew ivTransformer
public:
  // these are in 'world' coordinates (3-d), X = horiz, Y = deep, Z = vert
  float 	scx;			// scale in the x dimension
  float 	scz;			// scale in the z dimension
  float 	dfx;			// delta-x
  float 	dfz;			// delta-z

  // these are the projected coordinates into screen points for a trapezoidal
  // projection of the square x = horiz, y = vertical
  // 0 = left_bot, 1 = right_bot, 2 = right_top, 3 = left_top

  float 	pt_x_1;			// offsets for the x & y dimension
  float 	pt_x_2;
  float 	pt_x_3;
  float 	pt_y_1;

  virtual void Set(TDCoord& max, float skew, float fpct_x = .9,
		   float fpct_y = .85,    float fpct_z = .95);

#ifndef __MAKETA__
  virtual void		Project(int x, int y, int z, float& ix, float& iy);
  virtual void		Project(TDCoord& td, float& ix, float& iy)
  { Project(td.x, td.y, td.z, ix, iy); }
#endif

  virtual void		InverseProjXY(float ix,float iy,int& x,int& y,int& z);
  virtual void		InverseProjXZ(float ix,float iy,int& x,int& y,int& z); 
  virtual void		InverseProjYZ(float ix,float iy,int& x,int& y,int& z); 

  virtual void		InverseProjXY(float ix, float iy, TDCoord& td) 
  { InverseProjXY(ix, iy, td.x, td.y, td.z); }
  virtual void		InverseProjXZ(float ix, float iy, TDCoord& td) 
  { InverseProjXZ(ix, iy, td.x, td.y, td.z); }
  virtual void		InverseProjYZ(float ix, float iy, TDCoord& td) 
  { InverseProjYZ(ix, iy, td.x, td.y, td.z); }

  TDTransform()	{ };
  virtual ~TDTransform() { };
};

class Network_G : public GraphicMaster {
public:	
  Network*	net;
  NetEditor*	owner;
  ivColor*	border;	        // 'unselected' border color
  const ivBrush* unitbrush;
  TDCoord	max_size;	// for the whole network (sets scaling)
  TDTransform	tdp;		// projection operator
  float		skew;
  MemberSpace	membs;		// list of all the members possible in units
  int		cur_mbr;	// current member to display
  bool		update_scale_only;
  // indicates whether to update only the scale on the next update_from_state()
  bool		changed_pickgroup;
  bool		reinit_display;	// subobjs set this flag to reinit after popping out

  virtual void	UpdateSelect(); // Updates the graphics to reflect the state
  virtual void	UpdatePick();	// of the pickgroup and selectgroup;

  virtual void	SelectObject(TAPtr obj); // selects just this one object
  virtual void	SelectObject_impl(TAPtr obj); // does the hard work..

  bool 		select(const ivEvent& e, Tool& tool, bool);
  bool		effect_select(bool set_select);

  bool		pick_me(const ivEvent& e, Tool& tool, bool);
  virtual void	GetMembs();
  virtual void	SetMax(TDCoord& max);
  virtual void	Reset();
  virtual void	Build();
  virtual void	RePick();

  virtual void	FixProjection(Projection* p, int index);
  virtual void	FixUnit(Unit* u, Layer* lay);
  virtual void	FixUnitGroup(Unit_Group* ug, Layer* lay);
  virtual void	FixLayer(Layer* lay);
  virtual void	RepositionPrjns();

  virtual void	RemoveGraphics();
  virtual bool	LoadGraphic(const char* nm);
  // load a graphic to display in the background of the network display

  virtual ivColor* GetLabelColor();

  bool		update_from_state(ivCanvas* c);
  void 		allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext);

  virtual void	SetMyTransform(const ivAllocation& a);
  // set my transformer based on allocation or any frozen transformer on the netview
  virtual void 	ReCenter();
  virtual void 	ScaleCenter(const ivAllocation& a);

  Network_G(Network*);
  ~Network_G();
  GRAPHIC_BASEFUNS(Network_G);
};

/////////////////////
// NetViewGraphic  //
/////////////////////

class NetViewGraphic_G : public PolyGraphic {
public:
  NetViewGraphic* 	netvg;

  bool		pickable() { return false; }


  bool	select(const ivEvent& e, Tool& tool, bool unselect);

  bool	effect_stretch(const ivEvent& e, Tool&, 
		       ivCoord ix,ivCoord iy,
		       ivCoord fx, ivCoord fy);

  bool	effect_move(const ivEvent& e, Tool& t,
		    ivCoord ix,ivCoord iy,
		    ivCoord fx, ivCoord fy);

  virtual bool	effect_scale(const ivEvent& e, Tool& t,
			     ivCoord ix,ivCoord iy,
			     ivCoord fx, ivCoord fy,
			     ivCoord ctrx, ivCoord ctry);

  virtual bool	effect_rotate(const ivEvent& e, Tool& t , 
			      ivCoord ideg, ivCoord fdeg,
			      ivCoord ctrx, ivCoord ctry);

  virtual void  flush();

  NetViewGraphic_G(NetViewGraphic* n, Graphic* g= nil);
  virtual ~NetViewGraphic_G();
  GRAPHIC_BASEFUNS(NetViewGraphic_G);
};

class NetViewLabelGroup_G : public PolyGraphic {
public:
  bool		pickable() { return false; }

  bool 		effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
				    ivCoord iy, ivCoord fx, ivCoord fy);

  NetViewLabelGroup_G(Graphic* g);
  virtual ~NetViewLabelGroup_G();
  GRAPHIC_BASEFUNS(NetViewLabelGroup_G);
};

class Layer_G : public PolyGraphic {
public:
  enum laysides { TOP,RIGHT,BOTTOM,LEFT,INDEPENDENT };
  Layer* 	lay;
  Network_G*	netg;
  TDCoord	move_pos;
  ivPolyGlyph*	proj_gs;	// projection glyphs associate with this layer

  virtual void	Position();
  virtual void	Build();
  virtual void	RePick();
		
  virtual void	RemoveProj_G(Projection_G* pjn_g);
  virtual void	RedistributeSide(int index);
  virtual void	FixText();
  virtual void	FixBox();
  virtual void	FixUnits();
  virtual void	FixUnitGroup(Unit_Group* ug);
  virtual void	FixUnit(Unit* u);
  virtual void	FixAll()	{ FixBox(); FixText(); FixUnits();}

  virtual LayerBox_G* GetActualLayerBox();
  virtual float GetCurrentXformScale();
  virtual float GetCurrentYformScale();
  bool     	select(const ivEvent& e, Tool& tool, bool unselect);
  bool		grasp_move(const ivEvent& e,Tool& tool,ivCoord, ivCoord);
  bool		manip_move(const ivEvent& e,Tool& tool,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
				   ivCoord cx, ivCoord cy);
  // this applies to the layer
  bool 		effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
				    ivCoord iy, ivCoord fx, ivCoord fy);
  virtual bool	grasp_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy);
  bool		manip_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
				   ivCoord cx, ivCoord cy);
  bool		effect_stretch(const ivEvent&, Tool&,ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy);
  virtual bool	effect_scale(const ivEvent&, Tool&,ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy,
				     ivCoord ctrx, ivCoord ctry);
  virtual void	damage_me(ivCanvas* c);
  void 		draw_gs (ivCanvas* c, Graphic* gs);

  void		flush()	 { };

  ivGlyph*	clone() const;
  Layer_G(Layer* lyr, Network_G* ng);
  virtual ~Layer_G();

  GRAPHIC_BASEFUNS(Layer_G);
};

class LayerNameEdit : public taNBase {
  // used by LayerText_G to edit layer name
public:
  Layer*	layer;		// #HIDDEN the layer for this name

  void	SetLayer(Layer*);
  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy();
  TA_BASEFUNS(LayerNameEdit);
};

class LayerText_G : public NoScale_Text_G {
public:
  Layer_G* 	lay_g;
  LayerNameEdit* lne;		// edit this layer name

  bool		pickable() { return false; }

  ivTransformer* transformer();
  void	transformer(ivTransformer* t);
  bool	effect_move(const ivEvent&, Tool&,ivCoord ix,ivCoord iy,
			    ivCoord fx, ivCoord fy);

  bool 	effect_select(bool set_select);

  LayerText_G(Layer_G* l,const ivFont* font, const ivColor* stroke,
	       const char* c, ivTransformer* t);
  ~LayerText_G();
  GRAPHIC_BASEFUNS(LayerText_G);
};

class LayerBox_G : public Polygon {
  // box representing layer and subgroups within layer
public:
  TAPtr		object;		// object that box represents
  PosTDCoord*	geom;		// geometry controlled by this
  Network_G* 	netg;

  bool		pickable() { return false; }

  virtual void 	draw_gs(ivCanvas*, Graphic*);
  bool		effect_select(bool set_select);

  void		SetPointsFmGeom();	// from geometry
  void 		SetPoints(ivCoord x0,ivCoord y0, ivCoord x1, ivCoord y1,
			  ivCoord x2,ivCoord y2, ivCoord x3, ivCoord y3);
  void 		GetPoints(ivCoord* x0,ivCoord* y0, ivCoord* x1, ivCoord* y1,
			  ivCoord* x2,ivCoord* y2, ivCoord* x3, ivCoord* y3);

  // cancel these out
  bool	grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord){return true;}
  bool	manip_move(const ivEvent& ,Tool&,ivCoord,ivCoord,ivCoord,
			   ivCoord,ivCoord,ivCoord) {return true;}
  bool	grasp_scale(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,ivCoord,
			    ivCoord,ivCoord) {return true;}
  bool	manip_scale(const ivEvent&,Tool&,ivCoord,ivCoord,ivCoord,ivCoord,
			    ivCoord,ivCoord,ivCoord,ivCoord) {return true;}

  LayerBox_G(TAPtr obj, PosTDCoord* gm, Network_G* ng,
	      const ivBrush* brush, const ivColor* stroke,
	      const ivColor* fill, ivCoord* x, ivCoord* y,
	      int ctrlpts, ivTransformer*);
  GRAPHIC_BASEFUNS(LayerBox_G);
};

class Unit_Group_G : public PolyGraphic {
public:
  Unit_Group*	group;
  Layer_G*	layg;
  TDCoord	move_pos;

  virtual void	Position();
  virtual void	Build();
  virtual void	RePick();

  virtual void  FixUnits();
  virtual void	FixUnit(Unit* u);

  bool		pickable() { return false; }

  bool		grasp_move(const ivEvent& , Tool& ,ivCoord ix, ivCoord iy);
  bool		manip_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
				   ivCoord cx, ivCoord cy);
  bool		effect_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord fx, ivCoord fy);
  bool    	select(const ivEvent& e, Tool& tool, bool unselect);

  virtual bool	grasp_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy);
  bool		manip_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
				   ivCoord cx, ivCoord cy);
  bool		effect_stretch(const ivEvent&, Tool&,ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy);

  ivGlyph*	clone() const;
  Unit_Group_G(Unit_Group* gp, Layer_G* lg);
  virtual ~Unit_Group_G()			{ };

  GRAPHIC_BASEFUNS(Unit_Group_G);
};

class Projection_G : public PolyGraphic {
public:
  Projection*	pjn;
  Layer_G*	from_g;
  Layer_G*	to_g;
  Network_G*	net_g;
  float		mid_dist;	// distance between midpoints of to/from layers

  ivCoord	l;		// temporary storage for grasp_move ect
  ivCoord	r;		// temporary storage
  ivCoord	t;		// temporary storage
  ivCoord	b;		// temporary storage
  int		move_end;	// 0 = from/send , 1 = to/receive

  bool		pickable() { return false; }

  virtual void	RemoveMe();
  virtual void  GetClosestSides(int& from_side_index,int& to_side_index);
  virtual void  GetSelfProjSides(int& from_side_index,int& to_side_index);
  virtual void 	RecomputePoints();
  virtual void	NewProjection(Projection* p, Layer_G *tg,Layer_G *fg,Network_G* n);

  // finds the closest points
  bool	grasp_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy);
  bool	manip_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
			   ivCoord cx, ivCoord cy);
  bool	effect_move(const ivEvent&, Tool&,ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy);

  Projection_G();
 ~Projection_G();
  GRAPHIC_BASEFUNS(Projection_G);
};

class PLine_G : public Polyline {
public:
  Projection_G*  proj_g;
  bool		editb_used;	// whether edit button was used

  virtual int   StartIndex() 	{ return 0; }
  virtual int   EndIndex() 	{ return 1; }

  virtual void	SetX(int i,ivCoord val) { _x[i] = val; recompute_shape(); }
  virtual void  SetStartX(ivCoord val) 	{ SetX(StartIndex(),val); }
  virtual void  SetEndX(ivCoord val)	{ SetX(EndIndex(),val); }
  virtual void	AddX(int i,ivCoord val)
  { SetX(i,val+_x[i]); recompute_shape(); }
  virtual void  MaxX(int i,ivCoord val)
  { if (_x[i] > val) SetX(i,val); recompute_shape(); }
  virtual void  MinX(int i,ivCoord val)
  { if (_x[i] < val) SetX(i,val); recompute_shape(); }
  ivCoord 	GetX(int i) { return _x[i]; }
  ivCoord 	GetStartX() { return _x[StartIndex()]; }
  ivCoord 	GetEndX()   { return _x[EndIndex()]; }

  virtual void	SetY(int i,ivCoord val) { _y[i] = val; recompute_shape(); }
  virtual void  SetStartY(ivCoord val) 	{ SetY(StartIndex(),val); }
  virtual void  SetEndY(ivCoord val) 	{ SetY(EndIndex(),val); }
  virtual void	AddY(int i,ivCoord val)
  { _y[i] += val; recompute_shape(); }
  virtual void  MaxY(int i,ivCoord val)
  { if (_y[i] > val) _y[i] = val; recompute_shape(); }
  virtual void  MinY(int i,ivCoord val)
  { if (_y[i] < val) _y[i] = val; recompute_shape(); }
  ivCoord 	GetY(int i) { return _y[i]; }
  ivCoord 	GetStartY() { return _y[StartIndex()]; }
  ivCoord 	GetEndY()   { return _y[EndIndex()]; }

  virtual void 	Set_Start_End(ivCoord x0,ivCoord y0, ivCoord x1, ivCoord y1)
  { SetStartX(x0); SetEndX(x1); SetStartY(y0); SetEndY(y1); }

  virtual void 	Xform_gs(ivCoord* tx, ivCoord* ty, Graphic* gs);
  virtual void 	Xform_Point(ivCoord ix, ivCoord iy, ivCoord* tx, ivCoord* ty,int side);
  
  void 	draw_gs(ivCanvas*, Graphic*);
  void	getextent_gs(ivCoord& l, ivCoord& b, ivCoord& cx, ivCoord& cy,
  		     ivCoord& tol, Graphic* gs);
  bool 	intersects_gs (BoxObj& userb, Graphic* gs);

  bool	pickable() { return false; }

  bool  select(const ivEvent& e, Tool& tool, bool unselect);
  bool 	effect_select(bool set_select);
  
  // cancel these out
  bool	grasp_move(const ivEvent&, Tool&,ivCoord,ivCoord){ return true;}
  bool	manip_move(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,ivCoord,
			   ivCoord,ivCoord){ return true;}
  bool	effect_move(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,
			    ivCoord ){return true;}
  bool	grasp_stretch(const ivEvent&, Tool&,ivCoord,ivCoord){return true;}
  bool	manip_stretch(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,
			      ivCoord,ivCoord,ivCoord){return true;}
  bool	effect_stretch(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,
			       ivCoord){return true;}

  PLine_G(Projection_G* pg, const ivBrush* brush,
	  const ivColor* stroke,  const ivColor* fill,
	  ivCoord* x, ivCoord* y, int size,ivTransformer* tx);
  GRAPHIC_BASEFUNS(PLine_G);
};

class SelfCon_PLine_G : public PLine_G {
  // four points plus 2 arrow points
public:
  int 		side_index;

  bool		pickable() { return false; }

  int   	StartIndex() 	{ return 0;}
  int  	 	EndIndex() 	{ return 3;}
  
  void		SetX(int i,ivCoord val);
  void		SetY(int i,ivCoord val);
  void  	ComputeArrowPoints();
  void  	Xform_gs(ivCoord* tx, ivCoord* ty, Graphic* gs);

  void 		draw_gs(ivCanvas*, Graphic*);

  void 		Initialize()	{ side_index = 0; }
  SelfCon_PLine_G(Projection_G* pg, const ivBrush* brush,
	  const ivColor* stroke,  const ivColor* fill,
	  ivCoord* x, ivCoord* y, int size,ivTransformer* tx);
  GRAPHIC_BASEFUNS(SelfCon_PLine_G);
};

class Unit_G : public PolyGraphic {
  // represents a unit as a collection of unitvalue_g graphics, one for each value being displayed
public:
  Unit*		unit;
  Network_G*	netg;
  ivCoord	x[4]; // x corners
  ivCoord	y[4]; // y corners
  UnitValue_G*	cur_uvg; // for manip_alter
  ivCanvas*	lastcanvas;
  
  virtual UnitValue_G* NewUnitValue(MemberDef* md);
  virtual void	AddMD(MemberDef* md, bool chosen=true);
  virtual void	ChangeMD(MemberDef* md); // changes the last membdef to md
  virtual void	RemoveMD(MemberDef* md);
  virtual int	FindValue_G(UnitValue_G* uvg);
  virtual void	Fix_UVGS();

  bool 		manipulating(const ivEvent&, Tool&);
  bool		grasp_move(const ivEvent& e,Tool& tool,ivCoord, ivCoord);
  bool		manip_move(const ivEvent& e,Tool& tool,ivCoord ix, ivCoord iy,
			   ivCoord lx, ivCoord ly, ivCoord cx, ivCoord cy);
  bool		effect_move(const ivEvent& e,Tool& tool,ivCoord ix, ivCoord iy,
			    ivCoord fx, ivCoord fy);

  bool		grasp_alter(const ivEvent&, Tool&,
			    ivCoord ix, ivCoord iy, ivCoord ctrx, ivCoord ctry,
			    ivCoord w, ivCoord h);
  bool		manip_alter(const ivEvent&, Tool&,
			    ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
			   ivCoord cx, ivCoord cy, ivCoord ctrx, ivCoord ctry);
  bool		effect_alter(const ivEvent&, Tool&,
			     ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy,
			     ivCoord ctrx, ivCoord ctry);

  bool 		select(const ivEvent& e, Tool& tool, bool unselect);
  virtual void	spread_pick(const ivEvent& e, Tool& tool,bool pick_state);
  // an element was picked, spread to others

  virtual void	Position(); // compute corners of unit space
  virtual void	DistributeUVGS(int index=-1);

  Unit_G(Network_G* ng,Unit* u);

  Unit_G();
 ~Unit_G()		{ };
  GRAPHIC_BASEFUNS(Unit_G);
};

class UnitValue_G : public Graphic {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS base type for graphical unit rep, handles all value info
public:
  enum MDType {
    UNKNOWN = -1,
    FLOAT,
    DOUBLE,
    INT 
  };

  Unit_G* 		ug;
  MemberDef*		spec_md; // memberdef specified as the one to view
  MemberDef*		act_md;	// actual md on unit, NULL if spec_md is not on given unit
  void*			base;
  int			pickindex; // if a con md, this is the index of the picked unit
  MDType		md_type;
  float			prv_val; 	// previous value
  bool			displabel;	// whether this unit displays the label info

  virtual void	UpdateConBase(); // get the base offset for connection objects
  virtual void	SetCurMD(MemberDef* md);
  virtual void	FixType();
  virtual float	GetDisplayVal();

  void 		Position();
  virtual void	RePick();

  virtual void	SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
		       ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3);

  virtual void 	render_text(ivCanvas* c, ScaleBar* cbar, float val, String& str,
			    bool from_top=false);
  // this actually does the drawing
  virtual void 	draw_text(ivCanvas* c, float val);
  // this is the overall control function for draw routine to draw text

  bool		update_from_state(ivCanvas*);

  bool 		select(const ivEvent& e, Tool& tool, bool unselect);
  bool 		pick_me(const ivEvent& e, Tool& tool, bool unselect);

  UnitValue_G(Unit_G* unitg, MemberDef* md, int num, bool curved);
  GRAPHIC_BASEFUNS(UnitValue_G);
};

class SquareUnit_G : public UnitValue_G {
public:
  void 		draw_gs(ivCanvas*, Graphic*);
  virtual void	SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
		       ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3);
  SquareUnit_G(Unit_G*, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(SquareUnit_G);
};

class AreaUnit_G : public SquareUnit_G {
public:
  ivColor* 	fg_color;
  ivColor* 	bg_color;

  virtual void	GetColors(ivCanvas* c, Graphic*gs);
  void 		draw_gs(ivCanvas*, Graphic*);
  void		draw_text(ivCanvas* c, float val); // draw text in better contrast color
  ~AreaUnit_G();
  AreaUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(AreaUnit_G);
};

class LinearUnit_G : public AreaUnit_G {
public:
  void 		draw_gs(ivCanvas*, Graphic*);
  LinearUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(LinearUnit_G);
};

class FillUnit_G : public AreaUnit_G {
public:
  void 		draw_gs(ivCanvas*, Graphic*);
  FillUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(FillUnit_G);
};

class DirFillUnit_G : public AreaUnit_G {
public:
  void 		draw_gs(ivCanvas*, Graphic*);
  DirFillUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(DirFillUnit_G);
};

class ThreeDUnit_G : public SquareUnit_G {
public:
  float 	last_height;
  void 		draw_gs(ivCanvas* c, Graphic* g);
  void 		getextent_gs(ivCoord& l, ivCoord& b,
			     ivCoord& cx, ivCoord& cy, ivCoord& tol, Graphic* gs);
  ThreeDUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(ThreeDUnit_G);
};

class RoundUnit_G : public UnitValue_G {
public:
  void 		draw_gs(ivCanvas*, Graphic*);
  void		SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
		       ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3);
  RoundUnit_G(Unit_G*, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(RoundUnit_G);
};

class HgtFieldUnit_G : public SquareUnit_G {
public:
  float		height;
  float		oldval;
  float		corner_heights[4];
  ivPolyGlyph 	neighbors;

  virtual void	GetNeighbors();
  virtual void	ComputeCorners();
  virtual void	ComputeHeight();
  virtual void  UpdateNeighbors(ivCanvas* c);

  void		SetPos(ivCoord x0, ivCoord y0, ivCoord x1,ivCoord y1,
		       ivCoord x2, ivCoord y2, ivCoord x3,ivCoord y3);
  bool		update_from_state(ivCanvas*);
  void 		draw_gs(ivCanvas* c, Graphic* g);
  void 		getextent_gs(ivCoord& l, ivCoord& b,
			     ivCoord& cx, ivCoord& cy, ivCoord& tol, Graphic* gs);

  HgtFieldUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(HgtFieldUnit_G);
};

class HgtPeakUnit_G : public HgtFieldUnit_G {
public:
  void 		draw_gs(ivCanvas* c, Graphic* g);
  HgtPeakUnit_G(Unit_G* g, MemberDef* md=NULL);
  GRAPHIC_BASEFUNS(HgtPeakUnit_G);
};

class NetViewer : public GlyphViewer {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS viewer for network
public:
  NetView* 	netv;
  void 		release(const ivEvent &);
  void 		init_graphic() 		{ initgraphic();}
  void 		allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext);
  NetViewer(NetView *n, float w, float h, const ivColor* bg);
};

class Process_MGroup;

class NetEditor {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS editor for network
public:
  enum ConActions {		// actions for the connect button
    NO_CONNECT, NEW_LABEL, NEW_PRJNS, NEW_SELF_PRJN, FILL_PRJNS, CONNECT_UNITS,
    SELFCON_UNIT, EDIT_CON, HOT_NEW_PRJNS, HOT_FILL_PRJNS,
    CON_COUNT
  };
  enum NewActions {
    NO_NEW, NEW_LAYERS, NEW_BIPRJNS, BICON_UNITS, RMV_CON, NEW_UNITS,
    HOT_NEW_LAYERS, HOT_NEW_UNITS,
    NEW_COUNT
  };
  enum RemoveActions {
    NO_REMOVE, REMOVE_LAYERS, REMOVE_UNITGPS, REMOVE_PRJNS, REMOVE_UNITS,
    REMOVE_LABELS, REMOVE_GRAPHICS, REMOVE_ALL, 
    REMOVE_COUNT
  };
  enum EditActions {
    NO_EDIT, EDIT_NAMES, EDIT_LAYERS, EDIT_UNITGPS, EDIT_PRJNS, EDIT_UNITS,
    EDIT_LABELS, EDIT_ALL, 
    EDIT_COUNT 
  };

  NetView*		owner;
  Network* 		net;
  NetViewer* 		viewer;
  Network_G*		netg;
  ivWindow*		win;

  ivPatch*		print_patch;

  bool			display_toggle; // display on/off
  bool			auto_scale;	// auto_scale on/off
  ConActions 		connect_action;
  NewActions 		new_action;
  RemoveActions 	remove_action;
  EditActions 		edit_action;
  NetView::UnitTextDisplay unit_text;	// how to display unit text
  NetView::UnitShape	shape;		// how to display unit information
  ivButton*		ckbox;	        // checkbox representation
  ivPatch*		pospatch;       // position indicator
  ColorScale*		scale;
  ScaleBar*		cbar;	        // colorbar
  ivButton*		as_ckbox;       // autoscale ck_box
  bool			autoscale;      // autoscale flag
  ivButton*		ut_ckbox;       //  unittext ck_box
  ivTelltaleGroup*	tool_gp;        // radio button group of tools
  ivPolyGlyph		var_gp;	        // group of variable buttons
  ivScrollBox*		var_rep;        // variable button representation
  ivPatch*		var_patch; 	// patch for variable reps

  ivMenu*		statbutton;
  taivHierMenu*		mstatmenu;
  taivMenu*		utextmenu;
  taivMenu*		dispmdmenu;

  ivGlyph*		actbutton[9]; 	// tool action buttons (move, select, etc)
  ivDeck* 		new_but;		// new button options (stacked in a deck)
  ivDeck* 		connect_but; 	// connect "
  ivDeck* 		remove_but;
  ivDeck* 		edit_but;
  ivDeck* 		build_but; 	// build-all, highlighting..
  ivDeck* 		con_but; 	// connect-all
  ivPatch*		buttonpatch;

  virtual void		Init(); 	// initialize

  virtual ivGlyph* 	GetLook();	// gets the whole thing
  virtual ivGlyph* 	GetTools();	// get the tool buttons
  virtual ivGlyph* 	GetVars();	// get the variable buttons

  virtual void	update_from_state();
  virtual void	InitDisplay();
  virtual void	UpdateDisplay();

  // get a 'pallete button' that is mutually exclusive
  virtual ivButton*	MEButton(ivTelltaleGroup* gp, char* txt, ivAction* a);
  // get the variable ivButton for this index
  virtual ivButton*	VarButton(char* txt, int mdx, int at=-1);
		
  virtual void	BuildUTextMenu(); // make the unit_text menu
  virtual void	UpdateUTextMenu(NetView::UnitTextDisplay utd); // set utext menu to correct value;
  virtual void	MenuSetUText(taivMenuEl*);  // gets the value from the utext menu
  virtual void	SetUText(taivMenuEl*);	   // gets the value from the utext menu
		
  virtual void	BuildDispMdMenu(); // make the display mode ('shape')
  virtual void	UpdateDispMdMenu(NetView::UnitShape ushp); // set display mode ("shape")
  virtual void	MenuSetDispMd(taivMenuEl*);  // gets the value from the dispmd menu
  virtual void	SetDispMd(taivMenuEl*);	   // gets the value from the dispmd menu
		
  virtual void	CBarAdjust();	   // what to so when the scaelbar is adjusted
  virtual void	ToggleDisplay(bool update=true);
  virtual void	MenuToggleDisplay(){ToggleDisplay();}
  virtual void	ToggleAutoScale(bool update=true);
  virtual void	MenuToggleAutoScale(){ToggleAutoScale();};

  virtual void	NumberVarButtons(); // number the varbuttons in order selected
  virtual void  SelectActButton(int toolnumber);

  virtual void	SetPick(); // sets cursor and tool mode to Pick
  virtual void	SetSelect(); // sets cursor and tool mode to Select
  virtual void	SetMove();// sets cursor and tool mode to Move
  virtual void	SetReScale();
  virtual void	SetReShape();
  virtual void	SetRotate();
  virtual void	SetAlter();
  virtual void	RePick();

  virtual void	FixEditorButtons();

  virtual void 	BuildAll();
  virtual void 	ConnectAll();
  virtual void	NewObjects();
  virtual void	ConnectObjects();
  virtual void	EditSelections();
  virtual void	EditConnection();
  virtual void	RemoveConnection();
  virtual void	RemoveSelections();
		
  virtual void	SetLayerXform(Layer* l, ivTransformer* t);
  virtual void	SetLayerLabelXform(Layer* l, ivTransformer* t);
		
  virtual void	UpdatePosivPatch(TDCoord& tdg)
  { UpdatePosivPatch(tdg.x, tdg.y, tdg.z); }
  virtual void	UpdatePosivPatch(int x, int y, int z);
  virtual void  UpdatePosivPatch(float val);
  virtual void	ClearPosivPatch();

  virtual Process_MGroup* GetProcessGroup();

  virtual void	BuildMStatMenu();
  virtual void	UpdateMStatMenu();
  virtual void	NewMStat();
  virtual void	SetMStatObjects(taivMenuEl* sel);
  virtual void	SetMStatMember(taivMenuEl* sel);
  virtual void	EditMStat(taivMenuEl* sel);
  virtual void	RemoveMStat(taivMenuEl* sel);
  virtual void	RemoveMonitors();
  virtual void	UpdateMonitors();

  NetEditor(Network* nt, NetView* nv, ivWindow* w);
  virtual ~NetEditor();
};

#endif // net_iv_h

