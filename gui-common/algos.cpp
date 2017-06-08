/*** /

 This file is part of Golly, a Game of Life Simulator.
 Copyright (C) 2013 Andrew Trevorrow and Tomas Rokicki.

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

#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"
#include "generationsalgo.h"
#include "ltlalgo.h"
#include "jvnalgo.h"
#include "ruleloaderalgo.h"

#include "utils.h"      // for Fatal, Warning, SetColor, Poller
#include "prefs.h"      // for maxhashmem
#include "layer.h"      // for currlayer
#include "algos.h"

#include <map>          // for std::map

// -----------------------------------------------------------------------------

// exported data:

algo_type initalgo = QLIFE_ALGO;    // initial algorithm
AlgoData* algoinfo[MAX_ALGOS];      // static info for each algorithm

gBitmapPtr* circles7x7;             // circular icons for scale 1:8
gBitmapPtr* circles15x15;           // circular icons for scale 1:16
gBitmapPtr* circles31x31;           // circular icons for scale 1:32

gBitmapPtr* diamonds7x7;            // diamond-shaped icons for scale 1:8
gBitmapPtr* diamonds15x15;          // diamond-shaped icons for scale 1:16
gBitmapPtr* diamonds31x31;          // diamond-shaped icons for scale 1:32

gBitmapPtr* hexagons7x7;            // hexagonal icons for scale 1:8
gBitmapPtr* hexagons15x15;          // hexagonal icons for scale 1:16
gBitmapPtr* hexagons31x31;          // hexagonal icons for scale 1:32

gBitmapPtr* triangles7x7;           // triangular icons for scale 1:8
gBitmapPtr* triangles15x15;         // triangular icons for scale 1:16
gBitmapPtr* triangles31x31;         // triangular icons for scale 1:32

// -----------------------------------------------------------------------------

// These default cell colors were generated by continuously finding the
// color furthest in rgb space from the closest of the already selected
// colors, black, and white.
static unsigned char default_colors[] = {
48,48,48, // better if state 0 is dark gray (was 255,127,0)
0,255,127,127,0,255,148,148,148,128,255,0,255,0,128,
0,128,255,1,159,0,159,0,1,255,254,96,0,1,159,96,255,254,
254,96,255,126,125,21,21,126,125,125,21,126,255,116,116,116,255,116,
116,116,255,228,227,0,28,255,27,255,27,28,0,228,227,227,0,228,
27,28,255,59,59,59,234,195,176,175,196,255,171,194,68,194,68,171,
68,171,194,72,184,71,184,71,72,71,72,184,169,255,188,252,179,63,
63,252,179,179,63,252,80,9,0,0,80,9,9,0,80,255,175,250,
199,134,213,115,100,95,188,163,0,0,188,163,163,0,188,203,73,0,
0,203,73,73,0,203,94,189,0,189,0,94,0,94,189,187,243,119,
55,125,32,125,32,55,32,55,125,255,102,185,102,185,255,120,209,168,
208,166,119,135,96,192,182,255,41,83,153,130,247,88,55,89,247,55,
88,55,247,87,75,0,0,87,75,75,0,87,200,135,59,51,213,127,
255,255,162,255,37,182,37,182,255,228,57,117,142,163,210,57,117,228,
193,255,246,188,107,123,123,194,107,145,59,5,5,145,59,59,5,145,
119,39,198,40,197,23,197,23,40,23,40,197,178,199,158,255,201,121,
134,223,223,39,253,84,149,203,15,203,15,149,15,149,203,152,144,90,
143,75,139,71,97,132,224,65,219,65,219,224,255,255,40,218,223,69,
74,241,0,241,0,74,0,74,241,122,171,51,220,211,227,61,127,87,
90,124,176,36,39,13,165,142,255,255,38,255,38,255,255,83,50,107,
224,142,165,255,181,9,9,255,181,181,9,255,140,238,70,255,74,5,
74,5,255,138,84,51,31,172,101,177,115,17,221,0,0,0,221,0,
0,0,221,220,255,200,0,41,50,255,150,205,178,45,116,113,255,189,
47,0,44,40,119,171,205,107,255,177,115,172,133,73,236,109,0,168,
168,46,207,188,181,203,212,188,35,90,97,52,39,209,184,41,164,152,
227,46,70,46,70,227,211,156,255,98,146,222,136,56,95,102,54,152,
86,142,0,142,0,86,0,86,142,86,223,96,246,135,46,4,208,120,
212,233,158,177,92,214,104,147,88,149,240,147,227,93,148,72,255,133,
209,27,194,147,255,255,44,93,0,160,36,158,182,233,0,96,94,217,
218,103,88,163,154,38,118,114,139,94,0,43,113,164,174,168,188,114,
0,23,119,42,86,93,255,226,202,80,191,155,255,158,136,0,247,62,
234,146,88,0,183,229,110,212,36,0,143,161,105,191,210,133,164,0,
41,30,89,164,0,132,30,89,42,178,222,217,121,22,11,221,107,22,
69,151,255,45,158,3,158,3,45,3,45,158,86,42,29,9,122,22,
213,209,110,53,221,57,159,101,91,93,140,45,247,213,37,185,34,0,
0,185,34,34,0,185,236,0,172,210,180,78,231,107,221,162,49,43,
43,162,49,49,43,162,36,248,213,114,0,214,213,36,248,149,34,243,
185,158,167,144,122,224,34,245,149,255,31,98,31,98,255,152,200,193,
255,80,95,128,123,63,102,62,72,255,62,148,151,226,108,159,99,255,
226,255,126,98,223,136,80,95,255,225,153,15,73,41,211,212,71,41,
83,217,187,180,235,79,0,166,127,251,135,243,229,41,0,41,0,229,
82,255,216,141,174,249,249,215,255,167,31,79,31,79,167,213,102,185,
255,215,83,4,2,40,224,171,220,41,0,4,6,50,90,221,15,113,
15,113,221,33,0,115,108,23,90,182,215,36
};

// -----------------------------------------------------------------------------

// Note that all the default icons are grayscale bitmaps.
// These icons are used for lots of different rules with different numbers
// of states, and at rendering time we will replace the white pixels in each
// icon with the cell's state color to avoid "color shock" when switching
// between icon and non-icon view.  Gray pixels are used to do anti-aliasing.

// XPM data for default 7x7 icon
static const char* default7x7[] = {
// width height ncolors chars_per_pixel
"7 7 4 1",
// colors
". c #000000",    // black will be transparent
"D c #404040",
"E c #E0E0E0",
"W c #FFFFFF",    // white
// pixels
".DEWED.",
"DWWWWWD",
"EWWWWWE",
"WWWWWWW",
"EWWWWWE",
"DWWWWWD",
".DEWED."
};

// XPM data for default 15x15 icon
static const char* default15x15[] = {
// width height ncolors chars_per_pixel
"15 15 5 1",
// colors
". c #000000",    // black will be transparent
"D c #404040",
"C c #808080",
"B c #C0C0C0",
"W c #FFFFFF",    // white
// pixels
"...............",
"....DBWWWBD....",
"...BWWWWWWWB...",
"..BWWWWWWWWWB..",
".DWWWWWWWWWWWD.",
".BWWWWWWWWWWWB.",
".WWWWWWWWWWWWW.",
".WWWWWWWWWWWWW.",
".WWWWWWWWWWWWW.",
".BWWWWWWWWWWWB.",
".DWWWWWWWWWWWD.",
"..BWWWWWWWWWB..",
"...BWWWWWWWB...",
"....DBWWWBD....",
"..............."
};

// XPM data for default 31x31 icon
static const char* default31x31[] = {
// width height ncolors chars_per_pixel
"31 31 5 1",
// colors
". c #000000",    // black will be transparent
"D c #404040",
"C c #808080",
"B c #C0C0C0",
"W c #FFFFFF",    // white
// pixels
"...............................",
"...............................",
"..........DCBWWWWWBCD..........",
".........CWWWWWWWWWWWC.........",
".......DWWWWWWWWWWWWWWWD.......",
"......BWWWWWWWWWWWWWWWWWB......",
".....BWWWWWWWWWWWWWWWWWWWB.....",
"....DWWWWWWWWWWWWWWWWWWWWWD....",
"....WWWWWWWWWWWWWWWWWWWWWWW....",
"...CWWWWWWWWWWWWWWWWWWWWWWWC...",
"..DWWWWWWWWWWWWWWWWWWWWWWWWWD..",
"..CWWWWWWWWWWWWWWWWWWWWWWWWWC..",
"..BWWWWWWWWWWWWWWWWWWWWWWWWWB..",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"..BWWWWWWWWWWWWWWWWWWWWWWWWWB..",
"..CWWWWWWWWWWWWWWWWWWWWWWWWWC..",
"..DWWWWWWWWWWWWWWWWWWWWWWWWWD..",
"...CWWWWWWWWWWWWWWWWWWWWWWWC...",
"....WWWWWWWWWWWWWWWWWWWWWWW....",
"....DWWWWWWWWWWWWWWWWWWWWWD....",
".....BWWWWWWWWWWWWWWWWWWWB.....",
"......BWWWWWWWWWWWWWWWWWB......",
".......DWWWWWWWWWWWWWWWD.......",
".........CWWWWWWWWWWWC.........",
"..........DCBWWWWWBCD..........",
"...............................",
"..............................."
};

// XPM data for the 7x7 icon used for hexagonal CA
static const char* hex7x7[] = {
// width height ncolors chars_per_pixel
"7 7 3 1",
// colors
". c #000000",    // black will be transparent
"C c #808080",
"W c #FFFFFF",    // white
// pixels
".WWC...",
"WWWWW..",
"WWWWWW.",
"CWWWWWC",
".WWWWWW",
"..WWWWW",
"...CWW."};

// XPM data for the 15x15 icon used for hexagonal CA
static const char* hex15x15[] = {
// width height ncolors chars_per_pixel
"15 15 3 1",
// colors
". c #000000",    // black will be transparent
"C c #808080",
"W c #FFFFFF",    // white
// pixels
"...WWC.........",
"..WWWWWC.......",
".WWWWWWWWC.....",
"WWWWWWWWWWW....",
"WWWWWWWWWWWW...",
"CWWWWWWWWWWWC..",
".WWWWWWWWWWWW..",
".CWWWWWWWWWWWC.",
"..WWWWWWWWWWWW.",
"..CWWWWWWWWWWWC",
"...WWWWWWWWWWWW",
"....WWWWWWWWWWW",
".....CWWWWWWWW.",
".......CWWWWW..",
".........CWW..."};

// XPM data for 31x31 icon used for hexagonal CA
static const char* hex31x31[] = {
// width height ncolors chars_per_pixel
"31 31 3 1",
// colors
". c #000000",    // black will be transparent
"C c #808080",
"W c #FFFFFF",    // white
// pixels
".....WWC.......................",
"....WWWWWC.....................",
"...WWWWWWWWC...................",
"..WWWWWWWWWWWC.................",
".WWWWWWWWWWWWWWC...............",
"WWWWWWWWWWWWWWWWWC.............",
"WWWWWWWWWWWWWWWWWWWC...........",
"CWWWWWWWWWWWWWWWWWWWWC.........",
".WWWWWWWWWWWWWWWWWWWWWW........",
".CWWWWWWWWWWWWWWWWWWWWWC.......",
"..WWWWWWWWWWWWWWWWWWWWWW.......",
"..CWWWWWWWWWWWWWWWWWWWWWC......",
"...WWWWWWWWWWWWWWWWWWWWWW......",
"...CWWWWWWWWWWWWWWWWWWWWWC.....",
"....WWWWWWWWWWWWWWWWWWWWWW.....",
"....CWWWWWWWWWWWWWWWWWWWWWC....",
".....WWWWWWWWWWWWWWWWWWWWWW....",
".....CWWWWWWWWWWWWWWWWWWWWWC...",
"......WWWWWWWWWWWWWWWWWWWWWW...",
"......CWWWWWWWWWWWWWWWWWWWWWC..",
".......WWWWWWWWWWWWWWWWWWWWWW..",
".......CWWWWWWWWWWWWWWWWWWWWWC.",
"........WWWWWWWWWWWWWWWWWWWWWW.",
".........CWWWWWWWWWWWWWWWWWWWWC",
"...........CWWWWWWWWWWWWWWWWWWW",
".............CWWWWWWWWWWWWWWWWW",
"...............CWWWWWWWWWWWWWW.",
".................CWWWWWWWWWWW..",
"...................CWWWWWWWW...",
".....................CWWWWW....",
".......................CWW....."
};

// XPM data for the 7x7 icon used for von Neumann CA
static const char* vn7x7[] = {
// width height ncolors chars_per_pixel
"7 7 2 1",
// colors
". c #000000",    // black will be transparent
"W c #FFFFFF",    // white
// pixels
"...W...",
"..WWW..",
".WWWWW.",
"WWWWWWW",
".WWWWW.",
"..WWW..",
"...W..."
};

// XPM data for the 15x15 icon used for von Neumann CA
static const char* vn15x15[] = {
// width height ncolors chars_per_pixel
"15 15 2 1",
// colors
". c #000000",    // black will be transparent
"W c #FFFFFF",    // white
// pixels
"...............",
".......W.......",
"......WWW......",
".....WWWWW.....",
"....WWWWWWW....",
"...WWWWWWWWW...",
"..WWWWWWWWWWW..",
".WWWWWWWWWWWWW.",
"..WWWWWWWWWWW..",
"...WWWWWWWWW...",
"....WWWWWWW....",
".....WWWWW.....",
"......WWW......",
".......W.......",
"..............."
};

// XPM data for 31x31 icon used for von Neumann CA
static const char* vn31x31[] = {
// width height ncolors chars_per_pixel
"31 31 2 1",
// colors
". c #000000",    // black will be transparent
"W c #FFFFFF",    // white
// pixels
"...............................",
"...............................",
"...............W...............",
"..............WWW..............",
".............WWWWW.............",
"............WWWWWWW............",
"...........WWWWWWWWW...........",
"..........WWWWWWWWWWW..........",
".........WWWWWWWWWWWWW.........",
"........WWWWWWWWWWWWWWW........",
".......WWWWWWWWWWWWWWWWW.......",
"......WWWWWWWWWWWWWWWWWWW......",
".....WWWWWWWWWWWWWWWWWWWWW.....",
"....WWWWWWWWWWWWWWWWWWWWWWW....",
"...WWWWWWWWWWWWWWWWWWWWWWWWW...",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"...WWWWWWWWWWWWWWWWWWWWWWWWW...",
"....WWWWWWWWWWWWWWWWWWWWWWW....",
".....WWWWWWWWWWWWWWWWWWWWW.....",
"......WWWWWWWWWWWWWWWWWWW......",
".......WWWWWWWWWWWWWWWWW.......",
"........WWWWWWWWWWWWWWW........",
".........WWWWWWWWWWWWW.........",
"..........WWWWWWWWWWW..........",
"...........WWWWWWWWW...........",
"............WWWWWWW............",
".............WWWWW.............",
"..............WWW..............",
"...............W...............",
"...............................",
"..............................."
};

// XPM data for the 7x7 icons used by 4-state rules emulating a triangular neighborhood
static const char* tri7x7[] = {
// width height ncolors chars_per_pixel
"7 21 2 1",
// colors
". c #000000",    // black will be transparent
"W c #FFFFFF",    // white
// pixels for state 1
".......",
"W......",
"WW.....",
"WWW....",
"WWWW...",
"WWWWW..",
"WWWWWW.",
// pixels for state 2
".WWWWWW",
"..WWWWW",
"...WWWW",
"....WWW",
".....WW",
"......W",
".......",
// pixels for state 3
".WWWWWW",
"W.WWWWW",
"WW.WWWW",
"WWW.WWW",
"WWWW.WW",
"WWWWW.W",
"WWWWWW."
};

// XPM data for the 15x15 icons used by 4-state rules emulating a triangular neighborhood
static const char* tri15x15[] = {
// width height ncolors chars_per_pixel
"15 45 2 1",
// colors
". c #000000",
"W c #FFFFFF",
// pixels for state 1
"...............",
"W..............",
"WW.............",
"WWW............",
"WWWW...........",
"WWWWW..........",
"WWWWWW.........",
"WWWWWWW........",
"WWWWWWWW.......",
"WWWWWWWWW......",
"WWWWWWWWWW.....",
"WWWWWWWWWWW....",
"WWWWWWWWWWWW...",
"WWWWWWWWWWWWW..",
"WWWWWWWWWWWWWW.",
// pixels for state 2
".WWWWWWWWWWWWWW",
"..WWWWWWWWWWWWW",
"...WWWWWWWWWWWW",
"....WWWWWWWWWWW",
".....WWWWWWWWWW",
"......WWWWWWWWW",
".......WWWWWWWW",
"........WWWWWWW",
".........WWWWWW",
"..........WWWWW",
"...........WWWW",
"............WWW",
".............WW",
"..............W",
"...............",
// pixels for state 3
".WWWWWWWWWWWWWW",
"W.WWWWWWWWWWWWW",
"WW.WWWWWWWWWWWW",
"WWW.WWWWWWWWWWW",
"WWWW.WWWWWWWWWW",
"WWWWW.WWWWWWWWW",
"WWWWWW.WWWWWWWW",
"WWWWWWW.WWWWWWW",
"WWWWWWWW.WWWWWW",
"WWWWWWWWW.WWWWW",
"WWWWWWWWWW.WWWW",
"WWWWWWWWWWW.WWW",
"WWWWWWWWWWWW.WW",
"WWWWWWWWWWWWW.W",
"WWWWWWWWWWWWWW."
};

// XPM data for the 31x31 icons used by 4-state rules emulating a triangular neighborhood
static const char* tri31x31[] = {
// width height ncolors chars_per_pixel
"31 93 2 1",
// colors
". c #000000",
"W c #FFFFFF",
// pixels for state 1
"...............................",
"W..............................",
"WW.............................",
"WWW............................",
"WWWW...........................",
"WWWWW..........................",
"WWWWWW.........................",
"WWWWWWW........................",
"WWWWWWWW.......................",
"WWWWWWWWW......................",
"WWWWWWWWWW.....................",
"WWWWWWWWWWW....................",
"WWWWWWWWWWWW...................",
"WWWWWWWWWWWWW..................",
"WWWWWWWWWWWWWW.................",
"WWWWWWWWWWWWWWW................",
"WWWWWWWWWWWWWWWW...............",
"WWWWWWWWWWWWWWWWW..............",
"WWWWWWWWWWWWWWWWWW.............",
"WWWWWWWWWWWWWWWWWWW............",
"WWWWWWWWWWWWWWWWWWWW...........",
"WWWWWWWWWWWWWWWWWWWWW..........",
"WWWWWWWWWWWWWWWWWWWWWW.........",
"WWWWWWWWWWWWWWWWWWWWWWW........",
"WWWWWWWWWWWWWWWWWWWWWWWW.......",
"WWWWWWWWWWWWWWWWWWWWWWWWW......",
"WWWWWWWWWWWWWWWWWWWWWWWWWW.....",
"WWWWWWWWWWWWWWWWWWWWWWWWWWW....",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWW...",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWW..",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW.",
// pixels for state 2
".WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"..WWWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"...WWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"....WWWWWWWWWWWWWWWWWWWWWWWWWWW",
".....WWWWWWWWWWWWWWWWWWWWWWWWWW",
"......WWWWWWWWWWWWWWWWWWWWWWWWW",
".......WWWWWWWWWWWWWWWWWWWWWWWW",
"........WWWWWWWWWWWWWWWWWWWWWWW",
".........WWWWWWWWWWWWWWWWWWWWWW",
"..........WWWWWWWWWWWWWWWWWWWWW",
"...........WWWWWWWWWWWWWWWWWWWW",
"............WWWWWWWWWWWWWWWWWWW",
".............WWWWWWWWWWWWWWWWWW",
"..............WWWWWWWWWWWWWWWWW",
"...............WWWWWWWWWWWWWWWW",
"................WWWWWWWWWWWWWWW",
".................WWWWWWWWWWWWWW",
"..................WWWWWWWWWWWWW",
"...................WWWWWWWWWWWW",
"....................WWWWWWWWWWW",
".....................WWWWWWWWWW",
"......................WWWWWWWWW",
".......................WWWWWWWW",
"........................WWWWWWW",
".........................WWWWWW",
"..........................WWWWW",
"...........................WWWW",
"............................WWW",
".............................WW",
"..............................W",
"...............................",
// pixels for state 3
".WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"W.WWWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"WW.WWWWWWWWWWWWWWWWWWWWWWWWWWWW",
"WWW.WWWWWWWWWWWWWWWWWWWWWWWWWWW",
"WWWW.WWWWWWWWWWWWWWWWWWWWWWWWWW",
"WWWWW.WWWWWWWWWWWWWWWWWWWWWWWWW",
"WWWWWW.WWWWWWWWWWWWWWWWWWWWWWWW",
"WWWWWWW.WWWWWWWWWWWWWWWWWWWWWWW",
"WWWWWWWW.WWWWWWWWWWWWWWWWWWWWWW",
"WWWWWWWWW.WWWWWWWWWWWWWWWWWWWWW",
"WWWWWWWWWW.WWWWWWWWWWWWWWWWWWWW",
"WWWWWWWWWWW.WWWWWWWWWWWWWWWWWWW",
"WWWWWWWWWWWW.WWWWWWWWWWWWWWWWWW",
"WWWWWWWWWWWWW.WWWWWWWWWWWWWWWWW",
"WWWWWWWWWWWWWW.WWWWWWWWWWWWWWWW",
"WWWWWWWWWWWWWWW.WWWWWWWWWWWWWWW",
"WWWWWWWWWWWWWWWW.WWWWWWWWWWWWWW",
"WWWWWWWWWWWWWWWWW.WWWWWWWWWWWWW",
"WWWWWWWWWWWWWWWWWW.WWWWWWWWWWWW",
"WWWWWWWWWWWWWWWWWWW.WWWWWWWWWWW",
"WWWWWWWWWWWWWWWWWWWW.WWWWWWWWWW",
"WWWWWWWWWWWWWWWWWWWWW.WWWWWWWWW",
"WWWWWWWWWWWWWWWWWWWWWW.WWWWWWWW",
"WWWWWWWWWWWWWWWWWWWWWWW.WWWWWWW",
"WWWWWWWWWWWWWWWWWWWWWWWW.WWWWWW",
"WWWWWWWWWWWWWWWWWWWWWWWWW.WWWWW",
"WWWWWWWWWWWWWWWWWWWWWWWWWW.WWWW",
"WWWWWWWWWWWWWWWWWWWWWWWWWWW.WWW",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWW.WW",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWW.W",
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW."
};

// -----------------------------------------------------------------------------

gBitmapPtr* CreateIconBitmaps(const char** xpmdata, int maxstates)
{
    if (xpmdata == NULL) return NULL;

    int wd, ht, numcolors, charsperpixel;
    sscanf(xpmdata[0], "%d %d %d %d", &wd, &ht, &numcolors, &charsperpixel);

    if (charsperpixel < 1 || charsperpixel > 2) {
        Warning("Error in XPM header data: chars_per_pixel must be 1 or 2");
        return NULL;
    };

    std::map<std::string,int> colormap;
    std::map<std::string,int>::iterator iterator;

    for (int i = 0; i < numcolors; i++) {
        std::string pixel;
        char ch1, ch2;
        int skip;
        if (charsperpixel == 1) {
            sscanf(xpmdata[i+1], "%c ", &ch1);
            pixel += ch1;
            skip = 2;
        } else {
            sscanf(xpmdata[i+1], "%c%c ", &ch1, &ch2);
            pixel += ch1;
            pixel += ch2;
            skip = 3;
        }
        if (strlen(xpmdata[i+1]) == (size_t)(skip+9)) {
            int rgb;
            sscanf(xpmdata[i+1]+skip, "c #%6x", &rgb);
            colormap[pixel] = rgb;
        } else {
            int r, g, b;
            sscanf(xpmdata[i+1]+skip, "c #%4x%4x%4x", &r, &g, &b);
            // only use top 8 bits of r,g,b
            colormap[pixel] = ((r>>8) << 16) | ((g>>8) << 8) | (b>>8);
        }
    }

    // allocate and clear memory for all icon bitmaps
    // (using calloc means black pixels will be transparent)
    unsigned char* rgba = (unsigned char*) calloc(wd * ht * 4, 1);
    if (rgba == NULL) return NULL;
    int pos = 0;
    for (int i = 0; i < ht; i++) {
        const char* rowstring = xpmdata[i+1+numcolors];
        for (int j = 0; j < wd * charsperpixel; j = j + charsperpixel) {
            std::string pixel;
            pixel += rowstring[j];
            if (charsperpixel == 2) pixel += rowstring[j+1];
            // find the RGB color for this pixel
            iterator = colormap.find(pixel);
            int rgb = (iterator == colormap.end()) ? 0 : iterator->second;
            if (rgb == 0) {
                // pixel is black and alpha is 0
                pos += 4;
            } else {
                rgba[pos] = (rgb & 0xFF0000) >> 16; pos++;  // red
                rgba[pos] = (rgb & 0x00FF00) >> 8;  pos++;  // green
                rgba[pos] = (rgb & 0x0000FF);       pos++;  // blue
                rgba[pos] = 255;                    pos++;  // alpha
            }
        }
    }

    int numicons = ht / wd;
    if (numicons > 255) numicons = 255;     // play safe

    gBitmapPtr* iconptr = (gBitmapPtr*) malloc(256 * sizeof(gBitmapPtr));
    if (iconptr) {
        for (int i = 0; i < 256; i++) iconptr[i] = NULL;
        unsigned char* nexticon = rgba;
        int iconbytes = wd * wd * 4;
        for (int i = 0; i < numicons; i++) {
            gBitmapPtr icon = (gBitmapPtr) malloc(sizeof(gBitmap));
            if (icon) {
                icon->wd = wd;
                icon->ht = wd;
                icon->pxldata = (unsigned char*) malloc(iconbytes);
                if (icon->pxldata) {
                    memcpy(icon->pxldata, nexticon, iconbytes);
                }
            }
            // add 1 to skip iconptr[0] (ie. dead state)
            iconptr[i+1] = icon;
            nexticon += iconbytes;
        }
        if (numicons < maxstates-1 && iconptr[numicons]) {
            // duplicate last icon
            nexticon -= iconbytes;
            for (int i = numicons; i < maxstates-1; i++) {
                gBitmapPtr icon = (gBitmapPtr) malloc(sizeof(gBitmap));
                if (icon) {
                    icon->wd = wd;
                    icon->ht = wd;
                    icon->pxldata = (unsigned char*) malloc(iconbytes);
                    if (icon->pxldata) {
                        memcpy(icon->pxldata, nexticon, iconbytes);
                    }
                }
                iconptr[i+1] = icon;
            }
        }
    }

    free(rgba);
    return iconptr;
}

// -----------------------------------------------------------------------------

static gBitmapPtr CreateSmallerIcon(unsigned char* indata, int insize, int outsize)
{
    gBitmapPtr icon = (gBitmapPtr) malloc(sizeof(gBitmap));
    if (icon) {
        icon->wd = outsize;
        icon->ht = outsize;
        icon->pxldata = (unsigned char*) malloc(outsize * outsize * 4);
        if (icon->pxldata) {
            // use same algorithm as in create_smaller_icons in icon-importer.py
            int sample = insize / outsize;
            int offset = sample / 2;
            int outpos = 0;
            for (int row = 0; row < outsize; row++) {
                for (int col = 0; col < outsize; col++) {
                    int x = offset + col * sample;
                    int y = offset + row * sample;
                    int inpos = (y * insize * 4) + (x * 4);
                    icon->pxldata[outpos++] = indata[inpos++];
                    icon->pxldata[outpos++] = indata[inpos++];
                    icon->pxldata[outpos++] = indata[inpos++];
                    icon->pxldata[outpos++] = indata[inpos++];
                }
            }
        }
    }
    return icon;
}

// -----------------------------------------------------------------------------

#define SETPIXEL(x, y) \
    outpos = (y) * outsize * 4 + (x) * 4; \
    icon->pxldata[outpos++] = r; \
    icon->pxldata[outpos++] = g; \
    icon->pxldata[outpos++] = b; \
    icon->pxldata[outpos++] = a;

static gBitmapPtr CreateBiggerIcon(unsigned char* indata, int insize, int outsize)
{
    if (insize != 15 || outsize != 31) {
        // we currently don't support scaling up 7x7 icons as it's highly unlikely
        // that a .rule file won't have 15x15 icons
        return NULL;
    }

    gBitmapPtr icon = (gBitmapPtr) malloc(sizeof(gBitmap));
    if (icon) {
        icon->wd = outsize;
        icon->ht = outsize;
        icon->pxldata = (unsigned char*) calloc(outsize * outsize * 4, 1);
        if (icon->pxldata) {
            // use same algorithm as in create31x31icons in icon-importer.py
            // to scale up a 15x15 bitmap into a 31x31 bitmap using a simple
            // method that conserves any vertical or horizontal symmetry
            int inpos = 0;
            for (int row = 0; row < insize; row++) {
                for (int col = 0; col < insize; col++) {
                    unsigned char r = indata[inpos++];
                    unsigned char g = indata[inpos++];
                    unsigned char b = indata[inpos++];
                    unsigned char a = indata[inpos++];
                    if (r || g || b) {
                        // non-black pixel
                        int outpos;
                        if (row == 7 && col == 7) {
                            // expand middle pixel into 9 pixels
                            int x = 15;
                            int y = 15;
                            SETPIXEL(x, y);
                            SETPIXEL(x, y+1);
                            SETPIXEL(x, y-1);
                            SETPIXEL(x+1, y);
                            SETPIXEL(x-1, y);
                            SETPIXEL(x+1, y+1);
                            SETPIXEL(x+1, y-1);
                            SETPIXEL(x-1, y+1);
                            SETPIXEL(x-1, y-1);
                        } else if (row == 7) {
                            // expand pixel in middle row into 6 pixels
                            int x = col * 2;
                            int y = row * 2;
                            if (col > 7) x++;
                            SETPIXEL(x, y);
                            SETPIXEL(x, y+1);
                            SETPIXEL(x+1, y);
                            SETPIXEL(x+1, y+1);
                            SETPIXEL(x, y+2);
                            SETPIXEL(x+1, y+2);
                        } else if (col == 7) {
                            // expand pixel in middle column into 6 pixels
                            int x = col * 2;
                            int y = row * 2;
                            if (row > 7) y++;
                            SETPIXEL(x, y);
                            SETPIXEL(x, y+1);
                            SETPIXEL(x+1, y);
                            SETPIXEL(x+1, y+1);
                            SETPIXEL(x+2, y);
                            SETPIXEL(x+2, y+1);
                        } else {
                            // expand all other pixels into 4 pixels
                            int x = col * 2;
                            int y = row * 2;
                            if (col > 7) x++;
                            if (row > 7) y++;
                            SETPIXEL(x, y);
                            SETPIXEL(x, y+1);
                            SETPIXEL(x+1, y);
                            SETPIXEL(x+1, y+1);
                        }
                    }
                }
            }
        }
    }
    return icon;
}

// -----------------------------------------------------------------------------

gBitmapPtr* ScaleIconBitmaps(gBitmapPtr* srcicons, int size)
{
    if (srcicons == NULL) return NULL;

    gBitmapPtr* iconptr = (gBitmapPtr*) malloc(256 * sizeof(gBitmapPtr));
    if (iconptr) {
        for (int i = 0; i < 256; i++) {
            if (srcicons[i] == NULL) {
                iconptr[i] = NULL;
            } else {
                int insize = srcicons[i]->wd;
                if (insize > size) {
                    iconptr[i] = CreateSmallerIcon(srcicons[i]->pxldata, insize, size);
                } else {
                    // assume insize < size
                    iconptr[i] = CreateBiggerIcon(srcicons[i]->pxldata, insize, size);
                }
            }
        }
    }
    return iconptr;
}

// -----------------------------------------------------------------------------

static void CreateDefaultIcons(AlgoData* ad)
{
    if (ad->defxpm7x7 || ad->defxpm15x15 || ad->defxpm31x31) {
        // create icons using given algo's default XPM data
        ad->icons7x7 = CreateIconBitmaps(ad->defxpm7x7, ad->maxstates);
        ad->icons15x15 = CreateIconBitmaps(ad->defxpm15x15, ad->maxstates);
        ad->icons31x31 = CreateIconBitmaps(ad->defxpm31x31, ad->maxstates);

        // create scaled bitmaps if size(s) not supplied
        if (!ad->icons7x7) {
            if (ad->icons15x15)
                // scale down 15x15 bitmaps
                ad->icons7x7 = ScaleIconBitmaps(ad->icons15x15, 7);
            else
                // scale down 31x31 bitmaps
                ad->icons7x7 = ScaleIconBitmaps(ad->icons31x31, 7);
        }
        if (!ad->icons15x15) {
            if (ad->icons31x31)
                // scale down 31x31 bitmaps
                ad->icons15x15 = ScaleIconBitmaps(ad->icons31x31, 15);
            else
                // scale up 7x7 bitmaps
                ad->icons15x15 = ScaleIconBitmaps(ad->icons7x7, 15);
        }
        if (!ad->icons31x31) {
            if (ad->icons15x15)
                // scale up 15x15 bitmaps
                ad->icons31x31 = ScaleIconBitmaps(ad->icons15x15, 31);
            else
                // scale up 7x7 bitmaps
                ad->icons31x31 = ScaleIconBitmaps(ad->icons7x7, 31);
        }
    } else {
        // algo didn't supply any icons so use static XPM data defined above
        ad->icons7x7 = CreateIconBitmaps(default7x7, ad->maxstates);
        ad->icons15x15 = CreateIconBitmaps(default15x15, ad->maxstates);
        ad->icons31x31 = CreateIconBitmaps(default31x31, ad->maxstates);
    }
}

// -----------------------------------------------------------------------------

AlgoData::AlgoData() {
    defbase = 0;
    icons7x7 = NULL;
    icons15x15 = NULL;
    icons31x31 = NULL;
}

// -----------------------------------------------------------------------------

AlgoData& AlgoData::tick() {
    AlgoData* r = new AlgoData();
    algoinfo[r->id] = r;
    return *r;
}

// -----------------------------------------------------------------------------

void InitAlgorithms()
{
    // QuickLife must be 1st and HashLife must be 2nd
    qlifealgo::doInitializeAlgoInfo(AlgoData::tick());
    hlifealgo::doInitializeAlgoInfo(AlgoData::tick());
    
    // these algos can be in any order
    generationsalgo::doInitializeAlgoInfo(AlgoData::tick());
    ltlalgo::doInitializeAlgoInfo(AlgoData::tick());
    jvnalgo::doInitializeAlgoInfo(AlgoData::tick());
    
    // RuleLoader must be last so we can display detailed error messages
    // (see LoadRule in file.cpp)
    ruleloaderalgo::doInitializeAlgoInfo(AlgoData::tick());

    // init algoinfo array
    for (int i = 0; i < NumAlgos(); i++) {
        AlgoData* ad = algoinfo[i];
        if (ad->algoName == 0 || ad->creator == 0)
            Fatal("Algorithm did not set name and/or creator");

        // does algo use hashing?
        ad->canhash = ad->defbase == 8;    // safer method needed???

        // set status bar background by cycling thru a few pale colors
        switch (i % 9) {
            case 0: SetColor(ad->statusrgb, 255, 255, 206); break;  // pale yellow
            case 1: SetColor(ad->statusrgb, 226, 250, 248); break;  // pale blue
            case 2: SetColor(ad->statusrgb, 255, 233, 233); break;  // pale pink
            case 3: SetColor(ad->statusrgb, 255, 227, 178); break;  // pale orange
            case 4: SetColor(ad->statusrgb, 225, 255, 225); break;  // pale green
            case 5: SetColor(ad->statusrgb, 243, 225, 255); break;  // pale purple
            case 6: SetColor(ad->statusrgb, 200, 255, 255); break;  // pale aqua
            case 7: SetColor(ad->statusrgb, 200, 200, 200); break;  // pale gray
            case 8: SetColor(ad->statusrgb, 255, 255, 255); break;  // white
        }

        // initialize default color scheme
        if (ad->defr[0] == ad->defr[1] &&
            ad->defg[0] == ad->defg[1] &&
            ad->defb[0] == ad->defb[1]) {
            // colors are nonsensical, probably unset, so use above defaults
            unsigned char* rgbptr = default_colors;
            for (int c = 0; c < ad->maxstates; c++) {
                ad->defr[c] = *rgbptr++;
                ad->defg[c] = *rgbptr++;
                ad->defb[c] = *rgbptr++;
            }
        }
        ad->gradient = ad->defgradient;
        SetColor(ad->fromrgb, ad->defr1, ad->defg1, ad->defb1);
        SetColor(ad->torgb, ad->defr2, ad->defg2, ad->defb2);
        for (int c = 0; c < ad->maxstates; c++) {
            ad->algor[c] = ad->defr[c];
            ad->algog[c] = ad->defg[c];
            ad->algob[c] = ad->defb[c];
        }

        CreateDefaultIcons(ad);
    }

    circles7x7 = CreateIconBitmaps(default7x7,256);
    circles15x15 = CreateIconBitmaps(default15x15,256);
    circles31x31 = CreateIconBitmaps(default31x31,256);

    diamonds7x7 = CreateIconBitmaps(vn7x7,256);
    diamonds15x15 = CreateIconBitmaps(vn15x15,256);
    diamonds31x31 = CreateIconBitmaps(vn31x31,256);

    hexagons7x7 = CreateIconBitmaps(hex7x7,256);
    hexagons15x15 = CreateIconBitmaps(hex15x15,256);
    hexagons31x31 = CreateIconBitmaps(hex31x31,256);

    // these icons can only be used with 4-state rules
    triangles7x7 = CreateIconBitmaps(tri7x7,4);
    triangles15x15 = CreateIconBitmaps(tri15x15,4);
    triangles31x31 = CreateIconBitmaps(tri31x31,4);
}

// -----------------------------------------------------------------------------

void FreeIconBitmaps(gBitmapPtr* icons)
{
    if (icons) {
        for (int i = 0; i < 256; i++) {
            if (icons[i]) {
                if (icons[i]->pxldata) free(icons[i]->pxldata);
                free(icons[i]);
            }
        }
        free(icons);
    }
}

// -----------------------------------------------------------------------------

void DeleteAlgorithms()
{
    for (int i = 0; i < NumAlgos(); i++) {
        delete algoinfo[i];
    }

    FreeIconBitmaps(circles7x7);
    FreeIconBitmaps(circles15x15);
    FreeIconBitmaps(circles31x31);

    FreeIconBitmaps(diamonds7x7);
    FreeIconBitmaps(diamonds15x15);
    FreeIconBitmaps(diamonds31x31);

    FreeIconBitmaps(hexagons7x7);
    FreeIconBitmaps(hexagons15x15);
    FreeIconBitmaps(hexagons31x31);

    FreeIconBitmaps(triangles7x7);
    FreeIconBitmaps(triangles15x15);
    FreeIconBitmaps(triangles31x31);
}

// -----------------------------------------------------------------------------

lifealgo* CreateNewUniverse(algo_type algotype, bool allowcheck)
{
    lifealgo* newalgo = algoinfo[algotype]->creator();

    if (newalgo == NULL) Fatal("Failed to create new universe!");

    if (algoinfo[algotype]->canhash) {
        newalgo->setMaxMemory(maxhashmem);
    }
    // non-hashing algos (QuickLife) use their default memory setting

    if (allowcheck) newalgo->setpoll(Poller());

    return newalgo;
}

// -----------------------------------------------------------------------------

const char* GetAlgoName(algo_type algotype)
{
    return algoinfo[algotype]->algoName;
}

// -----------------------------------------------------------------------------

int NumAlgos()
{
    return staticAlgoInfo::getNumAlgos();
}

// -----------------------------------------------------------------------------

bool MultiColorImage(gBitmapPtr image)
{
    // return true if image contains at least one color that isn't a shade of gray
    if (image == NULL) return false;
    
    unsigned char* pxldata = image->pxldata;
    if (pxldata == NULL) return false;          // play safe
    int numpixels = image->wd * image->ht;
    int byte = 0;
    for (int i = 0; i < numpixels; i++) {
        unsigned char r = pxldata[byte];
        unsigned char g = pxldata[byte+1];
        unsigned char b = pxldata[byte+2];
        if (r != g || g != b) return true;      // multi-color
        byte += 4;
    }
    return false;       // grayscale
}
