/* -*- C++ -*- */
// test of pdp++-style network computations, should be easily compiled
// on any system without complications of compiling all of pdp++

// 

#ifndef pdp_bench_h
#ifdef __GNUG__
#pragma interface
#endif
#define pdp_bench_h

// base contains all the basic stuff 
#include "pdp++base.h"

// now just implement bp

//////////////////////////////////////////
//		bp.h			//
//////////////////////////////////////////

class BpUnit;
class BpCon_Group;

class BpCon : public Connection {
  // Bp connection
public:
  float 		dwt; 		// #NO_SAVE Change in weight
  float 		dEdW; 		// #NO_SAVE derivative of Error wrt weight

  BpCon()		{ dwt = dEdW = 0.0f; }
};

class BpConSpec : public ConSpec {
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
  void 		(*decay_fun)(BpConSpec* spec, BpCon* cn, BpUnit* ru, BpUnit* su);
  // #LIST_BpConSpec_WtDecay the weight decay function to use

  void 		C_InitWtDelta(Connection* cn, Unit* ru, Unit* su)
  { ConSpec::C_InitWtDelta(cn, ru, su); ((BpCon*)cn)->dEdW = 0.0f; }

  void 		C_InitWtState(Connection* cn, Unit* ru, Unit* su) 	
  { ConSpec::C_InitWtState(cn, ru, su); ((BpCon*)cn)->dwt = 0.0f;}

  inline float		C_Compute_dEdA(BpCon* cn, BpUnit* ru, BpUnit* su);
  inline virtual float 	Compute_dEdA(BpCon_Group* cg, BpUnit* su);
  // get error from units I send to

  inline void 		C_Compute_dWt(BpCon* cn, BpUnit* ru, BpUnit* su);
  inline void 		Compute_dWt(Con_Group* cg, Unit* ru);
  inline virtual void	B_Compute_dWt(BpCon* cn, BpUnit* ru);
  // Compute dE with respect to the weights

  inline void 	C_Compute_WtDecay(BpCon* cn, BpUnit* ru, BpUnit* su);
  // call the decay function

  inline void	C_BEF_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su); // BEFORE_LRATE
  inline void	C_AFT_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su); // AFTER_LRATE
  inline void	C_NRM_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su); // NORMALIZED
  inline void	UpdateWeights(Con_Group* cg, Unit* ru);
  inline virtual void	B_UpdateWeights(BpCon* cn, BpUnit* ru);
  // for the bias unit
  
  BpConSpec();
};

class BpCon_Group : public Con_Group {
public:
  float Compute_dEdA(BpUnit* su)
  { return ((BpConSpec*)spec)->Compute_dEdA(this, su); }
};

class BpUnit;

class BpUnitSpec : public UnitSpec {
public:
  SigmoidSpec	sig;		// sigmoid activation parameters
  float		err_tol;	// error tolerance (no error signal if |t-o|<err_tol)
  void 		(*err_fun)(BpUnitSpec* spec, BpUnit* u);
  // #LIST_BpUnit_Error this points to the error fun, set appropriately

  // generic unit functions
  void		InitState(Unit* u);
  void 		Compute_Act(Unit* u);		// activation from net input (sigmoid)
  void 		Compute_dWt(Unit* u); 		// for all of my recv weights
  void 		UpdateWeights(Unit* u);		// modify to update bias weight
  
  // bp special functions
  virtual void 	Compute_Error(BpUnit* u); 	// call the error function (testing only)
  virtual void 	Compute_dEdA(BpUnit* u); 	// error wrt unit activation
  virtual void 	Compute_dEdNet(BpUnit* u); 	// error wrt net input

  BpUnitSpec();
};

// the following functions are possible error functions. 

// #REG_FUN
void Bp_Squared_Error(BpUnitSpec* spec, BpUnit* u) 
// #LIST_BpUnit_Error Squared error function for bp
     ;				// term here so scanner picks up comment
// #REG_FUN
void Bp_CrossEnt_Error(BpUnitSpec* spec, BpUnit* u)
// #LIST_BpUnit_Error Cross entropy error function for bp
     ;				// term here so scanner picks up comment

class BpUnit : public Unit {
  // standard feed-forward Bp unit
public:
  float 	err; 		// this is E, not dEdA
  float 	dEdA;		// #LABEL_dEdA error wrt activation
  float 	dEdNet;		// #LABEL_dEdNet error wrt net input

  // these are "convenience" functions for those defined in the spec
  void Compute_Error()		{ ((BpUnitSpec*)spec)->Compute_Error(this); }
  void Compute_dEdA()		{ ((BpUnitSpec*)spec)->Compute_dEdA(this); }
  void Compute_dEdNet()		{ ((BpUnitSpec*)spec)->Compute_dEdNet(this); }
  void Compute_dEdA_dEdNet() 	{ Compute_dEdA(); Compute_dEdNet(); }

  BpUnit();
};

// inline functions (for speed)

inline float BpConSpec::C_Compute_dEdA(BpCon* cn, BpUnit* ru, BpUnit*) {
  return cn->wt * ru->dEdNet;
}
inline float BpConSpec::Compute_dEdA(BpCon_Group* cg, BpUnit* su) {
  float rval = 0.0f;
  CON_GROUP_LOOP(cg,rval += C_Compute_dEdA((BpCon*)cg->Cn(i), (BpUnit*)cg->Un(i), su));
  return rval;
}


inline void BpConSpec::C_Compute_dWt(BpCon* cn, BpUnit* ru, BpUnit* su) {
  cn->dEdW += *(su->act) * ru->dEdNet;
}
inline void BpConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg,C_Compute_dWt((BpCon*)cg->Cn(i), (BpUnit*)ru, (BpUnit*)cg->Un(i)));
}
inline void BpConSpec::B_Compute_dWt(BpCon* cn, BpUnit* ru) {
  cn->dEdW += ru->dEdNet;
}

inline void BpConSpec::C_Compute_WtDecay(BpCon* cn, BpUnit* ru, BpUnit* su) {
  if(decay_fun != NULL)
    (*decay_fun)(this, cn, ru, su);
}
inline void BpConSpec::C_AFT_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su); // decay goes into dEdW...
#endif
  cn->dwt = lrate * cn->dEdW + momentum * cn->dwt;
  cn->wt += cn->dwt;
  cn->dEdW = 0.0f;
}
inline void BpConSpec::C_BEF_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su);
#endif
  cn->dwt = cn->dEdW + momentum * cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dEdW = 0.0f;
}
inline void BpConSpec::C_NRM_UpdateWeights(BpCon* cn, BpUnit* ru, BpUnit* su) {
#ifndef NO_DECAY
  C_Compute_WtDecay(cn, ru, su);
#endif
  cn->dwt = momentum_c * cn->dEdW + momentum * cn->dwt;
  cn->wt += lrate * cn->dwt;
  cn->dEdW = 0.0f;
}

inline void BpConSpec::UpdateWeights(Con_Group* cg, Unit* ru) {
#ifndef NO_MOMENTUM_OPTS
  if(momentum_type == AFTER_LRATE) {
    CON_GROUP_LOOP(cg, C_AFT_UpdateWeights((BpCon*)cg->Cn(i),
					   (BpUnit*)ru, (BpUnit*)cg->Un(i)));
  }
  else if(momentum_type == BEFORE_LRATE)
#endif
  {
    CON_GROUP_LOOP(cg, C_BEF_UpdateWeights((BpCon*)cg->Cn(i),
					   (BpUnit*)ru, (BpUnit*)cg->Un(i)));
  }
#ifndef NO_MOMENTUM_OPTS
  else {
    CON_GROUP_LOOP(cg, C_NRM_UpdateWeights((BpCon*)cg->Cn(i),
					   (BpUnit*)ru, (BpUnit*)cg->Un(i)));
  }
#endif
#ifndef NO_WT_LIMITS
  ApplyLimits(cg, ru);
#endif
}

inline void BpConSpec::B_UpdateWeights(BpCon* cn, BpUnit* ru) {
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

#endif // pdp_bench_h
