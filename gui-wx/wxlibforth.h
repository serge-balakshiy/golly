// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXLIBFORTH_H_
#define _WXLIBFORTH_H_

void RunLibForthScript(const wxString& filepath);
// Run the given .fth or .fs or .lf file.

void AbortLibForthScript();
// Abort the currently running LibForth script.

void FinishLibForthScripting();
// Called when app is quitting.

#endif //  _WXLIBFORTH_H_
