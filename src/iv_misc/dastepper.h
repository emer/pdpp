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

#ifndef dastepper_h
#define dastepper_h


// the do-action stepper and other buttons

#include <ta/enter_iv.h>
#include <IV-look/stepper.h>
#include <InterViews/action.h>
#include <InterViews/telltale.h>
#include <ta/leave_iv.h>

class DAStepper : public ivStepper {
public:
  int button;
  virtual void adjust();
  virtual void press(const ivEvent& e);
  virtual void release(const ivEvent&);
  DAStepper(ivGlyph*,ivStyle*,ivTelltaleState*,ivAction*);
  virtual ~DAStepper() {};
};


class IntAction : public ivAction {
public:
  IntAction(){};
  virtual ~IntAction(){};
  virtual void execute();
  virtual void call_me(int i=0) = 0;
};

/*
 * IntegerAction denoted by an object and member function to call on the object.
 */

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __IntActionCallback(T) T##_IntActionCallback
#define IntActionCallback(T) __IntActionCallback(T)
#define __IntActionMemberFunction(T) T##_IntActionMemberFunction
#define IntActionMemberFunction(T) __IntActionMemberFunction(T)
#else
#define __IntActionCallback(T) T/**/_IntActionCallback
#define IntActionCallback(T) __IntActionCallback(T)
#define __IntActionMemberFunction(T) T/**/_IntActionMemberFunction
#define IntActionMemberFunction(T) __IntActionMemberFunction(T)
#endif

#define declareIntActionCallback(T) \
typedef void (T::*IntActionMemberFunction(T))(int); \
class IntActionCallback(T) : public IntAction { \
public: \
    IntActionCallback(T)(T*, IntActionMemberFunction(T)); \
\
    virtual void call_me(int i); \
private: \
    T* obj_; \
    IntActionMemberFunction(T) func_; \
};

#define implementIntActionCallback(T) \
IntActionCallback(T)::IntActionCallback(T)(T* obj, IntActionMemberFunction(T) func) { \
    obj_ = obj; \
    func_ = func; \
} \
\
void IntActionCallback(T)::call_me(int i) { (obj_->*func_)(i); }

  
class ParameterStepper : public DAStepper {
public:
  virtual void adjust();
  ParameterStepper(ivGlyph*,ivStyle*,ivTelltaleState*,IntAction*);
};


class NoUnSelectButton : public ivButton{
public:
  virtual void release(const ivEvent&);
  NoUnSelectButton(ivGlyph* g, ivStyle* s, ivTelltaleState* t, ivAction* at=nil)
   : ivButton(g,s,t,at)	{ };
  virtual void	chooseme ();
  ~NoUnSelectButton(){};
};


// this button stays inset when it it pressed
// it only changes its visual rep on button 1 or 2
// but it calls different actions depending on the button pressed
class InsetActiveButton: public ivButton {
public:
  enum ButtonState{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_FLIP,
    BUTTON_SAME 
  };

  ivAction* action_1;
  ivAction* action_2;
  ivAction* action_3;
  int down;
  ButtonState b1;
  ButtonState b2;
  ButtonState b3;

  void press(const ivEvent&);
  void release(const ivEvent&);
  void SetState(ButtonState);
  InsetActiveButton(ivGlyph* g, ivStyle* s, ivTelltaleState* t,
		    ivAction* at1=nil,ivAction* at2=nil,ivAction* at3=nil,
		    ButtonState s1=BUTTON_UP,
		    ButtonState s2=BUTTON_FLIP,
		    ButtonState s3=BUTTON_SAME);
  ~InsetActiveButton();
};

class Teller : public ivTelltaleState {
public:
    Teller(const TelltaleFlags, ivAction*);
    virtual void set(const TelltaleFlags, osboolean);
protected:
    ivAction* _action;
};

// active chosen makes itself chosen whenever it is active?


#endif // dastepper_h

