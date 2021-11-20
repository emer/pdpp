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

#include <stdlib.h>
#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/layout.h>
#include <InterViews/session.h>
#include <InterViews/window.h>
#include <InterViews/page.h>
#include <InterViews/patch.h>
#include <InterViews/debug.h>
#include <InterViews/scrbox.h>
#include <InterViews/color.h>

#include <iv_misc/lrScrollBox.h>
#include <iv_misc/tbScrollBox.h>

#include <InterViews/cursor.h>
#include <ta_misc/picker.bm>
#include <ta_misc/picker_mask.bm>
#include <InterViews/bitmap.h>

#include <iv_misc/scrollable.h>
#include <iv_graphic/graphic.h>
#include <InterViews/brush.h>


static Cursor* picker_cursor = nil;
static Cursor* mywindow_cursor = nil;

tbScrollBox tb;
Patch* 	pat;


Glyph* the;

class App {
public:
  void msg();
  int cntr;
  float z;
  Glyph* newstring;
  void  zoomin();
  void  zoomout();
  Scrollable* scrb;
  App() {cntr = 0; z = 1;};
  ~App() {cntr = 0;};
};

declareActionCallback(App)
implementActionCallback(App)


void App::msg() {
    WidgetKit& kit = *WidgetKit::instance();
    char c[100];
    cntr++;
    printf("Adding Line\n");
    sprintf(c,"%d this is a line with %d",cntr,cntr);
    newstring = kit.fancy_label(c);
    tb.append(newstring);
    tb.scroll_to(Dimension_Y,0);
    if(pat) pat->reallocate();
    tb.notify(Dimension_Y);
    if(pat) pat->redraw();
}

void App::zoomin(){
  z =  z * 0.9;
  scrb->zoom_to(z);
}

void App::zoomout(){
  z = z * 1.1;
  scrb->zoom_to(z);
}

int main(int argc, char** argv) {
  Session* session = new Session("Test", argc, argv);
  WidgetKit& kit = *WidgetKit::instance();
  const LayoutKit& layout = *LayoutKit::instance();
  App* a = new App;
  TelltaleGroup* group = new TelltaleGroup;

  tb.naturalnum = 5;
  char c[10];
  sprintf(c,"Genline");
  pat = new Patch
    (new Background
     (layout.hflexible
	(layout.vbox
	 (kit.push_button(c , new ActionCallback(App)(a, &App::msg)),
	  layout.hbox(kit.vscroll_bar(&tb),&tb))),kit.background()));
  
//  ApplicationWindow win(pat);

  Glyph* str1 = kit.fancy_label("This is a blank formatted diskette");
  Glyph* str2 = kit.fancy_label("Set the cup upon the table");
  Glyph* str3 = kit.fancy_label("Did you see the cat in the tree?");

  Glyph* gl =
    kit.inset_frame
    (layout.vbox
     (layout.hcenter(str1,0),
      layout.hcenter(str2,0),
      layout.hcenter(str3,0),
      layout.hcenter(kit.push_button
		     ("Zoom In ",
		      new ActionCallback(App)(a,&App::zoomin)),0),
      layout.hcenter(kit.push_button
		     ("Zoom Out",
		      new ActionCallback(App)(a,&App::zoomout)),0)));

  a->scrb = new Scrollable
    (layout.vcenter(gl,0),100,100,0,-1);

  ApplicationWindow win
    (new Background
     (layout.vbox
      (kit.outset_frame(a->scrb),kit.hscroll_bar(a->scrb)),
      kit.background()));

//   ApplicationWindow win
//     (new Background
//      (layout.vbox
//       (layout.hbox
//        (kit.vscroll_bar(a->scrb),layout.vcenter(a->scrb,0)),
//        layout.hbox(kit.outset_frame(layout.hglue(16.0, 0.0,0.0)),
// 		   kit.hscroll_bar(a->scrb))),
//       kit.background()));

  if (picker_cursor == nil) {
    Bitmap* picker = new Bitmap(
				 picker_bits, picker_width, picker_height,
				 picker_x_hot, picker_y_hot
				 );
    Bitmap* picker_mask = new Bitmap(
				      pickerMask_bits, pickerMask_width, pickerMask_height,
				      pickerMask_x_hot, pickerMask_y_hot
				      );
    picker_cursor = new Cursor(picker, picker_mask);
  }
  
  if (mywindow_cursor == nil) {
    mywindow_cursor = win.cursor();
  }
  win.cursor(picker_cursor);

  session->run_window(&win);

}






