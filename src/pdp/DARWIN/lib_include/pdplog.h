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

#ifndef pdplog_h
#define pdplog_h

#include <pdp/process.h>
#include <ta_misc/datatable.h>
#include <ta_misc/ta_file.h>
#include <ta_misc/fontspec.h>

class LogView;
class GridLogView;
class GraphLogView;
class TextLogView;
class NetLogView;

class PDPLog : public WinMgr {
  // ##EXT_log Records data from processes, displaying and saving it
public:
  taFile	log_file;	// optional file for saving
  int		log_lines;	// #NO_SAVE number of lines in the log
  LogData	log_data;	// #NO_SAVE #HIDDEN our own log data for reading fm file

  DataTable	data;		// data for the log
  int		data_bufsz;	// how big a data buffer size to keep
  float		data_shift;	// percentage to shift buffer upon overflow
  MinMaxInt	data_range;	// #NO_SAVE range of lines in the data buffer (in log lines)
  bool		record_proc_name; // whether to record process name in log file or not

  Process_Group log_proc;	// #LINK_GROUP processes which use this log
  SchedProcess*	cur_proc;	// #READ_ONLY #NO_SAVE current process sending to log

  String_Array	display_labels;	// ordered list of labels to use for views and log files

  virtual void 	GetHeaders(bool keep_display_settings = true);
  // #MENU #MENU_ON_Actions Get header information (list of data to display) from processes and clear out all current data (except save display info if checked)
  virtual void	SetSaveFile(const char* nm = NULL, bool no_dlg = false);
  // #MENU #MENU_ON_LogFile #ARGC_0 open file to save log data to (overwrite existing file)
  virtual void	SetAppendFile(const char* nm = NULL, bool no_dlg = false);
  // #MENU #ARGC_0 open file to append log data to
  virtual void	LoadFile(const char* nm = NULL, bool no_dlg = false);
  // #MENU #ARGC_0 read in existing log file data
  virtual void	CloseFile();
  // #MENU #MENU_SEP_AFTER close any open files being logged to
  virtual void	BufferToFile(const char* nm = NULL, bool no_dlg = false);
  // #MENU #ARGC_0 send current buffer of data to file: if null args and already open, to it, else opens a new file sends header and closes after

  virtual void	HeadToFile();
  // send current header to currently open log file -- if not already open, quietly fails

  virtual void 	Buffer_F();	// forward by data_shift
  virtual void 	Buffer_R();	// rewind by data_shift
  virtual void 	Buffer_FF();	// forward to end
  virtual void 	Buffer_FR();	// rewind to begining

  virtual void	Clear();	// clears out the data 
  virtual void	UpdateViewHeaders();           // update headers for all views

  virtual void	AddUpdater(SchedProcess* updt_proc);
  /* #NEW_FUN #MENU #MENU_ON_Object #MENU_SEP_BEFORE
     Tell updt_proc to update this log with new log data as the process runs */
  virtual void	RemoveUpdater(SchedProcess* updt_proc);
  // #MENU #MENU_ON_Object #FROM_GROUP_log_proc remove given updating process
  virtual void	RemoveAllUpdaters(); 		// remove this from all process logs
  virtual void	SyncLogViewUpdaters(); 		// #IGNORE 

  // for processes communicating stuff to the log (and log acting on this)
  virtual void  NewData(LogData& ld, SchedProcess* sproc);
  // This is the primary call to make when sending data to the log
  virtual void	NewHead(LogData& ld, SchedProcess* sproc);
  // #IGNORE use this to specifically update the field headers without updating data
  virtual void	HeadToLogFile(LogData& ld);	// #IGNORE
  virtual void	DataToLogFile(LogData& ld);	// #IGNORE
  virtual void	HeadToBuffer(LogData& ld);	// #IGNORE
  virtual void	DataToBuffer(LogData& ld);	// #IGNORE

  // for reading log files from disk to buffer
  virtual int 	LogLineType(char* lnc);		// #IGNORE determine type of given line
  virtual int  	FileScanTo(istream& strm, int log_ln); // #IGNORE scan to log_ln no
  virtual int	GetFileLength();		// #IGNORE gets length of current file
  virtual void	HeadFromLogFile(String& hdln);	// #IGNORE put log header into buffer
  virtual void	DataFromLogFile(String& dtln);	// #IGNORE put log data into buffer
  virtual void	ReadNewLogFile();		// #IGNORE a new log file was opened, read it
  virtual void	LogFileToBuffer();		// #IGNORE read current data_range into buffer
  virtual void 	InitFile();			// #IGNORE initialize log_file object

  // helper routines to set header/data for buffer field
  virtual void	SetFieldHead(DataItem* ditem, DataTable* dat, int idx); // #IGNORE
  virtual void	SetFieldData(LogData& ld, int ldi, DataItem* ditem, DataTable* dat, int idx); // #IGNORE
  static ostream& LogColumn(ostream& strm, String& str, int tabs);
  // #IGNORE output one column of data (string str), tabs is width (1 or 2)

  // for maintaining an internal version of the buffer format in log-data terms
  virtual DataItem* DataItemFromDataArray(DataArray_impl* da); 	   // #IGNORE
  virtual void 	LogDataFromDataTable(DataTable* dt, int st_idx=0); // #IGNORE
  virtual void	LogDataFromBuffer();	// #IGNORE get log data records from current buffer

  // functions for manually updating logs in ad-hoc manner
  virtual void	ViewAllData();		// make views display all available data

  TypeDef*	GetDefaultView()	{ return &TA_LogView; }

  int	Dump_Save_Value(ostream& strm, TAPtr par, int indent); // check for open log_file..

  const ivColor* GetEditColor() { return pdpMisc::GetObjColor(GET_MY_OWNER(Project),&TA_PDPLog); }

  void	UpdateAfterEdit();
  void	Initialize();
  void 	Destroy();
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const PDPLog& cp); 
  COPY_FUNS(PDPLog, WinMgr);
  TA_BASEFUNS(PDPLog);
};

BaseGroup_of(PDPLog);

class PDPLog_MGroup : public PDPMGroup {
public:
  static bool nw_itm_def_arg;	// #IGNORE default arg val for FindMake..

  virtual PDPLog* FindMakeLog(const char* nm, TypeDef* td, bool& nw_itm = nw_itm_def_arg);

  const ivColor* GetEditColor() { return pdpMisc::GetObjColor(GET_MY_OWNER(Project),&TA_PDPLog); }

  void	Initialize() 		{ SetBaseType(&TA_PDPLog); }
  void 	Destroy()		{ };
  TA_BASEFUNS(PDPLog_MGroup);
};

class TextLog : public PDPLog {
  // log with textview as a default display
public:
  TypeDef*	GetDefaultView()	{ return &TA_TextLogView; }
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(TextLog);
};

class NetLog: public TextLog {
  // log with netlog as a default display
public:
  virtual void	SetNetwork(Network* net);
  // #MENU #MENU_ON_Object select given network as the one to update views on

  TypeDef*	GetDefaultView()	{ return &TA_NetLogView; }
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(NetLog);
};
  
class GraphLog : public TextLog {
  // log with graph as a default display
public:
  TypeDef*	GetDefaultView()	{ return &TA_GraphLogView; }
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(GraphLog);
};

class GridLog : public TextLog {
  // log with grid as a default display
public:
  TypeDef*	GetDefaultView()	{ return &TA_GridLogView; }
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(GridLog);
};

class ivLayoutKit;		// #IGNORE
class ivWidgetKit;		// #IGNORE
class ivButton;			// #IGNORE

class LogView : public PDPView {
  // parent class of all log views
public:
  int		view_bufsz;	// Maximum number of lines in visible buffer
  float		view_shift;	// precentage of buffer to shift
  MinMaxInt	view_range;	// #NO_SAVE range of visible lines (in log lines)
  DT_ViewSpec*	viewspec;	// #NO_SAVE global view spec for all data 

  DT_ViewSpec 	viewspecs;	// #HIDDEN specification for display of datatable
  PDPLog* 	log_mgr;	// #READ_ONLY #NO_SAVE Parent Log

  // graphical implementation stuff
  ivButton*	disp_tog_but;	// #IGNORE display toggle button
  ivButton*	frev_but;	// #IGNORE full rev
  ivButton*	fsrev_but;	// #IGNORE fast rev
  ivButton*	rev_but;	// #IGNORE rev
  ivButton*	fwd_but;	// #IGNORE fwd
  ivButton*	fsfwd_but;	// #IGNORE fast fwd
  ivButton*	ffwd_but;	// #IGNORE full fwd
  ivButton*	clear_but;	// #IGNORE 
  ivButton*	init_but;	// #IGNORE 
  ivButton*	update_but;	// #IGNORE 

  // log interface
  virtual void  NewHead(); // new header information is in log data
  virtual void  NewData(){ }; // new data information is in log's datatable

  // relation to the buffer
  virtual void	UpdateFromBuffer() { }; // update view from buffer

  // view control
  virtual void 	View_F();	// forward by view_shift
  virtual void 	View_R();	// rewind by view_shift
  virtual void 	View_FSF();	// fast forward 
  virtual void 	View_FSR();	// fast rewind 
  virtual void 	View_FF();	// forward to end
  virtual void 	View_FR();	// rewind to begining

  virtual void 	Clear();	// Clear the display and the data
  virtual void	Clear_impl();

  void	InitDisplay();
  void  UpdateDisplay(TAPtr updtr=NULL);
  void	UpdateCallback()	{ UpdateDisplay(NULL); } // #IGNORE

  virtual void 	ToggleDisplay();
  virtual void 	SetToggle(bool value);

  virtual void	Reset() { };	// just clear the display, not the data..

  void		EditViewSpec(taBase* column);
  // #MENU #MENU_ON_Actions #MENU_SEP_BEFORE #FROM_GROUP_viewspec edit view specs for given column
  void		SetVisibility(taBase* column, bool visible);
  // #MENU #FROM_GROUP_viewspec set whether this column of data is displayed in the log display
  void		SetLogging(taBase* column, bool log_data, bool also_chg_vis = true);
  // #MENU #FROM_GROUP_viewspec set whether this column of data is logged to a file (if also_chg_vis, then visibility in view is changed to match logging)
  void		UpdateDispLabels();
  // #MENU #CONFIRM update the display labels in the view using the display_labels stored in the log object

  // following are analysis routines that copy log data to a temporary environment
  // and then run the associated function on that environment

  virtual void 	CopyToEnv(taBase* data, taBase* labels, Environment* env);
  // #MENU #MENU_ON_Analyze #FROM_GROUP_1_viewspec #NULL_OK output data (must be group) with labels to environment env (NULL = new env). WARNING: reformats env to fit data!)
  virtual void	DistMatrixGrid(taBase* data, taBase* labels, GridLog* disp_log, float_RArray::DistMetric metric=float_RArray::HAMMING,
			   bool norm=false, float tol=0.0f);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec output to grid log (NULL=make new one) distance matrix for data (must be group) with labels
  virtual void	ClusterPlot(taBase* data, taBase* labels, GraphLog* disp_log,
			    float_RArray::DistMetric metric=float_RArray::EUCLIDIAN,
			    bool norm=false, float tol=0.0f);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec produce a cluster plot (in graph log, NULL = make a new one) of the given data (with labels)
  virtual void	CorrelMatrixGrid(taBase* data, taBase* labels, GridLog* disp_log);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec produce a correlation matrix of given data and plot in given grid log (NULL = make new one)
  virtual void	PCAEigenGrid(taBase* data, taBase* labels, GridLog* disp_log);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec plot eigenvectors of correlation matrix (principal components analysis = PCA) of given data in given grid log (NULL = make new one)
  virtual void	PCAPrjnPlot(taBase* data, taBase* labels, GraphLog* disp_log, int x_axis_component=0, int y_axis_component=1);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec plot projections of data onto two principal components (eigenvectors) into a graph log (NULL = make new one)
  virtual void	MDSPrjnPlot(taBase* data, taBase* labels, GraphLog* disp_log, int x_axis_component=0, int y_axis_component=1);
  // #MENU #NULL_OK #FROM_GROUP_1_viewspec plot multidimensional scaling of distance matrix of data onto two components into a graph log (NULL = make new one)

  virtual void		CreateViewSpec(); // #IGNORE create view spec of appropriate type
  virtual TypeDef*	DT_ViewSpecType(); // #IGNORE get type of DataTable for this log
  virtual TypeDef*	DA_ViewSpecType(); // #IGNORE get type of DataArray for this log

  // gui implementation stuff
  virtual void		CreateLogButtons(); // #IGNORE just make them
  virtual ivGlyph*	GetLogButtons(); // #IGNORE create and package

  // winview stuff
  void		AddNotify(TAPtr ud); // #IGNORE
  void	 	RemoveNotify(TAPtr ud);	// #IGNORE
  void  	SetWinName();
  void		CloseWindow();
  ivGlyph*	GetPrintData();	// #IGNORE

  const ivColor* GetEditColor() { return pdpMisc::GetObjColor(GET_MY_OWNER(Project),&TA_PDPLog); }

  void 	UpdateAfterEdit();
  void 	Initialize();
  void 	Destroy()	{ CutLinks(); }
  void 	InitLinks();
  void	CutLinks();
  void	Copy_(const LogView& cp);
  COPY_FUNS(LogView, PDPView);
  TA_BASEFUNS(LogView);
};

class ivPolyGlyph; // #IGNORE
class ivGlyph; // #IGNORE
class ivPatch; // #IGNORE

class TextLogView : public LogView {
  // a textual view of the log data
public:
  ivPolyGlyph*  sb;		// #IGNORE scrollbox for screenlog
  ivGlyph*	h_line;		// #IGNORE headerline for screenlog
  ivPatch*	patch;		// #IGNORE update patch;
  ivPatch*	scrb;		// #IGNORE update scrollable;
  ivPolyGlyph*	vbox;		// #IGNORE vbox with headerline and scrollbox
  ivPolyGlyph*	ovbox;		// #IGNORE outer vbox with buttons and vbox
  float		font_width;	// #IGNORE width of font spacing
  ivPolyGlyph*  cur_line;	// #IGNORE current Line we are creating

  void  		NewHead();
  void  		NewData();

  float		GetFieldWidth(DA_TextViewSpec* vs);// determines width of a field
  void		MakeLine(int from_end = 0); // creates a data line 
  void		UpdateFromBuffer(); 
  
  void		Reset();
  void		Clear_impl();

  int		ShowMore(int newsize); // #IGNORE
  int		ShowLess(int newsize); // #IGNORE

  TypeDef*	DT_ViewSpecType(); // #IGNORE get type of DataTable for this log
  TypeDef*	DA_ViewSpecType(); // #IGNORE get type of DataArray for this log

  // winview stuff
  void 		GetBodyRep();	// #IGNORE
  ivGlyph*	GetPrintData();	// #IGNORE

  void		UpdateDisplay(TAPtr updtr=NULL);
  void 		InitDisplay();
  void 		CloseWindow();

  // gui implementation stuff
  virtual void	AddToLine(const char* name, float width); // add name of width to line
  virtual void	AddLine();			  // add line to scrollbox
  virtual void  RemoveLine(int index);		  // remove line from scrollbox, -1 = all

  virtual void  StartHeaderLine();
  virtual void  AddToHeaderLine(const char* name, float width);
  virtual void  EndHeaderLine();
  virtual void	FixSize();

  void 	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  TA_BASEFUNS(TextLogView);
};

class NetLogView : public LogView {
  // displays log information in the network view window
public:
  Network*      network;		// Network to whose views  data is logged

  virtual void	SetNetwork(Network* net);
  // #MENU #MENU_ON_Actions #MENU_SEP_BEFORE select given network as the one to update views on
  virtual void  ArrangeLabels(int cols = -1, int rows = -1, int width = 12, float left = 0.0, float top = .9);
  // #MENU arrange the display labels in the netview in a grid of rows and cols starting at given top-left position (as fraction of display size) -1 = auto-compute based on size
  virtual void	RemoveLabels();	// remove all my labels from the netview

  void 		NewHead();
  void 		NewData();
  void		UpdateDisplay(TAPtr updtr=NULL);
  void 		InitDisplay();
  void		UpdateFromBuffer(); 
  void 		UpdateAfterEdit();
  TypeDef*	DA_ViewSpecType(); // #IGNORE get type of DataArray for this log

  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void	InitLinks();
  void	CutLinks();
  TA_BASEFUNS(NetLogView);
};

class DTEditor;		// #IGNORE

class GridLogView : public LogView {
  // displays log information as colored grids
public:
  DT_GridViewSpec::BlockFill	fill_type;
  // how the grid blocks are filled in to display their value
  ColorScaleSpec* colorspec; 	// The color spectrum for this display
  int		block_size;	// *maximum* block size -- blocks will be smaller if needed to fit
  int		block_border_size; // size of the border around the blocks
  bool		header_on;	// is the log header visible?
  bool		auto_scale;	// whether to auto-scale on values or not
  FontSpec	view_font;	// the font to use for the labels in the display
  MinMax        scale_range;	// #HIDDEN range of scalebar
  MinMaxInt	actual_range;	// #HIDDEN #NO_SAVE range in actual lines of data
  
  // gui implementation
  DTEditor* 	editor;		// #IGNORE
  ivButton*	auto_sc_but;	// #IGNORE toggle auto-scale button
  ivButton*	head_tog_but;	// #IGNORE header toggle button
  ivAction*	adjuster;	// #IGNORE callback for the auto-scale button action

  void  	NewHead();
  void  	NewData();
  void		UpdateFromBuffer();
  void 		Clear_impl();
  
  TypeDef*	DT_ViewSpecType(); // #IGNORE get type of DataTable for this log
  TypeDef*	DA_ViewSpecType(); // #IGNORE get type of DataArray for this log
  
  // winview stuff
  void 		GetBodyRep();		// #IGNORE
  void		UpdateDisplay(TAPtr updtr=NULL);
  void 		InitDisplay();
  void 		CloseWindow();
  ivGlyph*	GetPrintData();	// #IGNORE
  
  virtual void 	SetColorSpec(ColorScaleSpec* colors);
  // #MENU #MENU_ON_Actions #NULL_OK #MENU_SEP_BEFORE set the color spectrum to use for color-coding values (NULL = use default)
  virtual void	SetBlockFill(DT_GridViewSpec::BlockFill fill_typ=DT_GridViewSpec::COLOR);
  // #MENU set the fill style of the grid blocks
  virtual void	SetBlockSizes(int block_sz = 8, int border_sz = 1);
  // #MENU set the MAXIMUM sizes of the blocks (could be smaller), and the border space between blocks
  virtual void	UpdateGridLayout(DT_GridViewSpec::MatrixLayout grid_layout=DT_GridViewSpec::LFT_RGT_BOT_TOP);
  // #MENU #ARGC_1 arrange columns to fit without gaps, according to geometry

  virtual void	SetViewFontSize(int point_size = 10);
  // #MENU #MENU_SEP_BEFORE set the point size of the font used for labels in the display
  virtual void	AllBlockTextOn();
  // #MENU turn text on for all block displayed items
  virtual void	AllBlockTextOff();
  // #MENU turn text off for all block displayed items

  virtual void	ToggleHeader();  // toggle header on or off
  virtual void	ToggleAutoScale();  // toggle header on or off

  // gui implementation
  void		CreateLogButtons(); // #IGNORE
  ivGlyph*	GetLogButtons(); // #IGNORE

  void 	UpdateAfterEdit();
  void  InitLinks();
  void  CutLinks();
  void	Initialize();
  void 	Destroy();
  void	Copy_(const GridLogView& cp);
  COPY_FUNS(GridLogView, LogView);
  TA_BASEFUNS(GridLogView);
};

class GraphEditor;		// #IGNORE

class GraphLogViewLabel : public ViewLabel {
  // view labels for graph logs
public:
  void		GetMasterViewer();

  void	Initialize()	{ };
  void	Destroy()	{ };
  TA_BASEFUNS(GraphLogViewLabel);
};

class GraphLogView : public LogView {
  // View log data as a graph of lines
public:
  int			x_axis_index;	// index of index in array
  GraphEditor*		editor;		// #IGNORE handles all the display stuff
  ViewLabel_List	labels;		// misc labels in the graph view
  MinMaxInt		actual_range;	// #HIDDEN #NO_SAVE range in actual lines of data
  ColorScaleSpec* 	colorspec; 	// The color spectrum for this display (for TRACE_COLOR or VALUE_COLOR line displays)
  bool			separate_graphs; // draw each group of lines sharing a Y axis using separate graphs
  PosTwoDCoord		graph_layout; 	// arrangement of graphs for separate graphs
  bool			anim_stop; 	// #IGNORE

  virtual void	Animate(int msec_per_point = 500);
  // #MENU #MENU_ON_Actions #MENU_SEP_BEFORE animate the display by incrementally displaying each new row of data, waiting given amount of time between points.  This will capture images to animation files if anim.capture is on.
  virtual void	StopAnimate();
  // #MENU stop the animation!

  virtual void 	SetColorSpec(ColorScaleSpec* colors);
  // #MENU #MENU_ON_Settings #NULL_OK set the color spectrum to use for color-coding values (NULL = use default)
  virtual void 	SetBackground(RGBA background);
  // #MENU set the display's background to the given color
  virtual void	UpdateLineFeatures(); 
  // #MENU #CONFIRM update color, line type, point type, etc of lines in accordance with the current settings for the ordering of these features.  Only visible lines are updated
  virtual void	SetLineFeatures
  (DT_GraphViewSpec::ColorType color_type,
   DT_GraphViewSpec::SequenceType sequence1=DT_GraphViewSpec::COLORS,
   DT_GraphViewSpec::SequenceType sequence2=DT_GraphViewSpec::LINES,
   DT_GraphViewSpec::SequenceType sequence3=DT_GraphViewSpec::POINTS,
   bool update_only_visible=true);
  // #MENU set color, line type, point type of lines by cycling through values

  virtual void	SetLineWidths(float line_width);
  // #MENU #MENU_SEP_BEFORE set the widths of all lines in the graph to given value
  virtual void	SetLineType(DA_GraphViewSpec::LineType line_type);
  // #MENU set all line types to given type

  virtual void	ShareAxisAfter(taBase* axis_var);
  // #MENU #MENU_SEP_BEFORE #FROM_GROUP_viewspec make all displayed variables after given axis_var share Y axis with axis_var
  virtual void	ShareAxes();
  // #MENU #CONFIRM make all groups of columns share the same Y axis
  virtual void	SeparateAxes();
  // #MENU #CONFIRM each column of data gets its own Y axis
  virtual void	PlotRows();
  // #MENU #CONFIRM plot the data across rows of grouped columns (x axis = column index, y axis = values for each column in one row) instead of down the columns
  virtual void	PlotCols();
  // #MENU #CONFIRM plot the data down columns (standard mode)

  virtual void	SeparateGraphs(int x_layout, int y_layout);
  // #MENU #MENU_SEP_BEFORE draw each group of lines sharing the same Y axis using separate graphs, with the given layout of graphs in the display
  virtual void	OneGraph();
  // #MENU #CONFIRM draw all data in one graph (default mode)

  virtual void	TraceIncrement(float x_increment = -2.0f, float y_increment = 2.0f);
  // #MENU #MENU_SEP_BEFORE each subsequent trace of data (pass through the same X axis values) is incremented by given amount, producing a 3D-like effect
  virtual void	StackTraces();
  // #MENU #CONFIRM arrange subsequent traces of data (pass through the same X axis values) in non-overlapping vertically-arranged stacks
  virtual void	UnStackTraces();
  // #MENU #CONFIRM subsequent traces of data (pass through the same X axis values) are plotted overlapping on top of each other
  virtual void	StackSharedAxes();
  // #MENU #CONFIRM arrange lines that share the same Y axis in non-overlapping vertically-arranged stacks
  virtual void	UnStackSharedAxes();
  // #MENU #CONFIRM lines that share the same Y axis are plotted overlapping on top of each other

  virtual void	SpikeRaster(float thresh);
  // #MENU #MENU_SEP_BEFORE display spike rasters with given threshold for cutting a spike (trace_incr.y = 1, vertical = NO_VERTICAL, line_type = TRESH_POINTS)
  virtual void	ColorRaster();
  // #MENU display values as rasters of lines colored according to the values of the lines
  virtual void	StandardLines();
  // #MENU get rid of raster-style display and return to 'standard' graph line display

  void  	NewHead();
  void  	NewData();
  void		UpdateFromBuffer();
  void 		Clear_impl();

  virtual int	SetXAxis(char* nm); // set the x axis to be this field
  virtual void	UpdateViewRange();  // update view range info from log

  TypeDef*	DT_ViewSpecType(); // #IGNORE get type of DataTable for this log
  TypeDef*	DA_ViewSpecType(); // #IGNORE get type of DataArray for this log

  void		InitColors();
  void 		GetBodyRep();	// #IGNORE
  void		CloseWindow();
  void 		UpdateDisplay(TAPtr updtr=NULL);
  void 		InitDisplay();

  ivGlyph*	GetPrintData();	// #IGNORE

  void 	UpdateAfterEdit();
  void	Initialize();
  void  Destroy();
  void	CutLinks();
  void 	InitLinks();
  void	Copy_(const GraphLogView& cp);
  COPY_FUNS(GraphLogView, LogView);
  TA_BASEFUNS(GraphLogView);
};

#endif // pdplog.h

