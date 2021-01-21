#include "wx/wxprec.h"      // for compilers that support precompilation
#ifndef WX_PRECOMP
    #include "wx/wx.h"      // for all others include the necessary headers
#endif

#include "wx/filename.h"    // for wxFileName
#include "wx/dir.h"         // for wxDir

#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"
#include "readpattern.h"
#include "writepattern.h"

#include "wxgolly.h"        // for wxGetApp, mainptr, viewptr, statusptr, stopwatch
#include "wxmain.h"         // for mainptr->...
#include "wxselect.h"       // for Selection
#include "wxview.h"         // for viewptr->...
#include "wxstatus.h"       // for statusptr->...
#include "wxutils.h"        // for Warning, Note, GetString, SaveChanges, etc
#include "wxprefs.h"        // for gollydir, etc
#include "wxinfo.h"         // for ShowInfo
#include "wxhelp.h"         // for ShowHelp
#include "wxundo.h"         // for currlayer->undoredo->...
#include "wxalgos.h"        // for *_ALGO, CreateNewUniverse, etc
#include "wxlayer.h"        // for AddLayer, currlayer, currindex, etc
#include "wxoverlay.h"      // for curroverlay->...
#include "wxscript.h"       // for inscript, abortmsg, GSF_*, etc
#include "wxkforth64.h"
#include <time.h>


// #include "lua.hpp"          // Lua header files for C++

#ifdef VERSION
const char* version="01-00"; //VERSION;
#else
const char* version="?";
#endif
const char* build="31.12.20"; //BUILD_DATE;


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

#include "wxargs.h"
//#include "kforth64.h"
//#include "kforth64_util.h"
//#include <errno.h>
//#include <stdbool.h>
//#include <assert.h>
//#include <string.h>
//#include <stdlib.h>
//#include <math.h>
//#include <ctype.h>
//#include <stdio.h>
#define KF64FORTH_ENC  wxConvUTF8
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
}

#include "fbc.h"
#include "ForthCompiler.h"
#include "ForthVM.h"
#include "VMerrors.h"
#include "kfmacros.h"

extern vector<WordList> Dictionary;
extern "C" long int* JumpTable;
extern "C" long int* BottomOfStack;
extern "C" long int* BottomOfReturnStack;
extern "C" char TIB[];
extern "C" {
  void set_start_mem();
  void echo_on(void);
  void echo_off(void);
}
int debug = 0;

#define kforth64_fatal(...)   do { } while(0) //kforth64_die()
#define kforth64_error(...)   do { } while(0)
#define kforth64_warning(...) do { } while(0)
#define kforth64_info(...)    do { } while(0)
#define kforth64_debug(...)   do { } while(0)

extern "C" long int* GlobalSp;
#define POP (++GlobalSp)
extern "C" long int* BottomOfStack;
extern "C" byte* GlobalTp;

using Cell = uintptr_t;
using Char = unsigned char;
using CAddr = Char*;

#define CELL(x)reinterpret_cast<Cell>(x)
#define CADDR(x) reinterpret_cast<Char*>(x)

extern vector<char*> StringTable;
extern "C" char* pTIB;
extern vector<byte>* pCurrentOps;

class AbortException: public runtime_error {
public:
explicit AbortException(const string& msg): runtime_error(msg) {}
explicit AbortException(const char* msg): runtime_error(msg) {}
explicit AbortException(const char* caddr, size_t count): runtime_error(string(caddr, count)) {}
};
void kf64_abort(){
throw AbortException("");
}

int kf64CheckEvents(){
  if (allowcheck) {
    wxGetApp().Poller()->checkevents();
  }
  if (insideYield) return 0;
  if (isaborted){
    //cout << "kf64:checkexents error" << endl;
    return -1;
  }
}

extern "C" int CPP_gprivet(){
cout << "CPP_gprivet: Privet from Golly!" << endl;
return 0;
}

extern "C" int CPP_gllshow(){
  kf64CheckEvents();
  if (GlobalSp > BottomOfStack){
    return E_V_STK_UNDERFLOW;
  }
  auto length = *((int*)(POP));
  auto caddr = (char*)*(GlobalSp + 1); //TOP  
  DROP
    bool get = true;

  inscript = false;
//  statusptr->DisplayMessage( wxString( "Privet from kForth64!", KF64FORTH_ENC));
  statusptr->DisplayMessage( wxString( caddr, KF64FORTH_ENC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();
  return 0;
}

extern "C" int CPP_G_kf64rotate(){
  cout << "CPP_G_kf64rotate: Rotate from Golly!" << endl;
  kf64CheckEvents();
  int direction = 1;
  cout << "CPP_G_kf64rotate: step 01" << endl;
  if (viewptr->SelectionExists()){
    cout << "CPP_G_kf64rotate: step 02" << endl;
    viewptr->RotateSelection(direction == 0);
    cout << "CPP_G_kf64rotate: step 03" << endl;
    DoAutoUpdate();
    cout << "CPP_G_kf64rotate: step 04" << endl;
    return 0;
  }
  cout << "CPP_G_kf64rotate: step 05" << endl;
  throw AbortException("not selected");
  //return 0;
}

extern "C" void CPP_G_kf64update(){
  kf64CheckEvents();
  GSF_update();


}

extern "C" int CPP_G_kf64getcell(){ // ( y x -- n = state )
  kf64CheckEvents();
  cout << "GlobalSp = " << GlobalSp << endl;
  //int x = *((int*)(++GlobalSp));
  int y = *((int*)(POP));
  cout << "POP GlobalSp = " << GlobalSp << endl;
  //int y = *((int*)(++GlobalSp));
  int x = *((int*)(POP));
  cout << "x = " << x << ", y = " << y << ", POP GlobalSp = " << GlobalSp << endl;
  const char* err = GSF_checkpos(currlayer->algo, x, y);
  if (err){
    throw AbortException("getcell: error: checkpos.");
  }else{
    *GlobalSp = currlayer->algo->getcell(x,y) ; // PUSH
    --GlobalSp;
    cout << "Push GlobalSp = " << GlobalSp << endl;
  }

  return 0;
}

extern "C" int CPP_G_kf64setcell(){
  kf64CheckEvents();
  const char* err;
  auto setcell_state = *((int*)(POP));
  auto y = *((int*)(POP));
  auto x = *((int*)(POP));
  err = GSF_setcell( x, y, setcell_state);
  if (err == NULL){
    return 0;
  }else{
    throw AbortException( err );
  }
}


extern "C" int CPP_G_kf64getcursor(){
  kf64CheckEvents();
  string s = CursorToString(currlayer->curs);
  long int ptr = *(GlobalSp + 1); //TOP  
  char* pstr = (char*)ptr;
  int nc = s.length();
  cout << "str =" << ptr << endl;
  pstr[0] = nc; pstr++;
  strncpy(pstr, s.c_str(), nc);
  pstr[nc+2] = '\0';
  cout << "str =" << ptr << ", cursor=" << pstr << endl;
  return 0;
}

extern "C" int CPP_G_kf64getevent(){ //( c-addr u1 -- c-addr u2 )
  kf64CheckEvents();
if (GlobalSp > BottomOfStack){
return E_V_STK_UNDERFLOW;
}
auto length = *((int*)(POP));
auto caddr = (char*)*(GlobalSp + 1); //TOP  
bool get = true;
wxString event;
string ss;
GSF_getevent(event, get ? 1 : 0);
// 
inscript = false;
statusptr->DisplayMessage(event);
inscript = true;
if (!showstatus) mainptr->ToggleStatusBar();
//
ss = (const char*)event.mb_str(KF64FORTH_ENC);

const char* cstr = event.mb_str(KF64FORTH_ENC);
//cout << "wxstr event:" << event.mb_str(KF64FORTH_ENC) << ", wxstr length " << event.length() << endl;
if (ss.length() > 0){
cout << "ss event:" << (char*)ss.c_str() << ", length " << ss.length() << endl;
cout << "cstr event:" << cstr << ", length " << endl;
}
strcpy((char*)caddr, cstr); //ss.data());
    *GlobalSp = (long int)event.length() ; // PUSH
    --GlobalSp;
return 0;
}

extern "C" int CPP_G_kf64getxy_01(){
  kf64CheckEvents();
if (GlobalSp > BottomOfStack){
return E_V_STK_UNDERFLOW;
}

long int n = TOS;
// надо разбираться с TOS
byte *p = new (nothrow) byte[48];
PUSH_ADDR( (long int) p )

return 0;
}

extern "C" int CPP_G_kf64getxy(){
  kf64CheckEvents();
  if (GlobalSp > BottomOfStack){
    return E_V_STK_UNDERFLOW;
  }
  auto length = *((int*)(POP));
  auto caddr = (char*)*(GlobalSp + 1); //TOP  
  bool get = true;

  wxString event;  //event;
  string ss;

  statusptr->CheckMouseLocation( mainptr->infront); // sets mousepos

  if ( viewptr->showcontrols ) mousepos = wxEmptyString;

  //cout << "mousepos " << mousepos.data() << endl;
  ss =  (const char*)mousepos.mb_str(KF64FORTH_ENC);
  //cout << "getxy:" << ss.data() << ", length " << ss.length() << endl;

  const char* cstr = mousepos.mb_str(KF64FORTH_ENC);
  strcpy((char*)caddr, cstr); //ss.data());
  *GlobalSp = (long int)mousepos.length() ; // PUSH
  --GlobalSp;
//PUSH_IVAL((long int)mousepos.length())  
return 0;
}

void RunkForth64Script(const wxString& filepath){
  char t[132];
  char **ArgAddPtr (char **pp, int size, char *str);
  
  int argc0 = 0;
  char **argv0 = 0;
  char* pchr = (const_cast<char*>((const char*)filepath.mb_str()));
  argv0 = ArgAddPtr( argv0, argc0, "kforth64");
  argc0++;
  printf("argc0=%d\n", argc0);
//  argv0 = ArgAddPtr( argv0, argc0, "-i");
//  argc0++;
//  argv0 = ArgAddPtr( argv0, argc0, "/home/serge/projects/gh/golly/Scripts/Embed/embed.blk");
//  argc0++;
  argv0 = ArgAddPtr( argv0, argc0, pchr);
  argc0++;

  for ( int i=0; i < argc0; i++){
    printf( "i=%d, %s\n", i, argv0[i]);
  }

//  CPP_privet();

  ostringstream initial_commands ( ostringstream::out);
  istringstream* pSS = NULL;
  const char* prompt = " ok\n";
  set_start_mem();
  int nWords = OpenForth();
  try{
    if (argc0 < 2) {
	cout << "kForth-64 v " << version << "\t (Build: " << build << ")" << endl;
	cout << "Copyright (c) 1998--2020 Krishna Myneni" << endl;
        cout << "Contributions by: dpw gd mu bk abs tn cmb bg dnw imss" << endl;
	cout << "Provided under the GNU Affero General Public License, v3.0 or later" 
	  << endl << endl;
    }else {
      int i = 1;
      
      while (i < argc0) {
	if (!strcmp(argv0[i], "-D")) {
	  debug = -1;
	}
	else if (!strcmp(argv0[i], "-e")) {
	  if (argc0 > i) {
	    initial_commands << argv0[i+1] << endl; 
	  }
	  ++i;
	}
	else {
	  initial_commands << "include " << argv0[i] << endl;
	}
	++i;
      }
      pSS = new istringstream(initial_commands.str());
    }

    if (debug) {
      cout << '\n' << nWords << " words defined.\n";
      cout << "Jump Table address:  " << &JumpTable << endl;
      cout << "Bottom of Stack:     " << BottomOfStack << endl;
      cout << "Bottom of Ret Stack: " << BottomOfReturnStack << endl;
    }

    SetForthOutputStream (cout);
    long int line_num = 0, ec = 0;
    vector<byte> op;
    
    if (pSS) {
      SetForthInputStream(*pSS);
      ec = ForthCompiler (&op, &line_num);
      if (ec) {
        PrintVM_Error(ec); exit(ec);
      }
      delete pSS; pSS = NULL;
      op.erase(op.begin(), op.end());
      cout << prompt ;
    }else{
      cout << "\nReady!\n";
    }
  }
  catch(AbortException){
    std::cerr << "main caught int exception\n";

  }
//----------------  the interpreter main loop

  ClearControlStacks();

/***
// Дальнейшее отключаю, так как нужно только обработать
// и выполнить скрипт

    char s[256], *input_line;

    while (1) {
        // Obtain commands and execute
      cout << "main loop" << endl;
        do {
	    if ((input_line = readline(NULL)) == NULL) CPP_bye();
	    if (strlen(input_line)) add_history(input_line);
	    strncpy(s, input_line, 255);
	    free(input_line);
	       
            pSS = new istringstream(s);
	    SetForthInputStream (*pSS);
	    echo_off();
            ec = ForthCompiler (&op, &line_num);
	    echo_on();
	    delete pSS;
	    pSS = NULL;

        } while (ec == E_V_END_OF_STREAM) ; // test for premature end of input
                                            //   that spans multiple lines
        if (ec) {
	    cout << "Line " << line_num << ": "; PrintVM_Error(ec);
	    cout << TIB << endl;
        }
	else
	    cout << prompt;
        op.erase(op.begin(), op.end());
    }
***/

}

void AbortkForth64Script(){
//  aborted = true;
  scripterr = wxString(abortmsg, wxConvLocal);
}

void FinishkForth64Scripting(){
  // no need to do anything
  printf("kForth64 finish \n");
}
