/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

#ifndef pdp_base_h
#ifdef __GNUG__
#pragma interface
#endif
#define pdp_base_h

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

//////////////////////////////////////////
//		ta_string.h		//
//////////////////////////////////////////

class StrRep {                   // internal String representations
public:
  unsigned short    len;         // string length 
  unsigned short    sz;          // allocated space
  char              s[1];        // the string starts here 
                                 // (at least 1 char for trailing null)
                                 // allocated & expanded via non-public fcts
};

class String {
protected:
  StrRep*           rep;   // Strings are pointers to their representations
public:
  String()	{ rep = NULL; }
  ~String()	{ };
};

//////////////////////////////////////////
//		typea.h			//
//////////////////////////////////////////

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

typedef unsigned int uint;

#ifdef NO_BUILTIN_BOOL
typedef	unsigned int 	bool;

#ifdef true
#undef true
#endif
#define true 1
#ifdef false
#undef false
#endif
#define false 0
#endif

extern "C" {
  extern double drand48(void);
  extern double erand48(unsigned short*);
  extern long lrand48(void);
  extern long nrand48(unsigned short*);
  extern long mrand48(void);
  extern long jrand48(unsigned short*);
  extern void srand48(long);
#ifdef CONST_48_ARGS
  extern unsigned short* seed48(const unsigned short*);
  extern void lcong48(const unsigned short*);
#else
  extern unsigned short* seed48(unsigned short*);
  extern void lcong48(unsigned short*);
#endif
#ifdef HP800
#define srandom srand48
#define random lrand48
#ifdef __GNUG__
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
#endif
#endif
#ifdef LINUX
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
// LINUX fails to define LN_MINFLOAT in values.h!
#ifndef LN_MINFLOAT
#define LN_MINFLOAT log(MINFLOAT)
#endif
#endif
#ifdef AIXV4
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
// AIX fails to define LN_MINFLOAT in values.h!
#ifndef LN_MINFLOAT
#define LN_MINFLOAT log(MINFLOAT)
#endif
#endif
#if defined (SUN4) || defined(SUN4debug)
  long random(void);
  void srandom(int);
#define sqrtf(x) sqrt(x)
#define expf(x)  exp(x)
#define logf(x)  log(x)
#define fabsf(x) fabs(x)
#define powf(x,y) pow(x,y)
#endif
#if defined(SGI) || defined(SGIdebug)
#define SIGNAL_PROC_FUN_TYPE void (*)(...)
#define SIGNAL_PROC_FUN_ARG(x) void (*x)(...)
#else
#define SIGNAL_PROC_FUN_TYPE void (*)(int)
#define SIGNAL_PROC_FUN_ARG(x) void (*x)(int)
#endif
  extern double acosh(double);
  extern double asinh(double);
  extern double atanh(double);
  extern double erf(double);
}

class taRefN {
protected:
  uint 		refn;
public:
  taRefN()	{ refn = 0; }
  virtual ~taRefN()	{ };
};

class TypeDef : public taRefN {
public:
  virtual ~TypeDef()	{ };
};

//////////////////////////////////////////
//		ta_base.h		//
//////////////////////////////////////////

class taBase {
protected:
  uint		refn;	// simulate taBase
public:
  taBase() 	{ refn = 0; }
  virtual ~taBase()	{ };
};

typedef taBase* TAPtr;

class taNBase : public taBase {
public:
  TAPtr		owner;		// #READ_ONLY #NO_SAVE pointer to owner
  String	name;		// name of the object
  
  taNBase() 	{ owner = NULL; }
};

//////////////////////////////////////////
//		ta_list.h		//
//////////////////////////////////////////

class taPtrList_impl {
public:
// put this in the actual thing literally
  void**	el;			// the elements themselves
  int 		alloc_size;		// allocation size
  void*		hash_table;	// #READ_ONLY #NO_SAVE a hash table (NULL if not used)
  int		size;		// #READ_ONLY #NO_SAVE number of elements in the list

  void*		El_(uint i) const				
  { TAPtr rval=NULL; if((int)i < size) rval = (TAPtr)el[i]; return rval; }

  virtual void	Alloc(int no);
  virtual void	Add_(void* itm);

  virtual int	Find(void* it) const; 			// #IGNORE

  taPtrList_impl() 	{ size = 0; el = NULL; alloc_size = 0; hash_table = NULL; }
  virtual ~taPtrList_impl()	{ el = NULL; size = alloc_size = 0; hash_table = NULL; }
};

class taList_impl : public taBase, public taPtrList_impl {
public:
  TypeDef*	el_base;	// #HIDDEN #NO_SAVE Base type for objects in group
  TypeDef* 	el_typ;		// #TYPE_ON_el_base Default type for objects in group
  uint		el_def;		// Index of default element in group
  String	name;		// name of the list
  TAPtr		owner;		// #READ_ONLY #NO_SAVE owner of the list

  TAPtr		El(uint idx) const		{ return (TAPtr)El_(idx); }
  TAPtr		FastEl(uint i) const		{ return (TAPtr)el[i]; } 
  TAPtr		operator[](uint i) const	{ return El(i); }

  taList_impl() { el_base = NULL; el_typ = NULL; el_def = 0; owner = NULL; }
  virtual ~taList_impl()	{ };
};

//////////////////////////////////////////
//		ta_group.h		//
//////////////////////////////////////////

class 	taGroup_impl;		// pre-declare
typedef taGroup_impl* TAGPtr;
typedef taList_impl TALOG; // list of groups (LOG)

class taSubGroup : public taList_impl {
public:
};

class	taLeafItr {		// contains the indicies for iterating over leafs
public:
  TAGPtr	cgp;		// pointer to current group
  int		g;		// index of current group
  int		i;		// index of current leaf element
};

#define FOR_ITR_EL(T, el, grp, itr) \
for(el = (T*) grp FirstEl(itr); el; el = (T*) grp NextEl(itr))

#define FOR_ITR_GP(T, el, grp, itr) \
for(el = (T*) grp FirstGp(itr); el; el = (T*) grp NextGp(itr))

class taGroup_impl : public taList_impl {
public:
  uint 		leaves;		// #READ_ONLY #NO_SAVE total number of leaves
  taSubGroup	gp; 		// #HIDDEN #NO_FIND #NO_SAVE sub-groups within this one
  TAGPtr	super_gp;	// #READ_ONLY #NO_SAVE super-group above this
  TALOG* 	leaf_gp; 	// #READ_ONLY #NO_SAVE 'flat' list of leaf-containing-gps 

  // only one group possible -- a sub group
  TAGPtr 	FirstGp(int& g) const  
  { g = 0; TAGPtr rval = NULL; if(size > 0) rval = (TAGPtr)this; else if(gp.size > 0) rval = (TAGPtr)gp[0]; return rval; }
  TAGPtr 	NextGp(int&)	const  { return NULL; }

  TAPtr		FirstEl(taLeafItr& lf) const
  { TAPtr rval=NULL; lf.i = 0; lf.cgp = FirstGp(lf.g);
    if(lf.cgp != NULL) rval=(TAPtr) lf.cgp->el[0]; return rval; }
  TAPtr	 	NextEl(taLeafItr& lf)	const
  { TAPtr rval=NULL; if(++lf.i >= lf.cgp->size) { lf.i = 0; lf.cgp = NULL; }
    if(lf.cgp != NULL) rval = (TAPtr)lf.cgp->el[lf.i]; return rval; }
  
  taGroup_impl() 	{ };
};


//////////////////////////////////////////
//		minmax.h		//
//////////////////////////////////////////

class MinMax : public taBase {
public:
  float		min;	// minimum value
  float		max;	// maximum value
  float		range;	// #HIDDEN distance between min and max
  float		scale;	// #HIDDEN scale (1.0 / range)

  bool	operator != (MinMax& mm) const
  { return ((mm.min != min) || (mm.max != max)); }

  bool	operator == (MinMax& mm) const
  { return ((mm.min == min) && (mm.max == max)); }

  void	Init(float it)	{ min = max = it; }  // initializes the max and min to this value

  inline float	Range()	const		{ return (max - min); }
  inline float	MidPoint() const	{ return .5 * (min + max); }
  // returns the range between the min and the max

  void	UpdateRange(MinMax& it)
  { min = MIN(it.min, min); max = MAX(it.max, max); }

  void	UpdateRange(float it)
  { min = MIN(it, min);	max = MAX(it, max); }  // updates the range

  void	MaxLT(float it)		{ max = MIN(it, max); }
  // max less than (or equal)

  void	MinGT(float it)		{ min = MAX(it, min); }
  // min greater than (or equal)

  void  WithinRange(MinMax& it)		// put my range within given one
  { min = MAX(it.min, min); max = MIN(it.max, max); }

  float	Normalize(float val) const	{ return (val - min) / Range(); }
  // normalize given value to 0-1 range given current in max

  float	Clip(float val) const		
  { val = MIN(max,val); val = MAX(min,val); return val; }
  // clip given value within current range

  void	UpdateAfterEdit()
  { range = Range(); if(range != 0) scale = 1.0 / range; }

  MinMax() 		{ min = 0; max = 1; range = 1; scale = 1; }
};

//////////////////////////////////////////
//		spec.h			//
//////////////////////////////////////////

#define MAX_SPEC_LONGS	((256 / (sizeof(long) * 8)) + 1)

class BaseSpec : public taNBase {
public:
  unsigned long unique[MAX_SPEC_LONGS]; // #HIDDEN bits representing members unique
  TypeDef*	min_obj_type;
  taGroup_impl 	children;
#ifndef SMALL_OBJS
  char		fill[12];
#endif
};

//////////////////////////////////////////
//		netstru.h		//
//////////////////////////////////////////

class Unit;
class Con_Group;
class Projection;
class Layer;

const float SIGMOID_MAX_VAL = 0.999999f; // max eval value
const float SIGMOID_MIN_VAL = 0.000001f; // min eval value 
const float SIGMOID_MAX_NET = 13.81551f;	// maximium net input value

class SigmoidSpec : public taBase {
public:
  float		off;		// offset for .5 point
  float		gain;		// gain

  static float	Clip(float y)	
  { y = MAX(y,SIGMOID_MIN_VAL); y = MIN(y,SIGMOID_MAX_VAL); return y; }
  static float	ClipNet(float x)
  { x = MAX(x,-SIGMOID_MAX_NET); x = MIN(x,SIGMOID_MAX_NET); return x; }
  float		Eval(float x)	
  { return Clip(1.0f / (1.0f + expf(-ClipNet((x - off) * gain)))); }
  float		Deriv(float x)	{ x = Clip(x); return x * (1.0f - x) * gain; }
  float		Inverse(float y)	{ y=y+off; return logf(y / (1.0f - y)) / gain; }

  SigmoidSpec()		{ off = 0.0f; gain = 1.0f; }
};

class WeightLimits : public taBase {
public:
  enum LimitType {
    NONE,			// no weight limitations
    GT_MIN,			// constrain weights to be greater than min value
    LT_MAX,			// constrain weights to be less than max value
    MIN_MAX 			// constrain weights to be within min and max values
  };
  LimitType	type;		// type of weight limitation to impose
  float		min;		// minimum weight value (if applicable)
  float		max;		// maximum weight value (if applicable)
  bool		sym;		// if true, also symmetrize with reciprocal connections

  void 	ApplyMinLimit(float& wt)	{ if(wt < min) wt = min; }
  void 	ApplyMaxLimit(float& wt)	{ if(wt > max) wt = max; }

  void	ApplyLimits(float& wt)
  { if(type == GT_MIN) 		ApplyMinLimit(wt);
    else if(type == LT_MAX)	ApplyMaxLimit(wt);
    else if(type == MIN_MAX)	{ ApplyMinLimit(wt); ApplyMaxLimit(wt); } }

  WeightLimits()		{ type = NONE; min = -1.0f; max = 1.0f; sym = false; }
};

class Connection : public taBase {
public:
  float 	wt;		// Weight of Connection

  Connection() { wt = 0.0f; }
};

#define	CON_GROUP_LOOP(cg, expr) \
  int i; for(i=0; i<cg->size; i++) \
    expr

class ConSpec : public BaseSpec {
public:
  TypeDef*		min_con_type;
  // #HIDDEN #NO_SAVE #TYPE_Connection mimimal con type required for spec (obj is con group)
//  Random	rnd;		// Weight randomization specification
  WeightLimits	wt_limits;	// limits on weight sign, symmetry

#ifndef SMALL_OBJS
  char		fill2[24];
#endif

  inline void		C_ApplyLimits(Connection* cn, Unit*, Unit*)
  { wt_limits.ApplyLimits(cn->wt); }
  inline virtual void	ApplyLimits(Con_Group* cg, Unit* ru);
  // apply weight limits (sign, magnitude)
    
  inline virtual void	C_InitWtState(Connection* cn, Unit* ru, Unit* su);
  inline virtual void 	InitWtState(Con_Group* cg, Unit* ru);
  // initialize state variables (ie. at beginning of training)
  inline virtual void	C_InitWtState_post(Connection*, Unit*, Unit*) { };
  // #IGNORE
  inline virtual void 	InitWtState_post(Con_Group* cg, Unit* ru); 
  // #IGNORE post-initialize state variables (ie. for scaling symmetrical weights, etc)
  inline virtual void 	C_InitWtDelta(Connection*, Unit*, Unit*)	{ };
  inline virtual void 	InitWtDelta(Con_Group* cg, Unit* ru);
  // initialize variables that change every delta-weight computation

  inline float 		C_Compute_Net(Connection* cn, Unit* ru, Unit* su);
  inline virtual float 	Compute_Net(Con_Group* cg, Unit* ru);
  // compute net input for weights in this con group
  inline void 		C_Send_Net(Connection* cn, Unit* ru, Unit* su);
  inline virtual void 	Send_Net(Con_Group* cg, Unit* su);
  // sender-based net input for con group (send net input to receivers)
  inline float 		C_Compute_Dist(Connection* cn, Unit* ru, Unit* su);
  inline virtual float 	Compute_Dist(Con_Group* cg, Unit* ru);
  // compute net distance for con group (ie. euclidean distance)
  inline void		C_Compute_dWt(Connection*, Unit*, Unit*)	{ };
  inline virtual void	Compute_dWt(Con_Group* cg, Unit* ru);
  // compute the delta-weight change
  inline void 		C_UpdateWeights(Connection*, Unit*, Unit*)	{ };
  inline virtual void 	UpdateWeights(Con_Group* cg, Unit* ru);
  // update weights (ie. add delta-wt to wt, zero delta-wt)
};

class Con_Group : public taGroup_impl {
public:
  taList_impl	units;
  ConSpec* 	spec;		// specification for connections
  Projection*	prjn; 		
  // pointer the the projection which created this Group
  int		other_idx;
  // index of other side of con group (for recv_gp = send_idx, send_gp = recv_idx)
  bool		own_cons;	
  // #READ_ONLY true if this group "owns" the connections (must be recv)

#ifndef SMALL_OBJS
  char		fill[20];
#endif

  // use these functions to get at the connection and unit
  Connection* 	Cn(uint i) const 	{ return (Connection*)FastEl(i); }
  Unit*		Un(uint i) const 	{ return (Unit*)units.FastEl(i); }


  void	Alloc(int no) { units.Alloc(no); taGroup_impl::Alloc(no); }

  void 	InitWtState(Unit* ru)	 	{ spec->InitWtState(this,ru); }
  void	InitWtState_post(Unit* ru) 	{ spec->InitWtState_post(this,ru); } // #IGNORE
  void 	InitWtDelta(Unit* ru)	 	{ spec->InitWtDelta(this,ru); }

  float Compute_Net(Unit* ru)	 	{ return spec->Compute_Net(this,ru); }
  void 	Send_Net(Unit* su)		{ spec->Send_Net(this, su); }
  float Compute_Dist(Unit* ru)	 	{ return spec->Compute_Dist(this,ru); }
  void 	UpdateWeights(Unit* ru)	 	{ spec->UpdateWeights(this,ru); }
  void  Compute_dWt(Unit* ru)	 	{ spec->Compute_dWt(this,ru); }

  virtual ~Con_Group()	{ };
};

class UnitSpec : public BaseSpec {
public:
  MinMax	act_range;		// range of activation for units
  ConSpec*	bias_spec;
  // con spec that controls the bias connection on the unit

#ifndef SMALL_OBJS
  char		fill2[24];
#endif

  virtual void 	InitState(Unit* u);	// initialize unit state variables
  virtual void 	InitWtDelta(Unit* u); 	// init weight delta variables
  virtual void 	InitWtState(Unit* u); 	// init weight state variables
  virtual void	InitWtState_post(Unit* u); // #IGNORE run after init wt state (ie. to scale wts)

  virtual void 	Compute_Net(Unit* u);
  virtual void 	Send_Net(Unit* u);
  virtual void 	Compute_Act(Unit* u);

  virtual void 	Compute_dWt(Unit* u); 	// compute change in weights
  virtual void 	UpdateWeights(Unit* u);	// update weights from deltas

  virtual ~UnitSpec()	{ };
};

class Unit : public taNBase {
public:
  enum ExtType {		// indicates type of external input unit received
    NO_EXTERNAL = 0x00,		// no input
    TARG 	= 0x01,		// target input (value is in targ)
    EXT 	= 0x02,		// external input (value is in ext)
    TARG_EXT 	= 0x03,		// target and external input
    COMP	= 0x04,		// comparison value (for error) (value is in targ)
    COMP_TARG	= 0x05,		// comparision and target
    COMP_EXT	= 0x06,		// comparison and external input
    COMP_TARG_EXT = 0x07 	// comparison, target, and external input
  };

  UnitSpec*	spec;		// unit specification
  ExtType	ext_flag;	// #READ_ONLY tells what kind of external input unit received
  float 	targ;		// target pattern
  float 	ext;		// external input
  float 	act;		// activation
  float 	net;		// net input
  Con_Group 	recv;		// Receiving Connection Groups
  Con_Group 	send;		// Sending Connection Groups
  Connection*	bias;		// #OWN_POINTER bias weight (type set in unit spec)

#ifndef SMALL_OBJS
  char		fill[40];
#endif

  void 		InitExterns()	{ ext = targ = 0.0f; ext_flag = NO_EXTERNAL; }
  // #MENU #MENU_ON_Actions initialize unit external input variables
  void		SetExtFlag(ExtType flg) { ext_flag = (ExtType)(ext_flag | flg); }
  void		UnSetExtFlag(ExtType flg) { ext_flag = (ExtType)(ext_flag & ~flg); }
  virtual void 	InitDelta()	{ net = 0.0f; }

  // these are convenience functions for those defined in the spec
  void 	InitState()		{ spec->InitState(this); }
  // #MENU initialize unit state variables
  void 	InitWtDelta()		{ spec->InitWtDelta(this); }
  // #MENU Initialze weight changes
  void 	InitWtState()		{ spec->InitWtState(this); }
  // #MENU Initialize weight values
  void	InitWtState_post() 	{ spec->InitWtState_post(this); } // #IGNORE
  void 	Compute_Net()		{ spec->Compute_Net(this); }	 
  void 	Send_Net()		{ spec->Send_Net(this); }	  
  void 	Compute_Act()		{ spec->Compute_Act(this); }	  
  void 	UpdateWeights()		{ spec->UpdateWeights(this); }	  
  void 	Compute_dWt()		{ spec->Compute_dWt(this); }	  

  virtual ~Unit()	{ };
};


//////////////////////////////////////////////////////////
// 	Inline Connection-level functions (fast)	//
//////////////////////////////////////////////////////////

inline void ConSpec::ApplyLimits(Con_Group* cg, Unit* ru) {
  if(wt_limits.type != WeightLimits::NONE) {
    CON_GROUP_LOOP(cg, C_ApplyLimits(cg->Cn(i), ru, cg->Un(i)));
  }
}

inline void ConSpec::C_InitWtState(Connection* cn, Unit* ru, Unit* su) {
  cn->wt = drand48() - 0.5f;
  C_ApplyLimits(cn,ru,su);
}

inline void ConSpec::InitWtState(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtState(cg->Cn(i), ru, cg->Un(i)));
  InitWtDelta(cg,ru);
}

inline void ConSpec::InitWtState_post(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtState_post(cg->Cn(i), ru, cg->Un(i)));
}

inline void ConSpec::InitWtDelta(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtDelta(cg->Cn(i), ru, cg->Un(i)));
}

inline float ConSpec::C_Compute_Net(Connection* cn, Unit*, Unit* su) {
  return cn->wt * su->act;
}
inline float ConSpec::Compute_Net(Con_Group* cg, Unit* ru) {
  float rval=0.0f;
  CON_GROUP_LOOP(cg, rval += C_Compute_Net(cg->Cn(i), ru, cg->Un(i)));
  return rval;
}

inline void ConSpec::C_Send_Net(Connection* cn, Unit* ru, Unit* su) {
  ru->net += cn->wt * su->act;
}
inline void ConSpec::Send_Net(Con_Group* cg, Unit* su) {
  CON_GROUP_LOOP(cg, C_Send_Net(cg->Cn(i), cg->Un(i), su));
}

inline float ConSpec::C_Compute_Dist(Connection* cn, Unit*, Unit* su) {
  float tmp = su->act - cn->wt;
  return tmp * tmp;
}
inline float ConSpec::Compute_Dist(Con_Group* cg, Unit* ru) {
  float rval=0.0f;
  CON_GROUP_LOOP(cg, rval += C_Compute_Dist(cg->Cn(i), ru, cg->Un(i)));
  return rval;
}

inline void ConSpec::UpdateWeights(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_UpdateWeights(cg->Cn(i), ru, cg->Un(i)));
  ApplyLimits(cg,ru); // ApplySymmetry(cg,ru);  don't apply symmetry during learning..
}

inline void ConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg, C_Compute_dWt(cg->Cn(i), ru, cg->Un(i)));
}

class Projection : public taNBase {
  // Projection describes connectivity between layers (from receivers perspective)
public:
  enum PrjnSource {
    NEXT,		// Recv from the next layer in network
    PREV,		// Recv from the previous layer in network
    SELF,		// Recv from the same layer
    CUSTOM 		// Recv from the layer spec'd in the projection
  };
  
  Layer* 		layer;    	// #READ_ONLY #NO_SAVE layer this prjn is in
  PrjnSource 		from_type;	// Source of the projections 
  Layer*		from;		// layer receiving from (set this for custom)
  TypeDef*		con_type;	// #TYPE_Connection Type of connection
  TypeDef*		con_gp_type;	// #TYPE_Con_Group Type of connection group
  int			recv_idx;	// #READ_ONLY receiving con_group index
  int			send_idx;	// #READ_ONLY sending con_group index
  int			recv_n;		// #READ_ONLY number of receiving con_groups
  int			send_n;		// #READ_ONLY number of sending con_groups

  bool			projected; 	 // #HIDDEN t/f if connected

#ifndef SMALL_OBJS
  char		fill[52];
#endif

  Projection();
};

class Layer : public taNBase {
public:
  taGroup_impl		units;		// units or groups of units
  taGroup_impl		projections;	// projections
  bool			lesion;		// inactivate this layer from processing (reversable)
  Unit::ExtType		ext_flag;	// #READ_ONLY indicates which kind of external input layer received

#ifndef SMALL_OBJS
  char		fill[160];
#endif

  virtual void  InitExterns();	// Initializes external and target inputs
  virtual void  InitDelta();	// Initialize the unit deltas
  virtual void  InitState();	// Initialize the unit state variables
  virtual void	ModifyState(); // Alters state in an algorithm-specific way
  virtual void  InitWtDelta();	// Initialize the deltas
  virtual void  InitWtState();	
  // #MENU #LABEL_Init_Weights #CONFIRM Initialize the weights
  virtual void	InitWtState_post(); // #IGNORE run after init wt state (ie. to scale wts..)

  virtual void	Compute_Net();	// Compute NetInput
  virtual void	Send_Net();	// Compute NetInput: sender based
  virtual void	Compute_Act();	// Compute Activation
  virtual void	UpdateWeights(); // update weights for whole layer
  virtual void	Compute_dWt();	 // update weights for whole layer

  void	SetExtFlag(int flg)   { ext_flag = (Unit::ExtType)(ext_flag | flg); }
  void	UnSetExtFlag(int flg) { ext_flag = (Unit::ExtType)(ext_flag & ~flg); }
  
  Layer();
};


class Network : public taNBase {
public:
  taGroup_impl	layers;		// Layers or Groups of Layers
  int		epoch;		// epoch counter (updated by process)
  bool		re_init;	// should net be initialized (InitWtState) by process?

#ifndef SMALL_OBJS
  char		fill[292];
#endif

  virtual void  InitExterns();	// Initializes external and target inputs
  virtual void  InitDelta();	// Initialize the unit deltas
  virtual void  InitState();
  // #MENU #MENU_SEP_BEFORE Initialize the unit state variables
  virtual void	ModifyState();  // Alters the state in an algorithm-specify way
  virtual void  InitWtDelta();	// Initialize the Weight deltas
  virtual void  InitWtState();
  // #MENU #CONFIRM Initialize the weights, reset the epoch ctr to 0
  virtual void	InitWtState_post(); // #IGNORE run after init wt state (ie. to scale wts..)

  virtual void	Compute_Net();	// Compute NetInput
  virtual void	Compute_Act();	// Compute Activation
  virtual void	UpdateWeights(); // update weights for whole net
  virtual void	Compute_dWt(); // update weights for whole net
};

#endif // pdp_base_h

