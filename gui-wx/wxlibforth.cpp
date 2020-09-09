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
#include "wxlibforth.h"

#include "libforth.h"          // libforth header files for C++

// -----------------------------------------------------------------------------
// some useful macros

#define CHECK_RGB(r,g,b,cmd)                                                \
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {         \
        char msg[128];                                                      \
        sprintf(msg, "%s error: bad rgb value (%d,%d,%d)", cmd, r, g, b);   \
        GollyError(L, msg);                                                 \
    }

#ifdef __WXMAC__
    // use decomposed UTF8 so fopen will work
    #define FILENAME wxString(filename,wxConvUTF8).fn_str()
#else
    #define FILENAME filename
#endif

// use UTF8 for the encoding conversion from LibForth string to wxString
// #define LF_ENC wxConvUTF8

// -----------------------------------------------------------------------------

static bool aborted = false;    // stop the current script?

//static void CheckEvents(lf_State* LF)
//{
    // this routine is called at the start of every g_* function so we can
    // detect user events (eg. hitting the stop button or escape key)

//    if (allowcheck) wxGetApp().Poller()->checkevents();
    
//    if (insideYield) return;
    
    // we're outside Yield so safe to do a longjmp from libforth_error
//    if (aborted) {
        // AbortLibforthScript was called
//        lf_pushstring(L, abortmsg);
//        lf_error(L);
//    }
//}

// -----------------------------------------------------------------------------

//static void GollyError(lf_State* L, const char* errmsg)
//{
    // handle an error detected in a g_* function;
    // note that libforthL_error will prepend file path and line number info
//    lf_error(L, "\n%s", errmsg);
//}

// -----------------------------------------------------------------------------

//static const struct  gollyfuncs [] = {
    // filing
//    { "open",         lf_open },         // open given pattern/script/rule/html file
//    { "save",         lf_save },         // save pattern in given file using given format
//    { "opendialog",   lf_opendialog },   // return input path and filename chosen by user
//    { "savedialog",   lf_savedialog },   // return output path and filename chosen by user
//    { "load",         lf_load },         // read pattern file and return cell array
//    { "store",        lf_store },        // write cell array to a file (in RLE format)
//    { "setdir",       lf_setdir },       // set location of specified directory
//    { "getdir",       lf_getdir },       // return location of specified directory
//    { "getfiles",     lf_getfiles },     // return array of files in specified directory
//    { "getpath",      lf_getpath },      // return the path to the current opened pattern
//    { "getinfo",      lf_getinfo },      // return comments from pattern file
    // editing
//    { "new",          lf_new },          // create new universe and set window title
//    { "cut",          lf_cut },          // cut selection to clipboard
//    { "copy",         lf_copy },         // copy selection to clipboard
//    { "clear",        lf_clear },        // clear inside/outside selection
//    { "paste",        lf_paste },        // paste clipboard pattern at x,y using given mode
//    { "shrink",       lf_shrink },       // shrink selection
//    { "randfill",     lf_randfill },     // randomly fill selection to given percentage
//    { "flip",         lf_flip },         // flip selection top-bottom or left-right
//    { "rotate",       lf_rotate },       // rotate selection 90 deg clockwise or anticlockwise
//    { "parse",        lf_parse },        // parse RLE or Life 1.05 string and return cell array
//    { "transform",    lf_transform },    // apply an affine transformation to cell array
//    { "evolve",       lf_evolve },       // generate pattern contained in given cell array
//    { "putcells",     lf_putcells },     // paste given cell array into current universe
//    { "getcells",     lf_getcells },     // return cell array in given rectangle
    // { "getcells2",     lf_getcells2 },     // experimental version (needs more thought!!!)
//    { "join",         lf_join },         // return concatenation of given cell arrays
//    { "hash",         lf_hash },         // return hash value for pattern in given rectangle
//    { "getclip",      lf_getclip },      // return pattern in clipboard (as wd, ht, cell array)
//    { "select",       lf_select },       // select {x, y, wd, ht} rectangle or remove if {}
//    { "getrect",      lf_getrect },      // return pattern rectangle as {} or {x, y, wd, ht}
//    { "getselrect",   lf_getselrect },   // return selection rectangle as {} or {x, y, wd, ht}
//    { "setcell",      lf_setcell },      // set given cell to given state
//    { "getcell",      lf_getcell },      // get state of given cell
//    { "setcursor",    lf_setcursor },    // set cursor (returns old cursor)
//    { "getcursor",    lf_getcursor },    // return current cursor
    // control
//    { "empty",        lf_empty },        // return true if universe is empty
//    { "run",          lf_run },          // run current pattern for given number of gens
//    { "step",         lf_step },         // run current pattern for current step
//    { "setstep",      lf_setstep },      // set step exponent
//    { "getstep",      lf_getstep },      // return current step exponent
//    { "setbase",      lf_setbase },      // set base step
//    { "getbase",      lf_getbase },      // return current base step
//    { "advance",      lf_advance },      // advance inside/outside selection by given gens
//    { "reset",        lf_reset },        // restore starting pattern
//    { "setgen",       lf_setgen },       // set current generation to given string
//    { "getgen",       lf_getgen },       // return current generation as string
//    { "getpop",       lf_getpop },       // return current population as string
//    { "numstates",    lf_numstates },    // return number of cell states in current universe
//    { "numalgos",     lf_numalgos },     // return number of algorithms
//    { "setalgo",      lf_setalgo },      // set current algorithm using given string
//    { "getalgo",      lf_getalgo },      // return name of given or current algorithm
//    { "setrule",      lf_setrule },      // set current rule using given string
//    { "getrule",      lf_getrule },      // return current rule
//    { "getwidth",     lf_getwidth },     // return width of universe (0 if unbounded)
//    { "getheight",    lf_getheight },    // return height of universe (0 if unbounded)
    // viewing
//    { "setpos",       lf_setpos },       // move given cell to middle of viewport
//    { "getpos",       lf_getpos },       // return x,y position of cell in middle of viewport
//    { "setmag",       lf_setmag },       // set magnification (0=1:1, 1=1:2, -1=2:1, etc)
//    { "getmag",       lf_getmag },       // return current magnification
//    { "fit",          lf_fit },          // fit entire pattern in viewport
//    { "fitsel",       lf_fitsel },       // fit selection in viewport
//    { "visrect",      lf_visrect },      // return true if given rect is completely visible
//    { "setview",      lf_setview },      // set pixel dimensions of viewport
//    { "getview",      lf_getview },      // get pixel dimensions of viewport
//    { "update",       lf_update },       // update display (viewport and status bar)
//    { "autoupdate",   lf_autoupdate },   // update display after each change to universe?
    // layers
//    { "addlayer",     lf_addlayer },     // add a new layer
//    { "clone",        lf_clone },        // add a cloned layer (shares universe)
//    { "duplicate",    lf_duplicate },    // add a duplicate layer (copies universe)
//    { "dellayer",     lf_dellayer },     // delete current layer
//    { "movelayer",    lf_movelayer },    // move given layer to new index
//    { "setlayer",     lf_setlayer },     // switch to given layer
//    { "getlayer",     lf_getlayer },     // return index of current layer
//    { "numlayers",    lf_numlayers },    // return current number of layers
//    { "maxlayers",    lf_maxlayers },    // return maximum number of layers
//    { "setname",      lf_setname },      // set name of given layer
//    { "getname",      lf_getname },      // get name of given layer
//    { "setcolors",    lf_setcolors },    // set color(s) used in current layer
//    { "getcolors",    lf_getcolors },    // get color(s) used in current layer
//    { "overlay",      lf_overlay },      // do an overlay command from a string
//    { "ovtable",      lf_overlaytable }, // do an overlay command from a table
    // miscellaneous
//    { "os",           lf_os },           // return the current OS (Windows/Mac/Linux)
//    { "millisecs",    lf_millisecs },    // return elapsed time since Golly started, in millisecs
//    { "sleep",        lf_sleep },        // sleep for the given number of millisecs
//    { "savechanges",  lf_savechanges },  // show standard save changes dialog and return answer
//    { "settitle",     lf_settitle },     // set window title to given string
//    { "setoption",    lf_setoption },    // set given option to new value (and return old value)
//    { "getoption",    lf_getoption },    // return current value of given option
//    { "setcolor",     lf_setcolor },     // set given color to new r,g,b (returns old r,g,b)
//    { "getcolor",     lf_getcolor },     // return r,g,b values of given color
//    { "setclipstr",   lf_setclipstr },   // set the clipboard contents to a given string value
//    { "getclipstr",   lf_getclipstr },   // retrieve the contents of the clipboard as a string
//    { "getstring",    lf_getstring },    // display dialog box and get string from user
//    { "getxy",        lf_getxy },        // return current grid location of mouse
//    { "getevent",     lf_getevent },     // return keyboard/mouse event or empty string if none
//    { "doevent",      lf_doevent },      // pass given keyboard/mouse event to Golly to handle
//    { "show",         lf_show },         // show given string in status bar
//    { "error",        lf_error },        // beep and show given string in status bar
//    { "warn",         lf_warn },         // show given string in warning dialog
//    { "note",         lf_note },         // show given string in note dialog
//    { "help",         lf_help },         // show given HTML file in help window
//    { "check",        lf_check },        // allow event checking?
//    { "continue",     lf_continue },     // continue executing code after a pcall error
//    { "exit",         lf_exit },         // exit script with optional error message
//    {NULL, NULL}
//};

// -----------------------------------------------------------------------------

#include "wxargs.h"
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
#include "wxlibforth.h"
#include "libforth.h" // for gf_open()

// =============================================================================

void RunLibForthScript(const wxString& filepath){
// Run the given .fth or .fs or .lf file.
  int argc0 = 0;
  char **argv0 = 0;
  char* pchr = (const_cast<char*>((const char*)filepath.mb_str()));

  argv0 = (char**)ArgAddPtr( argv0, argc0, "forth");
  argc0++;

//argv0 = AddPtr( argv0, argc0, "/home/serge/projects/golly-code/Scripts/Forth/test1.fs");
  argv0 = ArgAddPtr( argv0, argc0, pchr);
  argc0++;

    for(int i = 0; i < argc0; i++)    //показываем все строки на экран
      printf("runlibforthscript: %s\n", argv0[i]);
//	  cerr << i << ": "<< argv0[i] << endl; 
//	cerr << endl;

  main_forth(argc0, argv0);
}

void AbortLibForthScript(){
// Abort the currently running LibForth script.
  printf("Abort libforth\n");
}
void FinishLibForthScripting(){
// Called when app is quitting.
  printf("Finish libforth\n");
}
