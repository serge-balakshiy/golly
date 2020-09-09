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
#include "wxprefs.h"       // for pythonlib, gollydir, etc
#include "wxhelp.h"        // for ShowHelp
#include "wxundo.h"        // for currlayer->undoredo->...
#include "wxalgos.h"       // for *_ALGO, CreateNewUniverse, etc
#include "wxlayer.h"       // for AddLayer, currlayer, currindex, etc
#include "wxscript.h"      // for inscript, abortmsg, GSF_*, etc
#include "wxpython.h"

// =============================================================================
void RunPythonScript(const wxString& filepath)
{
printf("this plug");   
// if (!InitPython()) return;
    
    // we must convert any backslashes to "\\" to avoid "\a" being treated as
    // escape char, then we must escape any apostrophes
    //wxString fpath = filepath;
//    fpath.Replace(wxT("\\"), wxT("\\\\"));
//    fpath.Replace(wxT("'"), wxT("\\'"));
    
    // execute the given script; note that we pass an empty dictionary
    // for the global namespace so that this script cannot change the
    // globals of a caller script (which is possible now that RunScript
    // is re-entrant)
//    wxString command = wxT("with open('") + fpath +
//        wxT("') as f:\n   code = compile(f.read(), '") + fpath +
//        wxT("', 'exec')\n   exec(code, {})");

//    PyRun_SimpleString(command.mb_str(wxConvLocal));
    // don't use wxConvUTF8 in above line because caller has already converted
    // filepath to decomposed UTF8 if on a Mac
    
    // note that PyRun_SimpleString returns -1 if an exception occurred;
    // the error message (in scripterr) is checked at the end of RunScript
}

// -----------------------------------------------------------------------------

void AbortPythonScript()
{
printf("this plug");   
    // raise an exception with a special message
//    PyErr_SetString(PyExc_KeyboardInterrupt, abortmsg);
}

// -----------------------------------------------------------------------------

void FinishPythonScripting()
{
printf("this plug");   
    // Py_Finalize can cause an obvious delay, so best not to call it
    // if (pyinited) Py_Finalize();
    
    // probably don't really need this either
//    FreePythonLib();
}
