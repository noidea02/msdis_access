#ifndef MSDIS_ACCESS_IMPL_IG
#define MSDIS_ACCESS_IMPL_IG

#ifndef MSDIS_ACCESS_PRV_UNLOCK_DETAIL
#error "Direct inclusion of detail headers is forbidden."
#endif

#include <string.h>
#include <wchar.h>

#include "msdis_access/msdis_access.h"

/* Constants
*/

#define MSDIS_PRV_NAME_DLL L"msvcdis140.dll"

#define MSDIS_PRV_SYMBOL_PDISNEW "?PdisNew@DISX86@@SAPEAVDIS@@W4DIST@2@@Z"
#define MSDIS_PRV_SYMBOL_SETADDR64 "?SetAddr64@DIS@@QEAAX_N@Z"
#define MSDIS_PRV_SYMBOL_CCHFORMATINSTR "?CchFormatInstr@DIS@@QEBA_KPEA_W_K@Z"

#define MSDIS_PRV_ARCH_VAL_X86 0x09
#define MSDIS_PRV_ARCH_VAL_X64 0x0b

/* Types
*/

typedef void* (*msdis_prv_pdisnew_t)(DWORD type);
typedef void (*msdis_prv_setaddr64_t)(msdis_handle_t dis, BOOL x64);
typedef void (*msdis_prv_cchformatinstr_t)(msdis_handle_t dis, LPWSTR out, SIZE_T out_len);

typedef void (*msdis_prv_unk_method_t)();
typedef void (*msdis_prv_dtor_t)(msdis_handle_t dis, int unknown);
typedef DWORD(*msdis_prv_cbdisassemble_t)(msdis_handle_t dis, DWORD64 addr, LPCVOID bytes, SIZE_T num_bytes);

typedef struct {

    HMODULE lib_handle;
    LPVOID dis_handle;
    msdis_prv_pdisnew_t pdisnew;
    msdis_prv_setaddr64_t setaddr64;
    msdis_prv_cchformatinstr_t cchformatinstr;

} msdis_prv_state_t;

typedef struct {

    msdis_prv_dtor_t dtor;
    msdis_prv_unk_method_t unk0, unk1, unk2, unk3, unk4, unk5, unk6;
    msdis_prv_cbdisassemble_t cbdisassemble;
    msdis_prv_unk_method_t unk7, unk8, unk9, unk10, unk11, unk12, unk13, unk14, unk15;

} msdis_prv_vtable_t;

/* Functions
*/

static const msdis_prv_vtable_t* msdis_prv_get_vtable(msdis_handle_t dis) {

    const void* const* dis_pp = (const void* const*)dis;
    return (const msdis_prv_vtable_t*)*dis_pp;
}

static void msdis_prv_free_state(msdis_prv_state_t* state) {

    const LPVOID dis_handle = state->dis_handle;
    if (dis_handle) {

        const msdis_prv_vtable_t* const vtable = msdis_prv_get_vtable(dis_handle);
        vtable->dtor(dis_handle, 1);
    }

    if (state->lib_handle)
        FreeLibrary(state->lib_handle);

    free(state);
}

static BOOL msdis_prv_lookup_arch_val(msdis_arch_t arch, DWORD* val) {

    switch (arch) {

    case MSDIS_ARCH_X86:

        *val = MSDIS_PRV_ARCH_VAL_X86;
        break;

    case MSDIS_ARCH_X64:

        *val = MSDIS_PRV_ARCH_VAL_X64;
        break;

    default:

        return FALSE;
    }

    return TRUE;
}

BOOL msdis_create(LPCWSTR dll_parent_folder, msdis_arch_t arch, msdis_handle_t* dis) {
    
    SIZE_T parent_folder_len = 0;
    WCHAR dll_name[MAX_PATH] = { 0 };
    msdis_prv_state_t* msdis_state = NULL;
    DWORD msdis_arch_val = 0;
    BOOL success = FALSE;
    
    if (!dll_parent_folder || !dis)
        return FALSE;

    parent_folder_len = wcslen(dll_parent_folder);
    if (!parent_folder_len)
        return FALSE;

    if (dll_parent_folder[parent_folder_len - 1] == L'\\')
        swprintf(dll_name, ARRAYSIZE(dll_name) - 1, L"%s%s", dll_parent_folder, MSDIS_PRV_NAME_DLL);
    else
        swprintf(dll_name, ARRAYSIZE(dll_name) - 1, L"%s\\%s", dll_parent_folder, MSDIS_PRV_NAME_DLL);

    msdis_state = (msdis_prv_state_t*)calloc(sizeof(*msdis_state), 1);
    if (!msdis_state)
        return FALSE;

    msdis_state->lib_handle = LoadLibraryW(dll_name);
    if (!msdis_state->lib_handle)
        goto cleanup;

    msdis_state->pdisnew = (msdis_prv_pdisnew_t)GetProcAddress(msdis_state->lib_handle, MSDIS_PRV_SYMBOL_PDISNEW);
    if (!msdis_state->pdisnew)
        goto cleanup;

    msdis_state->setaddr64 = (msdis_prv_setaddr64_t)GetProcAddress(msdis_state->lib_handle, MSDIS_PRV_SYMBOL_SETADDR64);
    if (!msdis_state->setaddr64)
        goto cleanup;

    msdis_state->cchformatinstr = (msdis_prv_cchformatinstr_t)GetProcAddress(msdis_state->lib_handle, MSDIS_PRV_SYMBOL_CCHFORMATINSTR);
    if (!msdis_state->cchformatinstr)
        goto cleanup;

    if (!msdis_prv_lookup_arch_val(arch, &msdis_arch_val))
        goto cleanup;

    msdis_state->dis_handle = msdis_state->pdisnew(msdis_arch_val);
    if (!msdis_state->dis_handle)
        goto cleanup;

    if (arch == MSDIS_ARCH_X64)
        msdis_state->setaddr64(msdis_state->dis_handle, TRUE);

    *dis = msdis_state;
    success = TRUE;

cleanup:

    if (!success) {
        
        if (msdis_state)
            msdis_prv_free_state(msdis_state);
    }

    return success;
}

BOOL msdis_disasm(msdis_handle_t dis, UINT64 addr, LPCBYTE bytes, SIZE_T num_bytes, msdis_result_t* result) {
    
    const msdis_prv_state_t* msdis_state = NULL;
    LPVOID dis_handle = NULL;
    const msdis_prv_vtable_t* vtable = NULL;
    SIZE_T num_bytes_disassembled = 0;

    if (!dis || !bytes || !result)
        return FALSE;

    msdis_state = (msdis_prv_state_t*)dis;
    dis_handle = msdis_state->dis_handle;
    vtable = msdis_prv_get_vtable(dis_handle);
    
    ZeroMemory(result, sizeof(*result));

    num_bytes_disassembled = vtable->cbdisassemble(dis_handle, addr, bytes, num_bytes);
    if (!num_bytes_disassembled) {
        
        if (!num_bytes)
            return TRUE;
        
        return FALSE;
    }

    result->num_bytes_disassembled = num_bytes_disassembled;
    msdis_state->cchformatinstr(dis_handle, result->str, ARRAYSIZE(result->str) - 1);

    return TRUE;
}

void msdis_free(msdis_handle_t dis) {

    if (!dis)
        return;

    msdis_prv_free_state((msdis_prv_state_t*)dis);
}

#endif
