/* -*- C++ -*- */
/*=============================================================================
//                                                                            //
// This file is part of the PDP++ software package.                           //
//                                                                            //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson,                 //
//                    James L. McClelland, and Carnegie Mellon University     //
//                                                                            //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted       //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//                                                                            //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions.  HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
//                                                                            //
// Note that the taString class, which is derived from the GNU String class,  //
// is Copyright (C) 1988 Free Software Foundation, written by Doug Lea, and   //
// is covered by the GNU General Public License, see ta_string.h.             //
// The iv_graphic library and some iv_misc classes were derived from the      //
// InterViews morpher example and other InterViews code, which is             //
// Copyright (C) 1987, 1988, 1989, 1990, 1991 Stanford University             //
// Copyright (C) 1991 Silicon Graphics, Inc.                                  //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,         //
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY           //
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.           //
//                                                                            //
// IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY SPECIAL,    //
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES  //
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT     //
// ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY,      //
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        //
// SOFTWARE.                                                                  //
==============================================================================*/

#ifndef moe_h
#ifdef __GNUG__
#pragma interface
#endif
#define moe_h

#include <moe/moe_TA_type.h>
#include <bp/bp.h>
#include <pdp/netstru_extra.h>

class MoeOutputUnit;
class MoeOutputUnitSpec;
class MoeOutputLayer;
class MoeGateUnit;
class MoeGateUnitSpec;
class MoeGateLayer;

// Configuration of the network
//   Note that (T) layers require target value to be present.
//   Thus, same target value must be provided both to summary
//   linear output unit and to each of the MoeOutputLayer units
//   (w=1 means that the weights for these cons should be mean=1,var=0)
// 
// (T) BpLinearUnit: prediction output, summarizes everything
//        / (full w=1) \
// (T)  MoeOutputLayer: units for each expt output
//    / (1-1 (w=1))  \ (1-1 (w=1))
// Experts        MoeGateLayer: units for each gate
//    \             /
//      Input Units

class MoeOutputUnitSpec : public BpUnitSpec {
  /* Spec for post-gated outputs of experts: units must have only two
     recv projections: 0 is from output of expert, 1 is from gating layer,
     both of which must be one-to-one */
public:
  float         var_init;	// initial value of the unit variance
  float         var_init_wt;	// how much to weight the initial value over learning

  void          InitWtState(Unit* u); // also init unit vars
  void          Compute_Net(Unit* u); // net gets expert activity and gating activity
  void          Compute_Act(Unit* u); // act mults expt act and gating

  virtual float Compute_GExpRaw(MoeOutputUnit* un, MoeOutputLayer* lay);
  // E-step1: compute raw value of expected gate value (based on unit error)
  virtual void  Compute_GExp(MoeOutputUnit* un, MoeOutputLayer* lay);
  // E-step2: compute final (normalized) gate value

  virtual void  Compute_VarDeltas(MoeOutputUnit* un);
  // compute new var_num and var_den (must be after E-step), maybe only once?

  void          Compute_dEdA(BpUnit* u); // don't get dEdA from elsewhere, do VarDeltas
  void          Compute_dEdNet(BpUnit* u);

  void          Compute_dWt(Unit*) { };
  void          UpdateWeights(Unit* u);

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(MoeOutputUnitSpec);
  COPY_FUNS(MoeOutputUnitSpec, BpUnitSpec);
  TA_BASEFUNS(MoeOutputUnitSpec);
};

class MoeOutputUnit : public BpUnit {
  // Unit for post-gated outputs of experts (net=act of expt, act=g_act*net)
public:
  float         g_act;		// actual gate activity
  float         g_exp;		// expected value of the gate (h in Weigend et al)
  float         g_exp_raw;	// #NO_VIEW pre-normalized (raw) value of g_exp
  float         var;		// variance of the output unit (sigma^2)
  float         var_num;        // #NO_VIEW sum numerator for updating var
  float         var_den;        // #NO_VIEW sum denominator for updating var

  float         Compute_GExpRaw(MoeOutputLayer* lay)
  { return ((MoeOutputUnitSpec*)spec.spec)->Compute_GExpRaw(this, lay); }
  void          Compute_GExp(MoeOutputLayer* lay)
  { ((MoeOutputUnitSpec*)spec.spec)->Compute_GExp(this, lay); }

  virtual BpUnit* GetExpertUnit(); // get the expert unit in recv.gp[0][0]
  virtual MoeGateUnit* GetGatingUnit(); // get the gating unit in recv.gp[1][0]

  virtual void  ReScaleVariance(float new_scale);
  // #BUTTON rescale variance denominator to new_scale, numerator to new_scale * var

  void  Initialize();
  void  Destroy()               { };
  void  Copy_(const MoeOutputUnit& cp);
  COPY_FUNS(MoeOutputUnit, BpUnit);
  TA_BASEFUNS(MoeOutputUnit);
};


class MoeOutputLayer : public Layer {
  // Layer for holding output units (softmax computation for estimates)
public:
  float         g_exp_raw_sum;	// sum of raw g_exp values (over units)

  void          Compute_Act();  // call Compute_GExp after every act update

  virtual void  Compute_GExp();	// compute expected values of gating

  virtual void  ReScaleVariance(float new_scale);
  // #BUTTON rescale variance denominator to new_scale, numerator to new_scale * var

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(MoeOutputLayer);
  COPY_FUNS(MoeOutputLayer, Layer);
  TA_BASEFUNS(MoeOutputLayer);
};


class MoeGateUnitSpec : public BpUnitSpec {
  /* Spec for units that gate outputs of experts: the units need a
     send prjn to the corresp output unit as send.gp[0] */
public:
  float              act_delta_cost;
  // penalty term for changes in gating unit activity states over time

  void               Compute_Net(Unit* u);
  virtual void       Compute_Act(Unit* u) { BpUnitSpec::Compute_Act(u); }
  virtual void       Compute_Act(MoeGateUnit* u, MoeGateLayer* lay);

  void               Compute_dEdA(BpUnit* u); // get dEdA from corresp output unit as g_exp
  void               Compute_dEdNet(BpUnit* u);

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(MoeGateUnitSpec);
  COPY_FUNS(MoeGateUnitSpec, BpUnitSpec);
  TA_BASEFUNS(MoeGateUnitSpec);
};

class MoeGateUnit : public BpUnit {
  // Softmax unit that gates outputs of experts (Weigend: net=s, act=g, dEdA=delta)
public:
  float              prv_act;
  // previous activity state (needed for act_delta_cost)

  void  Compute_Act(MoeGateLayer* lay)
  { ((MoeGateUnitSpec*)spec.spec)->Compute_Act(this, lay); }

  MoeOutputUnit*     GetOutputUnit(); // get corresponding output unit as send.gp[0][0]

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(MoeGateUnit);
  COPY_FUNS(MoeGateUnit, BpUnit);
  TA_BASEFUNS(MoeGateUnit);
};

class MoeGateLayer : public Layer {
  // Layer for holding gating units (softmax computation)
public:
  float         act_sum;	// sum of net input over layer (for normalizing)

  void          Compute_Net();	// compute sum for two-step normalization of activations
  void          Compute_Act();	// pass layer to units for normalization purposes

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(MoeGateLayer);
  COPY_FUNS(MoeGateLayer, Layer);
  TA_BASEFUNS(MoeGateLayer);
};

class MoeTrial : public BpTrial {
  /* Mixture-of-experts trial process: allows multiple M-steps per E step.
     If m_step.max > 1, UpdateWeights is automatically ON_LINE, so epoch
     should be set to BATCH to minimize redundancy */
public:
  Counter       m_step;	        // controls iteration over M(aximization) steps

  void          Loop();
  bool          Crit() { return SchedProcess::Crit(); }
  // trial subverts the crit, so unsubvert it..
  bool          CheckUnit(Unit* ck);

  void  Initialize();
  void  Destroy()               { };
  void  InitLinks();
  SIMPLE_COPY(MoeTrial);
  COPY_FUNS(MoeTrial, BpTrial);
  TA_BASEFUNS(MoeTrial);
};

class MoeMStep : public SettleProcess {
  // does one maximization of network step for mixture-of-experts
public:

  virtual void  Compute_dEdA_dEdNet();
  virtual void  Compute_dWt();
  virtual void  UpdateWeights();

  void          Loop();
  bool          Crit() { return true; }	// loop only once

  void  Initialize();
  void  Destroy()               { };
  TA_BASEFUNS(MoeMStep);
};

class ManyToOnePrjnSpec : public OneToOnePrjnSpec {
  // many senders to one receiver connectivity
public:
  int           send_n;        // number of sending units to connect

  void          Connect_impl(Projection* prjn);

  void  Initialize();
  void  Destroy()               { };
  SIMPLE_COPY(ManyToOnePrjnSpec);
  COPY_FUNS(ManyToOnePrjnSpec, OneToOnePrjnSpec);
  TA_BASEFUNS(ManyToOnePrjnSpec);
};

#endif // moe.h
