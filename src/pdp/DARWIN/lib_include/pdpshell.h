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

// pdpshell.h
  
#ifndef pdpshell_h
#define pdpshell_h 1

#include <pdp/netstru.h>
#include <pdp/enviro.h>
#include <pdp/procs_extra.h>
#include <pdp/pdplog.h>
#include <ta/tdefaults.h>
#include <ta_misc/colorscale.h>
#include <ta_misc/fontspec.h>

class TypeDefault_MGroup : public PDPMGroup {
  // #DEF_PATH_$PDPDIR$/defaults group of type default objects
public:
  int	Dump_Load_Value(istream& strm, TAPtr par=NULL);
  // reset members before loading..
  
  void	Initialize() 		{ SetBaseType(&TA_TypeDefault); }
  void 	Destroy()		{ };
  TA_BASEFUNS(TypeDefault_MGroup);
};

//////////////////////////////////////////////////
//			Wizard			//
//////////////////////////////////////////////////

class LayerWizEl : public taNBase {
  // ##NO_TOKENS #NO_UPDATE_AFTER specifies basic parameters for a layer
public:
  enum InputOutput {
    INPUT,
    HIDDEN,
    OUTPUT
  };

  int		n_units;	// number of units in the layer
  InputOutput 	io_type;	// is it an input, hidden, or output layer -- determines environment patterns

  void	Initialize();
  void	Destroy() 	{ };
  SIMPLE_COPY(LayerWizEl);
  COPY_FUNS(LayerWizEl, taNBase);
  TA_BASEFUNS(LayerWizEl);
};

class Wizard : public taNBase {
  // ##BUTROWS_2 ##EDIT_WIDTH_60 wizard for automating construction of simulation objects
public:
  enum Connectivity {
    FEEDFORWARD,		// each layer projects to the next one in sequence
    BIDIRECTIONAL		// layers are bidirectionally connected in sequence (each sends and receives from its neighbors)
  };

  bool		auto_open;	// open this wizard dialog upon startup
  int		n_layers;	// number of layers
  taBase_List	layer_cfg;	// provides configuration information for each layer
  Connectivity	connectivity;	// how to connect the layers
  TypeDef*	event_type;	// #TYPE_Event type of event to create by default

  virtual void	ThreeLayerNet();
  // #MENU_BUTTON #MENU_ON_Defaults set configuration to a standard three-layer network (input, hidden, output) -- DOESN'T MAKE NETWORK (use StdEnv!)
  virtual void	MultiLayerNet(int n_inputs = 1, int n_hiddens = 1, int n_outputs = 1);
  // #MENU_BUTTON set configuration for specified number of each type of layer -- DOESN'T MAKE NETWORK (use StdEnv!)

  virtual void	StdNetwork(Network* net=NULL);
  // #MENU_BUTTON #MENU_ON_Network #NULL_OK make a standard network according to the current settings (if net == NULL, new network is created)

  virtual void	StdEnv(Environment* env=NULL, int n_events = 0);
  // #MENU_BUTTON #MENU_ON_Environment #NULL_OK make a standard environment according to the current settings (if env == NULL, new environment is created)
  virtual void	UpdateEnvFmNet(Environment* env);
  // #MENU_BUTTON #MENU_SEP_BEFORE update environment (event specs controlling layout of events) based on current configuration of network (default network if multiple exist)
  virtual void	SequenceEvents(Environment* env, int n_seqs = 10, int events_per_seq = 4);
  // #MENU_BUTTON #MENU_SEP_BEFORE make sequences (groups) of events, for use with SequenceEpoch and SequenceProcess
  virtual void	TimeSeqEvents(TimeEnvironment* env, int n_seqs = 10, int events_per_seq = 4, float start_time = 0.0, float time_inc = 1.0);
  // #MENU_BUTTON make sequences (groups) of TimeEvents, with each sequence having the same sequence of times (start + inc for each addnl event)

  virtual void	StdProcs();
  // #MENU_BUTTON #MENU_ON_Processes #CONFIRM create a standard set of processes, starting with a batch process
  virtual void	NetAutoSave(SchedProcess* process_level_to_save_at, bool just_weights = false);
  // #MENU_BUTTON #MENU_SEP_BEFORE make the given process save a network when it is complted (just_weights = just save the weights)
  virtual EpochProcess*	AutoTestProc(SchedProcess* training_process, Environment* test_env);
  // #MENU_BUTTON create an automatic testing epoch process that runs on the test environment, and is called automatically from the training process
  virtual EpochProcess*	CrossValidation(SchedProcess* training_process, Environment* test_env);
  // #MENU_BUTTON create a cross-validation setup where training stops when testing on the test environment goes below a threshold
  virtual void	ToSequenceEvents(SchedProcess* process);
  // #MENU_BUTTON #MENU_SEP_BEFORE make the given process hierarchy work with event groups (sequences) (process can be any one in hierarchy)
  virtual void	NoSequenceEvents(SchedProcess* process);
  // #MENU_BUTTON get rid of sequence-level processing in given process hierarchy  (process can be any one in hierarchy)

  virtual MonitorStat* RecordLayerValues(SchedProcess* process_level_record_at, SchedProcess::StatLoc at_level, Layer* layer, const char* var = "act");
  // #MENU_BUTTON #MENU_ON_Stats record (e.g., in the log associated with given process) at the given level (loop or final) the given variable for the given layer (e.g., Trial FINAL = record at end of each trial)
  virtual CopyToEnvStat* SaveValuesInDataEnv(MonitorStat* stat);
  // #MENU_BUTTON save the values recorded by the given monitor statistic into a data environment (for viewing, clustering, distance computations, etc)
  virtual DispDataEnvProc* AutoAnalyzeDataEnv(Environment* data_env, int pattern_no, DispDataEnvProc::DispType analysis_disp, SchedProcess* process_level_analyze_at, SchedProcess::ProcLoc at_level);
  // #MENU_BUTTON automatically analyze (and display results) on given data environment and pattern number, at given processing level (e.g., Epoch FINAL = at end of each epoch)
  virtual DispDataEnvProc* AnalyzeNetLayer(SchedProcess* process_level_record_at, SchedProcess::StatLoc rec_at_level, Layer* layer, const char* var = "act",
		   DispDataEnvProc::DispType analysis_disp = DispDataEnvProc::CLUSTER_PLOT, SchedProcess* process_level_analyze_at = NULL,
		   SchedProcess::ProcLoc analyze_at_level = SchedProcess::FINAL_PROCS);
  // #MENU_BUTTON record data from the named layer, variable at given level (e.g., Trial FINAL) (RecordLayerValues), and send it to a data environment (SaveValuesInDataEnv), and then automatically analyze the data at given processing level (e.g., Epoch FINAL) (AutoAnalyzeDataEnv)
  virtual UnitActRFStat* ActBasedReceptiveField(SchedProcess* process_level_record_at, SchedProcess::StatLoc rec_at_level,
		Layer* recv_layer, Layer* send_layer, Layer* send2_layer,
		SchedProcess* process_level_disp_rfs_at, SchedProcess::ProcLoc disp_at_level = SchedProcess::FINAL_PROCS);
  // #MENU_BUTTON #NULL_OK record activation-based data from the named layer, variable at given level (e.g., Trial FINAL) (RecordLayerValues), and send it to a data environment (SaveValuesInDataEnv), and then automatically analyze the data at given processing level (e.g., Epoch FINAL) (AutoAnalyzeDataEnv)
  virtual DispNetWeightsProc* DisplayNetWeights(Layer* recv_layer, Layer* send_layer,
		SchedProcess* process_level_disp_wts_at, SchedProcess::ProcLoc disp_at_level = SchedProcess::FINAL_PROCS);
  // #MENU_BUTTON #NULL_OK automatically display network weight values from send_layer to recv_layer in given process and level (e.g., Epoch FINAL)
  virtual void	StopOnActThresh(SchedProcess* process_to_stop, Layer* layer, float thresh = .75);
  // #MENU_BUTTON #MENU_SEP_BEFORE make the given process stop (e.g., settle process) when activations in given layer get above given threshold
  virtual void	AddCountersToTest(SchedProcess* testing_process, SchedProcess* training_process);
  // #MENU_BUTTON add training process counters (e.g., epoch, batch) to the given testing process log output (via ProcCounterStat)
  virtual void	GetStatsFromProc(SchedProcess* proc_with_stats, SchedProcess* proc_to_get_stats, SchedProcess::StatLoc trg_stat_loc, Aggregate::Operator agg_op = Aggregate::LAST);
  // #MENU_BUTTON have proc_to_get_stats get all the stats (into its trg_stat_loc) from the proc_with_stats (using given aggregation operator)
  virtual TimeCounterStat* AddTimeCounter(SchedProcess* proc_where_time_incr, SchedProcess::StatLoc inc_at_level, SchedProcess* proc_reset_time);
  // #MENU_BUTTON add a time counter statistic to given process (useful as an X axis in graph plotting over longer time scales), resetting the time counter in given process

  // compare successive events (two mon stats with offsetting mon.off, compare stat, etc)

  virtual void	StdLogs(SchedProcess* process);
  // #MENU_BUTTON #MENU_ON_Logs #CONFIRM create standard logs (Trial TextLog, Epoch GraphLog, Batch TextLog) for given process hierarchy (process can be any proc in hierarchy)
  virtual void	LogProcess(SchedProcess* process, TypeDef* log_type);
  // #MENU_BUTTON #MENU_SEP_BEFORE #TYPE_PDPLog create log of a given type for given process
  // todo: various ways of configuring specific types of logs?? should probably be on logs themselves
  // what about making everything black & white vs. color?

  const ivColor* GetEditColor() { return pdpMisc::GetObjColor(GET_MY_OWNER(Project),&TA_Wizard); }

  void	UpdateAfterEdit();
  void 	Initialize();
  void 	Destroy()	{ CutLinks(); }
  void 	InitLinks();
  void	CutLinks();
  SIMPLE_COPY(Wizard);
  COPY_FUNS(Wizard, taNBase);
  TA_BASEFUNS(Wizard);
};

class Wizard_MGroup : public PDPMGroup {
  // group of wizard objects
public:
  virtual void	AutoEdit();

  void	Initialize() 		{ SetBaseType(&TA_Wizard); }
  void 	Destroy()		{ };
  TA_BASEFUNS(Wizard_MGroup);
};

class SelectEdit_MGroup : public PDPMGroup {
  // group of select edit dialog objects
public:
  virtual void	AutoEdit();

  void	Initialize() 		{ SetBaseType(&TA_SelectEdit); }
  void 	Destroy()		{ };
  TA_BASEFUNS(SelectEdit_MGroup);
};

class ProjViewState : public taNBase {
  // ##NO_TOKENS saves state variables associated with the project view, lookup by name
public:
  bool		in_use;		// #READ_ONLY #NO_SAVE whether this info is being used now
  bool		iconify;	// iconify (minimize) the view of this object
  bool 		draw_links;	// show the links between this object and others
  bool		selected;	// is this object selected?

  void Initialize();
  void Destroy()	{ };
  SIMPLE_COPY(ProjViewState);
  COPY_FUNS(ProjViewState, taNBase);
  TA_BASEFUNS(ProjViewState);
};

class ProjViewState_List : public taList<ProjViewState> {
  // ##NO_TOKENS #NO_UPDATE_AFTER list of project view state information
public:
  virtual void	UnSetInUse();		// unset in_use flags for all elements
  virtual void	RemoveNotInUse();	// remove all elements no longer in use
  virtual void	UnSelectAll();		// toggle selected button off for all objects

  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(ProjViewState_List);
};

class ProjEditor;

class Project : public WinBase {
  // ##EXT_proj ##COMPRESS A Project has everything
public:
  static bool nw_itm_def_arg;	// #IGNORE default arg val for FindMake..

  enum ViewColors {		// indicies for view_colors
    TEXT,
    BACKGROUND,
    NETWORK,
    ENVIRONMENT,
    SCHED_PROC,
    STAT_GROUP,
    SUBPROC_GROUP,
    STAT_PROC,
    OTHER_PROC,
    PDPLOG,
    STAT_AGG,
    GEN_GROUP,
    INACTIVE,
    STOP_CRIT,
    AGG_STAT,
    CON_SPEC,
    UNIT_SPEC,
    PRJN_SPEC,
    LAYER_SPEC,
    WIZARD,
    COLOR_COUNT
  };

  WinGeometry		root_win_pos; 	// position/size of the root window for saving

  TypeDefault_MGroup    defaults;	// #NO_FIND #NO_SAVE default initial settings for objects
  Wizard_MGroup    	wizards;	// Wizards for automatically configuring simulation objects
  BaseSpec_MGroup     	specs;		// Specifications for network parameters
  Network_MGroup	networks;	// Networks of interconnected units 
  Environment_MGroup	environments;	// Environments of patterns to present to networks
  Process_MGroup	processes;	// Processes to coordinate training/testing, etc
  PDPLog_MGroup		logs;		// Logs to display statistics in processes
  Script_MGroup		scripts;	// Scripts to control arbitrary actions
  SelectEdit_MGroup	edits;		// special edit dialogs for selected elements

  bool			save_rmv_units; // remove units from network before saving (makes project file much smaller!)
  bool			use_sim_log; 	// record project changes in the SimLog file
  String		prev_file_nm; 	// #READ_ONLY #SHOW previous file name for this project
  String		desc1;		// description of the project
  String		desc2;
  String		desc3;
  String		desc4;

  // parameters for controlling the view
  ProjEditor*		editor;		// #IGNORE controls display of project information

  int			obj_width; 	// width of the objects in view (in units of font m's)
  FontSpec	  	view_font;	// the font to use for the labels in the display
  RGBA_List		view_colors; 	// colors to use in the project view

  TAColor_List		the_colors; 	// #IGNORE actual colors
  ProjViewState_List	view_states;	// #HIDDEN state information for view objects
  bool			mnu_updating; 	// #READ_ONLY #NO_SAVE if menu is already being updated (don't init display)
  bool			deleting; 	// #READ_ONLY #NO_SAVE if object is currently being deleted

  virtual void	LoadDefaults();
  // load defaults according to root::default_file or precompiled defaults

  virtual void	InitDisplay();  // #MENU initialize the display of object information
  virtual void	FixDispButtons();  // initialize the display of buttons in window
  virtual void	Minimize();	// #MENU make display smallest possible size
  virtual void	Maximize();	// #MENU grow display to fit everything in view

  virtual void	SetViewFontSize(int point_size=8);
  // #MENU #MENU_SEP_BEFORE set the point size of the font for the project view display
  virtual void	UpdateColors();	// #BUTTON update the actual colors based on settings (
  virtual void	GetDefaultColors(); // #BUTTON get default colors for various project objects (in view and edit dialogs)

  virtual void	UpdateSimLog();
  // #MENU update simulation log (SimLog) for this project, storing the name of the project and the description as entered here.  click off use_simlog if you are not using this feature

  virtual void	Select(taNBase* obj, bool add=true);
  // select (for subsequent operation) given object in the projectview (NULL = deselect all)

  virtual const ivColor* GetObjColor(TypeDef* td); // #IGNORE get default color for object (for edit, project view)
  
  void 		GetBodyRep();
  ivGlyph*	GetPrintData();

  void	GetWinPos();
  void	UpdateMenus();
  void	SetFileName(const char* fname);

  // wizard construction functions:
  virtual void MakeDefaultWiz(bool auto_opn); // make the default wizard(s)
  virtual BaseSpec_MGroup* FindMakeSpecGp(const char* nm, bool& nw_itm = nw_itm_def_arg); // find a given spec group and if not found, make it

  int	Load(istream& strm, TAPtr par=NULL);
  int	Save(ostream& strm, TAPtr par=NULL, int indent=0);
  int 	SaveAs(ostream& strm, TAPtr par=NULL, int indent=0);

  void	UpdateAfterEdit();
  void	Initialize();
  void 	Destroy()		{ CutLinks(); }
  void 	InitLinks();
  void	CutLinks();
  void	Copy_(const Project& cp);
  COPY_FUNS(Project, WinBase);
  TA_BASEFUNS(Project);
};

class Project_MGroup : public PDPMGroup {
public:
  int		Load(istream& strm, TAPtr par=NULL); // call reconnect on nets afterwards
  
  void	Initialize() 		{ SetBaseType(&TA_Project); }
  void 	Destroy()		{ };
  TA_BASEFUNS(Project_MGroup);
};


class PDPRoot : public WinBase {
  // structural root of object heirarchy
public:
  static String		version_no; 	// #READ_ONLY #SHOW current version number
  String		default_file; 	// default name of defaults file
  Project_MGroup	projects; 	// The projects
  ColorScaleSpec_MGroup colorspecs;	// Color Specs

  bool	ThisMenuFilter(MethodDef* md); // don't include saving and loading..
  void	SetWinName();
  void	GetWindow();		// make an app-window
  void  Settings();		// #MENU #MENU_ON_Object edit global settings/parameters (taMisc)
  void	SaveConfig();		// #MENU #CONFIRM save current configuration to file ~/.pdpconfig that is automatically loaded at startup: IMPORTANT: DO NOT HAVE A PROJECT LOADED!
  void	LoadConfig();		// #MENU #CONFIRM load current configuration from file ~/.pdpconfig that is automatically loaded at startup
  void	Info();			// #MENU get information/copyright notice
  TAPtr	Browse(const char* init_path=NULL);
  // #MENU #ARGC_0 #USE_RVAL #NO_REVERT_AFTER use object browser to find an object, starting with initial path if given
  void	Quit();			
  // #MENU #CONFIRM #MENU_SEP_BEFORE #NO_REVERT_AFTER quit from software..

  // following use project to store position info!
  void	SetWinPos(float left=0, float bottom=0, float width=0, float height=0);
  void	GetWinPos();

  void 	Initialize();
  void	InitLinks();
  void 	Destroy();
  TA_BASEFUNS(PDPRoot);
};


#endif // pdpshell_h
