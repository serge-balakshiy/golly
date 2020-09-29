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
#include "wxembed.h"
#include <time.h>


// #include "lua.hpp"          // Lua header files for C++

#include <iostream>
#include <string.h>
using namespace std;

#include "wxargs.h"
#include "embed.h"
#include "embed_util.h"
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>


#define embed_fatal(...)   do { } while(0) //embed_die()
#define embed_error(...)   do { } while(0)
#define embed_warning(...) do { } while(0)
#define embed_info(...)    do { } while(0)
#define embed_debug(...)   do { } while(0)


struct vm_extension_t;
typedef struct vm_extension_t vm_extension_t;
typedef float vm_float_t;
typedef int32_t sdc_t;  // signed double cell type
typedef int (*embed_callback_extended_t)(vm_extension_t *v);
typedef struct {
 const char *name; 
 embed_callback_extended_t cb;
  bool use;
} callbacks_t;

struct vm_extension_t {
  embed_t *h;
  callbacks_t *callbacks;
  size_t callbacks_length;
  embed_opt_t o;
  cell_t error;
};
#define CALLBACK_XMACRO \
        X("privet", cb_privet, true)\
	X("f.",       cb_flt_print,  true)\
	X("show",       cb_show,  true)\
	X("rotate",       cb_rotate,  true)\
	X("update",       cb_emb_update,  true)\
	X("getcell", cb_emb_getcell, true)\
	X("setcell", cb_emb_setcell, true)\
	X("s>f",      cb_s2f,        true)\

#define X(NAME, FUNCTION, USE) static int FUNCTION ( vm_extension_t * const v );
  CALLBACK_XMACRO
#undef X

static callbacks_t callbacks[] = {
#define X(NAME, FUNCTION, USE) { .name = NAME, .cb = FUNCTION, .use = USE },
				  CALLBACK_XMACRO
#undef X
};

static inline size_t number_of_callbacks(void) { return sizeof(callbacks) / sizeof(callbacks[0]); }

static inline cell_t eset(vm_extension_t * const v, const cell_t error)
{ /**< set error register if not set */
        assert(v);
        if(!(v->error))
                v->error = error;
        return v->error;
}

static inline cell_t eget(vm_extension_t const * const v) 
{ /**< get current error register */
        assert(v);
        return v->error;
}

static inline cell_t eclr(vm_extension_t * const v) 
{ /**< clear error register and return value before clear */
        assert(v);
        const cell_t error = v->error;
        v->error = 0;
        return error;
}

static inline cell_t emb_pop(vm_extension_t *v) {
        assert(v);
        if(eget(v))
                return 0;
        cell_t rv = 0;
        int e = 0;
        if((e = embed_pop(v->h, &rv)) < 0)
                eset(v, e);
        return rv;
}

static inline void emb_push(vm_extension_t * const v, const cell_t value) {
        assert(v);
char buf[128] = { 0 }; 
        if(eget(v)) { 
	snprintf(buf, sizeof(buf)-1, "emb_push: error");
                return;
	}
        int e = 0;
        if((e = embed_push(v->h, value)) < 0)
                eset(v, e);
}

static inline void udpush(vm_extension_t * const v, const double_cell_t value) {
	emb_push(v, value);
	emb_push(v, value >> 16);
}

static inline double_cell_t udpop(vm_extension_t * const v) {
	const double_cell_t hi = emb_pop(v);
	const double_cell_t lo = emb_pop(v);
	const double_cell_t d  = (hi << 16) | lo;
	return d;
}

static inline sdc_t dpop(vm_extension_t * const v)                     { return udpop(v); }
static inline void  dpush(vm_extension_t * const v, const sdc_t value) { udpush(v, value); }

typedef union { vm_float_t f; double_cell_t d; } fd_u;


static inline vm_float_t fpop(vm_extension_t * const v) {
	BUILD_BUG_ON(sizeof(vm_float_t) != sizeof(double_cell_t));
	const fd_u fd = { .d = udpop(v) };
	return fd.f;
}

static inline void fpush(vm_extension_t * const v, const vm_float_t f) {
	const fd_u fd = { .f = f };
	udpush(v, fd.d);
}

static int cb_flt_print(vm_extension_t * const v) {
	const vm_float_t flt = fpop(v);
	char buf[512] = { 0 }; /* floats can be quite large */
	if(eget(v))
		return eclr(v);
	snprintf(buf, sizeof(buf)-1, "%e", flt);
	embed_puts(v->h, buf);
	return eclr(v);
}

static int cb_s2f(vm_extension_t * const v) {
	int16_t i = emb_pop(v);
	fpush(v, i);
	return eclr(v);
}


// * instance of 'vm_extension_t' */
static int callback_selector(embed_t *h, void *param) {
        assert(h);
        assert(param);
        vm_extension_t *e = (vm_extension_t*)param;
        if(e->h != h)
                embed_fatal("embed extensions: instance corruption");
        eclr(e);
        const cell_t func = emb_pop(e);
        if(eget(e))
                return eclr(e);
        if(func >= e->callbacks_length)
                return -21;
        const callbacks_t *cb = &e->callbacks[func];
        if(!(cb->use))
                return -21;
        return cb->cb(e);
}

static int callbacks_add(embed_t * const h, const bool optimize,  callbacks_t *cb, const size_t number) {
        assert(h && cb);
        const char *optimizer = optimize ? "-2 cells allot ' vm chars ," : " ";
        static const char *preamble = "only forth definitions system +order\n";
        int r = 0;
        if((r = embed_eval(h, preamble)) < 0) {
                embed_error("embed: eval(%s) returned %d", preamble, r);
                return r;
        }
        
        for(size_t i = 0; i < number; i++) {
                char line[80] = { 0 };
                if(!cb[i].use)
                        continue;
                r = snprintf(line, sizeof(line), ": %s %u vm ; %s\n", cb
[i].name, (unsigned)i, optimizer);
                assert(strlen(line) < sizeof(line) - 1);
                if(r < 0) {
                        embed_error("format error in snprintf (returned%d)", r);
                        return -1;
                }
                if((r = embed_eval(h, line)) < 0) {
                        embed_error("embed: eval(%s) returned %d", line, r);
                        return r;
                }
        }
        embed_reset(h);
        return 0;
}

static vm_extension_t *vm_extension_new(void) {
        vm_extension_t *v = (vm_extension_t*)embed_alloc(sizeof(*v));
        if(!v)
                return NULL;
        v->h = embed_new();
        if(!(v->h))
                goto fail;

        v->callbacks_length = number_of_callbacks(),
        v->callbacks        = callbacks;
        v->o                = embed_opt_default_hosted();
        v->o.callback       = callback_selector;
        v->o.param          = v;
        embed_opt_set(v->h, &v->o);

        if(callbacks_add(v->h, true, v->callbacks, v->callbacks_length)< 0) {
                embed_error("adding callbacks failed");
                goto fail;
        }
        
        return v;
fail:
        if(v->h)
                embed_free(v->h);
        return NULL;
}

static int vm_extension_run(vm_extension_t *v) {
        assert(v);
        return embed_vm(v->h);
}

static void vm_extension_free(vm_extension_t *v) {
        assert(v);
        embed_free(v->h);
        memset(v, 0, sizeof(*v));
        free(v);
}

static bool aborted = false;    // stop the current script?
static void embCheckEvents()
{
    // this routine is called at the start of every g_* function so we can
    // detect user events (eg. hitting the stop button or escape key)
    if (allowcheck) wxGetApp().Poller()->checkevents();
    
    if (insideYield) return;
    
    // we're outside Yield so safe to do a longjmp from lua_error
    if (aborted) {
        // AbortLuaScript was called
	    cout << "embed: checkexents error" << endl;
	    return;
    }
    return;
}


static int cb_privet(vm_extension_t * const v){
//  printf("privet from golly\n");
	char buf[512] = { 0 }; /* floats can be quite large */
	if(eget(v))
		return eclr(v);
	snprintf(buf, sizeof(buf)-1, "Privet from Golly");
	embed_puts(v->h, buf);
//	return eclr(v);

  emb_push(v, 42);
  return eclr(v);
}

static int cb_emb_update(vm_extension_t * const v){
  embCheckEvents();
  if(eget(v))
    return eclr(v);
  GSF_update();
  return eclr(v);
}


static int cb_rotate(vm_extension_t * const v){
  embCheckEvents();
  int direction = 1;
  if (viewptr->SelectionExists()){
	viewptr->RotateSelection(direction == 0);
	DoAutoUpdate();
	return eclr(v);
    //push(0);
  }else{
    return eclr(v);
  }
}

static int cb_emb_getcell(vm_extension_t * const v){ // ( x y -- n = state )
  embCheckEvents();
  cell_t y=emb_pop(v);
  cell_t x=emb_pop(v);
//  int e = embed_pop(v->h, &e);
  printf("cb_emb_getcell: x=%d, y=%d, ", x, y);
//  e = embed_pop(v->h, &x);
  const char* err = GSF_checkpos(currlayer->algo, x, y);
  if (err){ 
    return eclr(v);
  }else{
    embed_push(v->h, currlayer->algo->getcell(x, y));
  }
return eclr(v);
}

static int cb_emb_setcell(vm_extension_t * const v){ // ( x y state -- - )
  embCheckEvents();
  cell_t e = emb_pop(v); 
  cell_t y=emb_pop(v); 
  cell_t x=emb_pop(v); 

//  printf("cb_emb_setcell: x=%d, y=%d, e=%d, ", x, y, e);

  const char* err = GSF_setcell(x, y, e ); 
  if (err){ 
    eset(v, -1);
//    return eclr(v);
  }
// embed_push(v->h, e);
  return eclr(v);
}

static int cb_show(vm_extension_t * const v){
  if(eget(v))
    return eclr(v);
  inscript = false;
  statusptr->DisplayMessage(wxString("Privet From Golly")); //, ATL_ENC));
  inscript = true;
  if (!showstatus) mainptr->ToggleStatusBar();
  return eclr(v);
}

void RunEmbedScript(const wxString& filepath){
  char t[132];
  char **ArgAddPtr (char **pp, int size, char *str);
  
  int argc0 = 0;
  char **argv0 = 0;
  char* pchr = (const_cast<char*>((const char*)filepath.mb_str()));
  argv0 = ArgAddPtr( argv0, argc0, "embed");
  argc0++;
  argv0 = ArgAddPtr( argv0, argc0, pchr);
  argc0++;

  BUILD_BUG_ON(sizeof(double_cell_t) != sizeof(sdc_t));
  vm_extension_t *v = vm_extension_new();
  printf("RunEmbedScript: vm_extension_new, cb.name %s\n", v->callbacks[0].name);
  if(!v)
    embed_fatal("embed extensions: load failed");
//  const int r = vm_extension_run(v);

//embed_new есть уже в vm_extension_new
  int res = 0;

//  embed_opt_t o = embed_opt_default_hosted();
//  embed_opt_set(v->h, &o);
//o.read = mmu_read_cb;

  if(argc0 > 1){
//    o.options |= (const_cast<embed_vm_option_e*>((int)EMBED_VM_QUITE_ON));
    for(int i =1; i < argc0; i++){
      FILE *in = embed_fopen_or_die(argv0[i], "rb");
      v->o.in = in;
      embed_opt_set(v->h, &v->o);
      res = embed_vm(v->h); // vm_extension_run
      fclose(in);
      v->o.in = stdin;
      embed_opt_set(v->h, &v->o);
      if(res<0) 
	printf("error\n"); // res;
    }
  }else{
    res = embed_vm(v->h);
  }

  vm_extension_free(v);
}

void AbortEmbedScript(){
//  aborted = true;
  scripterr = wxString(abortmsg, wxConvLocal);
}

void FinishEmbedScripting(){
  // no need to do anything
  printf("embed finish \n");
}

/*

g++ -DPACKAGE_NAME=\"golly\" -DPACKAGE_TARNAME=\"golly\" -DPACKAGE_VERSION=\"3.3b2\" -DPACKAGE_STRING=\"golly\ 3.3b2\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_LIMITS_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_STDLIB_H=1 -DHAVE_MALLOC=1 -DHAVE_STDLIB_H=1 -DHAVE_REALLOC=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_MEMSET=1 -DHAVE_STRCHR=1 -DHAVE_STRDUP=1 -DHAVE_STRRCHR=1 -DHAVE_WX_BMP_HANDLER=1 -DHAVE_WX_GIF_HANDLER=1 -DHAVE_WX_PNG_HANDLER=1 -DHAVE_WX_TIFF_HANDLER=1 -DPYTHON_SHLIB=libpython2.7.so -DZLIB=1 -DVERSION=3.3b2 -I.  -I./../../gollybase/ -I/usr/lib/x86_64-linux-gnu/wx/include/gtk3-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -I'/usr/include/python2.7'   -DLUA_USE_LINUX -DMEMSTAT -DEXPORT -DREADONLYSTRINGS  -I./../../atlast-2.0 -I./../../libforth -I./../../embed -I./../../lua  -DGOLLYDIR="/usr/local/share/golly" -Wall -fno-strict-aliasing  -g -O2 -MT util.o libembed.a golly-wxembed.o -MD -MP -MF .deps/golly-wxembed.Tpo -c -o golly-wxembed.o `test -f '../../gui-wx/wxembed.cpp' || echo './'`../../gui-wx/wxembed.cpp

*/
