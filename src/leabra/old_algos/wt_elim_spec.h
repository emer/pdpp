// weight elimination spec

class WtElimSpec : public taBase {
  // ##INLINE ##NO_TOKENS #NO_UPDATE_AFTER weight elimination specifications
public:
  bool		on;		// whether weight decay is on (active)
  float		sig;		// sigma for comparison 
  float		gain;		// gain of weight elimination
  float		sig_sq;		// #HIDDEN squared sigma

  float		Eval(float w) {
    float denom = sig_sq + (w * w);
    return (gain * sig_sq * w) / (denom * denom); }

  void	UpdateAfterEdit();
  void	Initialize();
  void	Destroy()	{ };
  SIMPLE_COPY(WtElimSpec);
  COPY_FUNS(WtElimSpec, taBase);
  TA_BASEFUNS(WtElimSpec);
};

void WtElimSpec::Initialize() {
  on = false;
  sig = .125;
  gain = .2;
  sig_sq = sig * sig;
}

void WtElimSpec::UpdateAfterEdit() {
  taBase::UpdateAfterEdit();
  sig_sq = sig * sig;
}

