// File Automatically Generated by MakeTA
// DO NOT EDIT


#include <ta/typea.h>
#include <ta/typea_constr.h>
#include <css/basic_types.h>
#include <css/c_ptr_types.h>
#include <css/ta_css.h>
#include <bpso.h>
#include "bpso_TA_type.h"
#include "bpso_TA_inst.h"


// Types


// Instances


// Type Data


// Init Function


static bool ta_Init_bpso_done = false;

void ta_Init_bpso() {
  TypeDef* sbt;

  if(ta_Init_bpso_done) return;
  ta_Init_bpso_done = true;

  ta_Init_bp();
  ta_Init_ta();
  ta_Init_pdp();
  ta_Init_ta_misc();
  ta_Init_iv_graphic();
  ta_Init_so();

  taMisc::in_init = true;



  taMisc::in_init = false;
} 
