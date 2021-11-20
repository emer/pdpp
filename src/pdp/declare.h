/* -*- C++ -*- */
/*=============================================================================
//									      //
// This file is part of the PDP++ software package.			      //
//									      //
// Copyright (C) 1995 Randall C. O'Reilly, Chadley K. Dawson, 		      //
//		      James L. McClelland, and Carnegie Mellon University     //
//     									      //
// Permission to use, copy, and modify this software and its documentation    //
// for any purpose other than distribution-for-profit is hereby granted	      //
// without fee, provided that the above copyright notice and this permission  //
// notice appear in all copies of the software and related documentation.     //
//									      //
// Permission to distribute the software or modified or extended versions     //
// thereof on a not-for-profit basis is explicitly granted, under the above   //
// conditions. 	HOWEVER, THE RIGHT TO DISTRIBUTE THE SOFTWARE OR MODIFIED OR  //
// EXTENDED VERSIONS THEREOF FOR PROFIT IS *NOT* GRANTED EXCEPT BY PRIOR      //
// ARRANGEMENT AND WRITTEN CONSENT OF THE COPYRIGHT HOLDERS.                  //
// 									      //
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

#ifndef declare_h
#define declare_h

// comments are not allowed here! they tend to get sucked into the description...
// this one is safe because it is only trailing comments that are offensive
// don't declare any group types because they all depend on templates
// which cannot be pre-declared (apparently)

// get rid of any overlapping iv defines
#ifdef Pattern
#undef Pattern
#endif
#ifdef Event
#undef Event
#endif

class PDPView;
class BaseSpec;
class BaseSpec_MGroup;

class Script;
class Process;

class SigmoidSpec;
class ConSpec;
class Connection;
class Con_Group;
class UnitSpec;
class Unit;
class ProjectionSpec;
class Projection;
class LayerSpec;
class Layer;
class Network;
class NetView;

class Project;
class PDPRoot;

class PatternSpec;
class EventSpec;
class Pattern;
class Event;
class Event_MGroup;
class Environment;
class ScriptEnv;
class TimeEnvironment;

class Stat;
class SE_Stat;
class MonitorStat;
class ClosestEventStat;
class CyclesToSettle;

class ScriptStat;
class CompareStat;
class EpochCounterStat;
class ProcCounterStat;

class SchedProcess;
class BatchProcess;
class TrainProcess;
class NEpochProcess;
class EpochProcess;
class SequenceProcess;
class TrialProcess;
class SettleProcess;
class CycleProcess;

class SequenceEpoch;

class PDPLog;
class TextLog;
class GraphLog;
class GridLog;
class LogView;
class TextLogView;
class GraphLogView;
class GridLogView;

#endif //declare_h


