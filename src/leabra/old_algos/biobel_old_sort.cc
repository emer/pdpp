
// inhib between thresholds for k and k+1th units, with gap!
void BioBelLayerSpec::Compute_Inhib_kWTA(BioBelLayer*, Unit_Group* ug, BioBelInhib* thr) {
  if(ug->leaves <= 1) {	// this is undefined
    thr->Inhib_SetVals(i_kwta_pt);
    return;
  }
  
  int k_plus_1 = thr->kwta.k + 1;	// expand cutoff to include N+1th one

  float net_k1 = MAXFLOAT;
  int k1_idx = 0;
  BioBelUnit* u;
  taLeafItr i;
  int j;
  if(thr->active_buf.size != k_plus_1) { // need to fill the sort buf..
    thr->active_buf.size = 0;
    j = 0;
    for(u = (BioBelUnit*)ug->FirstEl(i); u && (j < k_plus_1);
	u = (BioBelUnit*)ug->NextEl(i), j++)
    {
      thr->active_buf.Add(u);		// add unit to the list
      if(u->net < net_k1) {
	net_k1 = u->net;	k1_idx = j;
      }
    }
    thr->inact_buf.size = 0;
    // now, use the "replace-the-lowest" sorting technique
    for(; u; u = (BioBelUnit*)ug->NextEl(i)) {
      if(u->net <=  net_k1) {	// not bigger than smallest one in sort buffer
	thr->inact_buf.Add(u);
	continue;
      }
      thr->inact_buf.Add(thr->active_buf.FastEl(k1_idx)); // now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->net;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }
  else {				// keep the ones around from last time, find net_k1
    for(j=0; j < k_plus_1; j++) { 	// these should be the top ones, very fast!!
      float tmp = thr->active_buf.FastEl(j)->net;
      if(tmp < net_k1) {
	net_k1 = tmp;		k1_idx = j;
      }
    }
    // now, use the "replace-the-lowest" sorting technique (on the inact_list)
    for(j=0; j < thr->inact_buf.size; j++) {
      u = thr->inact_buf.FastEl(j);
      if(u->net <=  net_k1)		// not bigger than smallest one in sort buffer
	continue;
      thr->inact_buf.Replace(j, thr->active_buf.FastEl(k1_idx));	// now inactive
      thr->active_buf.Replace(k1_idx, u);// replace the smallest with it
      net_k1 = u->net;		// assume its the smallest
      for(j=0; j < k_plus_1; j++) { 	// and recompute the actual smallest
	float tmp = thr->active_buf.FastEl(j)->net;
	if(tmp < net_k1) {
	  net_k1 = tmp;		k1_idx = j;
	}
      }
    }
  }

  // active_buf now has k+1 most active units, get the next-highest one
  int k_idx = -1;
  int max_idx = -1;
  float net_k = MAXFLOAT;
  float net_max = -MAXFLOAT;
  for(j=0; j < k_plus_1; j++) {
    float tmp = thr->active_buf.FastEl(j)->net;
    if((tmp < net_k) && (tmp != net_k1)) {
      net_k = tmp;		k_idx = j;
    }
    if(tmp > net_max) {
      net_max = tmp;		max_idx = j;
    }
  }
  if(k_idx == -1) {		// we didn't find the next one
    k_idx = k1_idx;
    net_k = net_k1;
  }
  if(max_idx == -1) {		// we didn't find the max
    max_idx = k1_idx;
    net_max = net_k1;
  }

  BioBelUnit* k1_u = (BioBelUnit*)thr->active_buf.FastEl(k1_idx);
  BioBelUnit* k_u = (BioBelUnit*)thr->active_buf.FastEl(k_idx);
  BioBelUnit* max_u = (BioBelUnit*)thr->active_buf.FastEl(max_idx);

  float k1_i = k1_u->Compute_IThresh();
  float k_i = k_u->Compute_IThresh();
  float max_i = max_u->Compute_IThresh();

  // place kwta inhibition between k and k+1
  float nw_gi = k1_i + i_kwta_pt * (k_i - k1_i);
  nw_gi = MAX(nw_gi, 0.0f);
  thr->i_val.kwta = nw_gi;

  // compute value due to max unit
  thr->i_val.max = i_max_pt * max_i;
  thr->i_val.max = MAX(thr->i_val.max, 0.0f);
}
