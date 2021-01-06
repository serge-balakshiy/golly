// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXKFORTH64_H_
#define _WXKFORTH64_H_

//#include "embed_util.h"
//#include "embed.h"

//void RunKForth64Script(const wxString& filepath);
void RunkForth64Script(const wxString& filepath);
// Run the given .efs or .emb file.

void AbortkForth64Script();
// Abort the currently running KForth64 script.

void FinishkForth64Scripting();
// Called when app is quitting.

#endif // WXKFORTH64_H_
