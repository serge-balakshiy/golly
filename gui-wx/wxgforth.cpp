// This file is part of Golly.
// See docs/License.html for the copyright notice

//#include <stdlib.h>
#include <iostream>
#include <string.h>
using namespace std;

char **AddPtr (char **pp, int size, char *str);
char **DelPtr (char **pp, int size, int ncell); // удаление строки и указателя
char **InsPtr (char **pp, int size, int ncell, char *str); //вставка строки и указателя

char **AddPtr (char **pp, int size, char *str)
{
    if(size == 0){
        pp = new char *[size+1];
    }
    else{                      
        char **copy = new char* [size+1];
        for(int i = 0; i < size; i++)
        {
            copy[i] = pp[i];
        }  
 
        delete [] pp;      
        pp = copy;     
    }
    pp[size] = new char [strlen(str) + 1];
    strcpy(pp[size], str);
 
    return pp;
}

char **DelPtr (char **pp, int size, int ncell)
{
    char **copy = new char* [size-1]; //временная копия
        //копируем адреса указателей, кроме ячейки с номером ncell
    for(int i = 0;  i < ncell; i++)
    {
        copy[i] = pp[i];
    }  
    for(int i = ncell;  i < size-1; i++)
    {
        copy[i] = pp[i+1];
    }
 
    delete [] pp[ncell]; // освобождаем память занимаемую строкой
    delete [] pp; //освобождаем память занимаемую массивом указателей
    pp = copy; //показываем, какой адрес хранить
 
    return pp;
}
 
char **InsPtr (char **pp, int size, int ncell, char *str)
{
    char **copy = new char* [size+1]; //временная копия
    for(int i = 0;  i < ncell; i++)
    {
        copy[i] = pp[i];
    }  
 
    copy[ncell] = new char[strlen(str) + 1]; //выделяем память для новой строки
    strcpy(copy[ncell], str);
 
    for(int i = ncell+1;  i < size+1; i++)
    {
        copy[i] = pp[i-1];
    }
    delete [] pp;
    pp = copy; 
 
    return pp;
}

#include <stdio.h>
#include <gforth.h>

#include "wx/wxprec.h"     // for compilers that support precompilation
#ifndef WX_PRECOMP
    #include "wx/wx.h"     // for all others include the necessary headers
#endif

#include "wx/filename.h"   // for wxFileName

#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"
#include "readpattern.h"
#include "writepattern.h"

#include "wxgolly.h"       // for wxGetApp, mainptr, viewptr, statusptr
#include "wxmain.h"        // for mainptr->...
#include "wxselect.h"      // for Selection
#include "wxview.h"        // for viewptr->...
#include "wxstatus.h"      // for statusptr->...
#include "wxutils.h"       // for Warning, Note, GetString, etc
#include "wxprefs.h"       // for perllib, gollydir, etc
#include "wxinfo.h"        // for ShowInfo
#include "wxhelp.h"        // for ShowHelp
#include "wxundo.h"        // for currlayer->undoredo->...
#include "wxalgos.h"       // for *_ALGO, CreateNewUniverse, etc
#include "wxlayer.h"       // for AddLayer, currlayer, currindex, etc
#include "wxscript.h"      // for inscript, abortmsg, GSF_*, etc
#include "wxgforth.h"
//#include "wxgforth1.h" // for gf_open()

// =============================================================================

// #ifdef ENABLE_GFORTH

/*
    Golly uses an embedded GForth interpreter to execute scripts.

*/


// restore wxWidgets definition for _ (from include/wx/intl.h)
#undef _
#define _(s) wxGetTranslation(_T(s))

#if (GFORTH_REVISION == 0) && (GFORTH_SUBVERSION >= 73)
#define GFORTH073_OR_LATER
#endif


// static ForthInterpreter* my_forth = NULL;

// EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);

// =============================================================================

// declare G_* wrappers for the functions we want to use from Perl lib

//void my_gforth_main(){

//char *my_env[] = {"my env"};

//char *argv0[] = { "gforth",  "-e", "3 4 + . bye " };
//  const int argc0 = sizeof(argv0)/sizeof(char*);
//  Cell retvalue;
//  retvalue=gforth_main(argc0, argv0, my_env);


//} // end: void my_gforth_main()


// #endif // ENABLE_GFORTH
// }

// Run the given .fs file.
// Доработать этот 

extern "C" int iadd(int x, int y){
  int z=x+y;
 printf("Sum: %i=%i+%i\n", z, x, y);
 return z;
}

extern "C" void gf_show(){
  inscript = false;
  statusptr->DisplayMessage(wxString("Privet From Golly")); //, ATL_ENC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();

}

extern "C" void gf_hello(void){
  printf("hello from Golly");
}



void RunGforthScript(const wxString& filepath){
char *my_env[] = {"my env"};
cout << "RunGForthScript: ";
gf_hello();

static char *ens[] = {NULL}, **env = &ens[0];
wxPuts (wxT ( "RunGForthScript: A wxWidgets c o n s o l e a p p l i c a t i o n " ) ) ;
wxPuts(filepath);

//const char* fileNameChar = filepath.mb_str();
char* pchr = (const_cast<char*>((const char*)filepath.mb_str()));
//    perl_eval_pv(command.mb_str(wxConvLocal), TRUE);

//int len;
//len = filepath.Length();
//char *ptrVar = (char*)malloc( len * sizeof(char) + 1 );

//char *argv0[] = { "gforth",  "-e", "3 4 + . bye " };
//char *argv0[] = { "gforth",  "/home/serge/projects/golly-code/Scripts/Forth/test1.fs" };

int argc0 = 0;
char **argv0 = 0;
 
argv0 = AddPtr( argv0, argc0, "gforth");
argc0++;

//argv0 = AddPtr( argv0, argc0, "/home/serge/projects/golly-code/Scripts/Forth/test1.fs");
argv0 = AddPtr( argv0, argc0, pchr);
argc0++;

    for(int i = 0; i < argc0; i++)    //показываем все строки на экран
	  cerr << i << ": "<< argv0[i] << endl; 
	cerr << endl;

//	Cell retvalue;
	//retvalue=gforth_main(argc0, argv0, env);

	Cell retvalue;
	retvalue=gforth_start(argc0, argv0);
	if(retvalue == -56) { // success is "quit"
	  gforth_setwinch();
	  gforth_bootmessage();
	  retvalue = gforth_quit();
	}
	gforth_cleanup();
//	return 0;

//my_gforth_main();
 } //end: void RunGForthScript

// Abort the currently running Forth script.
void AbortGforthScript(){
  scripterr = wxString(abortmsg,wxConvLocal);
}

// Called when app is quitting.
void FinishGforthScripting(){
  printf("gforth finish \n");
}

