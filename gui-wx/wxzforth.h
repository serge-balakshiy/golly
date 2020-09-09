// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXZFORTH_H_
#define _WXZFORTH_H_

void RunzForthScript(const wxString& filepath);
// Run the given .lua file.

void AbortzForthScript();
// Abort the currently running ZForth script.

void FinishzForthScripting();
// Called when app is quitting.

#endif
