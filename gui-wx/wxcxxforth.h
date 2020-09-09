// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXCXXFORTH_H_
#define _WXCXXFORTH_H_

void RunCxxForthScript_a(const wxString& filepath);
void RunCxxForthScript(const wxString& filepath);
// Run the given .lua file.

void AbortCxxForthScript();
// Abort the currently running CxxForth script.

void FinishCxxForthScripting();
// Called when app is quitting.

#endif // _WXCXXFORTH_H_
