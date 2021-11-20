// script.cc Feb 25th 1992



// sample code as target for script language:
// want to have it be transparent between c++ and script
// so that this code can be in a .cc or in a library
// with some kind of translator to change our types into
// c++ types (vs.  the idea of using SInt directly in script)

//int y;   // translates into SInt
//y = 10;

//for(y=0; y < 10; y++)
//{
//  cout << y;
//}

// plan:  
// 1. parse each line into linked list of ScrFunc 's
//     yacc for this (can use the cp_parse.y from g++)
// 2. bind args of function??
// 3. step thru list of funcs


#include "script.h"



ScrFunc::ScrFunc(int ln, String sl, String nm, StructType tp,
		  *ScrVar_Array av=NULL; ScrVar *f=NULL)
{
   LineNo = ln;
   Line   = sl;
   name   = nm;
   type   = tp;
   argv   = av;
   Do     = f;
}
   
ScrFunc::ScrFunc(String nm, StructType tp, ScrVar *f=NULL)
{
   name   = nm;
   type   = tp;
   Do     = f;
}
   



ScrVar ScrHdrDo()
{
  ScrFunc temp;
  for(temp=*first;(temp!=NULL);temp=temp->next) {
    temp->Do();
  }
}
  


int ScrHdr::AddToScript(ScrFunc *func) {
  ScrFunc temp,temp2=this;
                // get to end of l-list
  for(temp=*first;(temp!=NULL);temp2=temp;temp=temp->next);
  temp = func;
  temp->prev = temp2;
  temp->up   = 
}  



ScrVar_Array ScrVar_Global;

ScrVar_Array::Add(ScrVar &it) {
  data = realloc((void *) data,++size);
  data[size-1] = it;
}


ScrVar &ScrVar_Array::operator[](uint i) {
  return (data[i]);
}

Scrvar &ScrVar_Array::GetVar(String *nm) {
  int i;
  for(i=0;i<size;i++) {
    if (data[i]->name == *nm) return (data[i]);
  }
  return(Null);
}


				// SInt



void InternalFunc_Init() {

  ScrF_SInt = ScrFunc("int",ST_Decl,&SInt_Do());
  ScrVar_Global.Add(ScrF_SInt);
}


main()
{

  /* Sample Program:


     int x;
     x = 5;

  */


  ScrHdr  head(ScrVar_Global);
  InternalFunc_Init();

  // Parser calls


  // First "int" is looked up on the variable list;

  ScrVar tmp;
  String tmpstring;
  
  tmpstring = "int";

  tmp = ScrVar_Global.GetVar(tempstring);
  if(tmp == Null) {
    head.current.Add(SString(tmpstring));
    exit(-1);
  }
  
  // If tmpstring was recognized then we need to open a new
  //  function call w/ variable array.

  


  // The argument "x" is searched for in the variable list
  // Since it is not found, it is represented as a string

  SString xvar("x");
  ScrVar_Array firstline.argv;
  firstline.argv.Add(xvar);

  


  // Script Setup

  // Look up function "int" and decide to call ScrF_SInt();

  firstline = ScrF_SInt(1,"int x;","int", ST_Void, &firstline.argv);

  head.AddToScript( &firstline);
  
  // Script Excecution

  head.Do();



  // remove firstline from script?


  // Second Line

  // this time the argument "x" is found in the variable listing.
  // a reference (arg1) to it is returned for that slot

  ScrVar_Array secondline.argv;
  secondline.argv.Add(arg1));

  // the expression "5" is determined to be an integer value (arg2)
  // and is returned as an integer

  secondline.argv.Add(arg2);
  
  // Script Setup

  // Look up function "=" and decide to call ScrF_Assign=?;

  secondline = ScrF_Assign(2,"x = 5'","=", ST_Void, &secondlineline.argv);

  head.AddToScript( &secondline);
  
  // Script Excecution

  head.Do();




};




