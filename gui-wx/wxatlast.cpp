#include "wx/wxprec.h"      // for compilers that support precompilation
#ifndef WX_PRECOMP
    #include "wx/wx.h"      // for all others include the necessary headers
#endif

#include "wx/filename.h"    // for wxFileName
#include "wx/dir.h"         // for wxDir

#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"
#include "readpattern.h"
#include "writepattern.h"

#include "wxgolly.h"        // for wxGetApp, mainptr, viewptr, statusptr, stopwatch
#include "wxmain.h"         // for mainptr->...
#include "wxselect.h"       // for Selection
#include "wxview.h"         // for viewptr->...
#include "wxstatus.h"       // for statusptr->...
#include "wxutils.h"        // for Warning, Note, GetString, SaveChanges, etc
#include "wxprefs.h"        // for gollydir, etc
#include "wxinfo.h"         // for ShowInfo
#include "wxhelp.h"         // for ShowHelp
#include "wxundo.h"         // for currlayer->undoredo->...
#include "wxalgos.h"        // for *_ALGO, CreateNewUniverse, etc
#include "wxlayer.h"        // for AddLayer, currlayer, currindex, etc
#include "wxoverlay.h"      // for curroverlay->...
#include "wxscript.h"       // for inscript, abortmsg, GSF_*, etc
#include "wxatlast.h"
#include <time.h>
//#include "atlast.h"
//#include "../../atlast-2.0/atldef.h"
#include "atldef.h"
// #include "lua.hpp"          // Lua header files for C++

#include <iostream>
#include <string.h>
using namespace std;


// -----------------------------------------------------------------------------
#define ATL_ENC wxConvUTF8
#define V   (void)

/*  Globals imported  */

/*  CTRLC  --  Catch a user console break signal.  If your C library
	       does not provide this Unix-compatibile facility
	       (registered with the call on signal() in main()),
	       just turn this code off or, better still, replace it
	       with the equivalent on your system.  */

static void ctrlc(int sig)
{
    if (sig == SIGINT)
	atl_break();
}


static bool atl_aborted = false;
static bool aborted = false;

static void atlCheckEvents()
{
    // this routine is called at the start of every g_* function so we can
    // detect user events (eg. hitting the stop button or escape key)

    if (allowcheck) wxGetApp().Poller()->checkevents();
    
    if (insideYield) return;
    
    // we're outside Yield so safe to do a longjmp from lua_error
    if (aborted) {
        // AbortLuaScript was called
//	    cout << "atl:checkexents error" << endl;
		atl_error("check_events: Outside Yield");
		return;

    }
}


//static void  
prim check_events();
prim pmyhi() {
  Push = 1;
  Push = 2;
  printf("privet from golly");
}

prim ptime(){
  So(1);
  Push = time(NULL);
}
prim phhmmss()
{
    struct tm *lt;

    Sl(1);
    So(2);
    lt = localtime(&S0);
    S0 = lt->tm_hour;
    Push = lt->tm_min;
    Push = lt->tm_sec;
}

prim pleibniz()
{
    long nterms;
    double sum = 0.0,
           numer = 1.0,
           denom = 1.0;

    Sl(1);
    nterms = S0;
    Pop; 

    So(Realsize);
    Push = 0;
    while (nterms-- > 0) {
        sum += numer / denom;
        numer = -numer;
        denom += 2.0;
    }
    SREAL0(sum * 4.0);
}

prim pnumalgos(){
  So(1);
  Push = NumAlgos();
}

static void protate(){
  check_events();
  int direction = 1;
  if (viewptr->SelectionExists()){
	viewptr->RotateSelection(direction == 0);
	DoAutoUpdate();
	So(1);
	Push = 0;
  }else{
	atl_error("rotate error: no selection.");
	So(1);
	Push = -1;
  }

}
prim check_events(){
  if (allowcheck) wxGetApp().Poller()->checkevents();
  if (insideYield) return;
  if (aborted){
	atl_error("Outside Yield");
  }
}
/*
 getcell (y x -- state)
*/
prim pgetcell(){
  check_events();
  int x , y;
  x = S0; Pop;
  y = S0; Pop;
  const char* err = GSF_checkpos(currlayer->algo, x, y);
  if (err){
	atl_error("getcell: error: checkpos");
	//return -1;
  }else{
	So(1);
	Push = currlayer->algo->getcell(x, y);
  }
}

prim psetcell(){ // ( x1 y1  state -- - | 0 = err )
  check_events();
  int x, y;
  int setcell_state;
  setcell_state = S0; Pop;
  y = S0; Pop;
  x = S0; Pop;

  printf("x=%d, y=%d, state=%d \n", x, y, setcell_state);
  const char* err = GSF_setcell(x, y , setcell_state);
// обработка ошибки вызывает ошибку
// Ошибка сегментирования
// именно if(err) дает эту ошибку

//  int err_len = stringlen(err);

//  const char* atl_err = //NULL; //""; //"atl_err setcell: error";
//  if (atl_err){
 // if (err != NULL){
//	atl_error("setcell error: GSF_setcell");
//	printf("\n err %d, %s", err, atl_err);
	//So(1);
	//Push = 0;
//  }
//  }
}

prim pgetrect(){ // getrect ( -- x y wd bt flag = true -1 | flag =  false 0 )
  if (!currlayer->algo->isEmpty()){
	bigint top, left, bottom, right;
	currlayer->algo->findedges(&top, &left, &bottom, &right);
	if (viewptr->OutsideLimits(top, left, bottom, right)){
	 atl_error("getrect error: pattern is too big."); 
	}
	int x = left.toint();
	int y = top.toint();
	int wd = right.toint() - x + 1;
	int bt = bottom.toint() - y + 1;
	So(1); Push = x;
	So(1); Push = y;
	So(1); Push = wd;
	So(1); Push = bt;
	So(1); Push = -1; // is true result
  }else{
	So(1); Push = 0; // is empty result	
  }
}

prim pnumstates(){ // ( -- n = numstates )
  check_events();
  So(1);
  Push = currlayer->algo->NumCellStates();
}

prim pgetalgo(){
  check_events();
  int index = currlayer->algtype;
  if (index < 0 || index >= NumAlgos()){
	// sprintf(msg, "getalgo error: bad index (%d)", index);
	// atl_error(msg);
  }
  //So(1); Push = GetAlgoName(index);
  So(1); Push = index;  
}
wxString my_str = "my string";
prim ptest_wxstring(){
//  wxString my_str = "my string";
  const char* pmy_str = my_str.mb_str();
  int len_mystr = my_str.length();
  So(1); Push = (long int)&my_str;
  So(1); Push = len_mystr;
}

prim pshow(){ // ( c-addr n -- )
  check_events();
  Sl(1);
  Hpc(S0);
  const char* msg = (char*)S0;
  inscript = false;
  wxPuts(msg); // 
  statusptr->DisplayMessage(wxString(msg, ATL_ENC)); //, ATL_ENC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();
  // return;
}

/*
Здесь применяем следующее. 
В самом макросе создаем строку
80 string mystr
Потом адрес этой строки проталкиваем в стек
и вызываем getcursor
mystr getcursor
После этого mystr будет содержать текущий тип курсора
в виде строки, например Draw or Select and etc.
Таким образом mystr это фактический параметр для getcorsor
*/
prim pgetcursor(){ // *string
  check_events();
  Sl(1);
  Hpc(S0); // получить адрес строки из макроса в S0
  const char* tmpstr;
  tmpstr = CursorToString(currlayer->curs);
  strcpy( (char*)S0, tmpstr ); // передаем результат в строку
  //lua_pushstring(L, CursorToString(currlayer->curs));
}

/*
сщздать строку и записать в нее значение 
80 string myoption
"drawingoption" myoption s!
вызвать 
myoption getoption
в стеке должно быть числовое значение опции
*/

prim psetcursor(){
  check_events();
  std::string newcursor;
  const char* pnewcursor;
  Hpc(S0);
  strcpy((char*)newcursor.c_str(), (char*)S0);
  pnewcursor = &newcursor[0];
  printf("setcursor: newcursor=%s\n", pnewcursor);
  const char* oldcursor = CursorToString(currlayer->curs);
  printf("setcursor: old cursor=%s\n", oldcursor);
  wxCursor* cursptr = StringToCursor(pnewcursor);
  if (cursptr){
	viewptr->SetCursorMode(cursptr);
	mainptr->UpdateUserInterface();
  }else{
	atl_error("setcusor error");
  }
  strcpy((char*)S0, oldcursor); //return oldcursor
}

prim pgetoption(){
  Sl(1);
  Hpc(S0);
  const char* optname = (char*)S0;
  int optval;
  if ( !GSF_getoption( optname, &optval )){
	printf("getoption error: unknow option.");
	return;
  }
  Pop; // удаляем со стека адрес строки
  So(1);
  Push = optval; // сохраняем в стек значение
}

prim pupdate(){
  check_events();
  GSF_update();
  So(1); Push = 0; // no result
}

prim pdrawline(){

}
/*
prim pinclude(){
	  int stat;
	  char fn[132];
	  FILE *fp;


	  Sl(1);
	  Hpc(S0); // получить строку из макроса в S0
	  strcpy(fn,  (char*)S0);
	  fp = fopen(fn, "r" );
	  if (fp == NULL) {
		fprintf(stderr, "Unable to open include file %s\n", fn);
	  }
	  stat = atl_load(fp);
	  fclose(fp);
	  if (stat != ATL_SNORM) {
		printf("\nError %d in include file %s\n", stat, fn);
	  }
}
*/
/*
сщздать строку и записать в нее значение 
80 string event
"some_event" event s!
вызвать 
event getevent
в стеке в строке должно остаться строка, в которой содержится имя события
*/
prim pgetevent(){ // ( string -- string )
  check_events();
  Sl(1);
  Hpc(S0); // получить адрес строки из макроса в S0
  const char* tmpstr;
//  tmpstr = CursorToString(currlayer->curs);
//  strcpy( (char*)S0, tmpstr ); // передаем результат в строку

  std::string s;
  s = "hello";
  bool get = true;
  wxString event;
  GSF_getevent( event, 0 ); //get ? 1 : 0 );

//  tmpstr = event.mb_str(); //
  s = event.ToStdString();
//  strcat((char *)tmpstr, (char *)s.c_str() );
  printf(". %s", s.c_str());
//  printf(". %s", event.mb_str(ATL_ENC));
  
  strcpy((char*)S0, s.c_str() );
}
//-----------------------------------------------------------------------------
// test
//-----------------------------------------------------------------------------
prim ptest_s (){
  Sl(1);
  Hpc(S0); // получить строку из макроса в S0
  const char* msg = (char*)S0;
//  wxPuts((char*)S0); // напечатать строку на консоль
  wxPuts(msg);
  printf("test string %s", (char*)S0);
  strcpy( (char*)S0, "hi"); // это сработало!!!!!
  printf("test string %s", (char*)S0);
//  statusptr->DisplayMessage(wxString((char*)S0));
  statusptr->DisplayMessage(wxString(msg, ATL_ENC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();
  return;
}
// top stack type
prim pdots(){

}
/*
закинуть в стак требуемое число (количество) памяти
вызвать allot
на стеке останется адрес выделенной памяти
скопировать в него строку
вставить в конце символ конца строки

Таким же образом можно возвращать массив
И можно придумать ккак возвращать структуру
список и т.д.

*/

prim ptest_str(){
  Sl(1);
  Hpc(S0); // получить строку из макроса в S0
  strcpy( (char*)S0, "Privet" );
}

// Как вернуть строку???
/*
функции должны возвращать результат в стекю 
На вершине стека 
если 0 - нет результата
если 1 - один результат любого типа
если 2 - два результата любого типа
и т.д.

protate
getcells
putcells
pgetrect
pgetcell
psetcell
setcolors
getnumstates
getalgo
getwidth
getheight
pnumstates
pnumalgos

*/
static struct primfcn  atlastfuncs [] = {
  {"0TIME", ptime},
    {"0HHMMSS", phhmmss},
    {"0LEIBNIZ", pleibniz},
    {"0ROTATE", protate},
    {"0GETRECT", pgetrect},
    { "0GETALGO", pgetalgo },
    { "0GETCELL", pgetcell}, //y x getcell (y x -- state)
    { "0SETCELL", psetcell}, //y x getcell (state y x -- - | 0 = err)
    { "0MYHI",         pmyhi },         // open given pattern/script/rule/html file
	{ "0NUMALGOS", pnumalgos },
	{ "0NUMSTATES", pnumstates },
    { "0SHOW", pshow },
    { "0GETCURSOR", pgetcursor },
    { "0SETCURSOR", psetcursor },
    { "0GETOPTION", pgetoption },
    { "0UPDATE", pupdate },
//    { "0INCLUDE", pinclude },
    { "0GETEVENT", pgetevent },
	{ "0TEST_S", ptest_s },
	{ "0TEST_STR", ptest_str },
	{ "0TESTWXSTRING", ptest_wxstring },
	{NULL, (codeptr) 0}
};

//include atlast script. The basic variant
void RunAtlastScript(const wxString& filepath){
  char* pchar = (const_cast<char*>((const char*)filepath.mb_str()));
  FILE* fp;
  int stat = 0;
  char t[132];
  atl_init();
  atl_primdef(atlastfuncs);
  fp = fopen( pchar, "r");
  if (fp == NULL) {
	fprintf( stderr, "Unable to open file %s\n", pchar);
	return;
  }
  stat = atl_load( fp );
  fclose( fp );
//  return;
  RunAtlastScript_a(filepath);
}

void RunAtlastScript_b(const wxString& filepath){
  char t[132];
  atl_init();
  atl_primdef(atlastfuncs);
  while (printf("-> "), fgets(t, 132, stdin) != NULL)
	atl_eval(t);
  //return 0;
}

// this run. Not chenge. The is test variant
void RunAtlastScript_a(const wxString& filepath){
  char t[132];
  atl_init();
  atl_primdef(atlastfuncs);
  while (printf("-> "), fgets(t, 132, stdin) != NULL)
	atl_eval(t);
  //return 0;
}

void AbortAtlastScript(){
//  aborted = true;
  scripterr = wxString(abortmsg, wxConvLocal);
}

void FinishAtlastScripting(){
  // no need to do anything
  printf("atlast finish \n");
}
//  int argc0 = 0;
//  char **argv0 = 0;

//  for(int i = 0; i < argc0; i++)    //показываем все строки на экран
//	cerr << i << ": "<< argv0[i] << endl; 
//  cerr << endl;

//  argv0 = AddPtr( argv0, argc0, "~/projects/golly-code/atlast");
//  argc0++;
//  argv0 = AddPtr( argv0, argc0, pchr);
//  argc0++;

//void RunAtlastScript_a(const wxString& filepath){
//  static char *ens[] = {NULL}, **env = &ens[0];
//  wxPuts (wxT ( "RunAtlastScript: A wxWidgets c o n s o l e a p p l i c a t i o n " ) ) ;
//  wxPuts(filepath);
//  char* pchr = (const_cast<char*>((const char*)filepath.mb_str()));
//  int fname = FALSE, defmode = FALSE;
//  FILE ifp;
//  FILE* fp;
//  int stat = 0;
//  char t[132];

//  atl_init();
//  atl_primdef(atlastfuncs);

//  fp = fopen(pchr, "r");
//  if (fp == NULL){
//	fprintf(stderr, "Unable to open file %s\n", pchr);
//	return;
//  }

//  wxPuts (wxT("file open \n"));

//  fname = TRUE;
//  defmode = TRUE;
// Так можно сделать включаемые файлы для include
//  stat = atl_load(fp);
//  fclose(fp);
//  wxPuts()
//  fname = defmode = FALSE;
//  fp = stdin;
//  wxPuts (wxT("File loaded \n"));
//  return;
//  ifp = fopen(pchr, "r");
// Так будем обрабатвать просто выполняемый файл


//    signal(SIGINT, ctrlc);
//    while (TRUE) {
//	  // char t[132];

//	  if (!fname) {
//		printf(atl_comment ? "(  " :  /* Show pending comment */
//				 /* Show compiling state */
//				 (((heap != NULL) && state) ? ":> " : "-> "));
//	  }
//	  wxPuts (wxT("run file \n"));
//	  if (fgets(t, 132, fp) == NULL) {
//	    if (fname && defmode) {
//		  fname = defmode = FALSE;
////		  fclose(fp);
//		  fp = stdin;
//		  continue;
//	    }
////		fclose(fp);
////	fp  = stdin;
//	    break;
//	  }
//	  wxPuts (t);
//	  atl_eval(t);
//    }
	
//    if (!fname){
//	  wxPuts (wxT("end file \n"));
//	  printf("\n");
//	}
//	wxPuts (wxT("end file \n"));
// поставил коммент чтобы выполнить
// обработку командной строки ниже
//    return;

// так можно подключать интерактивную часть 
// для работы из командной строки
  // char t[132];

//  while (printf("-> "), fgets(t, 132, stdin) != NULL){
//	wxPuts (wxT("begin atl_eval: "));
//	wxPuts (t);
//	atl_eval(t);
//	wxPuts (wxT("end atl_eval\n"));
//  }
//  return;
//}



/***
g++ -DPACKAGE_NAME=\"golly\" -DPACKAGE_TARNAME=\"golly\" -DPACKAGE_VERSION=\"3.3b2\" -DPACKAGE_STRING=\"golly\ 3.3b2\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_LIMITS_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_STDLIB_H=1 -DHAVE_MALLOC=1 -DHAVE_STDLIB_H=1 -DHAVE_REALLOC=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_MEMSET=1 -DHAVE_STRCHR=1 -DHAVE_STRDUP=1 -DHAVE_STRRCHR=1 -DHAVE_WX_BMP_HANDLER=1 -DHAVE_WX_GIF_HANDLER=1 -DHAVE_WX_PNG_HANDLER=1 -DHAVE_WX_TIFF_HANDLER=1 -DPYTHON_SHLIB=libpython2.7.so -DZLIB=1 -DVERSION=3.3b2 -I.  -I./../../gollybase/ -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -I'/usr/include/python2.7'   -DLUA_USE_LINUX -I./../../lua  -DGOLLYDIR="/usr/local/share/golly" -Wall -fno-strict-aliasing  -DMEMSTAT -DEXPORT -DREADONLYSTRINGS -g -O2  -MT golly-wxatlast.o -MD -MP -MF .deps/golly-wxatlast.Tpo -c -o golly-wxatlast.o `test -f '../../gui-wx/wxatlast.cpp' || echo './'`../../gui-wx/wxatlast.cpp


g++ -DGOLLYDIR="/usr/local/share/golly" -Wall -fno-strict-aliasing  -DMEMSTAT -DEXPORT -DREADONLYSTRINGS -g -O2  -Wl,--as-needed  -o golly golly-wxalgos.o golly-wxatlast.o golly-wxcontrol.o golly-wxcppforth.o golly-wxedit.o golly-wxfile.o golly-wxgforth.o golly-wxgolly.o golly-wxhelp.o golly-wxinfo.o golly-wxip1.o golly-wxlayer.o golly-wxlua.o golly-wxmain.o golly-wxoverlay.o golly-wxperl.o golly-wxprefs.o golly-wxpython.o golly-wxrender.o golly-wxrule.o golly-wxscript.o golly-wxselect.o golly-wxstatus.o golly-wxtimeline.o golly-wxundo.o golly-wxutils.o golly-wxview.o golly-wxzforth.o -L. -latlast -L/usr/lib/x86_64-linux-gnu -pthread   -lwx_gtk2u_gl-3.0 -lwx_gtk2u_xrc-3.0 -lwx_gtk2u_html-3.0 -lwx_gtk2u_qa-3.0 -lwx_gtk2u_adv-3.0 -lwx_gtk2u_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu_net-3.0 -lwx_baseu-3.0  -lgforth libgolly.a liblua.a libatlast.a  -latlast -lGL -ldl -lz -lm

***/
