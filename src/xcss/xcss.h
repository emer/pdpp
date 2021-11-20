// Xperiment running CSS 

#ifndef xcss_h
#define xcss_h

class ivWindow;			// #IGNORE
class ivCanvas;			// #IGNORE
class ivColor;			// #IGNORE
class ivBrush;			// #IGNORE
class ivFont;			// #IGNORE
class ivCursor;			// #IGNORE
class XCssInput;		// #IGNORE
class ivInputHandler;		// #IGNORE

#include <ta/ta_base.h>
#include <ta_misc/tdgeometry.h>
#include <xcss/xcss_TA_type.h>

class XCssEvent : public taBase {
  // represents events
public:
  enum EventType {
    UNDEFINED,
    MOTION,
    DOWN,
    UP,
    KEY,
    OTHER_EVENT
  };

  enum Button {
    NONE,
    ANY,
    LEFT,
    MIDDLE,
    RIGHT,
    OTHER_BUTTON
  };

  enum CtrlType {
    NO_CTRL = 0x00,
    CTRL = 0x01,
    META = 0x02,
    SHIFT = 0x04,
    CAPSLOCK = 0x08,
    LEFT_BUT = 0x10,
    MIDDLE_BUT = 0x20,
    RIGHT_BUT = 0x40
  };
    
  unsigned long 	time;
  EventType		type;
  FloatTwoDCoord	pointer;
  Button		button;
  int			keycode;
  int			keysym;
  int			ctrl;
  
  void	Initialize();
  void 	Destroy();
  void	InitLinks();
  TA_BASEFUNS(XCssEvent);
};

class XCssEvent_List : public taList<XCssEvent> {
  // ##NO_TOKENS ##NO_UPDATE_AFTER
public:
  void	Initialize() 		{ };
  void 	Destroy()		{ };
  TA_BASEFUNS(XCssEvent_List);
};

class XCss : public taNBase {
  // makes an interface available in X (actually IV)
public:
  enum EventFilter {
    NO_EVENTS = 0x00,
    PRESS = 0x01,
    RELEASE = 0x02,
    KEY = 0x04
  };
  
  ivWindow*	window;		// #IGNORE
  ivCanvas*	canvas;		// #IGNORE
  ivInputHandler* input_hand;	// #IGNORE 
  ivColor*	pen_color;	// #IGNORE
  ivColor*	fill_color;	// #IGNORE
  ivBrush*	brush;		// #IGNORE
  ivFont*	font;		// #IGNORE
  XCssInput*	input;		// #IGNORE input handler
  ivCursor*	blankptr;	// #IGNORE blank pointer
  FloatTwoDCoord win_geom;	// geometry of the overall window
  int		ev_filter;	// events to process
  XCssEvent_List events;	// list of events

  virtual void 	Size(float width, float height);
  virtual void 	PSize(int width, int height);

  virtual float Width() const;
  virtual float Height() const;
  virtual int 	PWidth() const;
  virtual int 	PHeight() const;

  virtual int 	ToPixels(float coord) const;
  virtual float ToCoord(int pix) const;
  virtual float ToPixelsCoord(float coord) const;

  virtual void 	PushTransform();
  virtual void 	PopTransform();
  virtual void 	Scale(float x, float y);
  virtual void 	Translate(float x, float y);
  virtual void 	Rotate(float angle);

  virtual void 	NewPath();
  virtual void 	MoveTo(float x, float y);
  virtual void 	LineTo(float x, float y);
  virtual void 	CurveTo(float x, float y, float x1, float y1, float x2, float y2);
  virtual void 	ClosePath();
  virtual void 	Stroke();
  virtual void 	Line(float x1, float y1, float x2, float y2);
  virtual void 	Rect(float l, float b, float r, float t);
  virtual void 	EllipsePath(float x, float y, float rx, float ry);
  virtual void 	Circle(float x, float y, float r);
  virtual void 	Ellipse(float x, float y, float rx, float ry);
  virtual void 	Fill();
  virtual void 	FillRect(float l, float b, float r, float t);
  virtual void	FillWindow();
  virtual void 	FillCircle(float x, float y, float r);
  virtual void 	FillEllipse(float x, float y, float rx, float ry);
  virtual void 	Character(long ch, float width, float x, float y);
  virtual void 	TextLeft(const char* txt, float x, float y, float space = 1.05f);
  virtual void 	TextCenter(const char* txt, float x, float y, float space = 1.05f);
  virtual void 	TextRight(const char* txt, float x, float y, float space = 1.05f);
  virtual void 	Beep();
  virtual void	BlankPointer();
  virtual void	UnBlankPointer();

  virtual void 	ClipRect(float l, float b, float r, float t);
  virtual void 	FrontBuffer();
  virtual void 	BackBuffer();
  virtual void 	DamageAll();

  virtual void	PenColorRGBA(float r, float g, float b, float a);
  virtual void	PenColorName(const char* name);
  virtual void	FillColorRGBA(float r, float g, float b, float a);
  virtual void	FillColorName(const char* name);
  virtual void	BrushWidth(float width);
  virtual void	FontName(const char* font_spec);

  virtual void	CreateWindow();
  virtual long 	TimeMs();
  virtual void	WaitTime(int ms);
  virtual void	WaitEventTime(int ms);
  virtual void	WaitEvent();

  virtual void	DumpJPEG(const char* fname);

  void	Initialize();
  void 	Destroy();
  void	InitLinks();
  TA_BASEFUNS(XCss);
};


#endif // xcss_h
