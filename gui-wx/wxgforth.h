// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXGFORTH_H_
#define _WXGFORTH_H_

//#include <gforth.h>

void RunGforthScript(const wxString & filepath);
// Run the given .fs file.

void AbortGforthScript();
// Abort the currently running Forth script.

void FinishGforthScripting();
// Called when app is quitting.

#endif //  _WXGFORTH_H_
