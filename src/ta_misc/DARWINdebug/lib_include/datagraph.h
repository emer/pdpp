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

#ifndef datagraph_h
#define datagraph_h

#include <ta/taiv_type.h>
#include <ta_misc/datatable.h>
#include <ta_misc/colorscale.h>
#include <ta_misc/colorbar.h>
#include <ta_misc/fontspec.h>

#include <ta/enter_iv.h>
#include <InterViews/monoglyph.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <IV-look/kit.h>
#include <OS/list.h>
#include <ta/leave_iv.h>

#include <iv_graphic/graphic.h>
#include <iv_graphic/graphic_viewer.h>

class ivBrush;
class ivColor;
class ivStyle;
class ivTransformer;
class ivWindow;
class ivTelltaleGroup;
class ivButton;
class ivAction;
class ivDialogKit;
class ivDeck;

class Axis;
class Grid;
class Grapher_G;		// the graphicmaster
class GraphButton;
class GraphGraph;
class GraphEditor;		// the overall editor

class GraphLine : public Graphic {
public:
  // these should be coordinated with those on GraphViewSpec
  enum LineType {
    LINE,			// just a line, no pts
    POINTS,			// just pts, no line
    LINE_AND_POINTS, 		// both
    STRINGS,			// string (text) values -- no lines
    TRACE_COLORS,		// subsequent traces (repeats through same X values) are color coded using colorscale
    VALUE_COLORS,		// Y values are represented with colors using colorscale
    THRESH_POINTS		// Y values are thresholded with thresh parameter and displayed as points when above threshold
  };
  enum LineStyle {
    SOLID,			// -----
    DOT,			// .....
    DASH,			// - - -
    DASH_DOT			// _._._
  };
  enum PointStyle {
    NONE,			//
    SMALL_DOT,			// .
    BIG_DOT,			// o
    TICK,			// |
    PLUS			// +
  };
  enum VerticalType {
    FULL_VERTICAL,		// use the full vertical axis for displaying Y values
    STACK_TRACES,		// stack multiple traces (repeats through same X values) on top of each other within the vertical axis
    NO_VERTICAL			// don't draw any vertical dimension at all (for COLORS or THESH_POINTS line_style)
  };
  enum SharedYType {
    OVERLAY_LINES,		// overlay multiple graph lines sharing same Y axis
    STACK_LINES			// stack multiple graph lines sharing same Y axis on top of each other
  };

  GraphEditor*	editor;
  GraphButton*	button;		// our graph button
  Axis*		axis;		// our axis
  Axis*		x_axis;		// the x_axis
  float_Array	data_x;		// original data values, x coords; ctrlpts are transformed by display
  float_Array	data_y;		// original data values, y coords; ctrlpts are transformed by display
  String_Array	string_vals;	// if plotting string values, these are the "values"
  int_Array	coord_points;	// indicies of points to plot coordinate labels of
  int_Array	trace_vals;	// trace coordinates: a trace is one contiguous line along X axis, broken when X resets to an earlier value
  ivBrush*	width0_brush;	// brush of width zero

  LineType	line_type;	// The way the line is drawn
  LineStyle	line_style;	// The style in which the line is drawn
  float		line_width;	// width of the line
  PointStyle	point_style;	// The style in which the points are drawn
  Modulo	point_mod;      // When to skip the drawing of point
  bool		negative_draw;	// Draw lines in the negative direction?
  VerticalType	vertical;	// what to do with the vertical Y axis 
  SharedYType 	shared_y;	// what to do with multiple graph lines that share the Y axis
  FloatTwoDCoord trace_incr; 	// increments in starting coordinates for each subsequent trace of a line across the same X axis values
  float		thresh;		// threshold for THRESH_POINTS line style

  int		last_coord;	// #IGNORE last coordinate selected to view
  float		init_mv_x;	// #IGNORE initial move x coord
  float		init_mv_y;	// #IGNORE initial move y coord
  int		n_traces;	// #IGNORE number of traces through same X points
  int		n_shared;	// #IGNORE number of lines sharing same Y axis
  int		share_i;	// #IGNORE index within set of shared Y axis points
  FloatTwoDCoord esc;		// #IGNORE extra scaling of lines (from stacking, trace_incr)
  FloatTwoDCoord eoff;		// #IGNORE extra offsets of lines (from stacking, trace_incr)
  ivCoord	plwd;		// #IGNORE one line width, in data coordinates
  ivCoord	plht;		// #IGNORE one line height, in data coordinates

  virtual void	Reset();	// clear existing points
  virtual bool	DrawLastPt(ivCanvas* c); // draw (damage) last point added to line: true if needs full redraw
  virtual void	Rescale();	// set xform so line just fits within display
  virtual bool	GetEscEoff(ivCanvas* c); // get the extra scaling and offsets: true if new
  virtual void	RenderPoint(ivCanvas* c, ivCoord xv, ivCoord yv,
			    const ivColor* stroke, const ivBrush* b);
  virtual void	RenderText(ivCanvas* c, ivCoord xv, ivCoord yv, String& str,
			   const ivColor* stroke, bool keep_vis = false);

  virtual const ivColor* GetValueColor(float val);	// get color from value
  virtual const ivColor* GetTraceColor(int trace);	// get color from trace count

  virtual void	AddPoint(ivCoord x, ivCoord y);
  // use this interface instead of add_point to add points to the line: updates relevant stuff
  void		draw_gs(ivCanvas* c, Graphic* gs);
  void 		drawclipped_gs(ivCanvas* c, ivCoord l, ivCoord b,
			       ivCoord r, ivCoord t, Graphic* gs);

  bool		grasp_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy);
  bool		manip_move(const ivEvent&, Tool&,ivCoord ix, ivCoord iy,
			   ivCoord lx, ivCoord ly,
			   ivCoord cx, ivCoord cy);
  bool		effect_move(const ivEvent&, Tool&,ivCoord ix,
			    ivCoord iy, ivCoord fx, ivCoord fy);

  bool		grasp_scale(const ivEvent&, Tool&,ivCoord,ivCoord,ivCoord,ivCoord,
			    ivCoord,ivCoord) {return true;}
  bool		manip_scale(const ivEvent&,Tool&,ivCoord,ivCoord,ivCoord,ivCoord,
			    ivCoord,ivCoord,ivCoord,ivCoord) {return true;}
  bool 		effect_border();

  GraphLine(GraphEditor* ge, const ivBrush* b, GraphButton* but, ivColor* c, 
	    LineType tp=LINE, LineStyle st=SOLID, float lw=1.0f, PointStyle ps=NONE,
	    int mod_off=0, int mod_m=1, bool nd=false, VerticalType vt=FULL_VERTICAL,
	    SharedYType sy=OVERLAY_LINES, float thr=0.5f);
  GraphLine(GraphEditor* ge, const ivBrush* b, GraphButton* but, DA_GraphViewSpec* spec);
  ~GraphLine();
};

class Grapher_G : public GraphicMaster {
  // simply a container for all the graphlines
public:
  GraphEditor*		editor;
  GraphGraph*		graph;

  virtual void		AddLine(GraphLine* gl);
  virtual void		Reset(); // clear all the lines in the graph
  virtual ivColor*	GetTextColor(); // return the background contrast color

  ivAllocation*		GetAllocation()	{ return &_a; }

  Grapher_G(GraphEditor* onr, GraphGraph* gg);
  ~Grapher_G();
};

class GraphViewer : public GlyphViewer {
protected:
  void rate_zoom();
  void grab_scroll();
public:
  GraphEditor* 	editor;
  GraphGraph*	graph;
  ivTransformer* init_xform;	// cached root transform for scaling axes

  GraphicMaster* root() { return GlyphViewer::root(); }
  void root(GraphicMaster*);
  void allocate(ivCanvas*, const ivAllocation&, ivExtension&);

  virtual void 	ReScaleAxes(bool redraw=true);  // updates axes scales based on what is displayed
  virtual void	SetCanvas(ivCanvas* c);
  virtual void 	ClearCachedAllocation();

  GraphViewer(GraphEditor* e, GraphGraph* gg, float w, float h, const ivColor* bg);
  ~GraphViewer();
};

class GraphGraph : public ivPatch {
  // represents one entire graph object, including a viewer, graphicmaster, and axes
public:
  GraphEditor* 		editor;		// owner editor
  DT_GraphViewSpec*	dtvsp;		// the display spec for the data
  GraphViewer* 		viewer;		// viewer controls graph viewing
  Grapher_G*		graphg;		// graphic master graphg

  Axis*			x_axis;		// the x axis
  ivPatch*		x_axis_patch; 	// patch to redraw the x axis
  bool			nodisp_x_axis; 	// do not display x axis

  ivGlyph_List		y_axis;	      	// the y axis(s)
  ivPolyGlyph*		y_axis_box; 	// hbox containing the y axes
  ivPatch*		y_axis_patch; 	// patch for the whole y axis (to redraw)

  ivPatch*		linespace; 	// patch around the viewer (to redraw)

  virtual void		Init();  // generate initial data members
  virtual void		Reset(); // clear out stuff (axes) from any existing graphs
  virtual void		SetXAxis(GraphButton* xbut);  // set x axis with this button as the x axis
  virtual void		AddYAxis(GraphButton* but);  // add a y axis to this graph
  virtual void		GetLook();  // get my own look into my own patch!

  GraphGraph(GraphEditor* ge);
  ~GraphGraph();
};  

class GraphButton : public ivButton {
  // the button is used both as a gui for each column of data, and as a holder for all relevant info about that column
public:
  GraphEditor* 	editor;		// overall editor
  int		field;		// the field/column number (leaf index into dt_viewspec)
  bool		is_string;	// is this a string value instead of a float?
  DA_GraphViewSpec* spec;	// spec for this line
  GraphGraph*	graph;		// which graph do I belong in
  GraphButton*	axis_but;	// button that contains my axis
  Axis*		axis;		// axis for this field
  Axis*		x_axis;		// the x axis
  GraphLine*	line;		// the line for this field
  int		n_shared;	// number of lines sharing same Y axis
  int		n_shared_tot;	// total number of lines sharing axis, including non-visible lines!
  int		share_i;	// index within set of shared Y axis points
  bool		is_shared_axis;	// true if this axis is shared by other axes (must always have an axis of its own, even if not visible)

  int		press_button;	// button actually pressed

  virtual void	InitData();	// reinint all associated data for line

  void		press(const ivEvent& e); // process mouse press
  void		release(const ivEvent& e); // process mouse release..

  ivGlyph*	GetLegend(DA_GraphViewSpec* vspec=NULL);
  virtual void	ToggleLine();	// toggle line on/off
  virtual void	SetXAxis();	// set this one to be the x axis (middle button)

  virtual void	SetAxis(); // set the righthand axis rep appropriately

  bool		IsChosen();	// is this button selected
  
  virtual float_Data* GetYData(String_Data*& str_ar); // get y-axis data array for this column (and string array if string)

  static GraphButton* GetButton(GraphEditor* o, DA_GraphViewSpec* gvs,int f);
  
  GraphButton(ivGlyph* g, ivStyle* s, ivTelltaleState* t,
	      GraphEditor* o, DA_GraphViewSpec* gvs, int f);
  ~GraphButton();
};

class GraphEditor {
  // ##NO_INSTANCE ##NO_MEMBERS ##NO_TOKENS ##NO_CSS
  // graph editor: contains all the junk for making a graph editor, not part of glyph hierarchy
public:
  TAPtr			owner;		// the GraphLogView in general
  int			x_axis_index;	// index of index in array
  bool			plot_rows; 	// if true, plot rows instead of columns
  int_Array		plot_rows_subgps; // indicies of subgroups for plotting the rows

  int			view_bufsz; 	// maximum number of lines of data to view
  MinMaxInt		view_range; 	// range of data to view
  int			last_pt_offset;	// where to get the last point of data from for AddLastPt function (0 = actual last point, higher #'s go further back..)

  ColorScale*		scale;		// internal colorscale
  HCBar* 		cbar;		// colorbar at bottom of screen
  ivDeck*		cbar_deck; 	// deck containing cbar and empty space 
  bool			use_cbar; 	// if true, actually display the cbar (if some kind of colors display used)
  bool			separate_graphs; // draw each group of lines sharing a Y axis using separate graphs
  PosTwoDCoord		graph_layout; 	// arrangement of graphs for separate graphs
  PosTwoDCoord		eff_layout; 	// effective arrangement of graphs for separate graphs
  int			n_graphs; 	// total number of graphs

  ivGlyph_List		buttons; 	// buttons representing fields: one for each da element (column of data)
  ivGlyph_List		graphs;		// the graphs

  GraphGraph*		first_graph; 	// the first graph is always present and is reused (contains labels)
  Grid*			graph_grid; 	// grid of graphs
  ivPatch*		graph_patch; 	// patch around the graph grid
  ivPatch*		boxpatch; 	// this covers the the axi & lines

  float_Data*		x_axis_ar; 	// the data array for the x axis
  float_RArray		rows_x_axis; 	// x axis values for plot rows mode

  DataTable*		data;		// the data to be graphed
  DT_GraphViewSpec*	dtvsp;		// the display spec for the data
  int			last_col_cnt; 	// no. of columns we had last time we updated
  int			last_row_cnt; 	// no. of rows we had (in x axis) last time we updated

  ivWindow*		win;
  ivTelltaleGroup*	tool_gp; 
  ivPolyGlyph* 		fieldbox; 	// box around fields
  ivMonoGlyph*		fieldframe; 	// frame around fields
  ivPatch*		fieldpatch; 	// field button representation
  ivScrollBox*	        field_rep; 

  void			(*SetOwnerXAxis)(TAPtr, int);

  virtual void		Init();		// initialization
  virtual ivGlyph* 	GetLook();	// gets the whole thing (only call this once)
  virtual ivGlyph* 	GetTools();	// get the tool buttons
  virtual ivGlyph* 	GetFields(ivScrollBox* sb);// get the variable buttons

  virtual void		SetAxisFromSpec(Axis* ax, GraphButton* but);
  virtual void		GetGraphs(); 	// get graphs and their axes all setup
  virtual void		UpdateCanvases(ivCanvas* c);

  virtual ivButton*	MEButton(ivTelltaleGroup* gp, char* txt, ivAction* a);
  // get a "pallete button" that is mutually exclusive

  // this is the main user interface
  virtual void		Reset();	// clear everything

  virtual void		InitDisplay();	  // reinit stuff
  virtual void		UpdateDisplay(); // redo the whole display

  virtual void		PlotCols(); 	// build entire display plotting in standard column display mode
  virtual void		PlotRows(); 	// build entire display plotting across a row

  virtual void		AddLastPoint(); // add last point in the table (update)
  virtual void		AddLastPoint_Cols(); // add last point in the table (update) -- cols mode
  virtual void		AddLastPoint_Rows(); // add last point in the table (update) -- rows mode

  virtual void		SetXAxis(int idx); 	// changes x axis
  virtual void		UpdateButtons(); 	// update all buttons!
  virtual void		ToggleLine(int idx); 	// toggles display of given line
  virtual void		InitColors(); 		// initializes the colors (calls updatelinefeatures)

  virtual void		UpdateLabels();	// update display of labels
  virtual ViewLabel*	Create_Label(char* label_name = NULL);	// create a label
  virtual void		Create_Default_Label() { Create_Label(); }
  virtual void		Delete_Labels();	// delete selected labels
  virtual void		Edit_Labels();	// edit selected labels

  GraphEditor(TAPtr own, DataTable* d, DT_GraphViewSpec* dtvs, ivWindow* w);
  virtual ~GraphEditor();
};



#endif // datagraph_h
