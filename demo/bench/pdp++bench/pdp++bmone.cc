/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

//////////////////////////////////////////
//		ta_string.h		//
//////////////////////////////////////////

class bmStrRep {                   // internal bmString representations
public:
  unsigned short    len;         // string length 
  unsigned short    sz;          // allocated space
  char              s[1];        // the string starts here 
                                 // (at least 1 char for trailing null)
                                 // allocated & expanded via non-public fcts
};

class bmString {
protected:
  bmStrRep*           rep;   // bmStrings are pointers to their representations
public:
  bmString()	{ rep = NULL; }
  ~bmString()	{ };
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

class bmtaRefN {
protected:
  uint 		refn;
public:
  bmtaRefN()	{ refn = 0; }
  virtual ~bmtaRefN()	{ };
};

class bmTypeDef : public bmtaRefN {
public:
  virtual ~bmTypeDef()	{ };
};

//////////////////////////////////////////
//		ta_base.h		//
//////////////////////////////////////////

class bmtaBase {
protected:
  uint		refn;	// simulate bmtaBase
public:
  bmtaBase() 	{ refn = 0; }
  virtual ~bmtaBase()	{ };
};

typedef bmtaBase* bmTAPtr;

class bmtaNBase : public bmtaBase {
public:
  bmTAPtr		owner;		// #READ_ONLY #NO_SAVE pointer to owner
  bmString	name;		// name of the object
  
  bmtaNBase() 	{ owner = NULL; }
};

//////////////////////////////////////////
//		ta_list.h		//
//////////////////////////////////////////

class bmtaPtrList_impl {
public:
// put this in the actual thing literally
  bmtaBase**	el;			// the elements themselves
  int 		alloc_size;		// allocation size
  void*		hash_table;	// #READ_ONLY #NO_SAVE a hash table (NULL if not used)
  int		size;		// #READ_ONLY #NO_SAVE number of elements in the list

  bmTAPtr		El_(uint i) const				
  { bmTAPtr rval=NULL; if((int)i < size) rval = el[i]; return rval; }

  bmtaPtrList_impl() 	{ size = 0; el = NULL; alloc_size = 0; hash_table = NULL; }
  virtual ~bmtaPtrList_impl()	{ el = NULL; size = alloc_size = 0; hash_table = NULL; }
};

class bmtaList_impl : public bmtaBase, public bmtaPtrList_impl {
public:
  bmTypeDef*	el_base;	// #HIDDEN #NO_SAVE Base type for objects in group
  bmTypeDef* 	el_typ;		// #TYPE_ON_el_base Default type for objects in group
  uint		el_def;		// Index of default element in group
  bmString	name;		// name of the list
  bmTAPtr		owner;		// #READ_ONLY #NO_SAVE owner of the list

  bmTAPtr		El(uint idx) const		{ return (bmTAPtr)El_(idx); }
  bmTAPtr		FastEl(uint i) const		{ return (bmTAPtr)el[i]; } 
  bmTAPtr		operator[](uint i) const	{ return El(i); }

  bmtaList_impl() { el_base = NULL; el_typ = NULL; el_def = 0; owner = NULL; }
  virtual ~bmtaList_impl()	{ };
};

//////////////////////////////////////////
//		ta_group.h		//
//////////////////////////////////////////

class 	bmtaGroup_impl;		// pre-declare
typedef bmtaGroup_impl* bmTAGPtr;
typedef bmtaList_impl bmTALOG; // list of groups (LOG)

class bmtaSubGroup : public bmtaList_impl {
public:
};

class	bmtaLeafItr {		// contains the indicies for iterating over leafs
public:
  bmTAGPtr	cgp;		// pointer to current group
  int		g;		// index of current group
  int		i;		// index of current leaf element
};

#define FOR_ITR_EL(T, el, grp, itr) \
for(el = (T*) grp ## FirstEl(itr); el; el = (T*) grp ## NextEl(itr))

#define FOR_ITR_GP(T, el, grp, itr) \
for(el = (T*) grp ## FirstGp(itr); el; el = (T*) grp ## NextGp(itr))

class bmtaGroup_impl : public bmtaList_impl {
public:
  uint 		leaves;		// #READ_ONLY #NO_SAVE total number of leaves
  bmtaSubGroup	gp; 		// #HIDDEN #NO_FIND #NO_SAVE sub-groups within this one
  bmTAGPtr	super_gp;	// #READ_ONLY #NO_SAVE super-group above this
  bmTALOG* 	leaf_gp; 	// #READ_ONLY #NO_SAVE 'flat' list of leaf-containing-gps 

  // only one group possible -- the group itself
  bmTAGPtr 	FirstGp(int& g) const  
  { g = 0; bmTAGPtr rval = NULL; if(size > 0) rval = (bmTAGPtr)this; return rval; }
  bmTAGPtr 	NextGp(int&)	const  { return NULL; }

  bmTAPtr		FirstEl(bmtaLeafItr& lf) const
  { bmTAPtr rval=NULL; lf.i = 0; lf.cgp = FirstGp(lf.g);
    if(lf.cgp != NULL) rval=(bmTAPtr) lf.cgp->el[0]; return rval; }
  bmTAPtr	 	NextEl(bmtaLeafItr& lf)	const
  { bmTAPtr rval=NULL; if(++lf.i >= lf.cgp->size) { lf.i = 0; lf.cgp = NULL; }
    if(lf.cgp != NULL) rval = (bmTAPtr)lf.cgp->el[lf.i]; return rval; }
  
  bmtaGroup_impl() 	{ };
};


//////////////////////////////////////////
//		minmax.h		//
//////////////////////////////////////////

class bmMinMax : public bmtaBase {
public:
  float		min;	// minimum value
  float		max;	// maximum value
  float		range;	// #HIDDEN distance between min and max
  float		scale;	// #HIDDEN scale (1.0 / range)

  bool	operator != (bmMinMax& mm) const
  { return ((mm.min != min) || (mm.max != max)); }

  bool	operator == (bmMinMax& mm) const
  { return ((mm.min == min) && (mm.max == max)); }

  void	Init(float it)	{ min = max = it; }  // initializes the max and min to this value

  inline float	Range()	const		{ return (max - min); }
  inline float	MidPoint() const	{ return .5 * (min + max); }
  // returns the range between the min and the max

  void	UpdateRange(bmMinMax& it)
  { min = MIN(it.min, min); max = MAX(it.max, max); }

  void	UpdateRange(float it)
  { min = MIN(it, min);	max = MAX(it, max); }  // updates the range

  void	MaxLT(float it)		{ max = MIN(it, max); }
  // max less than (or equal)

  void	MinGT(float it)		{ min = MAX(it, min); }
  // min greater than (or equal)

  void  WithinRange(bmMinMax& it)		// put my range within given one
  { min = MAX(it.min, min); max = MIN(it.max, max); }

  float	Normalize(float val) const	{ return (val - min) / Range(); }
  // normalize given value to 0-1 range given current in max

  float	Clip(float val) const		
  { val = MIN(max,val); val = MAX(min,val); return val; }
  // clip given value within current range

  void	UpdateAfterEdit()
  { range = Range(); if(range != 0) scale = 1.0 / range; }

  bmMinMax() 		{ min = 0; max = 1; range = 1; scale = 1; }
};

//////////////////////////////////////////
//		spec.h			//
//////////////////////////////////////////

#define MAX_SPEC_LONGS	((256 / (sizeof(long) * 8)) + 1)

class bmBaseSpec : public bmtaNBase {
public:
  unsigned long unique[MAX_SPEC_LONGS]; // #HIDDEN bits representing members unique
  bmTypeDef*	min_obj_type;
  bmtaGroup_impl 	children;
#ifndef SMALL_OBJS
  char		fill[12];
#endif
};

//////////////////////////////////////////
//		netstru.h		//
//////////////////////////////////////////

class bmUnit;
class bmCon_Group;
class bmProjection;
class bmLayer;

const float SIGMOID_MAX_VAL = 0.999999f; // max eval value
const float SIGMOID_MIN_VAL = 0.000001f; // min eval value 
const float SIGMOID_MAX_NET = 13.81551f;	// maximium net input value

class bmSigmoidSpec : public bmtaBase {
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

  bmSigmoidSpec()		{ off = 0.0f; gain = 1.0f; }
};

class bmWeightLimits : public bmtaBase {
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

  bmWeightLimits()		{ type = NONE; min = -1.0f; max = 1.0f; sym = false; }
};

class bmConnection : public bmtaBase {
public:
  float 	wt;		// Weight of bmConnection

  bmConnection() { wt = 0.0f; }
};

#define	CON_GROUP_LOOP(cg, expr) \
  int i; for(i=0; i<cg->size; i++) \
    expr

class bmConSpec : public bmBaseSpec {
public:
  bmTypeDef*		min_con_type;
  // #HIDDEN #NO_SAVE #TYPE_bmConnection mimimal con type required for spec (obj is con group)
//  Random	rnd;		// Weight randomization specification
  bmWeightLimits	wt_limits;	// limits on weight sign, symmetry

#ifndef SMALL_OBJS
  char		fill2[24];
#endif

  inline void		C_ApplyLimits(bmConnection* cn, bmUnit*, bmUnit*)
  { wt_limits.ApplyLimits(cn->wt); }
  inline virtual void	ApplyLimits(bmCon_Group* cg, bmUnit* ru);
  // apply weight limits (sign, magnitude)
    
  inline virtual void	C_InitWtState(bmConnection* cn, bmUnit* ru, bmUnit* su);
  inline virtual void 	InitWtState(bmCon_Group* cg, bmUnit* ru);
  // initialize state variables (ie. at beginning of training)
  inline virtual void	C_InitWtState_post(bmConnection*, bmUnit*, bmUnit*) { };
  // #IGNORE
  inline virtual void 	InitWtState_post(bmCon_Group* cg, bmUnit* ru); 
  // #IGNORE post-initialize state variables (ie. for scaling symmetrical weights, etc)
  inline virtual void 	C_InitWtDelta(bmConnection*, bmUnit*, bmUnit*)	{ };
  inline virtual void 	InitWtDelta(bmCon_Group* cg, bmUnit* ru);
  // initialize variables that change every delta-weight computation

  inline float 		C_Compute_Net(bmConnection* cn, bmUnit* ru, bmUnit* su);
  inline virtual float 	Compute_Net(bmCon_Group* cg, bmUnit* ru);
  // compute net input for weights in this con group
  inline void 		C_Send_Net(bmConnection* cn, bmUnit* ru, bmUnit* su);
  inline virtual void 	Send_Net(bmCon_Group* cg, bmUnit* su);
  // sender-based net input for con group (send net input to receivers)
  inline float 		C_Compute_Dist(bmConnection* cn, bmUnit* ru, bmUnit* su);
  inline virtual float 	Compute_Dist(bmCon_Group* cg, bmUnit* ru);
  // compute net distance for con group (ie. euclidean distance)
  inline void		C_Compute_dWt(bmConnection*, bmUnit*, bmUnit*)	{ };
  inline virtual void	Compute_dWt(bmCon_Group* cg, bmUnit* ru);
  // compute the delta-weight change
  inline void 		C_UpdateWeights(bmConnection*, bmUnit*, bmUnit*)	{ };
  inline virtual void 	UpdateWeights(bmCon_Group* cg, bmUnit* ru);
  // update weights (ie. add delta-wt to wt, zero delta-wt)
};

class bmCon_Group : public bmtaGroup_impl {
public:
  bmtaList_impl	units;
  bmConSpec* 	spec;		// specification for connections
  bmProjection*	prjn; 		
  // pointer the the projection which created this Group
  int		other_idx;
  // index of other side of con group (for recv_gp = send_idx, send_gp = recv_idx)
  bool		own_cons;	
  // #READ_ONLY true if this group "owns" the connections (must be recv)

#ifndef SMALL_OBJS
  char		fill[20];
#endif

  // use these functions to get at the connection and unit
  bmConnection* 	Cn(uint i) const 	{ return (bmConnection*)FastEl(i); }
  bmUnit*		Un(uint i) const 	{ return (bmUnit*)units.FastEl(i); }

  void 	InitWtState(bmUnit* ru)	 	{ spec->InitWtState(this,ru); }
  void	InitWtState_post(bmUnit* ru) 	{ spec->InitWtState_post(this,ru); } // #IGNORE
  void 	InitWtDelta(bmUnit* ru)	 	{ spec->InitWtDelta(this,ru); }

  float Compute_Net(bmUnit* ru)	 	{ return spec->Compute_Net(this,ru); }
  void 	Send_Net(bmUnit* su)		{ spec->Send_Net(this, su); }
  float Compute_Dist(bmUnit* ru)	 	{ return spec->Compute_Dist(this,ru); }
  void 	UpdateWeights(bmUnit* ru)	 	{ spec->UpdateWeights(this,ru); }
  void  Compute_dWt(bmUnit* ru)	 	{ spec->Compute_dWt(this,ru); }

  virtual ~bmCon_Group()	{ };
};

class bmUnitSpec : public bmBaseSpec {
public:
  bmMinMax	act_range;		// range of activation for units
  bmConSpec*	bias_spec;
  // con spec that controls the bias connection on the unit

#ifndef SMALL_OBJS
  char		fill2[24];
#endif

  virtual void 	InitState(bmUnit* u);	// initialize unit state variables
  virtual void 	InitWtDelta(bmUnit* u); 	// init weight delta variables
  virtual void 	InitWtState(bmUnit* u); 	// init weight state variables
  virtual void	InitWtState_post(bmUnit* u); // #IGNORE run after init wt state (ie. to scale wts)

  virtual void 	Compute_Net(bmUnit* u);
  virtual void 	Send_Net(bmUnit* u);
  virtual void 	Compute_Act(bmUnit* u);

  virtual void 	Compute_dWt(bmUnit* u); 	// compute change in weights
  virtual void 	UpdateWeights(bmUnit* u);	// update weights from deltas

  virtual ~bmUnitSpec()	{ };
};

class bmUnit : public bmtaNBase {
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

  bmUnitSpec*	spec;		// unit specification
  ExtType	ext_flag;	// #READ_ONLY tells what kind of external input unit received
  float 	targ;		// target pattern
  float 	ext;		// external input
  float 	act;		// activation
  float 	net;		// net input
  bmCon_Group 	recv;		// Receiving bmConnection Groups
  bmCon_Group 	send;		// Sending bmConnection Groups
  bmConnection*	bias;		// #OWN_POINTER bias weight (type set in unit spec)

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

  virtual ~bmUnit()	{ };
};


//////////////////////////////////////////////////////////
// 	Inline bmConnection-level functions (fast)	//
//////////////////////////////////////////////////////////

inline void bmConSpec::ApplyLimits(bmCon_Group* cg, bmUnit* ru) {
  if(wt_limits.type != bmWeightLimits::NONE) {
    CON_GROUP_LOOP(cg, C_ApplyLimits(cg->Cn(i), ru, cg->Un(i)));
  }
}

inline void bmConSpec::C_InitWtState(bmConnection* cn, bmUnit* ru, bmUnit* su) {
  cn->wt = drand48() - 0.5f;
  C_ApplyLimits(cn,ru,su);
}

inline void bmConSpec::InitWtState(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtState(cg->Cn(i), ru, cg->Un(i)));
  InitWtDelta(cg,ru);
}

inline void bmConSpec::InitWtState_post(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtState_post(cg->Cn(i), ru, cg->Un(i)));
}

inline void bmConSpec::InitWtDelta(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg, C_InitWtDelta(cg->Cn(i), ru, cg->Un(i)));
}

inline float bmConSpec::C_Compute_Net(bmConnection* cn, bmUnit*, bmUnit* su) {
  return cn->wt * su->act;
}
inline float bmConSpec::Compute_Net(bmCon_Group* cg, bmUnit* ru) {
  float rval=0.0f;
  CON_GROUP_LOOP(cg, rval += C_Compute_Net(cg->Cn(i), ru, cg->Un(i)));
  return rval;
}

inline void bmConSpec::C_Send_Net(bmConnection* cn, bmUnit* ru, bmUnit* su) {
  ru->net += cn->wt * su->act;
}
inline void bmConSpec::Send_Net(bmCon_Group* cg, bmUnit* su) {
  CON_GROUP_LOOP(cg, C_Send_Net(cg->Cn(i), cg->Un(i), su));
}

inline float bmConSpec::C_Compute_Dist(bmConnection* cn, bmUnit*, bmUnit* su) {
  float tmp = su->act - cn->wt;
  return tmp * tmp;
}
inline float bmConSpec::Compute_Dist(bmCon_Group* cg, bmUnit* ru) {
  float rval=0.0f;
  CON_GROUP_LOOP(cg, rval += C_Compute_Dist(cg->Cn(i), ru, cg->Un(i)));
  return rval;
}

inline void bmConSpec::UpdateWeights(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg, C_UpdateWeights(cg->Cn(i), ru, cg->Un(i)));
  ApplyLimits(cg,ru); // ApplySymmetry(cg,ru);  don't apply symmetry during learning..
}

inline void bmConSpec::Compute_dWt(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg, C_Compute_dWt(cg->Cn(i), ru, cg->Un(i)));
}

class bmProjection : public bmtaNBase {
  // bmProjection describes connectivity between layers (from receivers perspective)
public:
  enum PrjnSource {
    NEXT,		// Recv from the next layer in network
    PREV,		// Recv from the previous layer in network
    SELF,		// Recv from the same layer
    CUSTOM 		// Recv from the layer spec'd in the projection
  };
  
  bmLayer* 		layer;    	// #READ_ONLY #NO_SAVE layer this prjn is in
  PrjnSource 		from_type;	// Source of the projections 
  bmLayer*		from;		// layer receiving from (set this for custom)
  bmTypeDef*		con_type;	// #TYPE_bmConnection Type of connection
  bmTypeDef*		con_gp_type;	// #TYPE_bmCon_Group Type of connection group
  int			recv_idx;	// #READ_ONLY receiving con_group index
  int			send_idx;	// #READ_ONLY sending con_group index
  int			recv_n;		// #READ_ONLY number of receiving con_groups
  int			send_n;		// #READ_ONLY number of sending con_groups

  bool			projected; 	 // #HIDDEN t/f if connected

#ifndef SMALL_OBJS
  char		fill[52];
#endif

  bmProjection();
};

class bmLayer : public bmtaNBase {
public:
  bmtaGroup_impl		units;		// units or groups of units
  bmtaGroup_impl		projections;	// projections
  bool			lesion;		// inactivate this layer from processing (reversable)
  bmUnit::ExtType		ext_flag;	// #READ_ONLY indicates which kind of external input layer received

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
  virtual void	Compute_Act();	// Compute Activation
  virtual void	UpdateWeights(); // update weights for whole layer
  virtual void	Compute_dWt();	 // update weights for whole layer

  void	SetExtFlag(int flg)   { ext_flag = (bmUnit::ExtType)(ext_flag | flg); }
  void	UnSetExtFlag(int flg) { ext_flag = (bmUnit::ExtType)(ext_flag & ~flg); }
  
  bmLayer();
};


class bmNetwork : public bmtaNBase {
public:
  bmtaGroup_impl	layers;		// bmLayers or Groups of bmLayers
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

/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

void bmUnitSpec::InitState(bmUnit* u) {
  u->InitExterns();
  u->InitDelta();
  u->act = 0.0f;
}

void bmUnitSpec::InitWtDelta(bmUnit* u) {
  bmCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtDelta(u);
  }
  if(u->bias != NULL)
    bias_spec->C_InitWtDelta(u->bias, u, NULL); // this is a virtual fun
}

void bmUnitSpec::InitWtState(bmUnit* u) {
  bmCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtState(u);
  }
  if(u->bias != NULL)
    bias_spec->C_InitWtState(u->bias, u, NULL); // this is a virtual fun
}

void bmUnitSpec::InitWtState_post(bmUnit* u) {
  bmCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->InitWtState_post(u);
  }
}

void bmUnitSpec::Compute_Net(bmUnit* u) {
  bmCon_Group* recv_gp;
  u->net = 0.0f;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      u->net += recv_gp->Compute_Net(u);
  }
  if(u->bias != NULL)
    u->net += u->bias->wt;
}

void bmUnitSpec::Send_Net(bmUnit* u) {
  bmCon_Group* send_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, send_gp, u->send., g) {
    if(!send_gp->prjn->layer->lesion)
      send_gp->Send_Net(u);
  }
  if(u->bias != NULL)
    u->net += u->bias->wt;
}

void bmUnitSpec::Compute_Act(bmUnit* u) {
  if(u->ext_flag & bmUnit::EXT)
    u->act = u->ext;
  else
    u->act = u->net;
}

void bmUnitSpec::Compute_dWt(bmUnit* u) {
  bmCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->Compute_dWt(u);
  }
  // NOTE: derived classes must supply bias->Compute_dWt call because C_Compute_dWt 
  // is not virtual, so if called here, only bmConSpec version would be called. 
  // This is not true of InitWtState and InitWtDelta, which are virtual.
}

void bmUnitSpec::UpdateWeights(bmUnit* u) {
  bmCon_Group* recv_gp;
  int g;
  FOR_ITR_GP(bmCon_Group, recv_gp, u->recv., g) {
    if(!recv_gp->prjn->from->lesion)
      recv_gp->UpdateWeights(u);
  }
  // NOTE: derived classes must supply bias->UpdateWeights call because C_UpdateWeights
  // is not virtual, so if called here, only bmConSpec version would be called. 
  // This is not true of InitWtState and InitWtDelta, which are virtual.
}

bmProjection::bmProjection() {
  layer = from = NULL;
  from_type = PREV;
  con_type = NULL;
  con_gp_type = NULL;
  recv_idx = -1;
  send_idx = -1;
  recv_n = 1;
  send_n = 1;
  projected = false;
}

//////////////////////////
//	bmLayer		// 
//////////////////////////

bmLayer::bmLayer() {
  lesion = false; ext_flag = bmUnit::NO_EXTERNAL;
}

void bmLayer::InitExterns() {
  if(ext_flag == bmUnit::NO_EXTERNAL)
    return;
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitExterns();
  ext_flag = bmUnit::NO_EXTERNAL;
}

void  bmLayer::InitDelta() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitDelta();
}

void  bmLayer::InitState() {
  ext_flag = bmUnit::NO_EXTERNAL;
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitState();
}

void bmLayer::ModifyState() {
}

void  bmLayer::InitWtDelta() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitWtDelta();
}

void bmLayer::InitWtState() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitWtState();
}

void bmLayer::InitWtState_post() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->InitWtState_post();
}


void bmLayer::Compute_Net() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->Compute_Net();
}

void bmLayer::Compute_Act() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->Compute_Act();
}
void bmLayer::UpdateWeights() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->UpdateWeights();
}
void bmLayer::Compute_dWt() {
  bmUnit* u;
  bmtaLeafItr i;
  FOR_ITR_EL(bmUnit, u, units., i)
    u->Compute_dWt();
}

//////////////////////////
//	bmNetwork		// 
//////////////////////////

void bmNetwork::InitExterns(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitExterns();
  }
}

void bmNetwork::InitDelta(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitDelta();
  }
}

void bmNetwork::InitState(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitState();
  }
}

void bmNetwork::ModifyState(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->ModifyState();
  }
}

void bmNetwork::InitWtDelta(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitWtDelta();
  }
}

void bmNetwork::InitWtState(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState();
  }
  InitWtState_post();		// done after all initialization (for scaling wts...)
  InitState();			// also re-init state at this point..
  epoch = 0;			// re-init epoch counter..
  re_init = false;		// no need to re-reinit!
}

void bmNetwork::InitWtState_post() {
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->InitWtState_post();
  }
}

void bmNetwork::Compute_Net(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Net();
  }
}

void bmNetwork::Compute_Act(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->Compute_Act();
  }
}
void bmNetwork::UpdateWeights(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->UpdateWeights();
  }
}
void bmNetwork::Compute_dWt(){
  bmLayer* l;
  bmtaLeafItr i;
  FOR_ITR_EL(bmLayer, l, layers., i) {
    if(!l->lesion)
      l->Compute_dWt();
  }
}

/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

// 


// now just implement bp

//////////////////////////////////////////
//		bp.h			//
//////////////////////////////////////////

class bmBpUnit;
class bmBpCon_Group;

class bmBpCon : public bmConnection {
  // Bp connection
public:
  float 		dwt; 		// #NO_SAVE Change in weight
  float 		dEdW; 		// #NO_SAVE derivative of Error wrt weight

  bmBpCon()		{ dwt = dEdW = 0.0f; }
};

class bmBpConSpec : public bmConSpec {
public:
  enum MomentumType {
    AFTER_LRATE,		// apply momentum after learning rate (old pdp style)
    BEFORE_LRATE,		// apply momentum before learning rate
    NORMALIZED 			// like BEFORE, but normalize direction to unit length
  };

  float 	lrate;		// learning rate
  float 	momentum;	// momentum
  MomentumType	momentum_type;	// type of momentum function to use
  float		momentum_c;	// #READ_ONLY complement of momentum (for NORMALIZED)
  float 	decay;		// decay rate (before lrate and momentum)
  void 		(*decay_fun)(bmBpConSpec* spec, bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su);
  // #LIST_bmBpConSpec_WtDecay the weight decay function to use

  void 		C_InitWtDelta(bmConnection* cn, bmUnit* ru, bmUnit* su)
  { bmConSpec::C_InitWtDelta(cn, ru, su); ((bmBpCon*)cn)->dEdW = 0.0f; }

  void 		C_InitWtState(bmConnection* cn, bmUnit* ru, bmUnit* su) 	
  { bmConSpec::C_InitWtState(cn, ru, su); ((bmBpCon*)cn)->dwt = 0.0f;}

  inline float		C_Compute_dEdA(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su);
  inline virtual float 	Compute_dEdA(bmBpCon_Group* cg, bmBpUnit* su);
  // get error from units I send to

  inline void 		C_Compute_dWt(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su);
  inline void 		Compute_dWt(bmCon_Group* cg, bmUnit* ru);
  inline virtual void	B_Compute_dWt(bmBpCon* cn, bmBpUnit* ru);
  // Compute dE with respect to the weights

  inline void 	C_Compute_WtDecay(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su);
  // call the decay function 

  inline void	C_BEF_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su); // BEFORE_LRATE
  inline void	C_AFT_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su); // AFTER_LRATE
  inline void	C_NRM_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su); // NORMALIZED
  inline void	UpdateWeights(bmCon_Group* cg, bmUnit* ru);
  inline virtual void	B_UpdateWeights(bmBpCon* cn, bmBpUnit* ru);
  // for the bias unit
  
  bmBpConSpec();
};

class bmBpCon_Group : public bmCon_Group {
public:
  float Compute_dEdA(bmBpUnit* su)
  { return ((bmBpConSpec*)spec)->Compute_dEdA(this, su); }
};

class bmBpUnit;

class bmBpUnitSpec : public bmUnitSpec {
public:
  bmSigmoidSpec	sig;		// sigmoid activation parameters
  float		err_tol;	// error tolerance (no error signal if |t-o|<err_tol)
  void 		(*err_fun)(bmBpUnitSpec* spec, bmBpUnit* u);
  // #LIST_bmBpUnit_Error this points to the error fun, set appropriately

  // generic unit functions
  void		InitState(bmUnit* u);
  void 		Compute_Act(bmUnit* u);		// activation from net input (sigmoid)
  void 		Compute_dWt(bmUnit* u); 		// for all of my recv weights
  void 		UpdateWeights(bmUnit* u);		// modify to update bias weight
  
  // bp special functions
  virtual void 	Compute_Error(bmBpUnit* u); 	// call the error function (testing only)
  virtual void 	Compute_dEdA(bmBpUnit* u); 	// error wrt unit activation
  virtual void 	Compute_dEdNet(bmBpUnit* u); 	// error wrt net input

  bmBpUnitSpec();
};

// the following functions are possible error functions. 

// #REG_FUN
void Bp_Squared_Error(bmBpUnitSpec* spec, bmBpUnit* u) 
// #LIST_bmBpUnit_Error Squared error function for bp
     ;				// term here so scanner picks up comment
// #REG_FUN
void Bp_CrossEnt_Error(bmBpUnitSpec* spec, bmBpUnit* u)
// #LIST_bmBpUnit_Error Cross entropy error function for bp
     ;				// term here so scanner picks up comment

class bmBpUnit : public bmUnit {
  // standard feed-forward Bp unit
public:
  float 	err; 		// this is E, not dEdA
  float 	dEdA;		// #LABEL_dEdA error wrt activation
  float 	dEdNet;		// #LABEL_dEdNet error wrt net input

  // these are "convenience" functions for those defined in the spec
  void Compute_Error()		{ ((bmBpUnitSpec*)spec)->Compute_Error(this); }
  void Compute_dEdA()		{ ((bmBpUnitSpec*)spec)->Compute_dEdA(this); }
  void Compute_dEdNet()		{ ((bmBpUnitSpec*)spec)->Compute_dEdNet(this); }
  void Compute_dEdA_dEdNet() 	{ Compute_dEdA(); Compute_dEdNet(); }

  bmBpUnit();
};

// inline functions (for speed)

inline float bmBpConSpec::C_Compute_dEdA(bmBpCon* cn, bmBpUnit* ru, bmBpUnit*) {
  return cn->wt * ru->dEdNet;
}
inline float bmBpConSpec::Compute_dEdA(bmBpCon_Group* cg, bmBpUnit* su) {
  float rval = 0.0f;
  CON_GROUP_LOOP(cg,rval += C_Compute_dEdA((bmBpCon*)cg->Cn(i), (bmBpUnit*)cg->Un(i), su));
  return rval;
}


inline void bmBpConSpec::C_Compute_dWt(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su) {
  cn->dEdW += su->act * ru->dEdNet;
}
inline void bmBpConSpec::Compute_dWt(bmCon_Group* cg, bmUnit* ru) {
  CON_GROUP_LOOP(cg,C_Compute_dWt((bmBpCon*)cg->Cn(i), (bmBpUnit*)ru, (bmBpUnit*)cg->Un(i)));
}
inline void bmBpConSpec::B_Compute_dWt(bmBpCon* cn, bmBpUnit* ru) {
  cn->dEdW += ru->dEdNet;
}

inline void bmBpConSpec::C_Compute_WtDecay(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su) {
  if(decay_fun != NULL)
    (*decay_fun)(this, cn, ru, su);
}
inline void bmBpConSpec::C_AFT_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su); // decay goes into dEdW...
#endif
  cn->dwt = lrate * cn->dEdW + momentum * cn->dwt;
  cn->wt += cn->dwt;
  cn->dEdW = 0.0f;
}
inline void bmBpConSpec::C_BEF_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su);
#endif
  cn->dwt = cn->dEdW + momentum * cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dEdW = 0.0f;
}
inline void bmBpConSpec::C_NRM_UpdateWeights(bmBpCon* cn, bmBpUnit* ru, bmBpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su);
#endif
  cn->dwt = momentum_c * cn->dEdW + momentum * cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dEdW = 0.0f;
}

inline void bmBpConSpec::UpdateWeights(bmCon_Group* cg, bmUnit* ru) {
#ifndef NO_MOMENTUM_OPTS
  if(momentum_type == AFTER_LRATE) {
    CON_GROUP_LOOP(cg, C_AFT_UpdateWeights((bmBpCon*)cg->Cn(i),
					   (bmBpUnit*)ru, (bmBpUnit*)cg->Un(i)));
  }
  else if(momentum_type == BEFORE_LRATE)
#endif
  {
    CON_GROUP_LOOP(cg, C_BEF_UpdateWeights((bmBpCon*)cg->Cn(i),
					   (bmBpUnit*)ru, (bmBpUnit*)cg->Un(i)));
  }
#ifndef NO_MOMENTUM_OPTS
  else {
    CON_GROUP_LOOP(cg, C_NRM_UpdateWeights((bmBpCon*)cg->Cn(i),
					   (bmBpUnit*)ru, (bmBpUnit*)cg->Un(i)));
  }
#endif
#ifndef NO_WT_LIMITS
  ApplyLimits(cg, ru);
#endif
}

inline void bmBpConSpec::B_UpdateWeights(bmBpCon* cn, bmBpUnit* ru) {
#ifndef NO_MOMENTUM_OPTS
  if(momentum_type == AFTER_LRATE) 
    C_AFT_UpdateWeights(cn, ru, NULL);
  else if(momentum_type == BEFORE_LRATE)
#endif
    C_BEF_UpdateWeights(cn, ru, NULL);
#ifndef NO_MOMENTUM_OPTS
  else
    C_NRM_UpdateWeights(cn, ru, NULL);
#endif
#ifndef NO_WT_LIMITS
  C_ApplyLimits(cn, ru, NULL);
#endif
}

/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++


//////////////////////////
//  BP:	bmCon, Spec	//
//////////////////////////

bmBpConSpec::bmBpConSpec() {
  lrate = .25;
  momentum_type = BEFORE_LRATE;
  momentum = .9;
  momentum_c = .1;
  decay = 0;
  decay_fun = NULL;
}


//////////////////////////
//  BP:	bmUnit, Spec	//
//////////////////////////

bmBpUnitSpec::bmBpUnitSpec() {
  err_tol = 0.0f;
  err_fun = Bp_Squared_Error;
}

void bmBpUnitSpec::InitState(bmUnit* u) {
  bmUnitSpec::InitState(u);
  bmBpUnit* bu = (bmBpUnit*)u;
  bu->err = bu->dEdA = bu->dEdNet = 0.0f;
}

void bmBpUnitSpec::Compute_Act(bmUnit* u) { // this does the sigmoid
  if(u->ext_flag & bmUnit::EXT)
    u->act = u->ext;
  else
    u->act = act_range.min + act_range.range * sig.Eval(u->net);
}

void bmBpUnitSpec::Compute_Error(bmBpUnit* u) {
  if(u->ext_flag & bmUnit::TARG) (*err_fun)(this, u);
}

void bmBpUnitSpec::Compute_dEdA(bmBpUnit* u) {
  if(u->ext_flag & bmUnit::TARG) {
    (*err_fun)(this, u);
  }
  else {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  bmBpCon_Group* send_gp;
  int g;
  FOR_ITR_GP(bmBpCon_Group, send_gp, u->send., g) {
    if(!send_gp->prjn->layer->lesion)
      u->dEdA += send_gp->Compute_dEdA(u);
  }
}

void bmBpUnitSpec::Compute_dEdNet(bmBpUnit* u) {
  u->dEdNet = u->dEdA * sig.gain * (u->act - act_range.min) *
    (act_range.max - u->act) * act_range.scale;
}

void bmBpUnitSpec::Compute_dWt(bmUnit* u) {
  if(u->ext_flag & bmUnit::EXT)  return; // don't compute dwts for clamped units
  bmUnitSpec::Compute_dWt(u);
  ((bmBpConSpec*)bias_spec)->B_Compute_dWt((bmBpCon*)u->bias, (bmBpUnit*)u);
}

void bmBpUnitSpec::UpdateWeights(bmUnit* u) {
  if(u->ext_flag & bmUnit::EXT)  return; // don't update for clamped units
  bmUnitSpec::UpdateWeights(u);
  ((bmBpConSpec*)bias_spec)->B_UpdateWeights((bmBpCon*)u->bias, (bmBpUnit*)u);
}

bmBpUnit::bmBpUnit() {
  err = dEdA = dEdNet = 0.0f;
}

void Bp_Squared_Error(bmBpUnitSpec* spec, bmBpUnit* u) {
  u->dEdA = u->targ - u->act;
  if(fabs(u->dEdA) < spec->err_tol) {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  else
    u->err = u->dEdA * u->dEdA;
}

void Bp_CrossEnt_Error(bmBpUnitSpec* spec, bmBpUnit* u) {
  u->dEdA = u->targ - u->act;
  if(fabs(u->dEdA) < spec->err_tol) {
    u->dEdA = 0.0f;
    u->err = 0.0f;
  }
  else {
    u->dEdA /= (u->act - spec->act_range.min) * (spec->act_range.max - u->act)
      * spec->act_range.scale;
    float a = spec->sig.Clip((u->act - spec->act_range.min) * spec->act_range.scale);
    float t = (u->targ - spec->act_range.min) * spec->act_range.scale;
    u->err = (t * logf(a) + (1.0f - t) * logf(1.0f - a));
  }
}


//////////////////////////////////////////////////////////
// 		actually run the simulation!		//
//////////////////////////////////////////////////////////

#ifdef NO_TIME
  extern "C" {
    long clock();
  }
#define CLOCKS_PER_SEC 100
#else
#include <time.h>
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

#define RPT_SZ(x,ex) printf("%s:\t%d\t%d\t%d\n", #x, (int)sizeof(x), ex, (int)(ex - sizeof(x)))
#define RPT_SZT(x,ex) printf("%s:\t\t%d\t%d\t%d\n", #x, (int)sizeof(x), ex, (int)(ex - sizeof(x)))

void ApplyPats(bmNetwork& net) {
  bmLayer* lay;
  bmtaLeafItr li;
  FOR_ITR_EL(bmLayer, lay, net.layers., li) {
    if(li.i == 0)
      lay->ext_flag = bmUnit::EXT;
    else if(li.i == net.layers.size-1)
      lay->ext_flag = bmUnit::TARG;
    else
      continue;
    bmUnit* un;
    bmtaLeafItr ui;
    FOR_ITR_EL(bmUnit, un, lay->units., ui) {
      if(li.i == 0) {
	un->ext_flag = bmUnit::EXT;
	un->ext = drand48();
      }
      else if(li.i==net.layers.size-1) {
	un->ext_flag = bmUnit::TARG;
	un->targ = drand48();
      }
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("must have 2 args:\n\t<n_units>\tnumber of units in each of 3 layers\n");
    printf("\t<n_pats>\tnumber of pattern presentations\n");

    printf("\nSizes:\n");
    RPT_SZT(bmtaBase, 8);
    RPT_SZ(bmtaNBase, 16);
    RPT_SZ(bmtaPtrList_impl, 20);
    RPT_SZ(bmtaList_impl, 48);
    RPT_SZ(bmtaGroup_impl, 108);
    RPT_SZ(bmBaseSpec, 176);
    RPT_SZ(bmConnection, 12);
    RPT_SZ(bmConSpec, 228);
    RPT_SZ(bmCon_Group, 192);
    RPT_SZ(bmUnitSpec, 228);
    RPT_SZT(bmUnit, 468);
    RPT_SZ(bmProjection, 108);
    RPT_SZT(bmLayer, 400);
    RPT_SZ(bmNetwork, 424);
    RPT_SZT(bmBpCon, 20);
    RPT_SZ(bmBpConSpec, 252);
    RPT_SZ(bmBpCon_Group, 192);
    RPT_SZ(bmBpUnitSpec, 252);
    RPT_SZT(bmBpUnit, 480);
    return 1;
  }

  int n_units = (int)strtol(argv[1], NULL, 0);
  int n_pats = (int)strtol(argv[2], NULL, 0);
  int n_lays = 3;

  int min_new = 8;

  bmBpUnitSpec us;
  bmBpConSpec cs;
  us.bias_spec = &cs;
  
  bmNetwork net;
  net.layers.el = new bmtaBase*[8];
  net.layers.size = n_lays;
  int l;
  for(l=0; l<n_lays; l++) {
    bmLayer* lay = new bmLayer;
    net.layers.el[l] = lay;
    lay->units.el = new bmtaBase*[n_units + min_new];
    lay->units.size = n_units;
    if(l == 0)
      lay->ext_flag = bmUnit::EXT;
    else if(l == n_lays-1)
      lay->ext_flag = bmUnit::TARG;
    if(l > 0) {
      lay->projections.el = new bmtaBase*[min_new];
      lay->projections.size = 1;
      bmProjection* prjn = new bmProjection;
      lay->projections.el[0] = prjn;
      prjn->layer = lay;
      prjn->from = (bmLayer*)net.layers.FastEl(l-1);
    }
    int u;
    for(u=0; u<n_units; u++) {
      bmBpUnit* un = new bmBpUnit;
      lay->units.el[u] = un;
      un->spec = &us;
      un->bias = new bmBpCon;
      un->spec->InitState(un);
    }
  }

#ifndef EXTRA_SPACE
  char* stuff = new char[8000000];
#endif

  // now connect them up
  bmLayer* lay;
  bmtaLeafItr li;
  FOR_ITR_EL(bmLayer, lay, net.layers., li) {
    bmUnit* un;
    bmtaLeafItr ui;
    FOR_ITR_EL(bmUnit, un, lay->units., ui) {
      if(li.i != 2) {
	un->send.el = new bmtaBase*[n_units + min_new];
	un->send.size = n_units;
	un->send.units.el = new bmtaBase*[n_units + min_new];
	un->send.units.size = n_units;
	bmLayer* rl = (bmLayer*)net.layers.el[li.i+1];
	un->send.prjn = (bmProjection*)rl->projections.FastEl(0);
	un->send.spec = &cs;
	int j;
	for(j=0; j<n_units; j++) {
	  bmBpCon* con = new bmBpCon;
	  un->send.el[j] = con;
	  bmBpUnit* ru = (bmBpUnit*)rl->units.el[j];
	  un->send.units.el[j] = ru;
	}
      }
      if(li.i > 0) {
	un->recv.el = new bmtaBase*[n_units + min_new];
	un->recv.size = n_units;
	un->recv.units.el = new bmtaBase*[n_units + min_new];
	un->recv.units.size = n_units;
	un->recv.prjn = (bmProjection*)lay->projections.FastEl(0);
	un->recv.spec = &cs;
	bmLayer* sl = (bmLayer*)net.layers.el[li.i-1];
	int j;
	for(j=0; j<n_units; j++) {
	  bmBpUnit* su = (bmBpUnit*)sl->units.el[j];
	  bmBpCon* con = (bmBpCon*)su->send.Cn(j);
	  un->recv.el[j] = con;
	  un->recv.units.el[j] = su;
	}
      }
    }
  }

  // this clears all external inputs!
  net.InitWtState();

  // so put them back in..
  ApplyPats(net);

  FILE* logfile = fopen("pdp++bench.log", "w");
  // now run the thing!

#ifdef NO_TIME
  long 	st_time, ed_time;
#else
  clock_t st_time, ed_time;
#endif
  st_time = clock();

  int pats;
  for(pats = 0; pats < n_pats; pats++) {
    ApplyPats(net);

    FOR_ITR_EL(bmLayer, lay, net.layers., li) {
      if(lay->lesion)	continue;
      if(!(lay->ext_flag & bmUnit::EXT))
	lay->Compute_Net();
      lay->Compute_Act();
    }

    float sse = 0.0f;
    bmLayer* lay;
    bmtaLeafItr li;
    FOR_ITR_EL(bmLayer, lay, net.layers., li) {
      if(!(lay->ext_flag & bmUnit::TARG))
	continue;
      bmUnit* un;
      bmtaLeafItr ui;
      FOR_ITR_EL(bmUnit, un, lay->units., ui) {
	float tmp = un->targ - un->act;
	sse += tmp * tmp;
      }
    }

//     if((pats % 10) == 0)
//       fprintf(logfile,"%d\t%g\n", pats, sse);

    for(l=net.layers.size-1; l>= 0; l--) {
      lay = (bmLayer*) net.layers.El(l);
      if(lay->lesion || (lay->ext_flag & bmUnit::EXT)) // dont compute err on inputs
	continue;

      bmBpUnit* u;
      bmtaLeafItr u_itr;
      FOR_ITR_EL(bmBpUnit, u, lay->units., u_itr)
	u->Compute_dEdA_dEdNet();
    }

    net.Compute_dWt();
    net.UpdateWeights();
  }

  ed_time = clock();

  fclose(logfile);

  double tot_time = (double)(ed_time - st_time) / (double)CLOCKS_PER_SEC;

  double n_wts = n_units * n_units * 2.0f;   // 2 fully connected layers
  double n_con_trav = 3.0f * n_wts * n_pats;
  double con_trav_sec = ((double)n_con_trav / tot_time) / 1.0e6;

  printf("weights: %g \tpats: %d \tcon_trav: %g\n", n_wts, n_pats, n_con_trav);
  printf("secs:\tMcon/sec:\n");
  printf("%g\t%g\n", tot_time, con_trav_sec);

#ifndef EXTRA_SPACE
  delete stuff;
#endif

}
