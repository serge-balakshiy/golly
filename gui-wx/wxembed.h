// This file is part of Golly.
// See docs/License.html for the copyright notice.

#ifndef _WXEMBED_H_
#define _WXEMBED_H_

//#include "embed_util.h"
//#include "embed.h"

//void RunEmbedScript(const wxString& filepath);
void RunEmbedScript(const wxString& filepath);
// Run the given .efs or .emb file.

void AbortEmbedScript();
// Abort the currently running Embed script.

void FinishEmbedScripting();
// Called when app is quitting.

#endif // WXEMBED_H_
