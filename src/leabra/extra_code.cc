class SearchBiasCon : public LeabraCon {
  // does heuristic search based on linear underlying weight: if +, eff wt = sigmoided +, if -, eff wt = sigmoided (large) - (try + for a while, then go -)
public:
  float		lwt;		// linear bias weight value
  float		zwt;		// 'zero' bias weight value -- baseline offset

  void 	Initialize()		{ lwt = zwt = 0.0f; }
  void	Destroy()		{ };
  void	Copy_(const SearchBiasCon& cp);
  COPY_FUNS(SearchBiasCon, LeabraCon);
  TA_BASEFUNS(SearchBiasCon);
};

class SearchBiasSpec : public LeabraBiasSpec {
  // does heuristic search based on linear underlying weight: if +, eff wt = sigmoided +, if -, eff wt = sigmoided (large) - (try + for a while, then go -)
public:
  SigMinMaxSpec	bias_sig;	// sigmoid parameters for min, mid, max effective value
  float		neg_thresh;	// weight threshold to go max negative

  inline void	B_UpdateWeights(LeabraCon* cn, LeabraUnit* ru) {
    SearchBiasCon* sc = (SearchBiasCon*)cn;
    if((sc->dwt > dwt_thresh) && (sc->lwt < neg_thresh)) {
      sc->pdw = 1.0f - sc->lwt;
      sc->lwt = 1.0f;		// goes up to max if receiving significant reward
    }
    else {
      sc->dwt = -cur_lrate * sc->lwt;
      if((sc->lwt > neg_thresh) && ((sc->lwt + sc->dwt) < neg_thresh)) { // change takes < thresh
	sc->pdw = -1.0f - sc->lwt;
	sc->lwt = -1.0f;
      }
      else {
	sc->pdw = sc->dwt;
	sc->lwt += cur_lrate * sc->dwt;
      }
    }
    sc->wt = sc->zwt + bias_sig.Eval(sc->lwt);
    sc->dwt = 0.0f;
    C_ApplyLimits(sc, ru, NULL);
  }

  void 		C_InitWtState(Con_Group* cg, Connection* cn, Unit* ru, Unit* su) {
    LeabraBiasSpec::C_InitWtState(cg, cn, ru, su); SearchBiasCon* lcn = (SearchBiasCon*)cn;
    lcn->zwt = lcn->wt; lcn->lwt = 0.0f; }

  virtual void	PlotBiasSigFun(GraphLog* glog = NULL, float mod_min = -1.0f, float mod_max = 1.0f, float mod_inc = .05f);
  // #BUTTON #NULL_OK plot sigmoid bias function for range of linear weight values from min to max

  void 	Initialize();
  void	Destroy()		{ };
  void	InitLinks();
  SIMPLE_COPY(SearchBiasSpec);
  COPY_FUNS(SearchBiasSpec, LeabraBiasSpec);
  TA_BASEFUNS(SearchBiasSpec);
};

void SearchBiasCon::Copy_(const SearchBiasCon& cp) {
  lwt = cp.lwt;
  zwt = cp.zwt;
}

void SearchBiasSpec::Initialize() {
  min_con_type = &TA_SearchBiasCon;
  bias_sig.min = -10.0f;
  bias_sig.mid = 0.0f;
  bias_sig.max = 2.0f;
  bias_sig.off = .5f;
  bias_sig.gain = 3.0f;
  lrate = .05f;
  neg_thresh = .05f;
  dwt_thresh = .5f;
}

void SearchBiasSpec::InitLinks() {
  LeabraBiasSpec::InitLinks();
  taBase::Own(bias_sig, this);
}

void SearchBiasSpec::PlotBiasSigFun(GraphLog* glog, float mod_min, float mod_max, float mod_inc) {
  if(glog == NULL) {
    Project* proj = GET_MY_OWNER(Project);
    if(proj == NULL)    return;
    glog = (GraphLog*) proj->logs.NewEl(1, &TA_GraphLog);
    winbMisc::DelayedMenuUpdate(proj);
  }
  fstream strm;
  strm.open("bias_sig_plot.log", ios::out);
  strm << "_H:\tx\ty\n";
  float x;
  for(x=mod_min; x<=mod_max; x+=mod_inc) {
    float y = bias_sig.Eval(x);
    strm << "_D:\t" << x << "\t" << y << "\n";
  }
  strm.close();
  glog->name = "BiasSig_GraphLog";
  glog->ViewWindow();
  glog->CloseFile();
  glog->LoadFile("bias_sig_plot.log", true);
  glog->CloseFile();
  glog->InitAllViews();
}
