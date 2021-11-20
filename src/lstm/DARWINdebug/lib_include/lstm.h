// -*- C++ -*-
// lstm.h 

#ifndef lstm_h
#ifdef __GNUG__
#pragma interface
#endif
#define lstm_h

#include "lstm_TA_type.h"
#include <bp/bp.h>

// This implements LSTM (Hochreiter & Schmidhuber (1997), Long Short
// Term Memory, Neural Computation, 9, 1735-1780; Gers, Schmidhuber &
// Cummins (2000). Learning to Forget: Continual Prediction with LSTM. Neural
// Computation, 12(10):2451--2471).

// This version has forget gates but not peephole connections

// As with all PDP++ algorithms, see the LstmBpTrial::Loop code for the basic
// computational steps, which are just loops over units passing activation forward
// and error backward.  LSTM also has a unique derivative computation stage,
// (Compute_Dmem) for computing the derivatives of the weights into the gates and
// memory units based on the current memory unit activities.  This requires storage
// at each connection into the gates that is a function of the number of memory cell
// units -- this requires an array of values in the connection, implemented as a fixed
// array with lstm_max_mem elements.  Somewhat awkward implementationally but this just
// goes to show that this part of the algorithm doesn't fit well within the standard 
// neural framework..

// Given the complexity of the algorithm, and the number of different specs required
// it is strongly recommended that you start with the lstm_std.proj.gz project and 
// build from there.

class LstmBpUnit;
class MemBpUnit;

static const int lstm_max_mem = 25;	// maximum number of memory cells per memory block!
// required for the MemDeriv con specs to hold separate derivatives for each cell.

class LstmBpConSpec : public BpConSpec {
  // connection spec for Lstm algorithm: uses previous act for weight change computation
public:
  enum GateType {
    NOT_GATE,
    IN_GATE,
    FRG_GATE,
    OUT_GATE
  };

  bool		dwt_snd_cur;	// learn on sender's current activation value (only for output cons -- otherwise should be prev)

  virtual GateType GetGateType() { return NOT_GATE; }

  inline void 	C_Compute_dWt(BpCon* cn, BpUnit* ru, LstmBpUnit* su);
  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);

  void 	Initialize()	{ dwt_snd_cur = false; };
  void 	Destroy()	{ };
  SIMPLE_COPY(LstmBpConSpec);
  COPY_FUNS(LstmBpConSpec, BpConSpec);
  TA_BASEFUNS(LstmBpConSpec);
};

class InGateToMemConSpec : public LstmBpConSpec {
  // connection sends input gating value to the mem unit receiver
public:

  GateType GetGateType() { return IN_GATE; }

  inline float 	Compute_dEdA(BpCon_Group*, BpUnit*) { return 0.0f; } // don't send error -- computed via Dmem

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(InGateToMemConSpec);
};
  
class FrgGateToMemConSpec : public LstmBpConSpec {
  // connection sends forget gating value to the mem unit receiver
public:

  GateType GetGateType() { return FRG_GATE; }

  inline float 	Compute_dEdA(BpCon_Group*, BpUnit*) { return 0.0f; } // don't send error -- computed via Dmem

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(FrgGateToMemConSpec);
};

class OutGateToMemConSpec : public LstmBpConSpec {
  // connection sends output gating value to the mem unit receiver
public:
  GateType GetGateType() { return OUT_GATE; }

  inline float	C_Compute_dEdA(BpCon* cn, MemBpUnit* ru, BpUnit* su);
  inline float 	Compute_dEdA(BpCon_Group* cg, BpUnit* su);
  // error has to take into account multiplication, etc on mem unit

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(OutGateToMemConSpec);
};
  
class DmemBpCon : public BpCon {
  // holds derivatives needed for input and forget gate connections
public:
  float		dMdw[lstm_max_mem]; // #NO_SAVE derivative of memory unit with respect to weight
  float		dMdw_sum;	// #NO_SAVE sum of derivatives

  void	InitMem() { int i; for(i=0;i<lstm_max_mem;i++) dMdw[i] = 0.0f; dMdw_sum = 0.0f; }

  void 	Initialize();
  void 	Destroy()		{ };
  void	Copy_(const DmemBpCon& cp);
  COPY_FUNS(DmemBpCon, BpCon);
  TA_BASEFUNS(DmemBpCon);
};

class DmemBpConSpec : public LstmBpConSpec {
  // base class for conspecs operating on DmemBpCon objects
public:
  inline float 	Compute_dEdA(BpCon_Group*, BpUnit*) { return 0.0f; } // truncate anything further

  virtual void 	InitMem(Con_Group* cg) {
    CON_GROUP_LOOP(cg, ((DmemBpCon*)cg->Cn(i))->InitMem());
  }
  virtual void 	InitMemSum(Con_Group* cg) {
    CON_GROUP_LOOP(cg, ((DmemBpCon*)cg->Cn(i))->dMdw_sum = 0.0f);
  }

  // this is the interface -- need to instantiate in each subclass
  virtual void 	Compute_Dmem(Con_Group*, LstmBpUnit*, MemBpUnit*, int) { };

  inline void 	C_Compute_DmemSum(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi);
  inline virtual void 	Compute_DmemSum(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  inline virtual void 	B_Compute_Dmem(DmemBpCon*, LstmBpUnit*, MemBpUnit*, int) { };

  inline void 	B_Compute_dWt(BpCon* cn, BpUnit*) {
    cn->dEdW += ((DmemBpCon*)cn)->dMdw_sum;
  }
  inline void 	C_Compute_dWt(DmemBpCon* cn, LstmBpUnit*, LstmBpUnit*) {
    cn->dEdW += cn->dMdw_sum;
  }
  inline void 	Compute_dWt(Con_Group* cg, Unit* ru) {
    CON_GROUP_LOOP(cg,C_Compute_dWt((DmemBpCon*)cg->Cn(i), (LstmBpUnit*)ru, (LstmBpUnit*)cg->Un(i)));
  }

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(DmemBpConSpec);
};

class InGateInConSpec : public DmemBpConSpec {
  // required for *inputs* to input gate units (InGateBpConSpec is for input gate to memory cells), requires DmemBpCon cons
public:
  inline void 	C_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi, LstmBpUnit* su);
  inline void 	Compute_Dmem(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  inline void 	B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  void 	Initialize()	{ };
  void 	Destroy()	{ };
  TA_BASEFUNS(InGateInConSpec);
};
  
class FrgGateInConSpec : public DmemBpConSpec {
  // required for *inputs* to forget gate units (FrgGateBpConSpec is for input gate to memory cells), requires DmemBpCon cons
public:
  inline void 	C_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi, LstmBpUnit* su);
  inline void 	Compute_Dmem(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  inline void 	B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  void 	Initialize()	{ };
  void 	Destroy()	{ };
  TA_BASEFUNS(FrgGateInConSpec);
};
  
class MemGateInConSpec : public DmemBpConSpec {
  // required for *inputs* to memory units, equires DmemBpCon cons
public:
  inline void 	C_Compute_Dmem(DmemBpCon* cn, MemBpUnit* mem, LstmBpUnit* su);
  inline void 	Compute_Dmem(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  inline void 	B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi);

  inline void 	C_Compute_dWt(DmemBpCon* cn, LstmBpUnit* ru, LstmBpUnit* su);
  inline void 	Compute_dWt(Con_Group* cg, Unit* ru);

  void 	Initialize() 	{ };
  void 	Destroy()	{ };
  TA_BASEFUNS(MemGateInConSpec);
};
  
class LstmBpUnitSpec : public BpUnitSpec {
  // LSTM unit spec: keeps prior activation state
public:
  void		InitState(Unit* u);
  void 		Compute_Act(Unit* u);
  
  bool 	CheckConfig(Unit* u, Layer* lay, TrialProcess* trl);

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(LstmBpUnitSpec);
};

class LstmBpUnit : public BpUnit {
  // LSTM unit: keeps prior activation state
public:
  float		act_prv;	// previous activation value (for learning)

  void 	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const LstmBpUnit& cp);
  COPY_FUNS(LstmBpUnit, BpUnit);
  TA_BASEFUNS(LstmBpUnit);
};

class DmemBpUnitSpec : public LstmBpUnitSpec {
  // spec for input and forget gate units, which require funky memory-cell specific derivatives stored in each input connection
public:
  void		InitState(Unit* u);
  void		Compute_dEdNet(BpUnit* u); // this updates dMdw_sum

  virtual void 	Compute_Dmem(LstmBpUnit* u);
  // after all the network activations are updated, compute the mem-derivatives
  
  bool 	CheckConfig(Unit* u, Layer* lay, TrialProcess* trl);

  void 	Initialize();
  void 	Destroy()	{ };
  TA_BASEFUNS(DmemBpUnitSpec);
};

class MemBpUnitSpec : public DmemBpUnitSpec {
  // spec for an LSTM gated memory unit: self con is implicit, act_range is for output xform
public:
  MinMaxRange	inx_range;	// range of activation for input transformation
  bool		skip_act_oux_deriv; // do not include h' derivative of output activation fctn in computing dEdNet (Gers & Schmidhuber do this)
  
  void		InitState(Unit* u);
  void 		Compute_Net(Unit* u);
  void 		Compute_Act(Unit* u);
  void 		Compute_Dmem(LstmBpUnit* u);
  void		Compute_dEdNet(BpUnit* u);
  
  bool 	CheckConfig(Unit* u, Layer* lay, TrialProcess* trl);

  void 	Initialize();
  void 	Destroy()	{ };
  void	InitLinks();
  SIMPLE_COPY(MemBpUnitSpec);
  COPY_FUNS(MemBpUnitSpec, DmemBpUnitSpec);
  TA_BASEFUNS(MemBpUnitSpec);
};

class MemBpUnit : public LstmBpUnit {
  // LSTM memory unit: act is post-output-gated value
public:
  float		in_gt;		// input gate value
  float		frg_gt;		// forget gate value
  float		out_gt;		// output gate value
  float		act_inx;	// input transformed activation (g)
  float		act_mem;	// internal memory activation value (s)
  float		mem_prv;	// previous memory activation value (s(t-1))
  float		act_oux;	// output transformed activation (h)

  void 	Initialize();
  void 	Destroy()	{ };
  void	Copy_(const MemBpUnit& cp);
  COPY_FUNS(MemBpUnit, LstmBpUnit);
  TA_BASEFUNS(MemBpUnit);
};

class LstmBpTrial : public BpTrial {
  // special trial process for LSTM algorithm
public:
  virtual void	Compute_ClampExt();
  virtual void	Compute_NetHid();
  virtual void	Compute_NetOut();
  virtual void	Compute_ActHid();
  virtual void	Compute_ActOut();
  virtual void	Compute_Dmem();
  void		Loop();

  void	Initialize();
  void 	Destroy()		{ };
  TA_BASEFUNS(LstmBpTrial);
};

//////////////////////////
// 	Inlines		//
//////////////////////////

inline float sig_deriv(MinMaxRange& rng, float act) {
  return (act - rng.min) * (rng.max - act) * rng.scale;
}

inline void LstmBpConSpec::C_Compute_dWt(BpCon* cn, BpUnit* ru, LstmBpUnit* su) {
  if(dwt_snd_cur)
    cn->dEdW += su->act * ru->dEdNet;
  else
    cn->dEdW += su->act_prv * ru->dEdNet;
}
inline void LstmBpConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg,C_Compute_dWt((BpCon*)cg->Cn(i), (BpUnit*)ru, (LstmBpUnit*)cg->Un(i)));
}

inline float OutGateToMemConSpec::C_Compute_dEdA(BpCon*, MemBpUnit* ru, BpUnit*) {
  return ru->dEdA * ru->act_oux;
}
inline float OutGateToMemConSpec::Compute_dEdA(BpCon_Group* cg, BpUnit* su) {
  float rval = 0.0f;
  CON_GROUP_LOOP(cg,rval += C_Compute_dEdA((BpCon*)cg->Cn(i), (MemBpUnit*)cg->Un(i), su));
  return rval;
}

inline void DmemBpConSpec::C_Compute_DmemSum(DmemBpCon* cn, LstmBpUnit*, MemBpUnit* mem, int memi) {
  cn->dMdw_sum += mem->dEdNet * cn->dMdw[memi];
}
inline void DmemBpConSpec::Compute_DmemSum(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi) {
  CON_GROUP_LOOP(cg, C_Compute_DmemSum((DmemBpCon*)cg->Cn(i), ru, mem, memi));
}

// todo: peephole = direct access to memory state, not xformed output state -- requires diff act_prv here
inline void InGateInConSpec::C_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi, LstmBpUnit* su) {
  cn->dMdw[memi] = (cn->dMdw[memi] * mem->frg_gt) + (mem->act_inx * (ru->act * (1.0f - ru->act)) * su->act_prv);
}
inline void InGateInConSpec::Compute_Dmem(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi) {
  CON_GROUP_LOOP(cg,C_Compute_Dmem((DmemBpCon*)cg->Cn(i), ru, mem, memi, (LstmBpUnit*)cg->Un(i)));
}
inline void InGateInConSpec::B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi) {
  cn->dMdw[memi] = (cn->dMdw[memi] * mem->frg_gt) + (mem->act_inx * (ru->act * (1.0f - ru->act)));
}

// todo: ditto for peehole
inline void FrgGateInConSpec::C_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi, LstmBpUnit* su) {
  cn->dMdw[memi] = (cn->dMdw[memi] * mem->frg_gt) + (mem->mem_prv * (ru->act * (1.0f - ru->act)) * su->act_prv);
}
inline void FrgGateInConSpec::Compute_Dmem(Con_Group* cg, LstmBpUnit* ru, MemBpUnit* mem, int memi) {
  CON_GROUP_LOOP(cg,C_Compute_Dmem((DmemBpCon*)cg->Cn(i), ru, mem, memi, (LstmBpUnit*)cg->Un(i)));
}
inline void FrgGateInConSpec::B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit* ru, MemBpUnit* mem, int memi) {
  cn->dMdw[memi] = (cn->dMdw[memi] * mem->frg_gt) + (mem->mem_prv * (ru->act * (1.0f - ru->act)));
}

inline void MemGateInConSpec::C_Compute_Dmem(DmemBpCon* cn, MemBpUnit* mem, LstmBpUnit* su) {
  cn->dMdw_sum = (cn->dMdw_sum * mem->frg_gt) + (sig_deriv(((MemBpUnitSpec*)mem->spec.spec)->inx_range, mem->act_inx) * mem->in_gt * su->act_prv);
}
inline void MemGateInConSpec::Compute_Dmem(Con_Group* cg, LstmBpUnit*, MemBpUnit* mem, int) {
  CON_GROUP_LOOP(cg,C_Compute_Dmem((DmemBpCon*)cg->Cn(i), mem, (LstmBpUnit*)cg->Un(i)));
}
inline void MemGateInConSpec::B_Compute_Dmem(DmemBpCon* cn, LstmBpUnit*, MemBpUnit* mem, int) {
  cn->dMdw_sum = (cn->dMdw_sum * mem->frg_gt) + (mem->mem_prv * sig_deriv(((MemBpUnitSpec*)mem->spec.spec)->inx_range, mem->act_inx) * mem->in_gt);
}
inline void 	MemGateInConSpec::C_Compute_dWt(DmemBpCon* cn, LstmBpUnit* ru, LstmBpUnit*) {
  cn->dEdW += ru->dEdNet * cn->dMdw_sum;
}
inline void 	MemGateInConSpec::Compute_dWt(Con_Group* cg, Unit* ru) {
  CON_GROUP_LOOP(cg,C_Compute_dWt((DmemBpCon*)cg->Cn(i), (LstmBpUnit*)ru, (LstmBpUnit*)cg->Un(i)));
}



#endif // lstm_h
