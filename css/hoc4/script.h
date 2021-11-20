// script.h Feb 25th 1992

#ifdef __GNUG__
#include <String.h>
#else
#include  <mystring.h>
#endif


const enum StructTypes {

  // non substructure type

  ST_Void,			// No Type
  ST_Int,			// Integer
  ST_Real,
  ST_String,
  ST_PI,			// Pointer/Index Union type
  ST_Process,			// Process Type


    //  Non structure arrays

  ST_Int_Array,			// Integer
  ST_Float_Array,
  ST_String_Array,
  ST_PI_Array,			// Pointer/Index Union type

    // substructure types;

  ST_Struct,			// structure is in parent struct
  ST_StructP,			// pointer to structure

    // Struct Array Types

  ST_PDPGroup,			// structure is in parent struct

    // 

  ST_Function_Int,		// for a function type (internal)
  ST_Function_Ext,		// for a function type (external)

};

// script stack element

struct sslEl {
private:
public:

  virtual char 		*GetName() { return "Basic Element"; };
  virtual StructTypes 	GetType() { return NULL; };
  virtual int 		GetSize(void) { return(sizeof(this)); };
  virtual 

}

struct sslInt : sslEl {
  bool operator<(const sslInt& s) const	{ return strcmp(p, s.p) < 0; }
  bool operator>(const sslInt& s) const	{ return strcmp(p, s.p) > 0; }
  bool operator<=(const sslInt& s) const	{ return strcmp(p, s.p) <= 0; }
  bool operator>=(const sslInt& s) const	{ return strcmp(p, s.p) >= 0; }
  bool operator==(const sslInt& s) const;
  bool operator!=(const sslInt& s) const	{ return !(*this==s); }
  
  bool operator<(const char* cs) const	{ return strcmp(p,cs) < 0; }   
  bool operator>(const char* cs) const	{ return strcmp(p,cs) > 0; }   
  bool operator<=(const char* cs) const	{ return strcmp(p,cs) <= 0; }
  bool operator>=(const char* cs) const	{ return strcmp(p,cs) >= 0; }
  bool operator==(const char* cs) const	{ return strcmp(p,cs) == 0; }
  bool operator!=(const char* cs) const	{ return strcmp(p,cs) != 0; }

  friend sslInt operator+(const sslInt& x, sslInt& cs);
  friend sslInt operator+(const sslInt& x, char* cs);
  friend sslInt operator+(const sslInt& x, char  cs);
  
  void operator +=(const sslInt& t);
  void operator +=(const char* t);
  void operator +=(const char  t);
  
  int contains(char *c);
  sslInt after(char *c);
  sslInt before(char *c);
  char* chars();
  void gsub(const char* p1, const char* p2);

  friend bool operator<(const char* cs, const sslInt& s) {
    return strcmp(cs, s.p) < 0;
  }
  friend bool operator>(const char* cs, const sslInt& s) {	 
    return strcmp(cs, s.p) > 0;
  }
  friend bool operator<=(const char* cs, const sslInt& s) { 
    return strcmp(cs, s.p) <= 0; 
  }
  friend bool operator>=(const char* cs, const sslInt& s) { 
    return strcmp(cs, s.p) >= 0;
  }
  friend bool operator==(const char* cs, const sslInt& s) {
    return strcmp(cs, s.p) == 0; 
  }
  friend bool operator!=(const char* cs, const sslInt& s) {
    return strcmp(cs, s.p) != 0; 
  }
  
}

struct ScrVar_Array {
public:
  uint size = 0;
  ScrVar **data=NULL;

  ScrVar &operator[](uint i);
  void Add(ScrVar &it);		// add new object
  ScrVar &GetVar(String *nm);	// lookup by name
}

extern ScrVar_Array ScrVar_Global; // global list

// every line of code gets translated into one of these
// script is just whole linked list of them

struct ScrFunc : ScrVar {
private:
  ScrFunc *prev=NULL;		// linked list of these
  ScrFunc *next=NULL;
  ScrFunc *up=NULL;
  int     LineNo;		// line in original text
  String  Line;			// the line of original code (for dbug)
public:
  StructTypes  type;
  ScrVar_Array argv= NULL;		// Arguments to function
  ScrVar  result;		// Result of fuction call;
  virtual ScrVar &Do();		// Pointer to actual function
  StructTypes GetType() { return type; }; 
  ScrFunc(int ln, String sl, String nm, StructType tp,
	  ScrVar_Array *av=NULL; ScrVar *f=NULL);
  ScrFunc(String nm, StructType tp, ScrVar *f);
}

// This is the "root pointer" to a script

struct ScrHdr : ScrFunc {	
private:
  ScrFunc *first = NULL;		// Branch on the scripting tree
public:
  ScrVar_Array *local = NULL;		// nothing yet
  ScrVar_Array *global = &ScrVar_Global;
  ScrVar_Array *current = &ScrVar_Global; // start global, change later
  int AddToScript(ScrFunc func)();
  friend ScrVar ScrHdrDo();
}


struct SString: ScrVar{
protected:
  String val;
public:
  StructTypes GetType(){ return ST_String;};

  SString() {val = ""};
  SString(String st) {val = st;};
}


struct SString : ScrVar {
public:
  StructTypes GetType() { return ST_String};

  SString() {name = ""};
  SString(String &nm) {name = nm;};
}

// integer type for scripts
struct SInt : ScrVar {
protected:
  int val;
public:
  StructTypes GetType() { return ST_Int; };
  
  SInt() {val = 0;};
  SInt(int vl)  {val = vl; };
  SInt(String &nm) {name = nm; };
  ~SInt();
  
  void            operator =  (const SInt&);
  void            operator =  (int);
}

inline void SInt::operator = (const SInt&  y)
{
  val = y.val;
}

inline void SInt::operator = (int y)
{
  val = y;
}



ScrF_SInt_Do()
{
  SInt *tmp;
  ScrVar arg0;
  String nm;

  if(argv.size == 1) {		// one argument, must be name
   arg = argv[0];
   if (arg0.GetType() != ST_String) {
     cerr << "Argument " << arg.name << " already defined \n";
   }
   tmp = new SInt(arg.name);
   current->Add(tmp);	
  }
  else {
    cerr << "Expected only one argument to Int\n";
  }
}
    
  



