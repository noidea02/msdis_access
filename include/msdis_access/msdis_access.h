#ifndef MSDIS_ACCESS_IG
#define MSDIS_ACCESS_IG

#ifndef _WIN64
    #error Unsupported target platform.
#endif

#include <Windows.h>

/* Types
*/

typedef enum {

    MSDIS_ARCH_X86,
    MSDIS_ARCH_X64

} msdis_arch_t;

typedef LPVOID msdis_handle_t;

typedef struct {

    SIZE_T num_bytes_disassembled;
    WCHAR str[256];

} msdis_result_t;

/* Functions
*/

BOOL msdis_create(LPCWSTR dll_parent_folder, msdis_arch_t arch, msdis_handle_t* dis);
BOOL msdis_disasm(msdis_handle_t dis, UINT64 addr, LPCBYTE bytes, SIZE_T num_bytes, msdis_result_t* result);
void msdis_free(msdis_handle_t dis);

/* Implementation
*/

#ifdef MSDIS_ACCESS_IMPL
    #define MSDIS_ACCESS_PRV_UNLOCK_DETAIL
    #include "detail/msdis_access_impl.h"
    #undef MSDIS_ACCESS_PRV_UNLOCK_DETAIL
#endif

#endif
