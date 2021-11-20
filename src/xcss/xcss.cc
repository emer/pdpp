
#include "xcss.h"

#include <css/machine.h>
#include <css/css_iv.h>
#include <css/css_builtin.h>
#include <css/ta_css.h>
#include <ta/taiv_data.h>

#include <ta/enter_iv.h>
#include <InterViews/font.h>
#include <InterViews/brush.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/session.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <InterViews/input.h>
#include <InterViews/event.h>
#include <InterViews/style.h>
#include <InterViews/transformer.h>
#include <IV-look/kit.h>
#include <InterViews/cursor.h>
#include <InterViews/bitmap.h>
#include <ta/leave_iv.h>

#include <sys/time.h>

#define blankptr_width 16
#define blankptr_height 16
#define blankptr_x_hot 1
#define blankptr_y_hot 1
static char blankptr_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define blankptrMask_width 16
#define blankptrMask_height 16
#define blankptrMask_x_hot 1
#define blankptrMask_y_hot 1
static char blankptrMask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static XCss* root = NULL;

class XCssInput : public ivInputHandler {
public:
  void move(const ivEvent& ev);
  void press(const ivEvent&  ev);
  void drag(const ivEvent& ev);
  void release(const ivEvent& ev);
  void keystroke(const ivEvent& ev);
  void double_click(const ivEvent& ev);

  XCssInput(ivGlyph* g, ivStyle* s);
};

XCssInput::XCssInput(ivGlyph* g, ivStyle* s) : ivInputHandler(g, s) {
}

void XCssInput::move(const ivEvent& ev) {
  ivInputHandler::move(ev);
}
void XCssInput::press(const ivEvent&  ev) {
  ivInputHandler::press(ev);
  if(!(root->ev_filter & XCss::PRESS))
    return;
  XCssEvent* xev = new XCssEvent;
  xev->time = ev.time();
  xev->type = XCssEvent::DOWN;
  xev->pointer.x = ev.pointer_x();
  xev->pointer.y = ev.pointer_y();
  xev->button = (XCssEvent::Button)ev.pointer_button();
  root->events.Add(xev);
}

void XCssInput::drag(const ivEvent& ev) {
  ivInputHandler::drag(ev);
}
void XCssInput::release(const ivEvent& ev) {
  ivInputHandler::release(ev);
  if(!(root->ev_filter & XCss::RELEASE))
    return;
  XCssEvent* xev = new XCssEvent;
  xev->time = ev.time();
  xev->type = XCssEvent::UP;
  xev->pointer.x = ev.pointer_x();
  xev->pointer.y = ev.pointer_y();
  xev->button = (XCssEvent::Button)ev.pointer_button();
  root->events.Add(xev);
}
void XCssInput::keystroke(const ivEvent& ev) {
  ivInputHandler::keystroke(ev);
  if(!(root->ev_filter & XCss::KEY))
    return;
  XCssEvent* xev = new XCssEvent;
  xev->time = ev.time();
  xev->type = XCssEvent::KEY;
  xev->pointer.x = ev.pointer_x();
  xev->pointer.y = ev.pointer_y();
  xev->keycode = ev.keycode();
  xev->keysym = ev.keysym();
  if(ev.control_is_down()) xev->ctrl |= XCssEvent::CTRL;
  if(ev.meta_is_down()) xev->ctrl |= XCssEvent::META;
  if(ev.shift_is_down()) xev->ctrl |= XCssEvent::SHIFT;
  if(ev.capslock_is_down()) xev->ctrl |= XCssEvent::CAPSLOCK;
  if(ev.left_is_down()) xev->ctrl |= XCssEvent::LEFT_BUT;
  if(ev.middle_is_down()) xev->ctrl |= XCssEvent::MIDDLE_BUT;
  if(ev.right_is_down()) xev->ctrl |= XCssEvent::RIGHT_BUT;
  root->events.Add(xev);
}
void XCssInput::double_click(const ivEvent& ev) {
  ivInputHandler::double_click(ev);
}

void XCssEvent::Initialize() {
  time = 0;
  type = UNDEFINED;
  button = NONE;
  keycode = 0;
  keysym = 0;
  ctrl = NO_CTRL;
}

void XCssEvent::Destroy() {
}

void XCssEvent::InitLinks() {
  taBase::InitLinks();
  taBase::Own(&pointer, this);
}

void XCss::Size(float width, float height) {
  canvas->size(width, height);
}
void XCss::PSize(int width, int height) {
  canvas->psize(width, height);
}

float XCss::Width() const {
  return canvas->width();
}

float XCss::Height() const {
  return canvas->height();
}
int XCss::PWidth() const {
  return canvas->pwidth();
}
int XCss::PHeight() const {
  return canvas->pheight();
}

int XCss::ToPixels(float coord) const {
  return canvas->to_pixels(coord);
}
float XCss::ToCoord(int pix) const {
  return canvas->to_coord(pix);
}
float XCss::ToPixelsCoord(float coord) const {
  return canvas->to_pixels_coord(coord);
}

void XCss::PushTransform() {
  canvas->push_transform();
}
void XCss::PopTransform() {
  canvas->pop_transform();
}
void XCss::Scale(float x, float y) {
  ivTransformer t = canvas->transformer();
  t.scale(x, y);
  canvas->transform(t);
}
void XCss::Translate(float x, float y) {
  ivTransformer t = canvas->transformer();
  t.translate(x, y);
  canvas->transform(t);
}
void XCss::Rotate(float angle) {
  ivTransformer t = canvas->transformer();
  t.rotate(angle);
  canvas->transform(t);
}

void XCss::NewPath() {
  canvas->new_path();
}
void XCss::MoveTo(float x, float y) {
  canvas->move_to(x, y);
}
void XCss::LineTo(float x, float y) {
  canvas->line_to(x,y);
}
void XCss::CurveTo(float x, float y, float x1, float y1, float x2, float y2) {
  canvas->curve_to(x, y, x1, y1, x2, y2);
}
void XCss::ClosePath() {
  canvas->close_path();
}
void XCss::Stroke() {
  canvas->damage_all();
  canvas->stroke(pen_color, brush);
}
void XCss::Line(float x1, float y1, float x2, float y2) {
  canvas->damage_all();
  canvas->line(x1, y1, x2, y2, pen_color, brush);
}
void XCss::Rect(float l, float b, float r, float t) {
  canvas->damage_all();
  canvas->rect(l, b, r, t, pen_color, brush);
}
static float p0 = 1.00000000;
static float p1 = 0.89657547;   // cos 30 * sqrt(1 + tan 15 * tan 15)
static float p2 = 0.70710678;   // cos 45 
static float p3 = 0.51763809;   // cos 60 * sqrt(1 + tan 15 * tan 15)
static float p4 = 0.26794919;   // tan 15

void XCss::EllipsePath(float x, float y, float rx, float ry) {
  float px0 = p0 * rx; float py0 = p0 * ry;
  float px1 = p1 * rx; float py1 = p1 * ry;
  float px2 = p2 * rx; float py2 = p2 * ry;
  float px3 = p3 * rx; float py3 = p3 * ry;
  float px4 = p4 * rx; float py4 = p4 * ry;

  canvas->new_path();
  canvas->move_to(x + rx, y);
  canvas->curve_to(x + px2, y + py2, x + px0, y + py4, x + px1, y + py3);
  canvas->curve_to(x, y + ry, x + px3, y + py1, x + px4, y + py0);
  canvas->curve_to(x - px2, y + py2, x - px4, y + py0, x - px3, y + py1);
  canvas->curve_to(x - rx, y, x - px1, y + py3, x - px0, y + py4);
  canvas->curve_to(x - px2, y - py2, x - px0, y - py4, x - px1, y - py3);
  canvas->curve_to(x, y - ry, x - px3, y - py1, x - px4, y - py0);
  canvas->curve_to(x + px2, y - py2, x + px4, y - py0, x + px3, y - py1);
  canvas->curve_to(x + rx, y, x + px1, y - py3, x + px0, y - py4);
  canvas->close_path();
}

void XCss::Circle(float x, float y, float r) {
  EllipsePath(x, y, r, r);
  Stroke();
}

void XCss::Ellipse(float x, float y, float rx, float ry) {
  EllipsePath(x, y, rx, ry);
  Stroke();
}

void XCss::Fill() {
  canvas->damage_all();
  canvas->fill(fill_color);
}

void XCss::FillRect(float l, float b, float r, float t) {
  canvas->damage_all();
  canvas->fill_rect(l, b, r, t, fill_color);
}

void XCss::FillWindow() {
  canvas->damage_all();
  canvas->fill_rect(0.0f, 0.0f, Width(), Height(), fill_color);
}

void XCss::FillCircle(float x, float y, float r) {
  EllipsePath(x, y, r, r);
  Fill();
}

void XCss::FillEllipse(float x, float y, float rx, float ry) {
  EllipsePath(x, y, rx, ry);
  Fill();
}

void XCss::Character(long ch, float width, float x, float y) {
  canvas->damage_all();
  canvas->character(font, ch, width, pen_color, x, y);
}

void XCss::TextLeft(const char* txt, float x, float y, float spc) {
  ivFontBoundingBox bbox;
  float xp = x;
  float yp = y;
  String str = txt;
  int i;
  for(i=0;i<(int)str.length();i++) {
    font->char_bbox(str[i], bbox);
    if(str[i] == '\n') {
      yp -= (bbox.font_ascent() + bbox.font_descent()) * spc;
      xp = x;
      continue;
    }
    float wdth = bbox.width() * spc;
    Character(str[i], wdth, xp, yp);
    xp += wdth;
  }
}

void XCss::TextCenter(const char* txt, float x, float y, float spc) {
  ivFontBoundingBox bbox;
  float totw = 0.0f;
  String str = txt;
  int i;
  for(i=0;i<(int)str.length();i++) {
    font->char_bbox(str[i], bbox);
    float wdth = bbox.width() * spc;
    totw += wdth;
  }
  float xp = x - totw * .5f;
  for(i=0;i<(int)str.length();i++) {
    font->char_bbox(str[i], bbox);
    float wdth = bbox.width() * spc;
    Character(str[i], wdth, xp, y);
    xp += wdth;
  }
}

void XCss::TextRight(const char* txt, float x, float y, float spc) {
  ivFontBoundingBox bbox;
  float totw = 0.0f;
  String str = txt;
  int i;
  for(i=0;i<(int)str.length();i++) {
    font->char_bbox(str[i], bbox);
    float wdth = bbox.width() * spc;
    totw += wdth;
  }
  float xp = x - totw;
  for(i=0;i<(int)str.length();i++) {
    font->char_bbox(str[i], bbox);
    float wdth = bbox.width() * spc;
    Character(str[i], wdth, xp, y);
    xp += wdth;
  }
}

void XCss::Beep() {
  cerr << (char)0x07;
}

void XCss::BlankPointer() {
  if(window->cursor() != blankptr) {
    window->push_cursor();
    window->cursor(blankptr);
  } 
}

void XCss::UnBlankPointer() {
  if(window->cursor() == blankptr) {
    window->pop_cursor();
  } 
}

void XCss::ClipRect(float l, float b, float r, float t) {
  canvas->damage_all();
  canvas->clip_rect(l, b, r, t);
}
void XCss::FrontBuffer() {
  canvas->front_buffer();
}
void XCss::BackBuffer() {
  canvas->back_buffer();
}
void XCss::DamageAll() {
  canvas->damage_all();
}

void XCss::PenColorRGBA(float r, float g, float b, float a) {
  if(pen_color != NULL)
    ivResource::unref(pen_color);
  pen_color = new ivColor(r,g,b,a);
  ivResource::ref(pen_color);
}
void XCss::PenColorName(const char* name) {
  float r, g, b;
  if(!(ivColor::find
       (ivSession::instance()->default_display(),name,r,g,b))){
    taMisc::Error("Color: " , name , " not found for this display");
  }
  PenColorRGBA(r,g,b,1.0f);
}
void XCss::FillColorRGBA(float r, float g, float b, float a) {
  if(fill_color != NULL)
    ivResource::unref(fill_color);
  fill_color = new ivColor(r,g,b,a);
  ivResource::ref(fill_color);
}
void XCss::FillColorName(const char* name) {
  float r, g, b;
  if(!(ivColor::find
       (ivSession::instance()->default_display(),name,r,g,b))){
    taMisc::Error("Color: " , name , " not found for this display");
  }
  FillColorRGBA(r,g,b,1.0f);
}
void XCss::BrushWidth(float width) {
  if(brush != NULL)
    ivResource::unref(brush);
  brush = new ivBrush(width);
  ivResource::ref(brush);
}
void XCss::FontName(const char* font_spec) {
  String pattern = font_spec;
  ivFont* f = (ivFont *) ivFont::lookup(pattern);
  if(f==NULL){
    f = (ivFont *) ivFont::lookup(String( "*-" + pattern + "-medium-r*"));
  }
  if(f==NULL){
    f = (ivFont *) ivFont::lookup(String( "*-" + pattern + "-r*"));
  }
  if(f==NULL){
    f = (ivFont *) ivFont::lookup(String( "*-" + pattern + "-*"));
  }
  if(f==NULL){
    taMisc::Error("Cannot find font: ", pattern);
    return;
  }
  if(font == f) return;
  if(font != NULL)
    ivResource::unref(font);
  font = f;
  ivResource::ref(font);
}

long XCss::TimeMs() {
  struct timezone tzp;
  struct timeval tp;
  gettimeofday(&tp, &tzp);
  long rval = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  return rval;
}

void XCss::WaitTime(int ms) {
  long start = TimeMs();
  long del = 0;
  while (del < ms) {
    taivMisc::RunIVPending();
    del = TimeMs() - start;
  }
}

void XCss::WaitEventTime(int ms) {
  events.Reset();
  long start = TimeMs();
  long del = 0;
  while ((events.size == 0) && (del < ms)) {
    taivMisc::RunIVPending();
    del = TimeMs() - start;
  }
}

void XCss::WaitEvent() {
  events.Reset();
  while (events.size == 0) {
    taivMisc::RunIVPending();
  }
}

void XCss::DumpJPEG(const char* fname) {
  const ivAllocation& alloc = input_hand->allocation();
  int xstart = canvas->to_pixels(alloc.left(), Dimension_X);
  int xend = canvas->to_pixels(alloc.right(), Dimension_X);
  int ystart = canvas->pheight() - canvas->to_pixels(alloc.top(), Dimension_Y);
  int yend = canvas->pheight() - canvas->to_pixels(alloc.bottom(), Dimension_Y);
  taivMisc::DumpJpegIv(window, fname, taMisc::jpeg_quality, xstart, ystart, xend-xstart, yend-ystart);
}

void XCss::CreateWindow() {
  ivLayoutKit* layout = ivLayoutKit::instance();
  ivWidgetKit* wkit = ivWidgetKit::instance();
  ivGlyph* win = layout->natural(layout->hbox(layout->vglue(), layout->hglue()),
				 win_geom.x, win_geom.y);
  input_hand = new XCssInput(win, wkit->style());
  window = new ivApplicationWindow(input_hand);
  canvas = window->canvas();
  BrushWidth(1);
  PenColorName("black");
  FillColorName("white");
  FontName("*-helvetica-medium-r-*-*-18*");
  window->map();

  ivBitmap* blank = new ivBitmap(blankptr_bits, blankptr_width, blankptr_height,
				   blankptr_x_hot, blankptr_y_hot);
  ivBitmap* blank_mask = new ivBitmap(blankptrMask_bits, blankptrMask_width, blankptrMask_height,
					blankptrMask_x_hot, blankptrMask_y_hot);
  blankptr = new ivCursor(blank, blank_mask);
}

void XCss::Initialize() {
  win_geom.x = 800;
  win_geom.y = 600;
  window = NULL;
  canvas = NULL;
  brush = NULL;
  pen_color = NULL;
  fill_color = NULL;
  font = NULL;
}

void XCss::Destroy() {
  if(window != NULL)
    delete window;
  ivResource::unref(brush);
  ivResource::unref(pen_color);
  ivResource::unref(fill_color);
  ivResource::unref(font);
}

void XCss::InitLinks() {
  taNBase::InitLinks();
  taBase::Own(&win_geom, this);
  taBase::Own(&events, this);
}

static ivOptionDesc xcss_options[] = {
    { NULL }
};

static ivPropertyData xcss_defs[] = {
  {"xcss++*gui", "sgimotif"},
  {"xcss++*PopupWindow*overlay", "true"},
  {"xcss++*PopupWindow*saveUnder", "on"},
  {"xcss++*TransientWindow*saveUnder", "on"},
  {"xcss++*double_buffered",	"on"},
  {"xcss++*flat",		"#c0c4d3"},
  {"xcss++*background",  	"#70c0d8"},
  {"xcss++*name*flat",		"#70c0d8"},
  {"xcss++*apply_button*flat",	"#c090b0"},
  {"xcss++*FieldEditor*background", "white"},
  {"xcss++*FileChooser*filter", 	"on"},
  {"xcss++*FileChooser.rows", 	"20"},
  {"xcss++*FileChooser.width", 	"100"},
  {"xcss++*PaletteButton*minimumWidth", "72.0"},
  {"xcss++*PushButton*minimumWidth", "72.0"},
  {"xcss++*TaIVButton*SmallWidth", "46.0"},
  {"xcss++*TaIVButton*MediumWidth", "72.0"},
  {"xcss++*TaIVButton*BigWidth", "115.0"},
  {"xcss++*toggleScale",		"1.5"},
#ifndef CYGWIN
  {"xcss++*font",		"*-helvetica-medium-r-*-*-10*"},
  {"xcss++*name*font",		"*-helvetica-medium-r-*-*-10*"},
  {"xcss++*title*font",		"*-helvetica-bold-r-*-*-10*"},
  {"xcss++*small_menu*font",	"*-helvetica-medium-r-*-*-10*"},
  {"xcss++*small_submenu*font",	"*-helvetica-medium-r-*-*-10*"},
  {"xcss++*big_menu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"xcss++*big_submenu*font",	"*-helvetica-medium-r-*-*-12*"},
  {"xcss++*big_menubar*font",	"*-helvetica-bold-r-*-*-14*"},
  {"xcss++*big_italic_menubar*font","*-helvetica-bold-o-*-*-14*"},
#else
  {"xcss++*font",		"*Arial*medium*--12*"},
  {"xcss++*name*font",		"*Arial*medium*--12*"},
  {"xcss++*title*font",		"*Arial*bold*--12*"},
  {"xcss++*small_menu*font",	"*Arial*medium*--12*"},
  {"xcss++*small_submenu*font",	"*Arial*medium*--12*"},
  {"xcss++*big_menu*font",	"*Arial*medium*--12*"},
  {"xcss++*big_submenu*font",	"*Arial*medium*--12*"},
  {"xcss++*big_menubar*font",	"*Arial*bold*--14*"},
  {"xcss++*big_italic_menubar*font","*Arial*italic*--14*"},
  // following are def'd in smf_kit.cpp
  {"xcss++*MenuBar*font", 	"*Arial*bold*--12*"},
  {"xcss++*MenuItem*font", 	"*Arial*bold*--12*"},
  {"xcss++*MenuBar*font", 	"*Arial*bold*--12*"},
  {"xcss++*MenuItem*font", 	"*Arial*bold*--12*"},
#endif
  { NULL } 
};

int main(int argc, char* argv[]) {
  ta_Init_xcss();

  new ivSession("xcss++", argc, argv, xcss_options, xcss_defs);

  root = new XCss();
  taBase::Ref(root);
  root->InitLinks();	// normally the owner would do this, but..
  root->name = "root";

  // tabMisc stuff
  tabMisc::root = (TAPtr)root;
  taMisc::default_scope = &TA_XCss;

  // cssMisc stuff
  cssMisc::HardVars.Push(cssBI::root = new cssTA_Base(root, 1, &TA_XCss,"root"));
  cssMisc::Initialize(argc, (const char**)argv);


  cssivSession::WaitProc = tabMisc::WaitProc;
  taivMisc::Initialize();

  root->CreateWindow();
  
  cssMisc::Top->StartupShell(cin, cout);

  delete root;
  return 0;
}
