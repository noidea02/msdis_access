/*
This sample console application uses msdis_access to disassemble user-provided x86 instruction bytes. It requires 2 start arguments:

>invoke_msdis [MODE] [INSTRUCTION_BYTES]

    MODE specifies the default address size (x86_32 or x86_64).
    INSTRUCTION_BYTES must be a properly formatted hex sequence (e.g. \x00\x00).

Exemplatory usages:

>invoke_msdis x86_32 "\x67\x01\x00"

    Stdout:
        add dword ptr[bx+si], eax

    Exit code: 0 (Success)

>invoke_msdis x86_64 "\x00\x00\x48"

    Stdout:
        add byte ptr[rax], al

    Exit code: 1 (Failure due to trailing \x48 in 64-bit mode)
*/
#include <stdio.h>

#define MSDIS_ACCESS_IMPL
#include <msdis_access/msdis_access.h>

/* Folder must contain MSVCDIS140.DLL. */
#define MSDIS_PARENT_FOLDER L"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\IDE"
#define DISASM_BASE_ADDRESS32 0x7FFF0000
#define DISASM_BASE_ADDRESS64 0x7FFFFFFF00000000

static BOOL lookup_mode(const char* str, msdis_arch_t* arch);
static BOOL hex_char_to_nibble(char chr, BYTE* nibble);
static BOOL hex_str_to_byte_array(const char* str, BYTE* arr, SIZE_T* arr_size);
static int disassemble_and_print(msdis_arch_t mode, BYTE* instruction, SIZE_T instruction_size);

int main(int argc, char* argv[]) {
    
    int exit_code = 1;
    const char* mode_str = NULL;
    const char* instruction_hex_str = NULL;

    msdis_arch_t arch = MSDIS_ARCH_X86;
    SIZE_T inst_buf_size = 0;
    BYTE* inst_buf = NULL;

    if (argc < 3)
        goto cleanup;

    mode_str = argv[1];
    instruction_hex_str = argv[2];

    /* Determine MSDis mode. */
    if (!lookup_mode(mode_str, &arch))
        goto cleanup;

    hex_str_to_byte_array(instruction_hex_str, 0, &inst_buf_size);

    inst_buf = calloc(inst_buf_size, 1);
    if (!hex_str_to_byte_array(instruction_hex_str, inst_buf, &inst_buf_size))
        goto cleanup;

    /* Hand over instruction bytes to MSDis and print results. */
    exit_code = disassemble_and_print(arch, inst_buf, inst_buf_size);

cleanup:

    if (inst_buf)
        free(inst_buf);

    return exit_code;
}

BOOL lookup_mode(const char* str, msdis_arch_t* arch) {

    if (!strcmp(str, "x86_32")) {
        *arch = MSDIS_ARCH_X86;
    }
    else if (!strcmp(str, "x86_64")) {
        *arch = MSDIS_ARCH_X64;
    }
    else {
        return FALSE;
    }

    return TRUE;
}

BOOL hex_char_to_nibble(char chr, BYTE* nibble) {

    if (chr >= '0' && chr <= '9') {

        *nibble = chr - '0';
        return TRUE;
    }

    if (chr >= 'a' && chr <= 'f') {

        *nibble = chr - 'a' + 10;
        return TRUE;
    }

    if (chr >= 'A' && chr <= 'F') {

        *nibble = chr - 'A' + 10;
        return TRUE;
    }

    return FALSE;
}

BOOL hex_str_to_byte_array(const char* str, BYTE* buf, SIZE_T* buf_size) {

    const SIZE_T str_len = strlen(str);
    SIZE_T str_index = 0;
    SIZE_T buf_index = 0;
    int cur_byte = -1;
    BOOL no_buf = FALSE;

    for (; str_index != str_len; ++str_index) {

        const char chr = str[str_index];
        BYTE nibble = 0;

        if (chr == '\\' || chr == 'x') {

            cur_byte = -1;
            continue;
        }

        if (!hex_char_to_nibble(chr, &nibble))
            return FALSE;

        if (cur_byte == -1) {

            cur_byte = nibble;
            continue;
        }

        cur_byte = (cur_byte << 4) | nibble;

        if (buf && buf_index < *buf_size) {
            buf[buf_index] = cur_byte;
        }
        else if (!no_buf) {
            no_buf = TRUE;
        }

        ++buf_index;
        cur_byte = -1;
    }

    if (cur_byte != -1)
        return FALSE;

    *buf_size = buf_index;
    return !no_buf;
}

int disassemble_and_print(msdis_arch_t arch, BYTE* instruction, SIZE_T instruction_size) {

    int exit_code = 1;
    msdis_handle_t msdis = NULL;
    SIZE_T byte_index = 0;
    UINT64 base_addr = 0;

    /* Select base address. */
    switch (arch) {
    case MSDIS_ARCH_X86:

        base_addr = DISASM_BASE_ADDRESS32;
        break;

    case MSDIS_ARCH_X64:

        base_addr = DISASM_BASE_ADDRESS64;
        break;

    default:

        break;
    }

    /* Load library and create MSDis handle. */
    if (!msdis_create(MSDIS_PARENT_FOLDER, arch, &msdis))
        goto cleanup;

    while (byte_index < instruction_size) {

        /* Do disassemble instruction bytes. */
        msdis_result_t msdis_result = { 0 };
        if (!msdis_disasm(msdis, base_addr + byte_index, &instruction[byte_index], instruction_size - byte_index, &msdis_result))
            goto cleanup;

        /* Print result. */
        wprintf(L"%s\n", msdis_result.str);
        byte_index += msdis_result.num_bytes_disassembled;
    }

    if (byte_index != instruction_size)
        goto cleanup;

    exit_code = 0;

cleanup:

    /* Free MSDis handle and library. */
    if (msdis)
        msdis_free(msdis);

    return  exit_code;
}
