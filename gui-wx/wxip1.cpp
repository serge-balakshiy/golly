

#include "wx/wxprec.h"     // for compilers that support precompilation
#ifndef WX_PRECOMP
    #include "wx/wx.h"     // for all others include the necessary headers
#endif
#include "wx/filename.h"   // for wxFileName
#include "bigint.h"
#include "lifealgo.h"
#include "qlifealgo.h"
#include "hlifealgo.h"
#include "readpattern.h"
#include "writepattern.h"

#include "wxgolly.h"       // for wxGetApp, mainptr, viewptr, statusptr
#include "wxmain.h"        // for mainptr->...
#include "wxselect.h"      // for Selection
#include "wxview.h"        // for viewptr->...
#include "wxstatus.h"      // for statusptr->...
#include "wxutils.h"       // for Warning, Note, GetString, etc
#include "wxprefs.h"       // for perllib, gollydir, etc
#include "wxinfo.h"        // for ShowInfo
#include "wxhelp.h"        // for ShowHelp
#include "wxundo.h"        // for currlayer->undoredo->...
#include "wxalgos.h"       // for *_ALGO, CreateNewUniverse, etc
#include "wxlayer.h"       // for AddLayer, currlayer, currindex, etc
#include "wxscript.h"      // for inscript, abortmsg, GSF_*, etc
// =============================================================================


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include "wxip1.h"

struct {
    uint8_t *ip;
    uint64_t accumulator;
} vm;

typedef enum {
    /* increment the register */
    OP_INC,
    /* decrement the register */
    OP_DEC,
    /* stop execution */
    OP_DONE
} opcode;

typedef enum interpret_result {
    SUCCESS,
    ERROR_UNKNOWN_OPCODE,
} interpret_result;

void vm_reset(void)
{
    puts("Reset vm state");
    vm = (typeof(vm)) { NULL };
}

interpret_result vm_interpret(uint8_t *bytecode)
{
    vm_reset();

    puts("Start interpreting");
    vm.ip = bytecode;
    for (;;) {
        uint8_t instruction = *vm.ip++;
        switch (instruction) {
        case OP_INC: {
            vm.accumulator++;
            break;
        }
        case OP_DEC: {
            vm.accumulator--;
            break;
        }
        case OP_DONE: {
            return SUCCESS;
        }
        default:
            return ERROR_UNKNOWN_OPCODE;
        }
    }

    return SUCCESS;
}

void RunIP1Script(const wxString & filepath){

//    (void) argc; (void) argv;

    {
        uint8_t code[] = { OP_INC, OP_INC, OP_DEC, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.accumulator);

        assert(result == SUCCESS);
        assert(vm.accumulator == 1);
    }

    {
        uint8_t code[] = { OP_INC, OP_DEC, OP_DEC, OP_DONE };
        interpret_result result = vm_interpret(code);
        printf("vm state: %" PRIu64 "\n", vm.accumulator);

        assert(result == SUCCESS);
        assert(vm.accumulator == UINT64_MAX);
    }

    //return EXIT_SUCCESS;
}
// Abort the currently running IP1 script.
void AbortIP1Script(){
  scripterr = wxString(abortmsg,wxConvLocal);
}

// Called when app is quitting.
void FinishIP1Scripting(){
  printf("ip1 finish \n");
}
