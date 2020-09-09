// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXATLAST_H_
#define _WXATLAST_H_

void RunAtlastScript_a(const wxString& filepath);
void RunAtlastScript(const wxString& filepath);
// Run the given .lua file.

void AbortAtlastScript();
// Abort the currently running Atlast script.

void FinishAtlastScripting();
// Called when app is quitting.

#endif
