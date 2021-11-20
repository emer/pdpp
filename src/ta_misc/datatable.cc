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

// datatable.cc

#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <ta_misc/datatable.h>
#include <ta_misc/datagraph.h>
#include <ta_misc/datatable_iv.h>
#include <ta/ta_base_tmplt.h> // for int_Array
#include <ta/enter_iv.h>
#include <InterViews/color.h> // for view specs
#include <ta/leave_iv.h>


//////////////////////////
//	DataArray	//
//////////////////////////

String DataArray_impl::opt_after;

void DataArray_impl::Initialize() {
  save_to_file = true;
}

void DataArray_impl::InitLinks() {
  taNBase::InitLinks();
}  

void DataArray_impl::Copy_(const DataArray_impl& cp) {
  disp_opts = cp.disp_opts;
}

void DataArray_impl::AddDispOption(const char* opt) {
  String nm = " ";		// pad with preceding blank to provide start cue
  nm += String(opt) + ",";
  if(HasDispOption(nm))
    return;
  disp_opts += nm;
}


//////////////////////////
//	float_RArray	//
//////////////////////////

void float_RArray::InitLinks() {
  float_Array::InitLinks();
  taBase::Own(range, this);
}

void float_RArray::Copy_(const float_RArray& cp) {
  range = cp.range;
}

void float_RArray::Add(const float& it) {
  if(size == 0)
    range.Init(it);		// first one is an 'init'
  else
    UpdateRange(it);
  float_Array::Add(it);
}

void float_RArray::Insert(const float& item, int idx, int n_els) {
  UpdateRange(item);
  float_Array::Insert(item, idx, n_els);
}

void float_RArray::UpdateAllRange() {
  if(size == 0) {
    range.Init(0.0f);
    return;
  }
  range.Init(FastEl(0));
  int i;
  for(i=1; i<size; i++)
    UpdateRange(FastEl(i));
}

float float_RArray::Pop(){
  float result = float_Array::Pop();
  UpdateAllRange();
  return result;
}

bool float_RArray::Remove(uint i, int n){
  bool result = float_Array::Remove(i,n);
  if(result) UpdateAllRange();
  return result;
}

bool float_RArray::Remove(const float& item){
  bool result = float_Array::Remove(item);
//  if(result) UpdateAllRange(); // done in Remove(i,n)
  return result;
}

float float_RArray::MaxVal(int& idx, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  idx = start;
  float rval = FastEl(start);
  int i;
  for(i=start+1;i<end;i++) {
    if(FastEl(i) > rval) {
      idx = i;
      rval = FastEl(i);
    }
  }
  return rval;
}

float float_RArray::AbsMaxVal(int& idx, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  idx = start;
  float rval = fabsf(FastEl(start));
  int i;
  for(i=start+1;i<end;i++) {
    if(fabsf(FastEl(i)) > rval) {
      idx = i;
      rval = fabsf(FastEl(i));
    }
  }
  return rval;
}

float float_RArray::MinVal(int& idx, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  idx = start;
  float rval = FastEl(start);
  int i;
  for(i=start+1;i<end;i++) {
    if(FastEl(i) < rval) {
      idx = i;
      rval = FastEl(i);
    }
  }
  return rval;
}

float float_RArray::Sum(int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++)
    rval += FastEl(i);
  return rval;
}

float float_RArray::Mean(int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end <= start)	return 0.0f;
  int sz = end - start;
  return Sum(start,end) / (float)sz;
}
  
float float_RArray::Var(float mean, bool use_mean, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end <= start)	return 0.0f;
  if(!use_mean)    mean = Mean(start,end);
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++)
    rval += (FastEl(i) - mean) * (FastEl(i) - mean);
  int sz = (end - start)-1;
  if(sz == 0) sz = 1;
  return rval / (float)sz;
}

float float_RArray::StdDev(float mean, bool use_mean, int start, int end) const {
  return sqrtf(Var(mean, use_mean, start, end));
}
  
float float_RArray::SEM(float mean, bool use_mean, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end <= start)	return 0.0f;
  return StdDev(mean, use_mean, start, end) / sqrtf(float(end - start));
}
  
float float_RArray::Covar(const float_RArray& oth, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  if(end <= start)	return 0.0f;
  float my_mean = Mean(start,end);
  float oth_mean = oth.Mean(start,end);
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++)
    rval += (FastEl(i) - my_mean) * (oth.FastEl(i) - oth_mean);
  int sz = end - start;
  return rval / (float)sz;
}
  
float float_RArray::Correl(const float_RArray& oth, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  if(end <= start)	return 0.0f;
  float my_mean = Mean(start,end);
  float oth_mean = oth.Mean(start,end);
  float my_var = 0.0f;
  float oth_var = 0.0f; 
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++) {
    float my_val = FastEl(i) - my_mean;
    float oth_val = oth.FastEl(i) - oth_mean;
    rval += my_val * oth_val;
    my_var += my_val * my_val;
    oth_var += oth_val * oth_val;
  }
  float var_prod = sqrtf(my_var * oth_var);
  if(var_prod != 0.0f)
    return rval / var_prod;
  else
    return 0.0f;
}

float float_RArray::SSLength(int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++)
    rval += FastEl(i) * FastEl(i);
  return rval;
}

float float_RArray::InnerProd(const float_RArray& oth, bool norm,
			      int start, int end) const
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  if(end == 0)    return 0.0f;
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++)
    rval += FastEl(i) * oth.FastEl(i);
  if(norm) {
    float dist = sqrtf(SSLength(start, end) * oth.SSLength(start, end));
    if(dist != 0.0f)
      rval /= dist;
  }
  return rval;
}

float float_RArray::SumSquaresDist(const float_RArray& oth, bool norm, float tolerance,
				   int start, int end) const
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++) {
    float d = FastEl(i) - oth.FastEl(i);
    if(fabs(d) <= tolerance)  d = 0.0f;
    rval += d * d;
  }
  if(norm) {
    float dist = SSLength(start, end) + oth.SSLength(start, end);
    if(dist != 0.0f)
      rval /= dist;
  }
  return rval;
}

float float_RArray::EuclidDist(const float_RArray& oth, bool norm, float tolerance,
				   int start, int end) const
{
  return sqrtf(SumSquaresDist(oth, norm, tolerance, start, end));
}

float float_RArray::HammingDist(const float_RArray& oth, bool norm, float tolerance,
				   int start, int end) const
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  float rval = 0.0f;
  float alen = 0.0f;
  float blen = 0.0f;
  int i;
  for(i=start;i<end;i++) {
    float d = fabs(FastEl(i) - oth.FastEl(i));
    if(d <= tolerance)  d = 0.0f;
    rval += d;
    if(norm) {
      alen += fabs(FastEl(i));
      blen += fabs(oth.FastEl(i));
    }
  }
  if(norm) {
    float dist = alen + blen;
    if(dist != 0.0f)
      rval /= dist;
  }
  return rval;
}

float float_RArray::CrossEntropy(const float_RArray& oth, int start, int end) const
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  if(end == 0)    return 0.0f;
  float rval = 0.0f;
  int i;
  for(i=start;i<end;i++) {
    float p = FastEl(i);
    float q = oth.FastEl(i);
    q = MAX(q,0.000001f); q = MIN(q,0.999999f);
    if(p >= 1.0f)
      rval += -logf(q);
    else if(p <= 0.0f)
      rval += -logf(1.0f - q);
    else
      rval += p * logf(p/q) + (1.0f - p) * logf((1.0f - p) / (1.0f - q));
  }
  return rval;
}

float float_RArray::Dist(const float_RArray& oth, DistMetric metric, bool norm,
			 float tolerance, int start, int end) const
{
  switch(metric) {
  case SUM_SQUARES:
    return SumSquaresDist(oth, norm, tolerance, start, end);
  case EUCLIDIAN:
    return EuclidDist(oth, norm, tolerance, start, end);
  case HAMMING:
    return HammingDist(oth, norm, tolerance, start, end);
  case COVAR:
    return Covar(oth, start, end);
  case CORREL:
    return Correl(oth, start, end);
  case INNER_PROD:
    return InnerProd(oth, norm, start, end);
  case CROSS_ENTROPY:
    return CrossEntropy(oth, start, end);
  }
  return 0.0f;
}

bool float_RArray::LargerFurther(DistMetric metric) {
  switch(metric) {
  case SUM_SQUARES:
    return true;
  case EUCLIDIAN:
    return true;
  case HAMMING:
    return true;
  case COVAR:
    return false;
  case CORREL:
    return false;
  case INNER_PROD:
    return false;
  case CROSS_ENTROPY:
    return false;
  }
  return 0.0f;
}
  
void float_RArray::Histogram(const float_RArray& oth, float bin_size) {
  Reset();
  float_RArray tmp = oth;	// need to sort it!
  tmp.Sort();
  float min = tmp[0];
  float max = tmp.Peek();
  int src_idx = 0;
  int trg_idx = 0;
  float cur_val;
  for(cur_val = min; cur_val <= max; cur_val += bin_size, trg_idx++) {
    float cur_max = cur_val + bin_size;
    Add(0);
    float& cur_hist = FastEl(trg_idx);
    while((src_idx < tmp.size) && (tmp[src_idx] < cur_max)) {
      cur_hist += 1.0f;
      src_idx++;
    }
  }
}

void float_RArray::AggToArray(const float_RArray& oth, Aggregate& agg,
			      int start, int end)
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  end = MIN(end, oth.size);
  int i;
  for(i=start;i<end;i++)
    agg.ComputeAggNoUpdt(FastEl(i), oth.FastEl(i));
  UpdateAllRange();
}

float float_RArray::AggToVal(Aggregate& agg, int start, int end) const {
  if(end == -1)	end = size;  else end = MIN(size, end);
  float rval = agg.InitAggVal();
  agg.Init();
  int i;
  for(i=start;i<end;i++)
    agg.ComputeAgg(rval, FastEl(i));
  return rval;
}

void float_RArray::CopyVals(const taArray_impl& from, int start, int end, int at) {
  float_Array::CopyVals(from, start, end, at);
  UpdateAllRange();
}
    
float float_RArray::NormLen(float len, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0) 	return 0.0f;
  float scale = (len * len) / SSLength(start,end);
  int i;
  for(i=start;i<end;i++) {
    float mag = (FastEl(i) * FastEl(i)) * scale;
    FastEl(i) = (FastEl(i) >= 0.0f) ? mag : -mag;
  }
  UpdateAllRange();
  return scale;
}

float float_RArray::NormSum(float sum, float min_val, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)	return 0.0f;
  float act_sum = 0.0f;
  int i;
  for(i=start;i<end;i++)
    act_sum += (FastEl(i) - range.min);
  float scale = (sum / act_sum);
  for(i=start;i<end;i++)
    FastEl(i) = ((FastEl(i) - range.min) * scale) + min_val;
  UpdateAllRange();
  return scale;
}

float float_RArray::NormMax(float max, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)	return 0.0f;
  int idx;
  float cur_max = MaxVal(idx, start, end);
  float scale = (max / cur_max);
  int i;
  for(i=start;i<end;i++)
    FastEl(i) *= scale;
  UpdateAllRange();
  return scale;
}

float float_RArray::NormAbsMax(float max, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)	return 0.0f;
  int idx;
  float cur_max = AbsMaxVal(idx, start, end);
  float scale = (max / cur_max);
  int i;
  for(i=start;i<end;i++)
    FastEl(i) *= scale;
  UpdateAllRange();
  return scale;
}

void float_RArray::SimpleMath(const SimpleMathSpec& math_spec, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)  return;
  int i;
  for(i=start;i<end;i++)
    FastEl(i) = math_spec.Evaluate(FastEl(i));
  UpdateAllRange();
}

void float_RArray::SimpleMathArg(const float_RArray& arg_ary, const SimpleMathSpec& math_spec,
				 int start, int end)
{
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)  return;
  SimpleMathSpec myms = math_spec;
  int i;
  for(i=start;i<end;i++) {
    if(i < arg_ary.size)
      myms.arg = arg_ary.FastEl(i);
    FastEl(i) = myms.Evaluate(FastEl(i));
  }
  UpdateAllRange();
}

int float_RArray::Threshold(float thresh, float low, float high, int start, int end) {
  if(end == -1)	end = size;  else end = MIN(size, end);
  if(end == 0)  return 0;
  int rval = 0;
  int i;
  for(i=start;i<end;i++) {
    if(FastEl(i) >= thresh) {
      FastEl(i) = high;
      rval++;
    }
    else
      FastEl(i) = low;
  }
  range.min = low;
  range.max = high;
  return rval;
}

void float_RArray::ShiftLeft(int nshift) {
  float_Array::ShiftLeft(nshift);
  UpdateAllRange();
} 

int float_RArray::Dump_Load_Value(istream& strm, TAPtr par) {
  int rval = float_Array::Dump_Load_Value(strm, par);
  UpdateAllRange();
  return rval;
}

void float_RArray::WritePoint(const TwoDCoord& geom, int x, int y, float color, bool wrap) {
  bool clipped = (TwoDCoord::WrapClipOne(wrap, x, geom.x) ||
		  TwoDCoord::WrapClipOne(wrap, y, geom.y));
  if(!wrap && clipped)
    return;
  SafeEl((y * geom.x) + x) = color;
} 

void float_RArray::RenderLine(const TwoDCoord& geom, int xs, int ys, int xe, int ye, 
			      float color, bool wrap)
{
  int xd = xe - xs;
  int yd = ye - ys;
  int x,y;
  if((xd == 0) && (yd == 0)) {
    WritePoint(geom, xs, ys, color, wrap);
    return;
  }
  if(xd == 0) {
    if(yd > 0) { for(y=ys;y<=ye;y++) WritePoint(geom, xs, y, color, wrap); }
    else       { for(y=ys;y>=ye;y--) WritePoint(geom, xs, y, color, wrap); }
    return;
  }
  if(yd == 0) {
    if(xd > 0)	{ for(x=xs;x<=xe;x++) WritePoint(geom, x, ys, color, wrap); }
    else	{ for(x=xs;x>=xe;x--) WritePoint(geom, x, ys, color, wrap); }
    return;
  }
  if(abs(xd) > abs(yd)) {
    if(yd > 0) yd++; else yd--;
    if(xd > 0) {
      for(x=xs; x<=xe; x++) {
	y = ys + (yd * (x - xs)) / abs(xd);
	if(yd > 0) y = MIN(ye, y);
	else y = MAX(ye, y);
	WritePoint(geom, x, y, color, wrap);
      }
    }
    else {
      for(x=xs; x>=xe; x--) {
	y = ys + (yd * (xs - x)) / abs(xd);
	if(yd > 0) y = MIN(ye, y);
	else y = MAX(ye, y);
	WritePoint(geom, x, y, color, wrap);
      }
    }
  }
  else {
    if(xd > 0) xd++; else xd--;
    if(yd > 0) {
      for(y=ys; y<=ye; y++) {
	x = xs + (xd * (y - ys)) / abs(yd);
	if(xd > 0) x = MIN(xe, x);
	else x = MAX(xe, x);
	WritePoint(geom, x, y, color, wrap);
      }
    }
    else {
      for(y=ys; y>=ye; y--) {
	x = xs + (xd * (ys - y)) / abs(yd);
	if(xd > 0) x = MIN(xe, x);
	else x = MAX(xe, x);
	WritePoint(geom, x, y, color, wrap);
      }
    }
  }
}

void float_RArray::WriteXPoints(const TwoDCoord& geom, int x, int y, 
				const float_RArray& color, int wdth, bool wrap)
{
  int del = (wdth - 1) / 2;
  int i;
  for(i=0; i<wdth; i++)
    WritePoint(geom, x+i-del, y, color[i], wrap);
}

void float_RArray::WriteYPoints(const TwoDCoord& geom, int x, int y, 
				const float_RArray& color, int wdth, bool wrap)
{
  int del = (wdth - 1) / 2;
  int i;
  for(i=0; i<wdth; i++)
    WritePoint(geom, x, y+i-del, color[i], wrap);
}

void float_RArray::RenderWideLine(const TwoDCoord& geom, int xs, int ys, int xe, int ye, 
				  const float_RArray& color, int wdth, bool wrap)
{
  int xd = xe - xs;
  int yd = ye - ys;
  int x,y;
  if((xd == 0) && (yd == 0)) {
    WriteXPoints(geom, xs, ys, color, wdth, wrap);
    WriteYPoints(geom, xs, ys, color, wdth, wrap);
    return;
  }
  if(xd == 0) {
    if(yd > 0) { for(y=ys;y<=ye;y++) WriteXPoints(geom, xs, y, color, wdth, wrap); }
    else       { for(y=ys;y>=ye;y--) WriteXPoints(geom, xs, y, color, wdth, wrap); }
    return;
  }
  if(yd == 0) {
    if(xd > 0)	{ for(x=xs;x<=xe;x++) WriteYPoints(geom, x, ys, color, wdth, wrap); }
    else	{ for(x=xs;x>=xe;x--) WriteYPoints(geom, x, ys, color, wdth, wrap); }
    return;
  }
  if(abs(xd) > abs(yd)) {
    if(xd > 0) {
      for(x=xs; x<=xe; x++) {
	y = ys + (yd * (x - xs)) / abs(xd);
	WriteXPoints(geom, x, y, color, wdth, wrap);
      }
    }
    else {
      for(x=xs; x>=xe; x--) {
	y = ys + (yd * (xs - x)) / abs(xd);
	WriteXPoints(geom, x, y, color, wdth, wrap);
      }
    }
  }
  else {
    if(yd > 0) {
      for(y=ys; y<=ye; y++) {
	x = xs + (xd * (y - ys)) / abs(yd);
	WriteYPoints(geom, x, y, color, wdth, wrap);
      }
    }
    else {
      for(y=ys; y>=ye; y--) {
	x = xs + (xd * (ys - y)) / abs(yd);
	WriteYPoints(geom, x, y, color, wdth, wrap);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////
//  Matricies,  Eigenvectors / Values

void float_RArray::GetMatCol(int col_dim, float_RArray& col_vec, int col_no) {
  col_vec.Reset();
  int rows = size / col_dim;
  for(int i=0;i<rows; i++) {
    col_vec.Add(FastMatEl(col_dim, i, col_no));
  }
}

void float_RArray::GetMatRow(int col_dim, float_RArray& row_vec, int row_no) {
  row_vec.Reset();
  int sti = row_no * col_dim;
  for(int i=0;i<col_dim; i++, sti++) {
    row_vec.Add(FastEl(sti));
  }
}

void float_RArray::CopyFmTriMat(int dim, const float_RArray& tri_mat) {
  EnforceSize(dim * dim);
  int i, j;
  for(i=0;i<dim;i++) {
    for(j=i;j<dim;j++) {
      FastMatEl(dim,i,j) = FastMatEl(dim,j,i) = tri_mat.FastTriMatEl(dim,i,j);
    }
  }
}

bool float_RArray::TriDiagMatRed(int n, float_RArray& d, float_RArray& e) {
  if(size != n * n) {
    taMisc::Error("*** TriDiagMatRed: matrix is not of appropriate size for dimensionality:",String(n));
    return false;
  }

  // this code is adapted from the NRC tred2.c function!

  d.EnforceSize(n);
  e.EnforceSize(n);

  int l,k,j,i;
  float scale,hh,h,g,f;
  for(i=n;i>=2;i--) {
    l=i-1;
    h=scale=0.0;
    if(l > 1) {
      for(k=1;k<=l;k++)
	scale += fabs(FastMatEl1(n,i,k));
      if(scale == 0.0)
	e.FastEl1(i) = FastMatEl1(n,i,l);
      else {
	for(k=1;k<=l;k++) {
	  float& vl = FastMatEl1(n, i,k);
	  vl /= scale;
	  h += vl * vl;
	}
	f= FastMatEl1(n, i, l);
	g = f > 0 ? -sqrtf(h) : sqrtf(h);
	e.FastEl1(i)=scale*g;
	h -= f*g;
	FastMatEl1(n,i,l)=f-g;
	f=0.0;
	for (j=1;j<=l;j++) {
	  /* Next statement can be omitted if eigenvectors not wanted */
	  FastMatEl1(n,j,i)=FastMatEl1(n,i,j)/h;
	  g=0.0;
	  for (k=1;k<=j;k++)
	    g += FastMatEl1(n,j,k)*FastMatEl1(n,i,k);
	  for (k=j+1;k<=l;k++)
	    g += FastMatEl1(n,k,j)*FastMatEl1(n,i,k);
	  e.FastEl1(j)=g/h;
	  f += e.FastEl1(j)*FastMatEl1(n,i,j);
	}
	hh=f/(h+h);
	for (j=1;j<=l;j++) {
	  f=FastMatEl1(n,i,j);
	  e.FastEl1(j)=g=e.FastEl1(j)-hh*f;
	  for (k=1;k<=j;k++)
	    FastMatEl1(n,j,k) -= (f*e.FastEl1(k)+g*FastMatEl1(n,i,k));
	}
      }
    } else
      e.FastEl1(i)=FastMatEl1(n,i,l);
    d.FastEl1(i)=h;
  }
  /* Next statement can be omitted if eigenvectors not wanted */
  d[0]=0.0;
  e[0]=0.0;
  /* Contents of this loop can be omitted if eigenvectors not
     wanted except for statement d[i]=a[i][i]; */
  for (i=1;i<=n;i++) {
    l=i-1;
    if (d.FastEl1(i)) {
      for (j=1;j<=l;j++) {
	g=0.0;
	for (k=1;k<=l;k++)
	  g += FastMatEl1(n,i,k)*FastMatEl1(n,k,j);
	for (k=1;k<=l;k++)
	  FastMatEl1(n,k,j) -= g*FastMatEl1(n,k,i);
      }
    }
    d.FastEl1(i)=FastMatEl1(n,i,i);
    FastMatEl1(n,i,i)=1.0;
    for (j=1;j<=l;j++)
      FastMatEl1(n,j,i)=FastMatEl1(n,i,j)=0.0;
  }
  return true;
}

#define SIGN(a,b) ((b)<0 ? -fabs(a) : fabs(a))
    
bool float_RArray::TriDiagQL(int n, float_RArray& d, float_RArray& e) {
  int m,l,iter,i,k;
  float s,r,p,g,f,dd,c,b;
    
  // this code is adapted from the NRC tqli.c function!

  for(i=2;i<=n;i++) e.FastEl1(i-1)=e.FastEl1(i);
  e.FastEl1(n)=0.0;
  for (l=1;l<=n;l++) {
    iter=0;
    do {
      for(m=l;m<=n-1;m++) {
	dd=fabs(d.FastEl1(m))+fabs(d.FastEl1(m+1));
	if (fabs(e.FastEl1(m))+dd == dd) break;
      }
      if(m != l) {
	if (iter++ == 30) {
	  taMisc::Error("*** TriDiagQL: Too many iterations!");
	  return false;
	}
	g=(d.FastEl1(l+1)-d.FastEl1(l))/(2.0*e.FastEl1(l));
	r=sqrt((g*g)+1.0);
	g=d.FastEl1(m)-d.FastEl1(l)+e.FastEl1(l)/(g+SIGN(r,g));
	s=c=1.0f;
	p=0.0;
	for(i=m-1;i>=l;i--) {
	  f=s*e.FastEl1(i);
	  b=c*e.FastEl1(i);
	  if(fabs(f) >= fabs(g)) {
	    c=g/f;
	    r=sqrt((c*c)+1.0);
	    e.FastEl1(i+1)=f*r;
	    c *= (s=1.0/r);
	  } else {
	    s=f/g;
	    r=sqrt((s*s)+1.0);
	    e.FastEl1(i+1)=g*r;
	    s *= (c=1.0/r);
	  }
	  g=d.FastEl1(i+1)-p;
	  r=(d.FastEl1(i)-g)*s+2.0*c*b;
	  p=s*r;
	  d.FastEl1(i+1)=g+p;
	  g=c*r-b;
	  /* Next loop can be omitted if eigenvectors not wanted */
	  for (k=1;k<=n;k++) {
	    f=FastMatEl1(n,k,i+1);
	    FastMatEl1(n,k,i+1)=s*FastMatEl1(n,k,i)+c*f;
	    FastMatEl1(n,k,i)=c*FastMatEl1(n,k,i)-s*f;
	  }
	}
	d.FastEl1(l) -= p;
	e.FastEl1(l)=g;
	e.FastEl1(m)=0.0;
      }
    } while (m != l);
  }
  return true;
}

bool float_RArray::Eigens(int n, float_RArray& evals) {
  evals.EnforceSize(n);
  
  if(size != n * n) {
    taMisc::Error("*** Eigens: matrix is not of appropriate size for dimensionality:",String(n));
    return false;
  }
  float_RArray off_diags;
  off_diags.EnforceSize(n);
  TriDiagMatRed(n, evals, off_diags);
  if(!TriDiagQL(n, evals, off_diags))
    return false;
  return true;
}

bool float_RArray::MDS(int dim, float_RArray& xcoords, float_RArray& ycoords, int x_axis_c, int y_axis_c, bool print_evals) {
  if(size != dim * dim) {
    taMisc::Error("*** MDS: matrix is not of appropriate size for dimensionality:",String(dim));
    return false;
  }
  if((x_axis_c < 0) || (x_axis_c >= dim)) {
    taMisc::Error("*** MDS: x_axis component must be between 0 and",String(dim-1));
    return false;
  }
  if((y_axis_c < 0) || (y_axis_c >= dim)) {
    taMisc::Error("*** MDS: y_axis component must be between 0 and",String(dim-1));
    return false;
  }

  int i,j;
  // first square the individual elements
  for(i=0;i<size;i++) FastEl(i) *= FastEl(i);

  // then double-center the matrix
  for(i=0; i<dim; i++) {
    double sum = 0.0;
    for(j=0; j<dim; j++)
      sum += FastMatEl(dim, i, j);
    sum /= (double)dim;
    for(j=0; j<dim; j++)
      FastMatEl(dim, i, j) -= sum;
  }
  for(j=0; j<dim; j++) {
    double sum = 0.0;
    for(i=0; i<dim; i++)
      sum += FastMatEl(dim, i, j);
    sum /= (double)dim;
    for(i=0; i<dim; i++)
      FastMatEl(dim, i, j) -= sum;
  }

  for(i=0;i<size;i++) FastEl(i) *= -.5f;

  float_RArray evals;
  Eigens(dim, evals);
  if(print_evals) {
    cerr << "eigen vals: " << endl;
    evals.List(cerr);
    cerr << endl;
  }

  // multiply the eigenvectors by sqrt(eigen values)
  xcoords.Reset();
  int x_axis_rev = dim - 1 - x_axis_c;
  if(print_evals) {
    cerr << "X coordinate (" << x_axis_c << ") eigen value: " << evals[x_axis_rev] << endl;
  }
  float evsq = sqrt(fabs(evals[x_axis_rev]));
  for(i=0;i<dim;i++) {
    float val = FastMatEl(dim, i, x_axis_rev) * evsq;
    xcoords.Add(val);
  }

  ycoords.Reset();
  int y_axis_rev = dim - 1 - y_axis_c;
  if(print_evals) {
    cerr << "Y coordinate (" << y_axis_c << ") eigen value: " << evals[y_axis_rev] << endl;
  }
  evsq = sqrt(fabs(evals[y_axis_rev]));
  for(i=0;i<dim;i++) {
    float val = FastMatEl(dim, i, y_axis_rev) * evsq;
    ycoords.Add(val);
  }

  return true;
}

//////////////////////////
//	DataTable	//
//////////////////////////

void DataTable::Initialize() {
  SetBaseType(&TA_DataArray);	// the impl doesn't inherit properly..
}

void DataTable::Destroy() {
}

void DataTable::InitLinks() {
  taGroup<DataArray_impl>::InitLinks();
}

void DataTable::UpdateAfterEdit() {
  taGroup<DataArray_impl>::UpdateAfterEdit();
}

void DataTable::ResetData() {
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    ar->AR()->Reset();
  }
}

void DataTable::RemoveRow(int row_num) {
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    ar->AR()->Remove(row_num);
  }
}

void DataTable::AddBlankRow() {
  if(leaves == 0)    return;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    if(ar->InheritsFrom(TA_String_Data))
      ((String_Array*)ar->AR())->Add("");
    else 
      ((float_RArray*)ar->AR())->Add(0);
  }
}

void DataTable::SetSaveToFile(bool save_to_file) {
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    ar->save_to_file = save_to_file;
  }
}

void DataTable::AddRowToArray(float_RArray& tar, int row_num) const {
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    float val = 0;
    if(ar->InheritsFrom(TA_float_Data))
      val = ((float_RArray*)ar->AR())->SafeEl(row_num);
    tar.Add(val);
  }
}

void DataTable::AggRowToArray(float_RArray& tar, int row_num, Aggregate& agg) const {
  if(tar.size < leaves)
    tar.Insert(0, tar.size, leaves - tar.size);
  int cnt = 0;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    float val = 0;
    if(ar->InheritsFrom(TA_float_Data))
      val = ((float_RArray*)ar->AR())->SafeEl(row_num);
    agg.ComputeAggNoUpdt(tar.FastEl(cnt), val);
    cnt++;
  }
}

float DataTable::AggRowToVal(int row_num, Aggregate& agg) const {
  float rval = agg.InitAggVal();
  agg.Init();
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    float val = 0;
    if(ar->InheritsFrom(TA_float_Data))
      val = ((float_RArray*)ar->AR())->SafeEl(row_num);
    agg.ComputeAgg(rval, val);
  }
  return rval;
}

void DataTable::AddArrayToRow(float_RArray& tar) {
  if(tar.size == 0)	return;
  int cnt = 0;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    if(ar->InheritsFrom(TA_float_Data))
      ((float_RArray*)ar->AR())->Add(tar.FastEl(cnt));
    cnt++;
    if(cnt >= tar.size)	break;
  }
}

void DataTable::AggArrayToRow(const float_RArray& tar, int row_num, Aggregate& agg) {
  if(tar.size == 0)	return;
  int cnt = 0;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    if(ar->InheritsFrom(TA_float_Data)) {
      float& val = ((float_RArray*)ar->AR())->SafeEl(row_num);
      agg.ComputeAggNoUpdt(val, tar.FastEl(cnt));
    }
    cnt++;
    if(cnt >= tar.size)	break;
  }
}

void DataTable::PutArrayToRow(const float_RArray& tar, int row_num) {
  if(tar.size == 0)	return;
  int cnt = 0;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    if(ar->InheritsFrom(TA_float_Data)) {
      ((float_RArray*)ar->AR())->SafeEl(row_num) = tar.FastEl(cnt);
    }
    cnt++;
    if(cnt >= tar.size)	break;
  }
}

void DataTable::UpdateAllRanges() {
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    if(ar->InheritsFrom(TA_float_Data)) {
      ((float_RArray*)ar->AR())->UpdateAllRange();
    }
  }
}

int DataTable::MaxLength() {
  int max = 0;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    max = MAX(max,ar->AR()->size);
  }
  return max;
}    

int DataTable::MinLength() {
  if(size == 0) return 0;
  int min = INT_MAX;
  taLeafItr i;
  DataArray_impl* ar;
  FOR_ITR_EL(DataArray_impl, ar, this->, i) {
    min = MIN(min,ar->AR()->size);
  }
  return min;
}

float_Data* DataTable::NewColFloat(const char* col_nm) {
  float_Data* rval = (float_Data*) NewEl(1, &TA_float_Data);
  rval->name = col_nm;
  return rval;
}

float_Data* DataTable::NewColInt(const char* col_nm) {
  float_Data* rval = (float_Data*) NewEl(1, &TA_float_Data);
  rval->name = String("|") + col_nm; // | = narrow/int value
  rval->AddDispOption("NARROW");
  return rval;
}

String_Data* DataTable::NewColString(const char* col_nm) {
  String_Data* rval = (String_Data*) NewEl(1, &TA_String_Data);
  rval->name = String("$") + col_nm; // $ = string data
  return rval;
}

DataTable* DataTable::NewGroupFloat(const char* col_nm, int n) {
  DataTable* rval = (DataTable*)NewGp(1);
  rval->el_typ = &TA_float_Data;
  rval->EnforceSize(n);
  rval->name = col_nm;
  if(n > 0) {
    float_Data* da = (float_Data*)rval->FastEl(0);
    da->name = String("<") + (String)n + ">" + col_nm + "_0"; // <n> indicates vector
  }
  int i;
  for(i=1;i<n;i++) {
    float_Data* da = (float_Data*)rval->FastEl(i);
    da->name = String(col_nm) + "_" + String(i);
  }
  return rval;
}

DataTable* DataTable::NewGroupInt(const char* col_nm, int n) {
  DataTable* rval = (DataTable*)NewGp(1);
  rval->el_typ = &TA_float_Data;
  rval->EnforceSize(n);
  rval->name = String("|") + col_nm;
  if(n > 0) {
    float_Data* da = (float_Data*)rval->FastEl(0);
    da->name = String("|<") + (String)n + ">" + col_nm + "_0"; // <n> indicates vector
    da->AddDispOption("NARROW");
  }
  int i;
  for(i=1;i<n;i++) {
    float_Data* da = (float_Data*)rval->FastEl(i);
    da->name = String("|") + String(col_nm) + "_" + String(i);
    da->AddDispOption("NARROW");
  }
  return rval;
}

DataTable* DataTable::NewGroupString(const char* col_nm, int n) {
  DataTable* rval = (DataTable*)NewGp(1);
  rval->el_typ = &TA_String_Data;
  rval->EnforceSize(n);
  rval->name = String("$") + col_nm;
  if(n > 0) {
    float_Data* da = (float_Data*)rval->FastEl(0);
    da->name = String("$<") + (String)n + ">" + col_nm + "_0"; // <n> indicates vector
  }
  int i;
  for(i=1;i<n;i++) {
    float_Data* da = (float_Data*)rval->FastEl(i);
    da->name = String("$") + String(col_nm) + "_" + String(i);
  }
  return rval;
}

DataArray_impl* DataTable::GetColData(int col, int subgp) {
  DataTable* tbl;
  if(subgp < 0) tbl = this;
  else { if(subgp >= gp.size) return NULL; tbl = (DataTable*)gp.FastEl(subgp); }
  if(col >= tbl->size) return NULL;
  return (DataArray_impl*)tbl->FastEl(col);
}

float_Data* DataTable::GetColFloatData(int col, int subgp) {
  DataArray_impl* da = GetColData(col, subgp);
  if((da != NULL) && da->InheritsFrom(TA_float_Data)) return (float_Data*)da;
  return NULL;
}

String_Data* DataTable::GetColStringData(int col, int subgp) {
  DataArray_impl* da = GetColData(col, subgp);
  if((da != NULL) && da->InheritsFrom(TA_String_Data)) return (String_Data*)da;
  return NULL;
}

float_RArray* DataTable::GetColFloatArray(int col, int subgp) {
  float_Data* dt = GetColFloatData(col, subgp);
  if(dt != NULL) return dt->ar;
  return NULL;
}

String_Array* DataTable::GetColStringArray(int col, int subgp) {
  String_Data* dt = GetColStringData(col, subgp);
  if(dt != NULL) return dt->ar;
  return NULL;
}

void DataTable::PutArrayToCol(const float_RArray& ar, int col, int subgp) {
  float_RArray* far = GetColFloatArray(col, subgp);
  far->CopyFrom((taBase*)&ar);
}

void DataTable::SetColName(const char* col_nm, int col, int subgp) {
  DataArray_impl* da = GetColData(col, subgp);
  if(da != NULL) da->name = col_nm;
}

void DataTable::AddColDispOpt(const char* dsp_opt, int col, int subgp) {
  DataArray_impl* da = GetColData(col, subgp);
  if(da != NULL) da->AddDispOption(dsp_opt);
}

void DataTable::AddFloatVal(float val, int col, int subgp) {
  float_RArray* dt = GetColFloatArray(col, subgp);
  if(dt != NULL) dt->Add(val);
}

void DataTable::AddStringVal(const char* val, int col, int subgp) {
  String_Array* dt = GetColStringArray(col, subgp);
  if(dt != NULL) dt->Add(val);
}

void DataTable::SetFloatVal(float val, int col, int row, int subgp) {
  float_RArray* dt = GetColFloatArray(col, subgp);
  if(dt != NULL) dt->SafeEl(row) = val;
}

void DataTable::SetStringVal(const char* val, int col, int row, int subgp) {
  String_Array* dt = GetColStringArray(col, subgp);
  if(dt != NULL) dt->SafeEl(row) = val;
}

void DataTable::SetLastFloatVal(float val, int col, int subgp) {
  float_RArray* dt = GetColFloatArray(col, subgp);
  if(dt != NULL) dt->Peek() = val;
}

void DataTable::SetLastStringVal(const char* val, int col, int subgp) {
  String_Array* dt = GetColStringArray(col, subgp);
  if(dt != NULL) dt->Peek() = val;
}

float DataTable::GetFloatVal(int col, int row, int subgp) {
  float_RArray* dt = GetColFloatArray(col, subgp);
  if(dt != NULL) return dt->SafeEl(row);
  return 0.0f;
}

String DataTable::GetStringVal(int col, int row, int subgp) {
  String_Array* dt = GetColStringArray(col, subgp);
  if(dt != NULL) return dt->SafeEl(row);
  return "";
}

//////////////////////////
// 	ClustNode	//
//////////////////////////

void ClustLink::Initialize() {
  dist = 0.0f;
  node = NULL;
}

void ClustLink::Copy_(const ClustLink& cp) {
  dist = cp.dist;
  taBase::SetPointer((TAPtr*)&node, cp.node);
}

void ClustLink::CutLinks() {
  taBase::DelPointer((TAPtr*)&node);
  taBase::CutLinks();
}

void ClustNode::Initialize() {
  pat = NULL;
  leaf_idx = -1;
  leaf_max = -1;
  leaf_dists = NULL;
  par_dist = 0.0f;
  nn_dist = 0.0f;
  tmp_dist = 0.0f;
  y = 0.0f;
}

void ClustNode::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(children, this);
  taBase::Own(nns, this);
}

void ClustNode::CutLinks() {
  taBase::DelPointer((TAPtr*)&leaf_dists);
  taBase::DelPointer((TAPtr*)&pat);
  children.Reset();
  nns.Reset();
  taNBase::CutLinks();
}

void ClustNode::SetPat(float_RArray* pt) {
  taBase::SetPointer((TAPtr*)&pat, pt);
}

void ClustNode::AddChild(ClustNode* nd, float dst) {
  ClustLink* lk = new ClustLink;
  taBase::SetPointer((TAPtr*)&(lk->node), nd);
  lk->dist = dst;
  children.Add(lk);
}

void ClustNode::LinkNN(ClustNode* nd, float dst) {
  ClustLink* lk = new ClustLink;
  taBase::SetPointer((TAPtr*)&(lk->node), nd);
  lk->dist = dst;
  nns.Add(lk);
}

bool ClustNode::RemoveChild(ClustNode* nd) {
  bool rval = false;
  int i;
  for(i=children.size-1;i>=0;i--) {
    if(GetChild(i) == nd) {
      children.Remove(i);
      rval = true;
    }
  }
  return rval;
}

int ClustNode::FindChild(ClustNode* nd) {
  int i;
  for(i=0;i<children.size;i++) {
    if(GetChild(i) == nd)
      return i;
  }
  return -1;
}

void ClustNode::Cluster(float_RArray::DistMetric metric,
			bool norm, float tol)
{
  if(!float_RArray::LargerFurther(metric)) {
    taMisc::Error("Cluster requires distance metric where larger = further apart");
    return;
  }

  if(children.size <= 1) {
    taMisc::Error("Cluster requires at least 2 items to operate on!");
    return;
  }

  // first get indicies for all leaves, and their distances!
  taBase::SetPointer((TAPtr*)&leaf_dists, new float_RArray);
  for(int i=0; i<children.size; i++) {
    ClustNode* nd = GetChild(i);
    nd->leaf_idx = i;
    nd->leaf_max = children.size;
    taBase::SetPointer((TAPtr*)&(nd->leaf_dists), leaf_dists);
    for(int j=i;j<children.size;j++) {
      ClustNode* ond = GetChild(j);
      float dst = nd->pat->Dist(*(ond->pat), metric, norm, tol);
      leaf_dists->Add(dst);
    }
  }

  do {
    // set nearest neighbor pointers
    NNeighbors(metric, norm, tol);
    // find closest and make a new node
  } while (ClustOnClosest(metric));

  SetParDists(0.0f, metric);

  taBase::DelPointer((TAPtr*)&leaf_dists);
}

void ClustNode::Graph(ostream& strm) {
  SetYs(0.5f);
  nn_dist = .1;
  Graph_impl(strm);
}

void ClustNode::Graph_impl(ostream& strm) {
  if(pat != NULL) {
    strm << par_dist - nn_dist << " " << y << "\n";
    strm << par_dist << " " << y << " \"" << name << "\"\n";
    strm << par_dist - nn_dist << " " << y << "\n";
    strm << par_dist - nn_dist << " " << y << "\n";
  }
  else {
    strm << par_dist - nn_dist << " " << y << "\n";
    strm << par_dist << " " << y << "\n";

    int i;
    for(i=0; i<children.size; i++) {
      ClustLink* nd = (ClustLink*)children[i];
      nd->node->Graph_impl(strm);
      strm << par_dist << " " << y << "\n";
    }
    
    strm << par_dist - nn_dist << " " << y << "\n";
  }
}

void ClustNode::XGraph(const char* fnm, const char* title) {
  fstream fh;
  fh.open(fnm, ios::out);
  fh << "TitleText: " << title << "\n";
  Graph(fh);
  fh.close(); fh.clear();
  String cmd = String("xgraph -0 \"\" ") + fnm + "&";
  system(cmd);
}

void ClustNode::GraphData(DataTable* dt) {
  SetYs(0.5f);
  nn_dist = 0.0f;
  dt->Reset();
  dt->NewColFloat("X");
  dt->NewColFloat("Y");
  dt->NewColString("label");
  dt->AddColDispOpt("DISP_STRING", 2);
  dt->AddColDispOpt("AXIS=1", 2); // labels use same axis as y values
  GraphData_impl(dt);
  dt->UpdateAllRanges();

  float_RArray* xar = dt->GetColFloatArray(0);
  dt->AddColDispOpt(String("MAX=") + String(xar->range.max * 1.15f), 0); // adds extra room for labels

  float_RArray* yar = dt->GetColFloatArray(1);
  dt->AddColDispOpt(String("MAX=") + String(yar->range.max + .3f), 1); // adds extra room for labels
  dt->AddColDispOpt("MIN=0.2", 1);
}

void ClustNode::GraphData_impl(DataTable* dt) {
  if(pat != NULL) {
    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist - nn_dist, 0);
    dt->SetLastFloatVal(y, 1);

    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist, 0);
    dt->SetLastFloatVal(y, 1);
    dt->SetLastStringVal(name, 2);

    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist - nn_dist, 0);
    dt->SetLastFloatVal(y, 1);

    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist - nn_dist, 0);
    dt->SetLastFloatVal(y, 1);
  }
  else {
    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist - nn_dist, 0);
    dt->SetLastFloatVal(y, 1);

    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist, 0);
    dt->SetLastFloatVal(y, 1);

    int i;
    for(i=0; i<children.size; i++) {
      ClustLink* nd = (ClustLink*)children[i];
      nd->node->GraphData_impl(dt);
      dt->AddBlankRow();
      dt->SetLastFloatVal(par_dist, 0);
      dt->SetLastFloatVal(y, 1);
    }
    
    dt->AddBlankRow();
    dt->SetLastFloatVal(par_dist - nn_dist, 0);
    dt->SetLastFloatVal(y, 1);
  }
}

static const float clust_dist_tol = 1.0e-6;

bool ClustNode::ClustOnClosest(float_RArray::DistMetric) {
  if(children.size < 2)
    return false;		// cannot have any more clustering to do!

  float min_d = FLT_MAX;
  int min_idx=-1;
  int i;
  for(i=0; i<children.size; i++) { // find node with closest neighbors
    ClustNode* nd = GetChild(i);
    if(nd->nn_dist < min_d) {
      min_d = nd->nn_dist;
      min_idx = i;
    }
  }
  if(min_idx < 0) return false;
  // make a new cluster around this node
  ClustNode* nd = GetChild(min_idx);
  ClustNode* new_clust = new ClustNode;
  AddChild(new_clust, min_d);
  // add the min node and its nearest neighbors to the new cluster
  new_clust->AddChild(nd);
  children.Remove(min_idx);
  for(i=0;i<nd->nns.size; i++) {
    ClustLink* nlk = (ClustLink*)nd->nns[i];
    if(fabs(nlk->dist - min_d) > clust_dist_tol)
      continue;
    new_clust->AddChild(nlk->node);
    RemoveChild(nlk->node); // and the nns
  }

  // then finally check if any other nns at min_d have other nns not already obtained
  for(i=0;i<nd->nns.size; i++) {
    ClustLink* nlk = (ClustLink*)nd->nns[i];
    if(fabs(nlk->dist - min_d) > clust_dist_tol)
      continue;
    ClustNode* nn = nlk->node;
    int j;
    for(j=0; j<nn->nns.size; j++) {
      ClustLink* nn_nlk = (ClustLink*)nn->nns[j];
      if(fabs(nn_nlk->dist - min_d) > clust_dist_tol)
	continue;
      if(new_clust->FindChild(nn_nlk->node) < 0) { // not in new clust yet
	new_clust->AddChild(nn_nlk->node);
	RemoveChild(nn_nlk->node); // and remove from main list
      }
    }
  }

  if(children.size < 2)
    return false;		// cannot have any more clustering to do!
  return true;
}
  
void ClustNode::NNeighbors(float_RArray::DistMetric metric,
		    bool norm, float tol)
{
  int i;
  for(i=0; i<children.size; i++) {
    ClustNode* nd = GetChild(i);
    nd->nns.Reset();
    nd->nn_dist = FLT_MAX;
  }
  for(i=0; i<children.size; i++) {
    ClustNode* nd = GetChild(i);
    float min_d = FLT_MAX;
    int j;
    for(j=i+1; j<children.size; j++) {
      ClustNode* ond = GetChild(j);
      ond->tmp_dist = nd->Dist(*ond, metric, norm, tol);
      min_d = MIN(ond->tmp_dist, min_d);
    }
    for(j=i+1; j<children.size; j++) {
      ClustNode* ond = GetChild(j);
      if(fabs(ond->tmp_dist - min_d) < clust_dist_tol) {
	nd->LinkNN(ond, min_d);	// link together with distance
	ond->LinkNN(nd, min_d);
      }
    }
  }
  // now make a 2nd pass and get smallest distance for each node and its neighbors
  for(i=0; i<children.size; i++) {
    ClustNode* nd = GetChild(i);
    nd->nn_dist = FLT_MAX;
    int j;
    for(j=0; j<nd->nns.size; j++) {
      ClustLink* nlk = (ClustLink*)nd->nns[j];
      nd->nn_dist = MIN(nd->nn_dist, nlk->dist);
    }
  }
}

float ClustNode::Dist(const ClustNode& oth, float_RArray::DistMetric metric,
		      bool norm, float tol) const
{
  float rval = 0.0f;
  if(pat != NULL) {
    if(oth.pat != NULL) {
      if(leaf_dists != NULL) {
	rval = leaf_dists->FastTriMatEl(leaf_max, leaf_idx, oth.leaf_idx);
      }
      else {
	rval = pat->Dist(*(oth.pat), metric, norm, tol);
      }
    }
    else {
      rval = oth.Dist(*this, metric, norm, tol);
    }
  }
  else {
    int i;
    for(i=0; i<children.size; i++) {
      ClustNode* nd = GetChild(i);
      if(nd->pat != NULL) {
	if(oth.pat != NULL) {
	  if(leaf_dists != NULL) {
	    rval += leaf_dists->FastTriMatEl(leaf_max, nd->leaf_idx, oth.leaf_idx);
	  }
	  else {
	    rval += nd->pat->Dist(*(oth.pat), metric, norm, tol);
	  }
	}
	else {
	  rval += oth.Dist(*nd, metric, norm, tol);
	}
      }
      else {
	rval += nd->Dist(oth, metric, norm, tol);
      }
    }
    if(children.size > 1)
      rval /= (float)children.size;
  }
  return rval;
}

void ClustNode::SetYs(float y_init) {
  static float global_y;
  if(y_init != -1.0f)
    global_y = y_init;
  if(pat == NULL) {
    float y_avg = 0.0f;
    int i;
    for(i=0; i<children.size; i++) {
      ClustNode* nd = GetChild(i);
      nd->SetYs();
      y_avg += nd->y;
    }
    if(children.size > 1)
      y_avg /= (float)children.size; // average of all y's of kids
    y = y_avg;
  }
  else {
    y = global_y;
    global_y += 1.0f;
  }
}

float ClustNode::SetParDists(float par_d, float_RArray::DistMetric metric) {
  par_dist = par_d + nn_dist;
  float max_d = -FLT_MAX;
  int i;
  for(i=0; i<children.size; i++) {
    ClustNode* nd = GetChild(i);
    float dst = nd->SetParDists(par_dist, metric);
    max_d = MAX(dst, max_d);
  }
  return max_d;
}

//////////////////////////
// 	DA View Specs	//
//////////////////////////

void DA_ViewSpec::Initialize(){
  visible = true;
  data_array = NULL;
}

void DA_ViewSpec::CutLinks() {
  taBase::DelPointer((TAPtr*)&data_array);
  taNBase::CutLinks();
}

void DA_ViewSpec::Copy_(const DA_ViewSpec& cp) {
  display_name = cp.display_name;
  visible = cp.visible;
}

void DA_ViewSpec::UpdateAfterEdit() {
  taNBase::UpdateAfterEdit();
  if(data_array == NULL) {
    DT_ViewSpec* dtv = GET_MY_OWNER(DT_ViewSpec);
    if(dtv != NULL) {
      int idx = dtv->Find(this);
      if(idx >= 0) {
	DataTable* dt = dtv->data_table;
	if(dt != NULL) {
	  DataArray_impl* da = dt->SafeEl(idx);
	  if(da != NULL)
	    taBase::SetPointer((TAPtr*)&data_array, da);
	}
	else
	  taMisc::Error("*** Null data_table in:", dtv->GetPath());
      }
    }
  }
  if(!(name.empty()) && (display_name.empty()))
    display_name = DA_ViewSpec::CleanName(name);
}

void DA_ViewSpec::UpdateView() {
  DT_ViewSpec* dtv = GET_MY_OWNER(DT_ViewSpec);
  if(dtv != NULL) dtv->UpdateAfterEdit();
}

void DA_ViewSpec::SetGpVisibility(bool vis) {
  DT_ViewSpec* dtv = GET_MY_OWNER(DT_ViewSpec);
  if(dtv != NULL) dtv->SetVisibility(vis);
}

void DA_ViewSpec::CopyToGp(MemberDef* md) {
  if(md == NULL) return;
  DT_ViewSpec* dtv = GET_MY_OWNER(DT_ViewSpec);
  if(dtv == NULL) return;
  int i;
  for(i=0;i<dtv->size;i++) {
    DA_ViewSpec* vs = (DA_ViewSpec*)dtv->FastEl(i);
    if(!vs->InheritsFrom(GetTypeDef())) continue;
    md->CopyFromSameType((void*)vs, (void*)this);
  }
  UpdateView();
}

bool DA_ViewSpec::BuildFromDataArray(DataArray_impl* nda) {
  float first_time = false;
  if(data_array == NULL) first_time = true;
  if(nda != NULL) {
    taBase::SetPointer((TAPtr*)&data_array, nda);
  }
  if(data_array == NULL)
    UpdateAfterEdit();
  if(data_array == NULL)	// still..
    return false;

  // always replace name and throwout old data
  // don't throw out the old data if we are just now creating this
  // view since this results in the user not being able to create
  // an additional view of data in a current view. Since the names
  // will never originally match up, the array's were always being reset.

  if(name != data_array->name) {
    if((first_time == false) && (data_array->AR() != NULL))
      data_array->AR()->Reset();
    name = data_array->name;
    display_name = DA_ViewSpec::CleanName(name);
  }
  if(data_array->HasDispOption(" HIDDEN,"))
    visible = false;
  return true;
}


String DA_ViewSpec::CleanName(String& name){
  String result = name;
  // <xxx> is a comment and should be removed
  if(result.empty()) return result;
  int pos;
  if(((pos = result.index('<')) != -1) && ((pos = result.index('>',pos+1)) != -1))
    result = result.after(pos);
  if(result.empty()) return result;
  if((result[0] == '|') || (result[0] == '$'))
    result = result.after(0);
  return result;
}

//////////////////////////
// 	DT View Specs	//
//////////////////////////

void DT_ViewSpec::Initialize(){
  data_table = NULL;
  visible = true;
}

void DT_ViewSpec::Destroy(){
  CutLinks();
}

void DT_ViewSpec::InitLinks() {
  taGroup<DA_ViewSpec>::InitLinks();
}  

void DT_ViewSpec::CutLinks() {
  taBase::DelPointer((TAPtr*)&data_table);
  taGroup<DA_ViewSpec>::CutLinks();
}

void DT_ViewSpec::Copy_(const DT_ViewSpec& cp) {
  display_name = cp.display_name;
  visible = cp.visible;
}

void DT_ViewSpec::UpdateAfterEdit(){
  taGroup<DA_ViewSpec>::UpdateAfterEdit();
  BuildFromDataTable(data_table);
  if(!(name.empty()) && (display_name.empty()))
    display_name = DA_ViewSpec::CleanName(name);
  TAPtr own = GetOwner();
  while((own != NULL) && (own->InheritsFrom(&TA_taList_impl))){
    own = own->GetOwner();
  }
  if(own != NULL){
    own->UpdateAfterEdit();
  }
}

void DT_ViewSpec::SetVisibility(bool vis) {
  visible = vis;
  int i;
  for(i=0;i<size;i++) {
    DA_ViewSpec* vs = (DA_ViewSpec*)FastEl(i);
    vs->visible = visible;	// always propagate
  }
  UpdateAfterEdit();
}

bool DT_ViewSpec::BuildFromDataTable(DataTable* tdt){
  if(tdt != NULL) {
    taBase::SetPointer((TAPtr*)&data_table, tdt);
  }
  if(data_table == NULL)
    return false;

  bool same_size = ((size == data_table->size) && (leaves == data_table->leaves)
    && (gp.size == data_table->gp.size));

  bool same_name = (name == data_table->name);

  if(same_size && same_name) {
    return false;
  }

  ReBuildFromDataTable();
  return true;
}

void DT_ViewSpec::ReBuildFromDataTable() {
  // first save the current view information, then try to update based on old info
  DT_ViewSpec* old = (DT_ViewSpec*)Clone();

  int delta = (int)fabs(float(data_table->size - size)); // change in size
  delta = MAX(delta, 4);	// minimum look-ahead
  int gp_delta = (int)fabs(float(data_table->gp.size - gp.size)); // change in size
  gp_delta = MAX(gp_delta, 4);

  // ensure number of elements is the same;
  EnforceSize(data_table->size);

  name = data_table->name;	// always get the name
  if(!name.empty() && display_name.empty())
    display_name = DA_ViewSpec::CleanName(name);

  int old_i = 0;
  int i;
  for(i=0;i<size;i++) {
    DataArray_impl* nda =     ((DataArray_impl *) data_table->FastEl(i));
    DA_ViewSpec*    ndavs = (DA_ViewSpec *) FastEl(i);
    ndavs->BuildFromDataArray(nda);
    
    if(old_i < old->size) {
      DA_ViewSpec* oldvs = (DA_ViewSpec *)old->FastEl(old_i);
      if(oldvs->name == nda->name) { // simple match, done
	old_i++;
	ndavs->CopyFrom(oldvs); // get the info from the old guy
	continue;
      }
      // look ahead for other options
      int j;
      for(j=old_i;(j<=old_i+delta) && (j < old->size);j++) {
	oldvs = (DA_ViewSpec *)old->FastEl(j);
	if(oldvs->name == ndavs->name) { // simple match, done
	  old_i = j;
	  ndavs->CopyFrom(oldvs); // get the info from the old guy
	  continue;
	}
      }
    }
  }

  // ensure the groups matchup
  gp.el_base = gp.el_typ = GetTypeDef();
  gp.EnforceSize(data_table->gp.size);

  old_i = 0;
  for(i=0;i<data_table->gp.size;i++){
    DT_ViewSpec* nvs = (DT_ViewSpec *) FastGp(i);
    DataTable* ndt = (DataTable *) data_table->FastGp(i);
    nvs->el_base = nvs->el_typ = el_typ;
    if(!nvs->BuildFromDataTable(ndt))
      nvs->ReBuildFromDataTable(); // make sure it is rebuilt!

    if(old_i < old->gp.size) {
      DT_ViewSpec* oldvs = (DT_ViewSpec *)old->FastGp(old_i);
      if(oldvs->name == nvs->name) { // simple match, done
	old_i++;
	nvs->CopyFrom(oldvs); // get the info from the old guy
	nvs->EnforceSize(ndt->size); // make sure we don't get bigger from the copy!!
	continue;
      }
      int j;
      for(j=old_i;(j<=old_i+gp_delta) && (j < old->gp.size);j++) {
	oldvs = (DT_ViewSpec *)old->FastGp(j);
	if(oldvs->name == nvs->name) { // simple match, done
	  old_i = j;
	  nvs->CopyFrom(oldvs); // get the info from the old guy
	  nvs->EnforceSize(ndt->size); // make sure we don't get bigger from the copy!!!
	  continue;
	}
      }
    }
  }

  delete old;
}

void DT_ViewSpec::SetDispNms(const char* base_name) {
  display_name = base_name;
  int i;
  for(i=0;i<size;i++) {
    DA_ViewSpec* vs = (DA_ViewSpec*)FastEl(i);
    vs->display_name = base_name;
    vs->display_name += String("_") + String(i);
  }
}

void DT_ViewSpec::RmvNmPrefix() {
  int i;
  for(i=0;i<size;i++) {
    DA_ViewSpec* vs = (DA_ViewSpec*)FastEl(i);
    if(vs->display_name.contains('_')) vs->display_name = vs->display_name.after('_');
  }
}

//////////////////////////////////
// 	DA Text View Specs	//
//////////////////////////////////

void DA_TextViewSpec::Initialize() {
  width = 2;
}

void DA_TextViewSpec::Destroy() {

}

bool DA_TextViewSpec::BuildFromDataArray(DataArray_impl* tda) {
  bool result = DA_ViewSpec::BuildFromDataArray(tda);
  if(data_array == NULL)	return result;
  if(data_array->HasDispOption(" NARROW,"))
    width = 1;
  String wd = data_array->DispOptionAfter(" WIDTH=");
  if(!wd.empty())
    width = (int)wd;
  return result;
}

//////////////////////////////////
// 	DA Net View Specs	//
//////////////////////////////////

void DA_NetViewSpec::Initialize() {
  label_index = -1;
}

void DA_NetViewSpec::Destroy() {
}


//////////////////////////////////
// 	DA Graph View Specs	//
//////////////////////////////////

void DA_GraphViewSpec::Initialize() {
  line_type = LINE;
  line_style = SOLID;
  line_width = 0.0f;
  point_style = NONE;
  axis_spec = NULL;
  taBase::SetPointer((TAPtr*)&axis_spec,this);
  negative_draw = false;
  vertical = FULL_VERTICAL;
  shared_y = OVERLAY_LINES;
  trace_incr.x = 0.0;
  trace_incr.y = 0.0;
  thresh = .5f;
  n_ticks = 10;
  string_coords = NULL;
}

void DA_GraphViewSpec::Destroy() {
  CutLinks();
}

void DA_GraphViewSpec::CutLinks(){
  taBase::DelPointer((TAPtr*)&axis_spec);
  taBase::DelPointer((TAPtr*)&string_coords);
}

void DA_GraphViewSpec::InitLinks(){
  DA_ViewSpec::InitLinks();
  taBase::Own(line_color, this);
  taBase::Own(point_mod, this);
  taBase::Own(trace_incr, this);
  taBase::Own(range, this);
  // if it is in a subgroup then it will have 3 layers of DT_ViewSpec
  // above it. 3 becuase the first DT_ViewSpec is actual in a gp.
  // if it is in a subgroup then defaul to haveing the first elements graph
  DT_ViewSpec* myowner = GET_MY_OWNER(DT_ViewSpec);
  if(myowner != NULL) myowner = GET_OWNER(myowner,DT_ViewSpec);
  if(myowner != NULL) myowner = GET_OWNER(myowner,DT_ViewSpec);
  if(myowner != NULL) {
    myowner = GET_MY_OWNER(DT_ViewSpec);
    SetAxis((DA_GraphViewSpec*)myowner->FastEl(0));
  }
}

void DA_GraphViewSpec::UpdateAfterEdit() {
  if(taMisc::iv_active) {
    ta_color.SetColor(line_color.r,line_color.g,line_color.b,line_color.a);
  }
  if(axis_spec == NULL)		// don't like null axis specs..
    SetAxis(this);
  if(axis_spec != this) {
    if(axis_spec->axis_spec != axis_spec) {
      taMisc::Error("DA_GraphViewsSpec: can't have axis_spec with its own axis_spec != itself -- this is now fixed for axis spec:", axis_spec->name);
      axis_spec->SetAxis(axis_spec);
    }
  }
  if((data_array != NULL) && data_array->InheritsFrom(TA_String_Data)) {
    line_type = STRINGS;
    if(string_coords == NULL) FindStringCoords();
    if((string_coords != NULL) && (axis_spec == this))
      SetAxis(string_coords);
  }
  if((vertical == NO_VERTICAL) && !((line_type == VALUE_COLORS) || (line_type == THRESH_POINTS))) {
    taMisc::Error("*** GraphViewSpec:: vertical = NO_VERTICAL only sensible with line_type = COLORS or THRESH_POINTS, resetting to FULL_VERTICAL");
    vertical = FULL_VERTICAL;
  }
  if((vertical == STACK_TRACES) && ((trace_incr.x != 0.0f) || (trace_incr.y != 0.0f))) {
    taMisc::Error("*** GraphViewSpec:: vertical = STACK_TRACES cannot be combined with trace_incr != 0, resetting trace_incr = 0");
    trace_incr.x = 0.0f; trace_incr.y = 0.0f;
  }
  DA_ViewSpec::UpdateAfterEdit();
}

void DA_GraphViewSpec::FindStringCoords() {
  DT_ViewSpec* myowner = GET_MY_OWNER(DT_ViewSpec);
  int idx = myowner->FindEl(this);
  DA_GraphViewSpec* prv_float = NULL;
  while((prv_float == NULL) && (idx > 0)) {
    prv_float = (DA_GraphViewSpec*)myowner->FastEl(--idx);
    if(!prv_float->data_array->InheritsFrom(TA_float_Data))
      prv_float = NULL;
  }
  if(prv_float == NULL) {
    taMisc::Error("GraphViewSpec: Could not find number data for plotting string data:",
		  name);
    return;
  }
  SetStringCoords(prv_float);
}

void DA_GraphViewSpec::Copy_(const DA_GraphViewSpec& cp){
  line_color = cp.line_color;
  line_type = cp.line_type;
  line_style = cp.line_style;
  point_style = cp.point_style;
  point_mod = cp.point_mod;
  negative_draw = cp.negative_draw;
  vertical = cp.vertical;
  shared_y = cp.shared_y;
  trace_incr = cp.trace_incr;
  thresh = cp.thresh;
  if((cp.axis_spec != NULL) && (owner != NULL)) {
    if(cp.axis_spec->owner == owner) {
      SetAxis(cp.axis_spec);
    }
    else {
      DA_GraphViewSpec* ax = (DA_GraphViewSpec*)((DT_GraphViewSpec*)owner)->FindName(cp.axis_spec->name);
      if(ax != NULL) {
	SetAxis(ax);
      }
    }
  }
  if((cp.string_coords != NULL) && (owner != NULL)) {
    if(cp.string_coords->owner == owner) {
      SetStringCoords(cp.string_coords);
    }
    else {
      DA_GraphViewSpec* ax = (DA_GraphViewSpec*)((DT_GraphViewSpec*)owner)->FindName(cp.string_coords->name);
      if(ax != NULL) {
	SetStringCoords(ax);
      }
    }
  }
  if(axis_spec == NULL) {
    SetAxis(this); // use this instead!
  }
  if((data_array != NULL) && data_array->InheritsFrom(TA_String_Data)) {
    line_type = STRINGS;
    if(string_coords == NULL) FindStringCoords();
  }
  range = cp.range;
  n_ticks = cp.n_ticks;
}

bool DA_GraphViewSpec::BuildFromDataArray(DataArray_impl* tda) {
  bool result = DA_ViewSpec::BuildFromDataArray(tda);
  if(data_array == NULL)	return result;
  if(data_array->InheritsFrom(TA_String_Data) && !data_array->HasDispOption(" DISP_STRING,"))
    visible = false;
  if(data_array->HasDispOption(" NARROW,"))
    visible = false;
  String val = data_array->DispOptionAfter(" MIN=");
  if(!val.empty()) {
    range.min = (float)val;
    range.fix_min = true;
  }
  val = data_array->DispOptionAfter(" MAX=");
  if(!val.empty()) {
    range.max = (float)val;
    range.fix_max = true;
  }
  String axis = data_array->DispOptionAfter(" AXIS=");
  if(!axis.empty()) {
    int axidx = (int)axis;
    DT_GraphViewSpec* owndt = (DT_GraphViewSpec*)owner;
    if((owndt != NULL) && (owndt->size > axidx))
      SetAxis((DA_GraphViewSpec*)owndt->FastEl(axidx));
  }
  if(data_array->HasDispOption(" NEGATIVE_DRAW,"))
    negative_draw = true;
  return result;
}

void DA_GraphViewSpec::PlotRows() {
  DT_GraphViewSpec* myowner = GET_MY_OWNER(DT_GraphViewSpec);
  if(myowner == NULL) return;
  myowner->PlotRows(); 
}

void DA_GraphViewSpec::PlotCols() {
  DT_GraphViewSpec* myowner = GET_MY_OWNER(DT_GraphViewSpec);
  if(myowner == NULL) return;
  myowner->PlotCols(); 
}

void DA_GraphViewSpec::GpShareAxis() {
  DT_GraphViewSpec* myowner = GET_MY_OWNER(DT_GraphViewSpec);
  if(myowner == NULL) return;
  myowner->ShareAxes();
}

void DA_GraphViewSpec::GpSepAxes() {
  DT_GraphViewSpec* myowner = GET_MY_OWNER(DT_GraphViewSpec);
  if(myowner == NULL) return;
  myowner->SeparateAxes();
}

//////////////////////////////////
// 	DT Graph View Specs	//
//////////////////////////////////

void DT_GraphViewSpec::Initialize() {
  background.name = "grey54";
  background.r = .541176f;
  background.g = .541176f;
  background.b = .541176f;
  color_type = C_RAINBOW;
  sequence_1 = COLORS;
  sequence_2 = POINTS;
  sequence_3 = LINES;
  plot_rows = false;
}

void DT_GraphViewSpec::Destroy(){
}

void DT_GraphViewSpec::InitLinks() {
  DT_ViewSpec::InitLinks();
  taBase::Own(background, this);

  if(taMisc::iv_active) {
    bg_color.SetColor(background.r,background.g,background.b,background.a);
    ivWidgetKit* wkit = ivWidgetKit::instance();
    String guiname = wkit->gui();
    if(guiname == "monochrome") {
      color_type = M_MONO;
      sequence_1 = POINTS;
      sequence_2 = LINES;
      sequence_3 = COLORS;
    }
    else {
      color_type = C_RAINBOW;
      sequence_1 = COLORS;
      sequence_2 = LINES;
      sequence_3 = POINTS;
    }
  }	
}

void DT_GraphViewSpec::Copy_(const DT_GraphViewSpec& cp) {
  background = cp.background;
  color_type = cp.color_type;
  sequence_1 = cp.sequence_1;
  sequence_2 = cp.sequence_2;
  sequence_3 = cp.sequence_3;
}

bool DT_GraphViewSpec::BuildFromDataTable(DataTable* tdt){
  bool rval = DT_ViewSpec::BuildFromDataTable(tdt);
  if(rval == true)
    UpdateLineFeatures();
  return rval;
}

void DT_GraphViewSpec::UpdateAfterEdit() {
  if(taMisc::iv_active) {
    bg_color.SetColor(background.r,background.g,background.b,background.a);
  }
  DT_ViewSpec::UpdateAfterEdit();
}

static char* c_rainbow_colors[] = {
  "red", "orange", "yellow", "green", "blue", "navy", "violet",
  "brown", "black", "white", "pink", "pale green", "light blue" };
static int c_rainbow_count = 13;

static char* c_greyscale_colors[] = {
  "grey0", "grey10", "grey20", "grey30", "grey40", "grey60", "grey70", "grey80",
  "grey90", "grey100"};
static int c_greyscale_count = 10;

static char* m_mono_colors[] = { "black", "white" };
static int m_mono_count = 2;

static char* p_rainbow_colors[] = {
  "red", "orange", "yellow", "green", "blue", "navy", "violet",
  "brown", "black", "grey50", "pink", "pale green", "light blue" };
static int p_rainbow_count = 13;

static char* p_greyscale_colors[] = {
  "grey0", "grey10", "grey20", "grey30", "grey40", "grey50", "grey60", "grey70",
  "grey80", "grey90"};
static int p_greyscale_count = 10;

static char* p_mono_colors[] = { "black" };
static int p_mono_count = 1;


char* DT_GraphViewSpec::ColorName(int color_no) {
  switch(color_type) {
  case C_RAINBOW:
    return c_rainbow_colors[color_no % c_rainbow_count];
  case C_GREYSCALE:
    return c_greyscale_colors[color_no % c_greyscale_count];
  case M_MONO:
    return m_mono_colors[color_no % m_mono_count];
  case P_RAINBOW:
    return p_rainbow_colors[color_no % p_rainbow_count];
  case P_GREYSCALE:
    return p_greyscale_colors[color_no % p_greyscale_count];
  case P_MONO:
    return p_mono_colors[color_no % p_mono_count];
  case CUSTOM:
    return "";
  }
  return "";
}

int DT_GraphViewSpec::ColorCount() {
  switch(color_type) {
  case C_RAINBOW:
    return c_rainbow_count;
  case C_GREYSCALE:
    return c_greyscale_count;
  case M_MONO:
    return m_mono_count;
  case P_RAINBOW:
    return p_rainbow_count;
  case P_GREYSCALE:
    return p_greyscale_count;
  case P_MONO:
    return p_mono_count;
  case CUSTOM:
    return 0;
  }
  return 0;
}

void DT_GraphViewSpec::SetBgColor() {
  switch(color_type) {
  case C_RAINBOW:
  case C_GREYSCALE:
    background.name = "grey54";
    background.UpdateAfterEdit();
    break;
  case M_MONO:
    background.name = "white";
    background.UpdateAfterEdit();
    background.a = 1;
    break;
  case P_RAINBOW:
  case P_GREYSCALE:
  case P_MONO:
    background.name = "white";
    background.UpdateAfterEdit();
    break;
  case CUSTOM:
    break;
  }
  if(taMisc::iv_active)
    bg_color.SetColor(background.r,background.g,background.b,background.a);
}

void DT_GraphViewSpec::ApplyOneFeature(DA_GraphViewSpec* dagv, SequenceType seq,
				       int val, bool& set_ln, bool& set_pt) {
  if(seq == COLORS) {
    dagv->line_color.name = ColorName(val);
    dagv->line_color.UpdateAfterEdit();
  }
  else if(seq == LINES) {
    dagv->line_style = (DA_GraphViewSpec::LineStyle)val;
    set_ln = true;
  }
  else if(seq == POINTS) {
    dagv->point_style = (DA_GraphViewSpec::PointStyle)val;
    set_pt = true;
  }
}


void DT_GraphViewSpec::UpdateLineFeatures(bool visible_only) {
  int counts[4];
  counts[COLORS] = ColorCount();
  counts[LINES] = 4;
  counts[POINTS] = 4;
  counts[NONE] = 100000;	// should cover it..
  
  SetBgColor();

  int i=0;
  int cnt = 0;
  while(true) {
    int j = 0;
    while(true) {
      int k = 0;
      while(true) {
	DA_GraphViewSpec* dagv = (DA_GraphViewSpec*)Leaf(cnt++);
	if(dagv == NULL)
	  return;

	if((visible_only == true) && !dagv->visible) { // turn off
	  dagv->line_color.name = "black";
	  dagv->line_color.UpdateAfterEdit();
	  dagv->line_style = (DA_GraphViewSpec::LineStyle)0;
	  dagv->point_style = (DA_GraphViewSpec::PointStyle)0;
	  dagv->line_type = DA_GraphViewSpec::LINE;
	  dagv->UpdateAfterEdit();
	  continue;
	}

	// initialize to default values first
	dagv->line_style = (DA_GraphViewSpec::LineStyle)0; 
	dagv->point_style = (DA_GraphViewSpec::PointStyle)0;
	bool set_ln = false;
	bool set_pt = false;
	ApplyOneFeature(dagv, sequence_1, k, set_ln, set_pt);
	ApplyOneFeature(dagv, sequence_2, j, set_ln, set_pt);
	ApplyOneFeature(dagv, sequence_3, i, set_ln, set_pt);

	if(set_ln && set_pt)
	  dagv->line_type = DA_GraphViewSpec::LINE_AND_POINTS;
	else if(set_pt) {
	  dagv->line_type = DA_GraphViewSpec::POINTS;
	  // add one to the point style since 0 is no point...
	  dagv->point_style = (DA_GraphViewSpec::PointStyle)
	    ((int)dagv->point_style + 1);
	}
	else			// default is line
	  dagv->line_type = DA_GraphViewSpec::LINE;

	dagv->UpdateAfterEdit();

	k++;
	if(k >= counts[sequence_1])			break;
      }
      j++;
      if(j >= counts[sequence_2])			break;
    }
    i++;
    if(i >= counts[sequence_3])				break;
  }
  
  UpdateAfterEdit();
}

void DT_GraphViewSpec::SetLineWidths(float line_width) {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->line_width = line_width;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->SetLineWidths(line_width);
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::SetLineType(DA_GraphViewSpec::LineType line_type) {
  if(line_type == DA_GraphViewSpec::STRINGS) return;
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    if(vs->line_type != DA_GraphViewSpec::STRINGS)
      vs->line_type = line_type;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->SetLineType(line_type);
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::ShareAxes() {
  if(size < 2) return;
  if(gp.size == 0) {		// only for terminal groups!
    DA_GraphViewSpec* axis = (DA_GraphViewSpec*)FastEl(0);
    for(int i=0;i<size;i++) {
      DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
      vs->SetAxis(axis);
    }
  }
  for(int i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->ShareAxes();
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::SeparateAxes() {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->SetAxis(vs);
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->SeparateAxes();
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::PlotRows() {
  if(gp.size == 0) {		// only act on subgroups w/out groups of their own
    plot_rows = true;
  }
  else {
    for(int i=0;i<gp.size;i++) {
      DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
      nvs->PlotRows();
    }
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::PlotCols() {
  if(gp.size == 0) {		// only act on subgroups w/out groups of their own
    plot_rows = false;
  }
  else {
    for(int i=0;i<gp.size;i++) {
      DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
      nvs->PlotCols();
    }
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::TraceIncrement(float x_inc, float y_inc) {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->trace_incr.x = x_inc; vs->trace_incr.y = y_inc;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->TraceIncrement(x_inc, y_inc);
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::StackTraces() {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->vertical = DA_GraphViewSpec::STACK_TRACES;
    vs->trace_incr.x = 0.0f; vs->trace_incr.y = 0.0f;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->StackTraces();
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::UnStackTraces() {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->vertical = DA_GraphViewSpec::FULL_VERTICAL;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->UnStackTraces();
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::StackSharedAxes() {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->shared_y = DA_GraphViewSpec::STACK_LINES;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->StackSharedAxes();
  }
  UpdateAfterEdit();
}

void DT_GraphViewSpec::UnStackSharedAxes() {
  int i;
  for(i=0;i<size;i++) {
    DA_GraphViewSpec* vs = (DA_GraphViewSpec*)FastEl(i);
    vs->shared_y = DA_GraphViewSpec::OVERLAY_LINES;
  }
  for(i=0;i<gp.size;i++) {
    DT_GraphViewSpec* nvs = (DT_GraphViewSpec*)FastGp(i);
    nvs->UnStackSharedAxes();
  }
  UpdateAfterEdit();
}




//////////////////////////////////
// 	DA Grid View Specs	//
//////////////////////////////////

void DA_GridViewSpec::Initialize(){
  display_style = BLOCK;
  scale_on = true;
}

void DA_GridViewSpec::Destroy() {

}

bool DA_GridViewSpec::BuildFromDataArray(DataArray_impl* nda){
  bool result = DA_ViewSpec::BuildFromDataArray(nda);
  if(data_array == NULL)
    return result;
  if(data_array->HasDispOption(" TEXT,"))
    display_style = TEXT;		
  else if(data_array->HasDispOption(" NARROW,"))
    display_style = TEXT;		// use text for narrow items as a default..
  else if(data_array->InheritsFrom(TA_String_Data))
    display_style = TEXT;
  return result;
}

void DA_GridViewSpec::InitLinks(){
  DA_ViewSpec::InitLinks();
  taBase::Own(pos, this);
}

void DA_GridViewSpec::Copy_(const DA_GridViewSpec& cp) {
  pos = cp.pos;
  display_style = cp.display_style;
}


//////////////////////////////////
// 	DT Grid View Specs	//
//////////////////////////////////

void DT_GridViewSpec::Initialize(){
  use_gp_name = true;
  display_style = DA_GridViewSpec::BLOCK;
  layout = LFT_RGT_BOT_TOP;
  scale_on = true;
  customized = false;
}

void DT_GridViewSpec::Destroy() {
}

void DT_GridViewSpec::Reset() {
  DT_ViewSpec::Reset();
  customized = false;
}

void DT_GridViewSpec::InitLinks(){
  DT_ViewSpec::InitLinks();
  taBase::Own(pos, this);
  taBase::Own(geom, this);
  taBase::Own(full_geom, this);
}

void DT_GridViewSpec::Copy_(const DT_GridViewSpec& cp) {
  pos = cp.pos;
  geom = cp.geom;
  full_geom = cp.full_geom;
  use_gp_name = cp.use_gp_name;
  display_style = cp.display_style;
}

bool DT_GridViewSpec::BuildFromDataTable(DataTable* tdt){
  bool result = DT_ViewSpec::BuildFromDataTable(tdt);
  if(!result) return result;
  if(size <= 0) return result;
  DA_GridViewSpec* vs = (DA_GridViewSpec *) FastEl(0); // first el determines group options!
  if(vs->data_array->disp_opts.contains(" GEOM_X=")) {
    String geo = vs->data_array->disp_opts;
    geo = geo.after(" GEOM_X=");
    String gx = geo.before(',');
    geo = geo.after(" GEOM_Y=");
    String gy = geo.before(',');
    geom.x = (int)gx;
    geom.y = (int)gy;
  }
  if(vs->data_array->HasDispOption(" USE_EL_NAMES,")) {
    use_gp_name = false;
  }
  if(vs->data_array->HasDispOption(" USE_GP_NAME,")) {
    use_gp_name = true;
  }

  if(customized)
    UpdateGeom();
  else
    UpdateLayout();
  return result;
}

void DT_GridViewSpec::UpdateGeom() {
  PosTDCoord gm;
  int i;
  for(i=0;i<size;i++) {
    DA_GridViewSpec* vs = (DA_GridViewSpec *) FastEl(i);
    if(!vs->visible) continue;
    gm.x = MAX(gm.x, (vs->pos.x - pos.x) + 1);	// subtract our own position!
    gm.y = MAX(gm.y, (vs->pos.y - pos.y) + 1);
  }
  full_geom.x = pos.x + gm.x;
  full_geom.y = pos.y + gm.y;
  for(i=0;i<gp.size;i++) {
    DT_GridViewSpec* dt = (DT_GridViewSpec *) FastGp(i);
    if(!dt->visible) continue;
    dt->UpdateGeom();
    full_geom.x = MAX(full_geom.x, dt->full_geom.x);
    full_geom.y = MAX(full_geom.y, dt->full_geom.y);
  }
}

void DT_GridViewSpec::UpdateAfterEdit(){
  DT_ViewSpec::UpdateAfterEdit();
  UpdateGeom();
}

int DT_GridViewSpec::UpdateLayout(MatrixLayout ml) {
  customized = false;
  if(ml == DEFAULT)
    ml = layout;
  else
    layout = ml;		// set current default to be this

  DA_GridViewSpec* da;
  int vis_size = 0;
  int i;
  for(i=0;i<size;i++){
    da = (DA_GridViewSpec *) FastEl(i);
    if(!da->visible) continue;
    vis_size++;
    if(da->display_style == DA_GridViewSpec::TEXT) {
      if(da->data_array->HasDispOption(" NARROW,"))
	vis_size+=1;		// narrow text = 2
      else
	vis_size+=3;		// wide text = 4
    }
  }
  // insufficient geometry so use sqrt
  if((geom.x * geom.y) < vis_size) {
    if(vis_size < 25) {		// make it linear if a reasonable width..
      geom.x = vis_size; 
      geom.y = 1;
    }
    else {
      geom.y = (int)sqrt(float(vis_size));
      geom.x = vis_size / geom.y;
      while((geom.x * geom.y) < vis_size)
	geom.x++;
    }
  }
  int loc[2];   int start[2];  int mod[2];
  int limiter = 0;
  int limit[2];  limit[0]= geom.x; limit[1] = geom.y;
  int maxx = 0;
  int extrax = 0;
  switch(ml){
  case LFT_RGT_TOP_BOT:
    loc[0] = start[0] = 0;   loc[1] = start[1] = geom.y-1;
    mod[0] = 1; mod[1] = -1; limiter = 0;
    break;
  case LFT_RGT_BOT_TOP:
    loc[0] = start[0] = 0;   loc[1] = start[1] = 0;
    mod[0] = 1; mod[1] = 1;  limiter = 0;
    break;
  case BOT_TOP_LFT_RGT:
    loc[0] = start[0] = 0;   loc[1] = start[1] = 0;
    mod[0] = 1; mod[1] = 1;  limiter = 1;
    break;
  case TOP_BOT_LFT_RGT:
    loc[0] = start[0] = 0;   loc[1] = start[1] = geom.y-1;
    mod[0] = 1; mod[1] = -1; limiter = 1;  limit[1] = -1;
    break;
  case DEFAULT:			// just to shut up the compiler
    break;
  }
  for(i=0;i<size;i++){
    da = (DA_GridViewSpec *) FastEl(i);
    if(!da->visible) continue;
    da->pos.x = pos.x + loc[0];
    da->pos.y = pos.y + loc[1];
    if(da->display_style == DA_GridViewSpec::TEXT) {
      int incv = 3;
      if(da->data_array->HasDispOption(" NARROW,"))
	incv = 1;
      if(limiter == 0) 
	loc[0] += incv;		
      else 
	extrax = MAX(incv, extrax);
    }
    maxx = MAX(maxx,loc[0]);
    loc[limiter] += mod[limiter];
    if(loc[limiter] == limit[limiter]){
      loc[limiter] = start[limiter];
      loc[!limiter] += mod[!limiter];
      if(limiter == 1)
	loc[0] += mod[0] * extrax;
      extrax = 0;
    }
  }
  loc[0] = maxx + 2;
  // now layout the subgroups to the right
  for(i=0;i<gp.size;i++){
    DT_GridViewSpec* dt = (DT_GridViewSpec*) gp.FastEl(i);
    if(!dt->visible) continue;
    dt->pos.x = pos.x + loc[0];
    dt->pos.y = pos.y;		// same position as us
    loc[0] += dt->UpdateLayout() + 2;	// always use default! and add a space between subsequent subgroups
    maxx = MAX(maxx,loc[0]);
  }
  UpdateGeom();
  return maxx;
}

void DT_GridViewSpec::GetMinMaxScale(MinMax& mm, bool first) {
  static bool frst;
  if(first)
    frst = true;
  int i;
  for(i=0;i<size;i++){
    DA_GridViewSpec* vs = (DA_GridViewSpec *) FastEl(i);
    if(!vs->visible || !vs->scale_on || (vs->display_style == DA_GridViewSpec::TEXT)) continue;
    if(vs->data_array->AR() == NULL) {
      vs->data_array->NewAR();
      continue;
    }
    if(!vs->data_array->InheritsFrom(&TA_float_Data)) continue;
    float_RArray* ar = (float_RArray*)vs->data_array->AR();
    if(frst) {
      frst = false;
      mm.max = ar->range.max;
      mm.min = ar->range.min;
    }
    else {
      mm.UpdateRange(ar->range);
    }
  }
  for(i=0;i<gp.size;i++){
    DT_GridViewSpec* dt = (DT_GridViewSpec*) gp.FastEl(i);
    if(!dt->visible || !dt->scale_on) continue;
    dt->GetMinMaxScale(mm, frst);
  }
}
