// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXGFORTH_H_
#define _WXGFORTH_H_

//#include <gforth.h>

void RunGForthScript(const wxString & filepath);
// Run the given .fs file.

void AbortGForthScript();
// Abort the currently running Forth script.

void FinishGForthScripting();
// Called when app is quitting.

#endif //  _WXGFORTH_H_
