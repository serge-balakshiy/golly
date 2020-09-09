// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _IP1_H_
#define _IP1_H_

#ifdef __cplusplus
//extern "C" {
#endif

// Run the given .ip1 file.
void RunIP1Script(const wxString & filepath);

// Abort the currently running IP1 script.
void AbortIP1Script();

// Called when app is quitting.
void FinishIP1Scripting();

#ifdef __cplusplus
//} //extern "C"
#endif

#endif //  _IP1_H_
