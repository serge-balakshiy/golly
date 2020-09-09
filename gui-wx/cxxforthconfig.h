// cxxforthconfig.h.in is processed by cmake to produce cxxforthconfig.h
// 
// If you are not using CMake, you can just create an empty
// file named "cxxforthconfig.h", and everything will build properly.

#ifndef cxxforthconfig_h_included
#define cxxforthconfig_h_included

#define CXXFORTH_DATASPACE_SIZE    ((16 * 1024 * sizeof(Cell)))
#define CXXFORTH_DSTACK_COUNT      (256)
#define CXXFORTH_RSTACK_COUNT      (256)

#define CXXFORTH_USE_READLINE
/* #undef CXXFORTH_SKIP_RUNTIME_CHECKS */
/* #undef CXXFORTH_DISABLE_MAIN */
/* #undef CXXFORTH_DISABLE_FILE_ACCESS */

#endif // cxxforthconfig_h_included

