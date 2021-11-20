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

#ifndef datatable_h
#define datatable_h

#include <ta/ta_group.h>
#include <ta_misc/ta_misc_TA_type.h>
#include <ta_misc/tdgeometry.h>
#include <ta_misc/minmax.h>
#include <ta_misc/aggregate.h>
#include <ta_misc/colorscale.h> // for DT && DA Viewspecs


class float_RArray : public float_Array {
  // #NO_UPDATE_AFTER float array with range, plus a lot of other mathematical functions
public:
  enum DistMetric {		// generalized distance metrics
    SUM_SQUARES,		// sum of squares:  sum[(x-y)^2]
    EUCLIDIAN,			// Euclidian distance (sqrt of sum of squares)
    HAMMING, 			// Hamming distance: sum[abs(x-y)]
    COVAR,			// covariance: sum[(x-<x>)(y-<y>)]
    CORREL,			// correlation: sum[(x-<x>)(y-<y>)] / sqrt(sum[x^2 y^2])
    INNER_PROD,			// inner product: sum[x y]
    CROSS_ENTROPY		// cross entropy: sum[x ln(x/y) + (1-x)ln((1-x)/(1-y))]
  };

  MinMax	range;		// #NO_SAVE min-max range of the data 

  virtual void	UpdateRange(float item)	{ range.UpdateRange(item); }
  virtual void	UpdateAllRange();	
  // #MENU #MENU_ON_Actions update range for all items in array

  virtual void	Set(int i, const float& item)
  { float_Array::Set(i, item); UpdateRange(item); }
  virtual void		Add(const float& item);
  virtual void		Insert(const float& item, int idx, int n_els=1);

  float		Pop();
  bool		Remove(const float& item);
  bool		Remove(uint indx, int n_els=1);
  void 		CopyVals(const taArray_impl& from, int start=0, int end=-1, int at=0);
  
  virtual float	MaxVal(int& idx, int start=0, int end = -1) const;
  // #MENU #MENU_ON_Actions #USE_RVAL value and index of the (first) element that has the maximum value
  virtual float	AbsMaxVal(int& idx, int start=0, int end = -1) const;
  // #MENU #MENU_ON_Actions #USE_RVAL value and index of the (first) element that has the maximum absolute value
  virtual float	MinVal(int& idx, int start=0, int end = -1) const;
  // #MENU #USE_RVAL value and index of the (first) element that has the minimum value
  virtual float	Sum(int start=0, int end = -1) const;
  // #MENU #MENU_ON_Actions #USE_RVAL compute the sum of the values in the array
  virtual float	Mean(int start=0, int end = -1) const;
  // #MENU #USE_RVAL compute the mean of the values in the array
  virtual float	Var(float mean=0, bool use_mean=false, int start=0, int end = -1) const;
  // #MENU #USE_RVAL compute the variance of the values, opt with given mean
  virtual float	StdDev(float mean=0, bool use_mean=false, int start=0, int end = -1) const;
  // #MENU #USE_RVAL compute the standard deviation of the values, opt with given mean
  virtual float	SEM(float mean=0, bool use_mean=false, int start=0, int end = -1) const;
  // #MENU #USE_RVAL compute the standard error of the mean of the values, opt with given mean
  virtual float	SSLength(int start=0, int end = -1) const;
  // #MENU #USE_RVAL sum-of-squares length of the array

  // generalized distance measures follow
  virtual float	SumSquaresDist(const float_RArray& oth, bool norm = false, 
			       float tolerance=0.0f, int start=0, int end = -1) const;
  // compute sum-squares dist between this and the oth, tolerance is by element
  virtual float	EuclidDist(const float_RArray& oth, bool norm = false, 
			   float tolerance=0.0f, int start=0, int end = -1) const;
  // compute Euclidian dist between this and the oth, tolerance is by element
  virtual float	HammingDist(const float_RArray& oth, bool norm = false, 
			    float tolerance=0.0f, int start=0, int end = -1) const;
  // compute Hamming dist between this and the oth, tolerance is by element
  virtual float	Covar(const float_RArray& oth, int start=0, int end = -1) const;
  // compute the covariance of this array the oth array
  virtual float	Correl(const float_RArray& oth, int start=0, int end = -1) const;
  // compute the correlation of this array with the oth array
  virtual float	InnerProd(const float_RArray& oth, bool norm = false, 
			  int start=0, int end = -1) const;
  // compute the inner product of this array and the oth array
  virtual float	CrossEntropy(const float_RArray& oth, int start=0, int end = -1) const;
  // compute cross entropy between this and other array, this is 'p' other is 'q'

  virtual float	Dist(const float_RArray& oth, DistMetric metric, bool norm = false, 
			float tolerance=0.0f, int start=0, int end = -1) const;
  // compute generalized distance metric with other array (calls appropriate fun above)
  static bool	LargerFurther(DistMetric metric);
  // returns true if a larger value of given distance metric means further apart

  virtual void	Histogram(const float_RArray& oth, float bin_size);
  // this gets a histogram (counts) of number of values in other array

  virtual void	AggToArray(const float_RArray& from, Aggregate& agg,
			   int start=0, int end = -1);
  // aggregate values from other array to this one using aggregation params of agg
  virtual float	AggToVal(Aggregate& agg, int start=0, int end = -1) const;
  // compute aggregate of values in this array using aggregation params of agg

  virtual float	NormLen(float len=1.0f, int start=0, int end = -1);
  // #MENU normalize array to total given length (1.0), returns scale
  virtual float	NormSum(float sum=1.0f, float min_val=0.0f, int start=0, int end = -1);
  // #MENU normalize array to total given sum (1.0) and min_val (0) (uses range), returns scale
  virtual float	NormMax(float max=1.0f, int start=0, int end = -1);
  // #MENU normalize array to given maximum value, returns scale
  virtual float	NormAbsMax(float max=1.0f, int start=0, int end = -1);
  // #MENU normalize array to given absolute maximum value, returns scale
  virtual void	SimpleMath(const SimpleMathSpec& math_spec, int start=0, int end = -1);
  // #MENU apply standard kinds of simple math operators to values in the array
  virtual void	SimpleMathArg(const float_RArray& arg_ary, const SimpleMathSpec& math_spec, int start=0, int end = -1);
  // apply simple math operators to values, other array provides 'arg' value for math_spec
  virtual int	Threshold(float thresh=.5f, float low=0.0f, float high=1.0f, int start=0, int end = -1);
  // #MENU threshold values in the array, low vals go to low, etc.

  virtual void 	WritePoint(const TwoDCoord& geom, int x, int y, float color=1.0, bool wrap=true);
  // write a single point, assuming geometry geom
  virtual void 	RenderLine(const TwoDCoord& geom, int xs, int ys, int xe, int ye,
			   float color=1.0, bool wrap=true);
  // #MENU render a line from given x,y starting, ending coords in 2d space of geometry geom

  virtual void 	WriteXPoints(const TwoDCoord& geom, int x, int y, const float_RArray& color,
			     int wdth=1, bool wrap=true);
  // write a series of points of given width in x dimension using colors in order
  virtual void 	WriteYPoints(const TwoDCoord& geom, int x, int y, const float_RArray& color,
			     int wdth=1, bool wrap=true);
  // write a series of points of given width in y dimension using colors in order
  virtual void 	RenderWideLine(const TwoDCoord& geom, int xs, int ys, int xe, int ye,
			       const float_RArray& color, int wdth=1, bool wrap=true);
  // #MENU render a wide line from given x,y starting, ending coords in 2d space of geometry geom

  inline float& SafeMatEl(int col_dim, int row, int col) const { return SafeEl((row * col_dim) + col); }
  // safe get element assuming a matrix layout of values with column (inner) dimension size = col_dim
  inline float& FastMatEl(int col_dim, int row, int col) const { return FastEl((row * col_dim) + col); }
  // fast get element assuming a matrix layout of values with column (inner) dimension size = col_dim
  inline float& FastMatEl1(int col_dim, int row, int col) const { return FastEl(((row-1) * col_dim) + (col-1)); }
  // fast get element assuming a matrix layout of values with column (inner) dimension size = col_dim, indicies use 1-n range instead of 0-n-1!
  inline float& FastEl1(int idx) const { return FastEl(idx-1); }
  // fast get element with index in 1-n range instead of 0-n-1
  virtual void 	GetMatCol(int col_dim, float_RArray& col_vec, int col_no);
  // extract given column from this matrix-formatted object
  virtual void 	GetMatRow(int col_dim, float_RArray& row_vec, int row_no);
  // extract given row from this matrix-formatted object

  // x  0 1 2 3  dim = 4
  // 0  0 1 2 3
  // 1    3 4 5 + dim = 4
  // 2      6 7 + dim + (dim - 1) = 4 + 3
  // 3        8 (y-x) + (x * dim - sum(1..x-1)); sum(1..x) = (x*(x+1)) / 2
  inline float& SafeTriMatEl(int dim, int x, int y) const {
    if(x < y)  	return SafeEl((y-x) + (x * dim) - (((x-1) * x) / 2));
    else	return SafeEl((x-y) + (y * dim) - (((y-1) * y) / 2));
  }
  // get element assuming an upper-triangular symmetric matrix (e.g., distance matrix) of dimension dim for two items, x, y
  inline float& FastTriMatEl(int dim, int x, int y) const {
    if(x < y)  return FastEl((y-x) + (x * dim) - (((x-1) * x) / 2));
    else       return FastEl((x-y) + (y * dim) - (((y-1) * y) / 2));
  }
  // get element assuming an upper-triangular symmetric matrix (e.g., distance matrix) of dimension dim for two items, x, y
  inline void	AllocSqMatSize(int dim) { EnforceSize(dim * dim); }
  // allocate space (enforcesize) for a square matrix of size dim
  inline void	AllocTriMatSize(int dim) { EnforceSize((dim * (dim + 1)) / 2); }
  // allocate space (enforcesize) for an upper-triangular matrix of size dim
  
  virtual void 	CopyFmTriMat(int dim, const float_RArray& tri_mat);
  // copy from upper-triangular symmetric matrix of given dimensionality into a full matrix

  virtual bool	TriDiagMatRed(int dim, float_RArray& diags, float_RArray& off_diags);
  // reduce current full square matrix to a tri-diagonal form using the Householder transformation (first step in computing eigenvectors/values) -- diags are eigenvalues
  virtual bool	TriDiagQL(int dim, float_RArray& diags, float_RArray& off_diags);
  // perform QL algorithm to compute eigenvector/values of a tri-diagonal matrix as computed by TriDiagMatRed (this = matrix)
  virtual bool	Eigens(int dim, float_RArray& evals);
  // compute eigenvalue/vector decomposition of this square matrix of dimension dim.  eigen values are in evals, and this matrix contains the eigenvectors
  virtual bool	MDS(int dim, float_RArray& xcoords, float_RArray& ycoords, int x_axis_component = 0, int y_axis_component = 1, bool print_eigen_vals = false);
  // perform multidiminesional scaling of this distace matrix (must be full square matrix), returning two-dimensional coordinates that best capture the distance relationships among the items in x,y coords using specified components

  // updtate range after
  void		ShiftLeft(int nshft);
  int		Dump_Load_Value(istream& strm, TAPtr par=NULL);

  void		Reset(){float_Array::Reset();range.Init(0.0f);}

  void	Initialize()		{ };
  void	Destroy()		{ };
  void	InitLinks();
  void 	Copy_(const float_RArray& cp);
  COPY_FUNS(float_RArray, float_Array);
  TA_BASEFUNS(float_RArray);
};


// specific ones are in the _tmplt file (String_Data, float_Data)

class DataArray_impl : public taNBase {
  // ##NO_TOKENS #NO_UPDATE_AFTER holds array data
protected:
  static String	opt_after;	// #IGNORE temp string for option after function
public:
  String	disp_opts;	// viewer default display options
  bool		save_to_file;	// save this data to a file (e.g., to a log file in PDP++)?

  // these are overloaded for the specific types
  virtual taArray_base* 	AR()	{ return NULL; } // the array pointer
  virtual void	NewAR() { };
  // #MENU #MENU_ON_Object create an array for yourself
  virtual void	SetAR(taArray_base*) { };	// set AR to existing array

  bool		HasDispOption(const char* opt) const
  { return disp_opts.contains(opt); } // check if a given display option is set
  String& 	DispOptionAfter(const char* opt)
  { opt_after = disp_opts.after(opt); opt_after = opt_after.before(',');
    return opt_after; }
  void		AddDispOption(const char* opt);

  void	Initialize();
  void  InitLinks();
  void	Destroy()	{ };
  void 	Copy_(const DataArray_impl& cp);
  COPY_FUNS(DataArray_impl, taNBase);
  TA_BASEFUNS(DataArray_impl);
};

class float_Data;
class String_Data;

class DataTable : public taGroup<DataArray_impl> {
  // #NO_UPDATE_AFTER table of data
public:
  virtual void	ResetData();
  // #MENU #MENU_ON_Actions reset all of the data arrays
  virtual void	RemoveRow(int row_num);
  // #MENU Remove an entire row of data
  virtual void	AddBlankRow();
  // #MENU add a new row to the data table, returns new row number
  virtual void	SetSaveToFile(bool save_to_file);
  // #MENU set the save_to_file flag for entire group of data elements

  virtual void	AddRowToArray(float_RArray& ar, int row_num) const;
  // add a row of the datatable to given array
  virtual void	AggRowToArray(float_RArray& ar, int row_num, Aggregate& agg) const;
  // aggregate a row of the datatable to given array using parameters in agg
  virtual float	AggRowToVal(int row_num, Aggregate& agg) const;
  // aggregate a row of the datatable to a value using parameters in agg
  virtual void	AddArrayToRow(float_RArray& ar);
  // add contents of array to datatable
  virtual void	AggArrayToRow(const float_RArray& ar, int row_num, Aggregate& agg);
  // aggregate contents of array to datatable at given row
  virtual void	PutArrayToRow(const float_RArray& ar, int row_num);
  // just put array values into given row of data
  virtual void	UpdateAllRanges();
  // update all min-max range data for all float_Data elements in log

  virtual float_Data*	NewColFloat(const char* col_nm); // create new column of floating point data
  virtual float_Data*	NewColInt(const char* col_nm); 	 // create new column of integer-level data (= narrow display, actually stored as float)
  virtual String_Data*	NewColString(const char* col_nm); // create new column of string data
  virtual DataTable*	NewGroupFloat(const char* base_nm, int n); // create new sub-group of floats of size n, named as base_nm_index 
  virtual DataTable*	NewGroupInt(const char* base_nm, int n); // create new sub-group of ints of size n, named as base_nm_index 
  virtual DataTable*	NewGroupString(const char* base_nm, int n); // create new sub-group of strings of size n, named as base_nm_index

  virtual DataArray_impl* GetColData(int col, int subgp=-1);
  // get data for given column (if subgp >= 0, column is in given subgroup)
  
  virtual float_Data* GetColFloatData(int col, int subgp=-1);
  // get float_Data for given column (if subgp >= 0, column is in given subgroup)
  virtual String_Data* GetColStringData(int col, int subgp=-1);
  // get string data for given column (if subgp >= 0, column is in given subgroup)

  virtual float_RArray* GetColFloatArray(int col, int subgp=-1);
  // get float_RArray for given column (if subgp >= 0, column is in given subgroup)
  virtual String_Array* GetColStringArray(int col, int subgp=-1);
  // get string data for given column (if subgp >= 0, column is in given subgroup)

  virtual void	PutArrayToCol(const float_RArray& ar, int col, int subgp=-1);
  // just put array values into given column (if subgp >= 0, column is in given subgroup)

  virtual void	SetColName(const char* col_nm, int col, int subgp=-1);
  // set column name for given column (if subgp >= 0, column is in given subgroup)
  virtual void	AddColDispOpt(const char* dsp_opt, int col, int subgp=-1);
  // add display option for given column (if subgp >= 0, column is in given subgroup)

  virtual void	AddFloatVal(float val, int col, int subgp=-1);
  // add float/int data for given column (if subgp >= 0, column is in given subgroup)
  virtual void	AddStringVal(const char* val, int col, int subgp=-1);
  // add string data for given column (if subgp >= 0, column is in given subgroup)

  virtual void	SetFloatVal(float val, int col, int row, int subgp=-1);
  // set float/int data for given column, row (if subgp >= 0, column is in given subgroup)
  virtual void	SetStringVal(const char* val, int col, int row, int subgp=-1);
  // set string data for given column, row (if subgp >= 0, column is in given subgroup)

  virtual void	SetLastFloatVal(float val, int col, int subgp=-1);
  // set last row float/int data for given column (if subgp >= 0, column is in given subgroup)
  virtual void	SetLastStringVal(const char* val, int col, int subgp=-1);
  // set last row string data for given column (if subgp >= 0, column is in given subgroup)

  virtual float GetFloatVal(int col, int row, int subgp=-1);
  // get float data for given column, row (if subgp >= 0, column is in given subgroup)
  virtual String GetStringVal(int col, int row, int subgp=-1);
  // get string data for given column, row (if subgp >= 0, column is in given subgroup)

  int  		MinLength();		// #IGNORE
  int  		MaxLength();		// #IGNORE

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy();
  void	InitLinks();
  TA_BASEFUNS(DataTable);
};

#include <ta_misc/datatable_tmplt.h>

class ClustNode;

class ClustLink : public taBase {
  // #INLINE ##NO_TOKENS ##NO_UPDATE_AFTER a link in the cluster tree with distance
public:
  float		dist;		// distance to this node from parent
  ClustNode*	node;		// cluster node

  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void	CutLinks();
  void	Copy_(const ClustLink& cp);
  COPY_FUNS(ClustLink, taBase);
  TA_BASEFUNS(ClustLink);
};
  

class ClustNode : public taNBase {
  /* ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER node in clustering algorithm
     use one with leaves as children as a root node for cluster */
public:
  float_RArray*	pat;		// pattern I point to (if leaf)
  int		leaf_idx;	// original leaf index, used for pointing into master distance table
  int		leaf_max;	// original max number of leaves
  float_RArray*	leaf_dists;	// distance matrix for all leaves
  float		par_dist;	// distance from parent cluster
  float		nn_dist;	// nearest neighbor (within cluster) distance
  float		tmp_dist;	// temporary distance value (for computations)
  float		y;		// y axis value
  taBase_List	children;	// my sub-nodes
  taBase_List	nns;		// nearest neighbor(s)

  ClustNode*	GetChild(int i)	const { return ((ClustLink*)children[i])->node; }
  ClustNode*	GetNN(int i)	const { return ((ClustLink*)nns[i])->node; }
  float& 	GetNNDist(int i) const { return ((ClustLink*)nns[i])->dist; }

  virtual void	SetPat(float_RArray* pt); // use setpointer to set pat
  virtual void	AddChild(ClustNode* nd, float dst = 0.0f);
  // add new child (via ClustLink)
  virtual void	LinkNN(ClustNode* nd, float dst = 0.0f);
  // add new neighbor (via ClustLink)
  virtual bool	RemoveChild(ClustNode* nd);// remove link with this node
  virtual int	FindChild(ClustNode* nd); // find child with this node link

  virtual void 	Cluster(float_RArray::DistMetric metric=float_RArray::EUCLIDIAN,
			bool norm=false, float tol=0.0f);
  // generate the cluster: call on a root node with a flat list of leaf children

  virtual void	Graph(ostream& strm);
  // generate commands to drive graph (or xgraph) for plotting cluster
  virtual void	Graph_impl(ostream& strm);
  // #IGNORE implementation

  virtual void	XGraph(const char* fnm, const char* title);
  // generate graph in given file name with given title, and call xgraph on result

  virtual void	GraphData(DataTable* dt);
  // generate graph as X, Y, label points in a datatable, suitable for graphing
  virtual void	GraphData_impl(DataTable* dt);
  // #IGNORE implementation

  virtual void	NNeighbors(float_RArray::DistMetric metric,
		    bool norm, float tol);
  // #IGNORE find nearest neighbors for everyone

  virtual bool	ClustOnClosest(float_RArray::DistMetric metric);
  // #IGNORE generate a new cluster by combining closest nearest neighbors together

  float	Dist(const ClustNode& oth, float_RArray::DistMetric metric,
	     bool norm, float tol) const;
  // #IGNORE compute distance between this node and other, averaging over individual patterns

  virtual void	SetYs(float y_init = -1.0);
  // #IGNORE traverse the tree computing y values (starting from initial given value)

  virtual float	SetParDists(float par_d, float_RArray::DistMetric metric);
  // #IGNORE traverse the tree computing par_dist values (starting from given dist)

  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void  InitLinks();
  void	CutLinks();
  SIMPLE_COPY(ClustNode);
  COPY_FUNS(ClustNode, taNBase);
  TA_BASEFUNS(ClustNode);
};

class DA_ViewSpec : public taNBase { 
  // ##SCOPE_DT_ViewSpec base specification for the display of log data_array (DA)
public:
  DataArray_impl*	data_array;	// #READ_ONLY #NO_SAVE the data array
  String		display_name;	// name used in display
  bool	        	visible;	// visibility flag

  virtual void	UpdateView(); 	// #BUTTON Update view to reflect current changes
  virtual void	SetGpVisibility(bool visible); 
  // #BUTTON set the visibility of all members of this group of items
  virtual void	CopyToGp(MemberDef* member); 
  // #BUTTON copy given member value setting to all view specs within this same group

  virtual bool	BuildFromDataArray(DataArray_impl* tda=NULL);

  static String	CleanName(String& name);

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ CutLinks(); }
  void	CutLinks();
  void	Copy_(const DA_ViewSpec& cp);
  COPY_FUNS(DA_ViewSpec,taNBase);
  TA_BASEFUNS(DA_ViewSpec);
};

class DT_ViewSpec :  public taGroup<DA_ViewSpec> {
  // base specification for the display of log data_table (DT)
public:
  DataTable*		data_table;	// #READ_ONLY the data table;
  String		display_name;	// name used in display
  bool	        	visible; 	// visibility flag

  virtual bool	BuildFromDataTable(DataTable* tdt=NULL);
  virtual void	ReBuildFromDataTable();

  virtual void	SetDispNms(const char* base_name);
  // #BUTTON set display_name for all view specs in group to base_name + "_" + no where no is number in group
  virtual void	RmvNmPrefix();
  // #BUTTON set display_name for all view specs in group to current name without prefix (i.e., "prefix_rest" -> "rest")
  virtual void	SetVisibility(bool visible); 
  // #BUTTON set the visibility of all members of this group of items 

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	CutLinks();
  void	Copy_(const DT_ViewSpec& cp);
  COPY_FUNS(DT_ViewSpec, taGroup<DA_ViewSpec>);
  TA_BASEFUNS(DT_ViewSpec);
};

class DA_TextViewSpec: public DA_ViewSpec {
  // data-array view spec for text-based display
public:
  int		width;			// number of columns to use for display

  bool		BuildFromDataArray(DataArray_impl* tda=NULL);

  void 	Initialize();
  void	Destroy();
  void Copy_(const DA_TextViewSpec& cp) { width = cp.width;}
  COPY_FUNS(DA_TextViewSpec,DA_ViewSpec);
  TA_BASEFUNS(DA_TextViewSpec);
};

class DA_NetViewSpec: public DA_ViewSpec {
  // data-array view spec for netview based display
public:
  int	label_index;	// index into the netview's label list
  void  Initialize();
  void  Destroy();
  void Copy_(const DA_NetViewSpec& cp) { label_index = cp.label_index;}
  COPY_FUNS(DA_NetViewSpec,DA_ViewSpec);
  TA_BASEFUNS(DA_NetViewSpec);
};
  
class DA_GraphViewSpec: public DA_ViewSpec {
  // #BUTROWS_1 data-array view spec for graph-based display
public:
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
    STACK_TRACES,		// arrange subsequent traces of data (pass through the same X axis values) in non-overlapping vertically-arranged stacks
    NO_VERTICAL			// don't draw any vertical dimension at all (for VALUE_COLORS or THESH_POINTS line_type
  };
  enum SharedYType {
    OVERLAY_LINES,		// overlay multiple graph lines sharing same Y axis
    STACK_LINES			// arrange lines that share the same Y axis in non-overlapping vertically-arranged stacks
  };

  RGBA		line_color;	 // color of the line
  TAColor	ta_color;	 // #IGNORE actual color pointer

  LineType	line_type;	// the way the line is drawn
  LineStyle	line_style;	// the style in which the line is drawn
  float		line_width;	// width of the line
  PointStyle	point_style;	// the style in which the points are drawn
  Modulo	point_mod;      // when to skip the drawing of point
  bool		negative_draw;	// draw lines in the negative X axis direction?
  SharedYType 	shared_y;	// what to do with multiple graph lines that share the Y axis
  VerticalType	vertical;	// what to do with the vertical Y axis 
  FloatTwoDCoord trace_incr; 	// increments in starting coordinates for each subsequent trace of a line across the same X axis values, producing a 3D-like effect
  float		thresh;		// threshold for THRESH_POINTS line style

  DA_GraphViewSpec* axis_spec;  // #NO_NULL spec which indicates the axis to use
  FixedMinMax 	range;		// specify fixed range of values to display (value only effective when corresponding fix button pressed)
  int		n_ticks;	// maximum number of ticks to use in display of axis number labels (actual number may be less depending on the labels)
  DA_GraphViewSpec* string_coords;  // column that contains vertical coordinate values for positioning String data labels

  virtual void	PlotRows();	// #BUTTON #CONFIRM plot the data across rows of grouped columns (x axis = column index, y axis = values for each column in one row) instead of down the columns
  virtual void	PlotCols();	// #BUTTON #CONFIRM plot the data down columns (standard mode)
  virtual void	GpShareAxis();	// #BUTTON #CONFIRM make every element in this group share the same Y axis, which is the first in the group
  virtual void	GpSepAxes();	// #BUTTON #CONFIRM make every element in this group have its own Y axis

  bool		BuildFromDataArray(DataArray_impl* tda=NULL);
  virtual void 	FindStringCoords(); // find previous float_Data for default string coordinates

  virtual void	SetAxis(DA_GraphViewSpec* as) {
    taBase::SetPointer((TAPtr*)&axis_spec, as);
  }
  virtual void	SetStringCoords(DA_GraphViewSpec* as) {
    taBase::SetPointer((TAPtr*)&string_coords, as);
  }

  void 	Initialize();
  void	Destroy();
  void 	InitLinks();
  void  CutLinks();
  void  UpdateAfterEdit();
  void Copy_(const DA_GraphViewSpec& cp);
  COPY_FUNS(DA_GraphViewSpec,DA_ViewSpec);
  TA_BASEFUNS(DA_GraphViewSpec);
};

class DT_GraphViewSpec : public DT_ViewSpec {
  // controls display of datatable in a graph format
public:
  enum ColorType {		// defines standard color sequences
    C_RAINBOW,			// color, rainbow
    C_GREYSCALE,		// color, greyscale
    M_MONO,			// monochrome
    P_RAINBOW,			// printer, rainbow
    P_GREYSCALE,		// printer, greyscale
    P_MONO,			// printer, monochrome
    CUSTOM 			// custom, let user set colors
  };

  enum SequenceType {		// defines sequences of line features
    COLORS,			// use sequential colors
    LINES,			// use sequential line styles
    POINTS,			// use sequential point styles
    NONE 			// no features
  };

  RGBA			background;	// background color
  TAColor		bg_color;	// #IGNORE actual color object
  ColorType		color_type;	// palette of colors to use
  SequenceType		sequence_1;	// first (innermost) sequence of features
  SequenceType		sequence_2;	// second sequence of features
  SequenceType		sequence_3;	// third sequence of features
  bool			plot_rows; 	// plot the data across a row of columns (x axis = column index, y axis = values for each column in one row) instead of down the columns

  virtual char*	ColorName(int color_no);
  // #MENU #MENU_ON_Actions #USE_RVAL gets color name for given line number for color_type
  virtual int 	ColorCount();		 // number of colors in palatte
  virtual void	SetBgColor();
  // sets background color based on color type
  virtual void	UpdateLineFeatures(bool visible_only=true);
  // #MENU #MENU_ON_Actions apply specified sequences to update line features
  virtual void	ApplyOneFeature(DA_GraphViewSpec* dagv, SequenceType seq, int val,
				bool& set_ln, bool& set_pt);
  // #IGNORE

  virtual void	SetLineWidths(float line_width);
  // #MENU set the line widths of all lines to this value
  virtual void	SetLineType(DA_GraphViewSpec::LineType line_type);
  // #MENU set all line types to given type

  virtual void	ShareAxes();
  // #MENU #MENU_SEP_BEFORE #CONFIRM make all columns share the same Y axis (first axis in group)
  virtual void	SeparateAxes();
  // #MENU #CONFIRM each column of data gets its own Y axis
  virtual void	PlotRows();
  // #MENU #CONFIRM plot the data across rows of grouped columns (x axis = column index, y axis = values for each column in one row) instead of down the columns
  virtual void	PlotCols();
  // #MENU #CONFIRM plot the data down columns (standard mode)

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

  bool		BuildFromDataTable(DataTable* tdt=NULL);

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy();
  void	InitLinks();
  void	Copy_(const DT_GraphViewSpec& cp);
  COPY_FUNS(DT_GraphViewSpec, DT_ViewSpec);
  TA_BASEFUNS(DT_GraphViewSpec);
};

class DA_GridViewSpec : public DA_ViewSpec {
  // information for display of a data array in a grid display
public:
  enum DisplayStyle {
    TEXT,			// Draw using text only
    BLOCK,			// Draw using color block only
    TEXT_AND_BLOCK 		// Draw using both color block with text
  };

  PosTDCoord	pos;		// position of the data in absolute coordinates
  DisplayStyle  display_style;	// can display as text, block, or both
  bool		scale_on;	// adjust overall scale including this data

  bool		BuildFromDataArray(DataArray_impl* tda=NULL);

  void 	Initialize();
  void	Destroy();
  void 	InitLinks();
  void	Copy_(const DA_GridViewSpec& cp);
  COPY_FUNS(DA_GridViewSpec, DA_ViewSpec);
  TA_BASEFUNS(DA_GridViewSpec);
};

class DT_GridViewSpec : public DT_ViewSpec {
  // information for display of a datatable in a grid display
public:
  enum BlockFill {		// ways that grid blocks can be filled
    COLOR,			// color indicates value
    AREA,			// area indicates value
    LINEAR 			// linear size of square side indicates value
  };

  enum MatrixLayout { // order of display of the grid elements
    DEFAULT,			// use current default layout
    LFT_RGT_BOT_TOP, // [3412] Incr col first, then decr row, start at bot left
    LFT_RGT_TOP_BOT, // [1234] Incr col first, then incr row, start at top left
    BOT_TOP_LFT_RGT, // [2413] Decr row first, then incr col, start at bot left
    TOP_BOT_LFT_RGT  // [1324] Incr row first, then incr col, start at top left
  };

  PosTDCoord	pos;		// position of the datatable in absolute coordinates
  PosTDCoord	geom;		// relative geometry (maximum extent) of the datatable, just for El's, not subgroups
  PosTDCoord	full_geom;	// #HIDDEN #NO_SAVE full absolute geometry (maximum extent) of everything under this one
  MatrixLayout	layout;		// current layout of the data table

  bool          use_gp_name;    // use the group name instead of the El names
  DA_GridViewSpec::DisplayStyle  display_style;	// can display as text, block, or both
  bool		scale_on;	// adjust overall scale including this data (or not)
  bool		customized;	// #READ_ONLY did the use customize the positions of elements in here?  if so, don't redo layout with new items

  virtual int 	UpdateLayout(MatrixLayout ml=DEFAULT);
  // #MENU #MENU_ON_Actions enforce the geometry to fit with no spaces or overlap, returns maxx
  virtual void 	UpdateGeom();
  // #MENU Get the Geometry from the positions of visibles

  bool		BuildFromDataTable(DataTable* tdt=NULL);
  virtual void	GetMinMaxScale(MinMax& mm, bool first=true); // get min and max data range for scaling

  void		Reset();

  void 	UpdateAfterEdit();
  void 	Initialize();
  void	Destroy();
  void 	InitLinks();
  void	Copy_(const DT_GridViewSpec& cp);
  COPY_FUNS(DT_GridViewSpec, DT_ViewSpec);
  TA_BASEFUNS(DT_GridViewSpec);
};

#endif // datatable_h
