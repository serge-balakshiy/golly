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
#define KF64_BNC  wxConvUTF8
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


extern "C" int CPP_gprivet(){
cout << "Privet from Golly!" << endl;
return 0;
}

extern "C" int CPP_gllshow(){
  inscript = false;
  statusptr->DisplayMessage( wxString( "Privet from kForth64!", KF64_BNC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();
  return 0;
}

extern "C" int kf64CheckEvents(){
  if (allowcheck) wxGetApp().Poller()->checkevents();
  if (insideYield) return 0;
  if (isaborted){
    cout << "kf64:checkexents error" << endl;
  }
}

extern "C" int kf64_rotate(){
kf64CheckEvents();
int direction = 1;
if (viewptr->SelectionExists()){
viewptr->RotateSelection(direction == 0);
DoAutoUpdate();
return 0;
}else{
return -1;
//throw 
}
}

//extern "C" int gop_show(){
//cout << "show from kForth64!" << endl;
//}

//WordTemplate GollyWords[] = {
//			     { "PRIVET", gop_privet, 0 },
//			     { "SHOW", gop_show, 0 },
//};

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

    if (argc0 < 2) {
	cout << "kForth-64 v " << version << "\t (Build: " << build << ")" << endl;
	cout << "Copyright (c) 1998--2020 Krishna Myneni" << endl;
        cout << "Contributions by: dpw gd mu bk abs tn cmb bg dnw imss" << endl;
	cout << "Provided under the GNU Affero General Public License, v3.0 or later" 
	  << endl << endl;
    }
    else {
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
    }
    else
        cout << "\nReady!\n";

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
