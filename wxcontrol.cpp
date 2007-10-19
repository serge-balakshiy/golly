                        /*** /

This file is part of Golly, a Game of Life Simulator.
Copyright (C) 2007 Andrew Trevorrow and Tomas Rokicki.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 Web site:  http://sourceforge.net/projects/golly
 Authors:   rokicki@gmail.com  andrew@trevorrow.com

                        / ***/

#include "wx/wxprec.h"     // for compilers that support precompilation
#ifndef WX_PRECOMP
   #include "wx/wx.h"      // for all others include the necessary headers
#endif

#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"

#include "wxgolly.h"       // for wxGetApp, statusptr, viewptr, bigview
#include "wxutils.h"       // for BeginProgress, GetString, etc
#include "wxprefs.h"       // for maxhashmem, allowundo, etc
#include "wxrule.h"        // for ChangeRule
#include "wxstatus.h"      // for statusptr->...
#include "wxview.h"        // for viewptr->...
#include "wxscript.h"      // for inscript, PassKeyToScript
#include "wxmain.h"        // for MainFrame
#include "wxundo.h"        // for undoredo->...
#include "wxlayer.h"       // for currlayer, etc

// Control menu functions:

// -----------------------------------------------------------------------------

bool reset_pending = false;      // user selected Reset while generating?
bool hash_pending = false;       // user selected Use Hashing while generating?

// -----------------------------------------------------------------------------

bool MainFrame::SaveStartingPattern()
{
   if ( currlayer->algo->getGeneration() > currlayer->startgen ) {
      // don't do anything if current gen count > starting gen
      return true;
   }
   
   // save current rule, dirty flag, scale, location, etc
   currlayer->startname = currlayer->currname;
   currlayer->startrule = wxString(currlayer->algo->getrule(), wxConvLocal);
   currlayer->startdirty = currlayer->dirty;
   currlayer->startmag = viewptr->GetMag();
   viewptr->GetPos(currlayer->startx, currlayer->starty);
   currlayer->startwarp = currlayer->warp;
   currlayer->starthash = currlayer->hash;
   
   // if this layer is a clone then save some settings in other clones
   if (currlayer->cloneid > 0) {
      for ( int i = 0; i < numlayers; i++ ) {
         Layer* cloneptr = GetLayer(i);
         if (cloneptr != currlayer && cloneptr->cloneid == currlayer->cloneid) {
            cloneptr->startname = cloneptr->currname;
            cloneptr->startx = cloneptr->view->x;
            cloneptr->starty = cloneptr->view->y;
            cloneptr->startmag = cloneptr->view->getmag();
            cloneptr->startwarp = cloneptr->warp;
         }
      }
   }
   
   // save current selection
   currlayer->starttop = currlayer->seltop;
   currlayer->startleft = currlayer->selleft;
   currlayer->startbottom = currlayer->selbottom;
   currlayer->startright = currlayer->selright;
   
   if ( !currlayer->savestart ) {
      // no need to save pattern; ResetPattern will load currfile
      currlayer->startfile.Clear();
      return true;
   }

   // save starting pattern in tempstart file
   if ( currlayer->hash ) {
      // much faster to save hlife pattern in a macrocell file
      const char* err = WritePattern(currlayer->tempstart, MC_format, 0, 0, 0, 0);
      if (err) {
         statusptr->ErrorMessage(wxString(err,wxConvLocal));
         // don't allow user to continue generating
         return false;
      }
   } else {
      // can only save qlife pattern if edges are within getcell/setcell limits
      bigint top, left, bottom, right;
      currlayer->algo->findedges(&top, &left, &bottom, &right);      
      if ( viewptr->OutsideLimits(top, left, bottom, right) ) {
         statusptr->ErrorMessage(_("Starting pattern is outside +/- 10^9 boundary."));
         // don't allow user to continue generating
         return false;
      }
      int itop = top.toint();
      int ileft = left.toint();
      int ibottom = bottom.toint();
      int iright = right.toint();      
      // use XRLE format so the pattern's top left location and the current
      // generation count are stored in the file
      const char* err = WritePattern(currlayer->tempstart, XRLE_format,
                                     itop, ileft, ibottom, iright);
      if (err) {
         statusptr->ErrorMessage(wxString(err,wxConvLocal));
         // don't allow user to continue generating
         return false;
      }
   }
   
   currlayer->startfile = currlayer->tempstart;   // ResetPattern will load tempstart
   return true;
}

// -----------------------------------------------------------------------------

void MainFrame::ResetPattern(bool resetundo)
{
   if (currlayer->algo->getGeneration() == currlayer->startgen) return;
   
   if (generating) {
      // terminate GeneratePattern loop and set reset_pending flag
      Stop();
      reset_pending = true;
      /* can't use wxPostEvent here because Yield processes all pending events
      // send Reset command to event queue
      wxCommandEvent resetevt(wxEVT_COMMAND_MENU_SELECTED, GetID_RESET());
      wxPostEvent(this->GetEventHandler(), resetevt);
      */
      return;
   }
   
   if (currlayer->algo->getGeneration() < currlayer->startgen) {
      // if this happens then startgen logic is wrong
      Warning(_("Current gen < starting gen!"));
      return;
   }
   
   if (currlayer->startfile.IsEmpty() && currlayer->currfile.IsEmpty()) {
      // if this happens then savestart logic is wrong
      Warning(_("Starting pattern cannot be restored!"));
      return;
   }
   
   if (allowundo && !currlayer->stayclean && inscript) {
      // script called reset()
      SavePendingChanges();
      currlayer->undoredo->RememberGenStart();
   }
   
   // restore pattern and settings saved by SaveStartingPattern;
   // first restore step size, hashing option and starting pattern
   currlayer->warp = currlayer->startwarp;
   currlayer->hash = currlayer->starthash;

   if ( !currlayer->startfile.IsEmpty() ) {
      // restore pattern from startfile
      LoadPattern(currlayer->startfile, wxEmptyString);
   } else {
      // restore pattern from currfile
      LoadPattern(currlayer->currfile, wxEmptyString);
   }
   // gen count has been reset to startgen
   
   // ensure savestart flag is correct
   currlayer->savestart = !currlayer->startfile.IsEmpty();
   
   // restore settings saved by SaveStartingPattern
   currlayer->currname = currlayer->startname;
   currlayer->algo->setrule(currlayer->startrule.mb_str(wxConvLocal));
   currlayer->dirty = currlayer->startdirty;
   viewptr->SetPosMag(currlayer->startx, currlayer->starty, currlayer->startmag);

   // if this layer is a clone then restore some settings in other clones
   if (currlayer->cloneid > 0) {
      for ( int i = 0; i < numlayers; i++ ) {
         Layer* cloneptr = GetLayer(i);
         if (cloneptr != currlayer && cloneptr->cloneid == currlayer->cloneid) {
            cloneptr->currname = cloneptr->startname;
            cloneptr->view->setpositionmag(cloneptr->startx, cloneptr->starty,
                                           cloneptr->startmag);
            cloneptr->warp = cloneptr->startwarp;
            // also synchronize dirty flags and update items in Layer menu
            cloneptr->dirty = currlayer->dirty;
            mainptr->UpdateLayerItem(i);
         }
      }
   }

   // restore selection
   currlayer->seltop = currlayer->starttop;
   currlayer->selleft = currlayer->startleft;
   currlayer->selbottom = currlayer->startbottom;
   currlayer->selright = currlayer->startright;   

   // update window title in case currname, rule or dirty flag changed;
   // note that UpdateLayerItem(currindex) gets called
   SetWindowTitle(currlayer->currname);
   UpdateEverything();
   
   if (allowundo && !currlayer->stayclean) {
      if (inscript) {
         // script called reset() so remember gen change
         currlayer->undoredo->RememberGenFinish();
      } else if (resetundo) {
         // wind back the undo history to the starting pattern
         currlayer->undoredo->SyncUndoHistory();
      }
   }
}

// -----------------------------------------------------------------------------

void MainFrame::RestorePattern(bigint& gen, const wxString& filename, const wxString& rule,
                               bigint& x, bigint& y, int mag, int warp, bool hash)
{
   // called to undo/redo a generating change
   if (gen == currlayer->startgen) {
      // restore starting pattern
      ResetPattern(false);                // false == don't call SyncUndoHistory
   } else {
      // restore pattern in filename along with given settings
      currlayer->warp = warp;
      // only update status bar if hashing state has changed
      bool updatestatus = currlayer->hash != hash;
      currlayer->hash = hash;

      LoadPattern(filename, wxEmptyString, updatestatus);
      
      if (gen != currlayer->algo->getGeneration()) {
         // current gen will be 0 if filename could not be loaded
         // for some reason, so best to set correct gen count
         currlayer->algo->setGeneration(gen);
      }

      // no need for setrule here??? readpattern should do it when loading file
      // currlayer->algo->setrule(rule.mb_str(wxConvLocal));
      wxString r = wxString(currlayer->algo->getrule(), wxConvLocal);
      if (r != rule)
         Warning(wxString::Format(_("Rules differ: %s != %s"), r.c_str(), rule.c_str()));
      
      viewptr->SetPosMag(x, y, mag);
      SetWindowTitle(wxEmptyString);      // in case rule changed
      UpdatePatternAndStatus();
   }
}

// -----------------------------------------------------------------------------

const char* MainFrame::ChangeGenCount(const char* genstring, bool inundoredo)
{
   // disallow alphabetic chars in genstring
   for (unsigned int i = 0; i < strlen(genstring); i++)
      if ( (genstring[i] >= 'a' && genstring[i] <= 'z') ||
           (genstring[i] >= 'A' && genstring[i] <= 'Z') )
         return "Alphabetic character is not allowed in generation string.";
   
   bigint oldgen = currlayer->algo->getGeneration();
   bigint newgen(genstring);

   if (genstring[0] == '+' || genstring[0] == '-') {
      // leading +/- sign so make newgen relative to oldgen
      bigint relgen = newgen;
      newgen = oldgen;
      newgen += relgen;
      if (newgen < bigint::zero) newgen = bigint::zero;
   }
   
   if (newgen == oldgen) return NULL;

   if (!inundoredo && allowundo && !currlayer->stayclean && inscript) {
      // script called setgen()
      SavePendingChanges();
   }

   if (!currlayer->hash && newgen.odd() != oldgen.odd()) {
      // qlife stores pattern in different bits depending on gen parity,
      // so we need to create a new qlife universe, set its gen, copy the
      // current pattern to the new universe, then switch to that universe
      bigint top, left, bottom, right;
      currlayer->algo->findedges(&top, &left, &bottom, &right);
      if ( viewptr->OutsideLimits(top, left, bottom, right) ) {
         return "Pattern is too big to copy.";
      }
      // create a new universe of same type
      lifealgo* newalgo = CreateNewUniverse(currlayer->hash);
      newalgo->setGeneration(newgen);
      // copy pattern
      if ( !viewptr->CopyRect(top.toint(), left.toint(), bottom.toint(), right.toint(),
                              currlayer->algo, newalgo, false, _("Copying pattern")) ) {
         delete newalgo;
         return "Failed to copy pattern.";
      }
      // switch to new universe
      delete currlayer->algo;
      currlayer->algo = newalgo;
      SetGenIncrement();
   } else {
      currlayer->algo->setGeneration(newgen);
   }
   
   if (!inundoredo) {
      // save some settings for RememberSetGen below
      bigint oldstartgen = currlayer->startgen;
      bool oldsave = currlayer->savestart;
      
      // may need to change startgen and savestart
      if (oldgen == currlayer->startgen || newgen <= currlayer->startgen) {
         currlayer->startgen = newgen;
         currlayer->savestart = true;
      }
   
      if (allowundo && !currlayer->stayclean) {
         currlayer->undoredo->RememberSetGen(oldgen, newgen, oldstartgen, oldsave);
      }
   }
   
   UpdateStatus();
   return NULL;
}

// -----------------------------------------------------------------------------

void MainFrame::SetGeneration()
{
   bigint oldgen = currlayer->algo->getGeneration();
   wxString result;
   wxString prompt = _("Enter a new generation count:");
   prompt += _("\n(+n/-n is relative to current count)");
   if ( GetString(_("Set Generation"), prompt,
                  wxString(oldgen.tostring(), wxConvLocal), result) ) {

      const char* err = ChangeGenCount(result.mb_str(wxConvLocal));
      
      if (err) {
         Warning(wxString(err,wxConvLocal));
      } else {
         // Reset/Undo/Redo items might become enabled or disabled
         // (we need to do this if user clicked "Generation=..." text)
         UpdateMenuItems(IsActive());
      }
   }
}

// -----------------------------------------------------------------------------

void MainFrame::GoFaster()
{
   currlayer->warp++;
   SetGenIncrement();
   // only need to refresh status bar
   UpdateStatus();
   if (generating && currlayer->warp < 0) {
      whentosee -= statusptr->GetCurrentDelay();
   }
}

// -----------------------------------------------------------------------------

void MainFrame::GoSlower()
{
   if (currlayer->warp > minwarp) {
      currlayer->warp--;
      SetGenIncrement();
      // only need to refresh status bar
      UpdateStatus();
      if (generating && currlayer->warp < 0) {
         if (currlayer->warp == -1) {
            // need to initialize whentosee rather than increment it
            whentosee = stopwatch->Time() + statusptr->GetCurrentDelay();
         } else {
            whentosee += statusptr->GetCurrentDelay();
         }
      }
   } else {
      wxBell();
   }
}

// -----------------------------------------------------------------------------

void MainFrame::DisplayPattern()
{
   // this routine is only used by GeneratePattern();
   // it's similar to UpdatePatternAndStatus() but if tiled windows exist
   // it only updates the current tile if possible; ie. it's not a clone
   // and tile views aren't synchronized
   
   if (!IsIconized()) {
      if (tilelayers && numlayers > 1 && !syncviews && currlayer->cloneid == 0) {
         // only update the current tile
         viewptr->Refresh(false);
         viewptr->Update();
      } else {
         // update main viewport window, possibly including all tile windows
         // (tile windows are children of bigview)
         bigview->Refresh(false);
         bigview->Update();
      }
      if (showstatus) {
         statusptr->CheckMouseLocation(IsActive());
         statusptr->Refresh(false);
         statusptr->Update();
      }
   }
}

// -----------------------------------------------------------------------------

void MainFrame::GeneratePattern()
{
   if (generating || viewptr->drawingcells || viewptr->waitingforclick) {
      wxBell();
      return;
   }

   lifealgo* curralgo = currlayer->algo;
   if (curralgo->isEmpty()) {
      statusptr->ErrorMessage(empty_pattern);
      return;
   }
   
   if (!SaveStartingPattern()) {
      return;
   }
      
   // GeneratePattern is never called while running a script so no need
   // to test inscript or currlayer->stayclean
   if (allowundo) currlayer->undoredo->RememberGenStart();

   // for DisplayTimingInfo
   begintime = stopwatch->Time();
   begingen = curralgo->getGeneration().todouble();
   
   generating = true;               // avoid recursion
   wxGetApp().PollerReset();
   UpdateUserInterface(IsActive());
   
   if (currlayer->warp < 0) {
      whentosee = stopwatch->Time() + statusptr->GetCurrentDelay();
   }
   int hypdown = 64;

   while (true) {
      if (currlayer->warp < 0) {
         // slow down by only doing one gen every GetCurrentDelay() millisecs
         long currmsec = stopwatch->Time();
         if (currmsec >= whentosee) {
            if (wxGetApp().Poller()->checkevents()) break;
            curralgo->step();
            if (currlayer->autofit) viewptr->FitInView(0);
            DisplayPattern();
            // add delay to current time rather than currmsec
            whentosee = stopwatch->Time() + statusptr->GetCurrentDelay();
         } else {
            // process events while we wait
            if (wxGetApp().Poller()->checkevents()) break;
            // don't hog CPU
            wxMilliSleep(1);     // keep small (ie. <= mindelay)
         }
      } else {
         // warp >= 0 so only show results every curralgo->getIncrement() gens
         if (wxGetApp().Poller()->checkevents()) break;
         curralgo->step();
         if (currlayer->autofit) viewptr->FitInView(0);
         DisplayPattern();
         if (currlayer->hyperspeed && curralgo->hyperCapable()) {
            hypdown--;
            if (hypdown == 0) {
               hypdown = 64;
               GoFaster();
            }
         }
      }
   }

   generating = false;

   // for DisplayTimingInfo
   endtime = stopwatch->Time();
   endgen = curralgo->getGeneration().todouble();
   
   // display the final pattern
   if (currlayer->autofit) viewptr->FitInView(0);
   if (reset_pending || hash_pending) {
      // UpdateEverything will be called at end of ResetPattern/ToggleHashing
   } else {
      UpdateEverything();
   }
   
   if (hash_pending) {
      hash_pending = false;
      
      // temporarily pretend the tool/layer bars are not showing
      // to avoid UpdateToolBar/UpdateLayerBar flashing their buttons
      bool saveshowtool = showtool;    showtool = false;
      bool saveshowlayer = showlayer;  showlayer = false;

      // temporarily set allowundo false so we don't remember hashing changes
      // while generating (RememberGenFinish below will do that)
      bool saveallowundo = allowundo;  allowundo = false;
      
      ToggleHashing();
      
      // restore tool/layer bar flags and allowundo
      showtool = saveshowtool;
      showlayer = saveshowlayer;
      allowundo = saveallowundo;
      
      // send Go command to event queue to call GeneratePattern again
      wxCommandEvent goevt(wxEVT_COMMAND_MENU_SELECTED, GetID_GO());
      wxPostEvent(this->GetEventHandler(), goevt);
   }

   // GeneratePattern is never called while running a script so no need
   // to test inscript or currlayer->stayclean
   if (allowundo) currlayer->undoredo->RememberGenFinish();

   if (reset_pending) {
      reset_pending = false;
      ResetPattern();
   }
}

// -----------------------------------------------------------------------------

void MainFrame::Stop()
{
   if (inscript) {
      PassKeyToScript(WXK_ESCAPE);
   } else if (generating) {
      wxGetApp().PollerInterrupt();
   }
}

// -----------------------------------------------------------------------------

void MainFrame::DisplayTimingInfo()
{
   if (viewptr->waitingforclick) return;
   if (generating) {
      endtime = stopwatch->Time();
      endgen = currlayer->algo->getGeneration().todouble();
   }
   if (endtime > begintime) {
      double secs = (double)(endtime - begintime) / 1000.0;
      double gens = endgen - begingen;
      wxString s;
      s.Printf(_("%g gens in %g secs (%g gens/sec)"), gens, secs, gens / secs);
      statusptr->DisplayMessage(s);
   }
}

// -----------------------------------------------------------------------------

void MainFrame::NextGeneration(bool useinc)
{
   if (generating || viewptr->drawingcells || viewptr->waitingforclick) {
      // don't play sound here because it'll be heard if user holds down tab key
      // wxBell();
      return;
   }

   lifealgo* curralgo = currlayer->algo;
   if (curralgo->isEmpty()) {
      statusptr->ErrorMessage(empty_pattern);
      return;
   }
   
   if (!SaveStartingPattern()) {
      return;
   }

   if (allowundo) {
      if (currlayer->stayclean) {
         // script has called run/step after a command (eg. new)
         // has set stayclean true by calling MarkLayerClean
         if (curralgo->getGeneration() == currlayer->startgen) {
            // starting pattern has just been saved so we need to remember
            // this gen change in case user does a Reset after script ends
            // (RememberGenFinish will be called at the end of RunScript)
            currlayer->undoredo->RememberGenStart();
         }
      } else {
         // !currlayer->stayclean
         if (inscript) {
            // pass in false so we don't test savegenchanges flag;
            // ie. we only want to save pending cell changes here
            SavePendingChanges(false);
         }
         currlayer->undoredo->RememberGenStart();
      }
   }

   // curralgo->step() calls checkevents so set generating flag to avoid recursion
   generating = true;
   
   // avoid doing some things if NextGeneration is called from a script;
   // ie. by a run/step command
   if (!inscript) {
      wxGetApp().PollerReset();
      viewptr->CheckCursor(IsActive());
   }

   if (useinc) {
      // step by current increment
      if (curralgo->getIncrement() > bigint::one && !inscript) {
         UpdateToolBar(IsActive());
         UpdateMenuItems(IsActive());
      }
      curralgo->step();
   } else {
      // make sure we only step by one gen
      bigint saveinc = curralgo->getIncrement();
      curralgo->setIncrement(1);
      curralgo->step();
      curralgo->setIncrement(saveinc);
   }

   generating = false;
   
   if (!inscript) {
      // autofit is only used when doing many gens
      if (currlayer->autofit && useinc && curralgo->getIncrement() > bigint::one)
         viewptr->FitInView(0);
      UpdateEverything();
   }

   if (allowundo && !currlayer->stayclean)
      currlayer->undoredo->RememberGenFinish();
}

// -----------------------------------------------------------------------------

void MainFrame::AdvanceOutsideSelection()
{
   if (generating || viewptr->drawingcells || viewptr->waitingforclick) return;

   if (!viewptr->SelectionExists()) {
      statusptr->ErrorMessage(no_selection);
      return;
   }

   if (currlayer->algo->isEmpty()) {
      statusptr->ErrorMessage(empty_outside);
      return;
   }
   
   bigint top, left, bottom, right;
   currlayer->algo->findedges(&top, &left, &bottom, &right);

   // check if selection encloses entire pattern
   if ( currlayer->seltop <= top && currlayer->selbottom >= bottom &&
        currlayer->selleft <= left && currlayer->selright >= right ) {
      statusptr->ErrorMessage(empty_outside);
      return;
   }
   
   // save cell changes if undo/redo is enabled and script isn't constructing a pattern
   bool savecells = allowundo && !currlayer->stayclean;
   if (savecells && inscript) SavePendingChanges();

   // check if selection is completely outside pattern edges;
   // can't do this if qlife because it uses gen parity to decide which bits to draw
   if ( currlayer->hash &&
        // also avoid this if undo/redo is enabled (too messy to remember cell changes)
        !(savecells) &&
         ( currlayer->seltop > bottom || currlayer->selbottom < top ||
           currlayer->selleft > right || currlayer->selright < left ) ) {
      generating = true;
      wxGetApp().PollerReset();

      // step by one gen without changing gen count
      bigint savegen = currlayer->algo->getGeneration();
      bigint saveinc = currlayer->algo->getIncrement();
      currlayer->algo->setIncrement(1);
      currlayer->algo->step();
      currlayer->algo->setIncrement(saveinc);
      currlayer->algo->setGeneration(savegen);
   
      generating = false;
      
      // if pattern expanded then may need to clear ONE edge of selection
      viewptr->ClearSelection();
      MarkLayerDirty();
      UpdateEverything();
      return;
   }

   // check that pattern is within setcell/getcell limits
   if ( viewptr->OutsideLimits(top, left, bottom, right) ) {
      statusptr->ErrorMessage(_("Pattern is outside +/- 10^9 boundary."));
      return;
   }
   
   lifealgo* oldalgo = NULL;
   if (savecells) {
      // copy current pattern to oldalgo, using same type and gen count
      // so we can switch to oldalgo if user decides to abort below
      oldalgo = CreateNewUniverse(currlayer->hash);
      oldalgo->setGeneration( currlayer->algo->getGeneration() );
      if ( !viewptr->CopyRect(top.toint(), left.toint(), bottom.toint(), right.toint(),
                              currlayer->algo, oldalgo, false, _("Saving pattern")) ) {
         delete oldalgo;
         return;
      }
   }
   
   // create a new universe of same type
   lifealgo* newalgo = CreateNewUniverse(currlayer->hash);
   newalgo->setGeneration( currlayer->algo->getGeneration() );
   
   // copy (and kill) live cells in selection to new universe
   int iseltop    = currlayer->seltop.toint();
   int iselleft   = currlayer->selleft.toint();
   int iselbottom = currlayer->selbottom.toint();
   int iselright  = currlayer->selright.toint();
   if ( !viewptr->CopyRect(iseltop, iselleft, iselbottom, iselright,
                           currlayer->algo, newalgo, true,
                           _("Saving and erasing selection")) ) {
      // aborted, so best to restore selection
      if ( !newalgo->isEmpty() ) {
         newalgo->findedges(&top, &left, &bottom, &right);
         viewptr->CopyRect(top.toint(), left.toint(), bottom.toint(), right.toint(),
                           newalgo, currlayer->algo, false,
                           _("Restoring selection"));
      }
      delete newalgo;
      if (savecells) delete oldalgo;
      UpdateEverything();
      return;
   }
   
   // advance current universe by 1 generation
   generating = true;
   wxGetApp().PollerReset();
   currlayer->algo->setIncrement(1);
   currlayer->algo->step();
   generating = false;
   
   if ( !currlayer->algo->isEmpty() ) {
      // find new edges and copy current pattern to new universe,
      // except for any cells that were created in selection
      // (newalgo contains the original selection)
      bigint t, l, b, r;
      currlayer->algo->findedges(&t, &l, &b, &r);
      int itop = t.toint();
      int ileft = l.toint();
      int ibottom = b.toint();
      int iright = r.toint();
      int ht = ibottom - itop + 1;
      int cx, cy;
   
      // for showing accurate progress we need to add pattern height to pop count
      // in case this is a huge pattern with many blank rows
      double maxcount = currlayer->algo->getPopulation().todouble() + ht;
      double accumcount = 0;
      int currcount = 0;
      bool abort = false;
      BeginProgress(_("Copying advanced pattern"));
   
      lifealgo* curralgo = currlayer->algo;
      for ( cy=itop; cy<=ibottom; cy++ ) {
         currcount++;
         for ( cx=ileft; cx<=iright; cx++ ) {
            int skip = curralgo->nextcell(cx, cy);
            if (skip >= 0) {
               // found next live cell in this row
               cx += skip;
               
               // only copy cell if outside selection
               if ( cx < iselleft || cx > iselright ||
                    cy < iseltop || cy > iselbottom ) {
                  newalgo->setcell(cx, cy, 1);
               }
               
               currcount++;
            } else {
               cx = iright;  // done this row
            }
            if (currcount > 1024) {
               accumcount += currcount;
               currcount = 0;
               abort = AbortProgress(accumcount / maxcount, wxEmptyString);
               if (abort) break;
            }
         }
         if (abort) break;
      }
      
      newalgo->endofpattern();
      EndProgress();
      
      if (abort && savecells) {
         // revert back to pattern saved in oldalgo
         delete newalgo;
         delete currlayer->algo;
         currlayer->algo = oldalgo;
         SetGenIncrement();
         UpdateEverything();
         return;
      }
   }
   
   // switch to new universe (best to do this even if aborted)
   delete currlayer->algo;
   currlayer->algo = newalgo;
   SetGenIncrement();
   
   if (savecells) {
      // compare patterns in oldalgo and currlayer->algo and call SaveCellChange
      // for each cell that has a different state; note that we expand the
      // original pattern rect by 1 in case generating caused expansion
      if ( viewptr->SaveDifferences(oldalgo, currlayer->algo,
                                    top.toint() - 1, left.toint() - 1,
                                    bottom.toint() + 1, right.toint() + 1) ) {
         delete oldalgo;
         if ( !currlayer->undoredo->RememberCellChanges(_("Advance Outside"), currlayer->dirty) ) {
            // pattern outside selection didn't change
            UpdateEverything();
            return;
         }
      } else {
         // revert back to pattern saved in oldalgo
         currlayer->undoredo->ForgetCellChanges();
         delete currlayer->algo;
         currlayer->algo = oldalgo;
         SetGenIncrement();
         UpdateEverything();
         return;
      }
   }
   
   MarkLayerDirty();
   UpdateEverything();
}

// -----------------------------------------------------------------------------

void MainFrame::AdvanceSelection()
{
   if (generating || viewptr->drawingcells || viewptr->waitingforclick) return;

   if (!viewptr->SelectionExists()) {
      statusptr->ErrorMessage(no_selection);
      return;
   }

   if (currlayer->algo->isEmpty()) {
      statusptr->ErrorMessage(empty_selection);
      return;
   }
   
   bigint top, left, bottom, right;
   currlayer->algo->findedges(&top, &left, &bottom, &right);

   // check if selection is completely outside pattern edges
   if ( currlayer->seltop > bottom || currlayer->selbottom < top ||
        currlayer->selleft > right || currlayer->selright < left ) {
      statusptr->ErrorMessage(empty_selection);
      return;
   }
   
   // save cell changes if undo/redo is enabled and script isn't constructing a pattern
   bool savecells = allowundo && !currlayer->stayclean;
   if (savecells && inscript) SavePendingChanges();

   // check if selection encloses entire pattern;
   // can't do this if qlife because it uses gen parity to decide which bits to draw
   if ( currlayer->hash &&
        // also avoid this if undo/redo is enabled (too messy to remember cell changes)
        !(savecells) &&
        currlayer->seltop <= top && currlayer->selbottom >= bottom &&
        currlayer->selleft <= left && currlayer->selright >= right ) {
      generating = true;
      wxGetApp().PollerReset();

      // step by one gen without changing gen count
      bigint savegen = currlayer->algo->getGeneration();
      bigint saveinc = currlayer->algo->getIncrement();
      currlayer->algo->setIncrement(1);
      currlayer->algo->step();
      currlayer->algo->setIncrement(saveinc);
      currlayer->algo->setGeneration(savegen);
   
      generating = false;
      
      // clear 1-cell thick strips just outside selection
      viewptr->ClearOutsideSelection();
      MarkLayerDirty();
      UpdateEverything();
      return;
   }
   
   // find intersection of selection and pattern to minimize work
   if (currlayer->seltop > top) top = currlayer->seltop;
   if (currlayer->selleft > left) left = currlayer->selleft;
   if (currlayer->selbottom < bottom) bottom = currlayer->selbottom;
   if (currlayer->selright < right) right = currlayer->selright;

   // check that intersection is within setcell/getcell limits
   if ( viewptr->OutsideLimits(top, left, bottom, right) ) {
      statusptr->ErrorMessage(selection_too_big);
      return;
   }
   
   // create a temporary universe of same type as current universe so we
   // don't have to update the global rule table (in case it's a Wolfram rule)
   lifealgo* tempalgo = CreateNewUniverse(currlayer->hash);
   
   // copy live cells in selection to temporary universe
   if ( !viewptr->CopyRect(top.toint(), left.toint(), bottom.toint(), right.toint(),
                           currlayer->algo, tempalgo, false, _("Saving selection")) ) {
      delete tempalgo;
      return;
   }
   
   if ( tempalgo->isEmpty() ) {
      statusptr->ErrorMessage(empty_selection);
      delete tempalgo;
      return;
   }
   
   // advance temporary universe by one gen
   generating = true;
   wxGetApp().PollerReset();
   tempalgo->setIncrement(1);
   tempalgo->step();
   generating = false;
   
   if ( !tempalgo->isEmpty() ) {
      // temporary pattern might have expanded
      bigint temptop, templeft, tempbottom, tempright;
      tempalgo->findedges(&temptop, &templeft, &tempbottom, &tempright);
      if (temptop < top) top = temptop;
      if (templeft < left) left = templeft;
      if (tempbottom > bottom) bottom = tempbottom;
      if (tempright > right) right = tempright;
   
      // but ignore live cells created outside selection edges
      if (top < currlayer->seltop) top = currlayer->seltop;
      if (left < currlayer->selleft) left = currlayer->selleft;
      if (bottom > currlayer->selbottom) bottom = currlayer->selbottom;
      if (right > currlayer->selright) right = currlayer->selright;
   }
   
   if (savecells) {
      // compare selection rect in currlayer->algo and tempalgo and call SaveCellChange
      // for each cell that has a different state
      if ( viewptr->SaveDifferences(currlayer->algo, tempalgo,
                                    top.toint(), left.toint(),
                                    bottom.toint(), right.toint()) ) {
         if ( !currlayer->undoredo->RememberCellChanges(_("Advance Selection"), currlayer->dirty) ) {
            // pattern inside selection didn't change
            delete tempalgo;
            return;
         }
      } else {
         currlayer->undoredo->ForgetCellChanges();
         delete tempalgo;
         return;
      }
   }
   
   // copy all cells in new selection from tempalgo to currlayer->algo
   viewptr->CopyAllRect(top.toint(), left.toint(), bottom.toint(), right.toint(),
                        tempalgo, currlayer->algo, _("Copying advanced selection"));

   delete tempalgo;
   MarkLayerDirty();
   UpdateEverything();   
}

// -----------------------------------------------------------------------------

void MainFrame::ToggleAutoFit()
{
   currlayer->autofit = !currlayer->autofit;
   // we only use autofit when generating; that's why the Auto Fit item
   // is in the Control menu and not in the View menu
   if (currlayer->autofit && generating) {
      viewptr->FitInView(0);
      UpdateEverything();
   }
}

// -----------------------------------------------------------------------------

void MainFrame::ToggleHashing()
{
   if ( global_liferules.hasB0notS8 && !currlayer->hash ) {
      statusptr->ErrorMessage(_("Hashing cannot be used with a B0-not-S8 rule."));
      return;
   }

   // check if current pattern is too big to use getcell/setcell
   bigint top, left, bottom, right;
   if ( !currlayer->algo->isEmpty() ) {
      currlayer->algo->findedges(&top, &left, &bottom, &right);
      if ( viewptr->OutsideLimits(top, left, bottom, right) ) {
         statusptr->ErrorMessage(_("Pattern cannot be converted (outside +/- 10^9 boundary)."));
         // ask user if they want to continue anyway???
         return;
      }
   }

   if (generating) {
      // terminate GeneratePattern loop and set hash_pending flag
      Stop();
      hash_pending = true;
      return;
   }

   // toggle hashing option and update status bar immediately
   currlayer->hash = !currlayer->hash;
   currlayer->warp = 0;
   UpdateStatus();

   // create a new universe of the right flavor
   lifealgo* newalgo = CreateNewUniverse(currlayer->hash);
   
   // even though universes share a global rule table we still need to call setrule
   // due to internal differences in the handling of Wolfram rules
   newalgo->setrule( (char*)currlayer->algo->getrule() );
   
   // set same gen count
   newalgo->setGeneration( currlayer->algo->getGeneration() );

   if ( !currlayer->algo->isEmpty() ) {
      // copy pattern in current universe to new universe
      int itop = top.toint();
      int ileft = left.toint();
      int ibottom = bottom.toint();
      int iright = right.toint();
      int ht = ibottom - itop + 1;
      int cx, cy;
   
      // for showing accurate progress we need to add pattern height to pop count
      // in case this is a huge pattern with many blank rows
      double maxcount = currlayer->algo->getPopulation().todouble() + ht;
      double accumcount = 0;
      int currcount = 0;
      bool abort = false;
      BeginProgress(_("Converting pattern"));
   
      lifealgo* curralgo = currlayer->algo;
      for ( cy=itop; cy<=ibottom; cy++ ) {
         currcount++;
         for ( cx=ileft; cx<=iright; cx++ ) {
            int skip = curralgo->nextcell(cx, cy);
            if (skip >= 0) {
               // found next live cell in this row
               cx += skip;
               newalgo->setcell(cx, cy, 1);
               currcount++;
            } else {
               cx = iright;  // done this row
            }
            if (currcount > 1024) {
               accumcount += currcount;
               currcount = 0;
               abort = AbortProgress(accumcount / maxcount, wxEmptyString);
               if (abort) break;
            }
         }
         if (abort) break;
      }
      
      newalgo->endofpattern();
      EndProgress();
   }
   
   // delete old universe and point current universe to new universe
   delete currlayer->algo;
   currlayer->algo = newalgo;   
   SetGenIncrement();
   UpdateEverything();
   
   if (allowundo && !currlayer->stayclean)
      currlayer->undoredo->RememberAlgoChange();
}

// -----------------------------------------------------------------------------

void MainFrame::ToggleHyperspeed()
{
   currlayer->hyperspeed = !currlayer->hyperspeed;
}

// -----------------------------------------------------------------------------

void MainFrame::ToggleHashInfo()
{
   currlayer->showhashinfo = !currlayer->showhashinfo;
   hlifealgo::setVerbose( currlayer->showhashinfo ) ;
}

// -----------------------------------------------------------------------------

void MainFrame::SetWarp(int newwarp)
{
   currlayer->warp = newwarp;
   if (currlayer->warp < minwarp) currlayer->warp = minwarp;
   SetGenIncrement();
}

// -----------------------------------------------------------------------------

void MainFrame::ShowRuleDialog()
{
   if (generating) return;
   if (ChangeRule()) {
      // show new rule in window title (file name doesn't change)
      SetWindowTitle(wxEmptyString);
   }
}
