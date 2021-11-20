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

#ifndef datatable_iv_h
#define datatable_iv_h

#include <ta_misc/colorscale.h>
#include <ta_misc/colorbar.h>
#include <ta_misc/datatable.h>
#include <ta_misc/fontspec.h>

#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_viewer.h>
#include <iv_graphic/graphic_text.h>

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
class ivFieldEditor;	// #IGNORE

// predeclarations of all types:

class DataLineBox_G;
class DataLine_G;
class Grid_G;
class DTViewer;
class DTEditor;

class DataTable_G : public GraphicMaster {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS
public:
  DataTable*	dt;		// the data table itself
  DT_GridViewSpec* dtvsp;	// and its grid view spec
  DTEditor*	owner;
  ivColor*	border;	        // 'unselected' border color
  const ivBrush* defbrush;	// default brush
  FloatTDCoord	max_size;	// for the whole DataTable (sets scaling)
  bool		reinit_display;	// subobjs set this flag to reinit after popping out
  ivTransformer* tx;

  virtual void	Reset();
  virtual void	Build();

  virtual ivColor* GetLabelColor();

  virtual void	AddOneLine();	// add one new line to display
  virtual void	ScrollLines();	// update existing boxes to use current line info

  bool		update_from_state(ivCanvas* c);

  bool 		select(const ivEvent& e, Tool& tool, bool);
  bool		effect_select(bool set_select);

  void 		allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext);
  void 		ReCenter();
  void 		ScaleCenter(const ivAllocation& a);

  // translate x,y unit-wise coordinates into screen coords
  ivCoord xcscale(ivCoord xval) { return xval / max_size.x; }
  ivCoord ycscale(ivCoord yval) { return yval / max_size.y; }

  // translate x,y unit-wise coordinates into text coords
  ivCoord xtscale(ivCoord xval) { return GetCurrentXformScale() * xcscale(xval); }
  ivCoord ytscale(ivCoord yval) { return GetCurrentYformScale() * ycscale(yval); }

  DataTable_G(DataTable* d, DT_GridViewSpec* dtv);
  ~DataTable_G();
  GRAPHIC_BASEFUNS(DataTable_G);
};

class DataLine_G : public PolyGraphic {
  // represents one line of data (or its template)
public:
  int 			line_no; // line of data to represent (FROM END), -1 = template
  DataTable_G*		dtg;

  virtual void	Build();
  virtual void	BuildTable(DT_GridViewSpec* dtv);

  virtual void	ScrollLines(int nw_ln);	// set new line number for all my elements

  bool		selectable() { return false; }
  bool		graspable() { return false; }

  bool     	select(const ivEvent& e, Tool& tool, bool unselect);
  bool		effect_select(bool set_select);

  ivGlyph*	clone() const;
  DataLine_G(int lno, DataTable_G* ng);
  virtual ~DataLine_G();
  GRAPHIC_BASEFUNS(DataLine_G);
};

class DataLineBox_G : public Polygon {
  // box representing DataLine (actually draws as a horizontal line)
public:
  DataLine_G*		dlg;
  DataTable_G* 		dtg;
  bool			editb_used; // the edit-button was used (right mouse button)

  virtual void 	SetPointsBox(ivCoord x,ivCoord y);
  virtual void 	SetPointsLine(ivCoord x,ivCoord y);

  bool		selectable() { return false; }
  bool		graspable() { return false; }

  bool     	select(const ivEvent& e, Tool& tool, bool unselect);
  bool		effect_select(bool set_select);

  DataLineBox_G(DataLine_G* eg, DataTable_G* dg);
  GRAPHIC_BASEFUNS(DataLineBox_G);
};

class TextData_G : public NoScale_Text_G {
  // represents textual data element
public:
  int 			line_no; // line of data to represent (FROM END), -1 = template
  DA_GridViewSpec* 	daspec;
  DataTable_G*		dtg;
  TDCoord		move_pos;
  bool			editb_used; // the edit-button was used (right mouse button)

  virtual void	Position();

  bool  select(const ivEvent& e, Tool& tool, bool unselect);
  bool	effect_select(bool set_select);

  bool	update_from_state(ivCanvas* c);

  bool	grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord);
  bool	manip_move(const ivEvent& ,Tool&,ivCoord,ivCoord,ivCoord,
		   ivCoord,ivCoord,ivCoord);
  bool 	effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
		    ivCoord iy, ivCoord fx, ivCoord fy);

  bool	grasp_stretch(const ivEvent&, Tool&,ivCoord, ivCoord) { return true; }
  bool	manip_stretch(const ivEvent&, Tool&,ivCoord, ivCoord, ivCoord, ivCoord,
    ivCoord, ivCoord) { return true; }
  bool	effect_stretch(const ivEvent&, Tool&,ivCoord,ivCoord, ivCoord, ivCoord)
    { return true; }

  TextData_G(int lno, DA_GridViewSpec* sp, DataTable_G* dg, const ivFont* , const ivColor* ,
		 const char*, ivTransformer*);
  GRAPHIC_BASEFUNS(TextData_G);
};

class Block_G : public Polygon {
  // represents a colored block data element
public:
  int 			line_no; // line of data to represent (FROM END), -1 = template
  DA_GridViewSpec* 	daspec;
  DataTable_G*		dtg;
  TDCoord		move_pos;
  bool			editb_used; // the edit-button was used (right mouse button)

  virtual void	Build();
  virtual void	Position();

  virtual void 	SetPoints(ivCoord x0,ivCoord y0, ivCoord x1, ivCoord y1,
			ivCoord x2,ivCoord y2, ivCoord x3, ivCoord y3);

  bool  select(const ivEvent& e, Tool& tool, bool unselect);
  bool	effect_select(bool set_select);

  void 	draw_gs (ivCanvas* c, Graphic* gs);
  bool	update_from_state(ivCanvas* c);

  virtual void render_text(ivCanvas* c, ScaleBar* cbar, float val, String& str,
			   FloatTwoDCoord& ll, FloatTwoDCoord& ur);
  virtual void render_color(ivCanvas* c, ScaleBar* cbar, float val, 
			    FloatTwoDCoord& ll, FloatTwoDCoord& ur);
  virtual void render_area(ivCanvas* c, ScaleBar* cbar, float val,
			   FloatTwoDCoord& ll, FloatTwoDCoord& ur);
  virtual void render_linear(ivCanvas* c, ScaleBar* cbar, float val,
			     FloatTwoDCoord& ll, FloatTwoDCoord& ur);

  bool	grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord);
  bool	manip_move(const ivEvent& ,Tool&,ivCoord,ivCoord,ivCoord,
		   ivCoord,ivCoord,ivCoord);
  bool 	effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
		    ivCoord iy, ivCoord fx, ivCoord fy);

  bool	grasp_stretch(const ivEvent&, Tool&,ivCoord, ivCoord) { return true; }
  bool	manip_stretch(const ivEvent&, Tool&,ivCoord, ivCoord, ivCoord, ivCoord,
    ivCoord, ivCoord) { return true; }
  bool	effect_stretch(const ivEvent&, Tool&,ivCoord,ivCoord, ivCoord, ivCoord)
    { return true; }

  ivGlyph*	clone() const;
  Block_G(int lno, DA_GridViewSpec* sp, DataTable_G* dg);
  virtual ~Block_G();
  GRAPHIC_BASEFUNS(Block_G);
};

class Grid_G : public Block_G {
  // represents a grid data element (colored squares)
public:
  DT_GridViewSpec* 	dtspec;

  virtual void	Build();
  virtual void	Position();

  bool  select(const ivEvent& e, Tool& tool, bool unselect);
  bool	effect_select(bool set_select);

  void 	draw_gs (ivCanvas* c, Graphic* gs);
  bool	update_from_state(ivCanvas* c);

  bool	grasp_move(const ivEvent&,Tool&,ivCoord, ivCoord);
  bool	manip_move(const ivEvent& ,Tool&,ivCoord,ivCoord,ivCoord,
		   ivCoord,ivCoord,ivCoord);
  bool 	effect_move(const ivEvent& ev, Tool& tl, ivCoord ix,
		    ivCoord iy, ivCoord fx, ivCoord fy);

  bool	grasp_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy);
  bool	manip_stretch(const ivEvent&, Tool&,ivCoord ix, ivCoord iy, ivCoord lx, ivCoord ly,
    ivCoord cx, ivCoord cy);
  bool	effect_stretch(const ivEvent&, Tool&,ivCoord ix,ivCoord iy, ivCoord fx, ivCoord fy);

  ivGlyph*	clone() const;
  Grid_G(int lno, DT_GridViewSpec* sp, DataTable_G* dg);
  virtual ~Grid_G();
  GRAPHIC_BASEFUNS(Grid_G);
};

class DTViewer : public GlyphViewer {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS viewer for DataTable
public:
  void 		press(const ivEvent &);
  void 		release(const ivEvent &);
  void 		init_graphic() 		{ initgraphic();}
  void 		allocate(ivCanvas* c, const ivAllocation& a, ivExtension& ext);
  void		pick(ivCanvas* c, const ivAllocation& a, int depth, ivHit& h);
  DTViewer(float w, float h, const ivColor* bg);
};

class ivGlyph;			// #IGNORE
class tbScrollBox;		// #IGNORE
class ivPolyGlyph;		// #IGNORE
class ivPatch;			// #IGNORE
class HCScaleBar;		// #IGNORE
class ColorScale;		// #IGNORE
class ivButton;			// #IGNORE
class HiLightButton;		// #IGNORE
class taivMenu;			// #IGNORE
class ivInputHandler;		// #IGNORE
class ivDeck;			// #IGNORE

class DTEditor {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS
  // DT editor: contains all the junk for making a datatable editor, not part of glyph hierarchy
public:
  DTViewer* 		viewer;
  DataTable_G*		dtg;
  ivWindow*		win;
  DataTable* 		dt;
  DT_GridViewSpec*	dtvsp;
  
  MinMaxInt 		view_range; 	// range in the datatable itself for what to view
  int			disp_lines; 	// actual number of viewable lines in the display (computed)
  DT_GridViewSpec::BlockFill fill_type;	// how to display block value
  int			block_size; 	// *maximum* block size -- blocks will be smaller if needed to fit
  int			block_border_size; // how much of a border to put around blocks
  bool			header_on; 	// whether to display header or not
  bool			auto_scale; 	// whether to auto-scale based on data or not
  FontSpec	  	view_font;	// the font to use for the labels in the display
  MinMax		scale_range; 	// current range of scale
  
  ivGlyph*	  	body;		// the entire contents of window
  taivMenu*		vtextmenu; 	// value text menu
  taivMenu*		dispmdmenu; 	// display mode menu

  ivPatch*	  	print_patch;	// encapsulates the printable region of viewer
  ivInputHandler* 	data;		// enables fieldeditors

  ColorScale*		scale;		// internal colorscale
  HCScaleBar* 		cbar;		// colorbar at bottom of screen

  virtual void		Init(); 	// initialize
  virtual ivGlyph* 	GetLook();	// gets the whole thing
  
  virtual void	update_from_state();
  virtual void	InitDisplay(); 		// redo the whole display
  virtual void	UpdateDisplay(); 	// update
  virtual void	AddOneLine();	// add one new line to display
  virtual void	ScrollLines();	// update existing boxes to use current line info

  virtual void	CBarAdjustNotify();
  virtual void	UpdateMinMaxScale();	// get display range info!

  virtual void	SetSelect(); 	// sets cursor and tool mode to Select
  virtual void	SetMove();	// sets cursor and tool mode to Move
  virtual void	SetReShape();
  virtual void	SetReScale();
  virtual void	SetRotate();
  virtual void	SetAlter();
		
  virtual void	EditSelections();

  DTEditor(DataTable* d, DT_GridViewSpec* sp, ivWindow* w);
  virtual ~DTEditor();
};

#endif // datatable_iv_h



