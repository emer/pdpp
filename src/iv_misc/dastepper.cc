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

#include <dastepper.h>

#include <ta/enter_iv.h>
#include <InterViews/glyph.h>
#include <InterViews/telltale.h>
#include <InterViews/event.h>
#include <ta/leave_iv.h>

static int GetButton(const ivEvent& e) {
  if(e.pointer_button() == ivEvent::left){
    if(e.shift_is_down() == ostrue) return ivEvent::middle;
    if(e.meta_is_down() == ostrue) return ivEvent::right;
    return ivEvent::left;
  }
  else return(e.pointer_button());
}


DAStepper::DAStepper
(ivGlyph* g ,ivStyle* s ,ivTelltaleState* t,ivAction* a) :ivStepper(g,s,t,a) {};

void DAStepper::press(const ivEvent& e){
  button = e.pointer_button();
  ivStepper::press(e);
}

void DAStepper::release(const ivEvent& e){
  stop_stepping();
  ivTelltaleState* s = state();
  if (s->test(ivTelltaleState::is_enabled)) {
    s->set(ivTelltaleState::is_active, osfalse);
    if (inside(e)) {
      osboolean chosen = s->test(ivTelltaleState::is_chosen);
      osboolean act = !chosen;
      if (s->test(ivTelltaleState::is_toggle)) {
	s->set(ivTelltaleState::is_chosen, act);
	act = ostrue;
      } else if (s->test(ivTelltaleState::is_choosable)) {
	s->set(ivTelltaleState::is_chosen, ostrue);
      }

      // don't call action on release!

//       if (act) {
// 	ivAction* a = action();
// 	if (a != nil) {
// 	  s->set(ivTelltaleState::is_running, ostrue);
// 	  a->execute();
// 	  s->set(ivTelltaleState::is_running, osfalse);
// 	}
//       }
    }
  }
}


void DAStepper::adjust(){
  ivTelltaleState* s = state();
  osboolean chosen = s->test(ivTelltaleState::is_chosen);
  osboolean act = !chosen;
  if (s->test(ivTelltaleState::is_toggle)) {
    s->set(ivTelltaleState::is_chosen, act);
    act = ostrue;
  } else if (s->test(ivTelltaleState::is_choosable)) {
    s->set(ivTelltaleState::is_chosen, ostrue);
  }
  if (act) {
    ivAction* a = action();
    if (a != nil) {
      s->set(ivTelltaleState::is_running, ostrue);
      int count = 1;
      if(button == ivEvent::middle) count = 10;
      else if(button == ivEvent::right) count = 100;
      int i;
      for(i=0;i<count;i++){
	a->execute();
      }
      s->set(ivTelltaleState::is_running, osfalse);
    }
  }
}

ParameterStepper::ParameterStepper(ivGlyph* g ,ivStyle* s ,
				   ivTelltaleState* t ,IntAction* a)
:DAStepper(g,s,t,a) {}


void ParameterStepper::adjust(){
  ivTelltaleState* s = state();
  osboolean chosen = s->test(ivTelltaleState::is_chosen);
  osboolean act = !chosen;
  if (s->test(ivTelltaleState::is_toggle)) {
    s->set(ivTelltaleState::is_chosen, act);
    act = ostrue;
  } else if (s->test(ivTelltaleState::is_choosable)) {
    s->set(ivTelltaleState::is_chosen, ostrue);
  }
  if (act) {
    IntAction* a = (IntAction *) action();
    if (a != nil) {
      s->set(ivTelltaleState::is_running, ostrue);
      int count = 1;
      if(button == ivEvent::middle) count = 10;
      else if(button == ivEvent::right) count = 100;
      a->call_me(count);
      s->set(ivTelltaleState::is_running, osfalse);
    }
  }
}


void NoUnSelectButton::chooseme(){
  state()->set(ivTelltaleState::is_chosen,ostrue);
  state()->set(ivTelltaleState::is_active,ostrue);
  update(nil);
  redraw();
  ivAction* a = action();
  if (a != nil) {
    state()->set(ivTelltaleState::is_running, ostrue);
    a->execute();
    state()->set(ivTelltaleState::is_running, osfalse);
  }
}

void NoUnSelectButton::release(const ivEvent& e) {
  ivTelltaleState* s = state();
  if (!s->test(ivTelltaleState::is_enabled))
    return;
  s->set(ivTelltaleState::is_active, osfalse);
  if(inside(e)){
    ivAction* a = action();
    if (a != nil) {
      s->set(ivTelltaleState::is_running, ostrue);
      a->execute();
      s->set(ivTelltaleState::is_running, osfalse);
    }
  }
}


Teller::Teller(const TelltaleFlags f, ivAction* a) : ivTelltaleState (f) {
    _action = a;
}

void Teller::set (const TelltaleFlags f, osboolean flag) {
  if (flag && (f == is_active)) {
    ivTelltaleState::set(is_active | is_chosen, flag);
    if (_action != nil) {
      _action->execute();
    }
  } else if (!flag && f == is_chosen) {
    ivTelltaleState::set(is_active | is_chosen, flag);

  } else if (!flag && f == is_active) {

  } else if (flag && f == is_chosen) {

  } else {
    ivTelltaleState::set(f, flag);
  }
}


InsetActiveButton::InsetActiveButton(ivGlyph* g, ivStyle* s, ivTelltaleState* t,
		    ivAction* at1,ivAction* at2,ivAction* at3,
				     ButtonState s1, ButtonState s2, ButtonState s3)
: ivButton(g,s,t,nil) {
  action_1 = at1;
  ivResource::ref(action_1);
  b1 = s1;
  action_2 = at2;
  ivResource::ref(action_2);
  b2 = s2;
  action_3 = at3;
  ivResource::ref(action_3);
  b3 = s3;
  down = 0;
}


InsetActiveButton::~InsetActiveButton(){
  ivResource::unref(action_1);
  ivResource::unref(action_2);
  ivResource::unref(action_3);
}

void InsetActiveButton::press(const ivEvent& e){
  ivButton::press(e);
}
    
void InsetActiveButton::SetState(ButtonState bs){
  ivTelltaleState* s = state();
  switch(bs){
  case BUTTON_UP: s->set(ivTelltaleState::is_active, osfalse); break;
  case BUTTON_DOWN: s->set(ivTelltaleState::is_active, ostrue); break;
  case BUTTON_FLIP: down = !down;
  case BUTTON_SAME:
    s->set(ivTelltaleState::is_active, down); break;
  }
};

void InsetActiveButton::release(const ivEvent& e) {
  ivTelltaleState* s = state();
  if (!s->test(ivTelltaleState::is_enabled))
    return;
  if(inside(e)){
    switch(GetButton(e)) {
    case ivEvent::right:  SetState(b3);
      if(action_3 != nil) {  action_3->execute(); break; }
    case ivEvent::middle: SetState(b2);
      if(action_2 != nil) {  action_2->execute(); break; }
    case ivEvent::left:  SetState(b1);
      if(action_1 != nil) {  action_1->execute(); break; }
    };
  }
}

void IntAction::execute() {
  call_me(1);
}


