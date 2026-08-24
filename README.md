# msdis_access
 
Wrapper library (C, header-only) for the undocumented Microsoft Disassembler DLL (MSDis). Provides easy access to the x86 instruction decoding functionality of MSDis.

This library only works on Windows-based systems and requires MSVCDIS140.DLL to be present. When initializing the library, the location of MSVCDIS140.DLL must be specified. In general,
being backwards compatible with older versions of MSDis is not a goal of this library.

The wrapper code itself is thread-safe, however the status of MSDis regarding thread safety is unverified. MSDis does use a handle system to identify different disassembler instances and
so far is being used in multithreaded applications successfully (without additional synchronization), however this still doesn't mean that MSDis is officially thread-safe. If you use this
library (and therefore MSDis) in multithreaded applications without additional synchronization, you are doing so at your own risk.

Minimal C++ sample:
```c++
#include <stdint.h>
#include <iostream>

// Include msdis_access and define symbols in this compilation unit.
#define MSDIS_ACCESS_IMPL
#include <msdis_access/msdis_access.h>

int main(int argc, char* argv[]) {

    // MSDis handle must be explicitly freed when it is no longer needed.
    msdis_handle_t msdis{};

    // Initialize MSDis. Notice that the location of MSVCDIS140.DLL and the target arch must be specified.
    if (!msdis_create(L"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\IDE", MSDIS_ARCH_X64, &msdis))
        return 1;

    // Initialize instruction data (CMPXCHG with LOCK prefix and SIB byte).
    uint64_t instruction_address{ 0x7ff000000000 };
    const uint8_t instruction[]{ 0xf0, 0x4f, 0x0f, 0xb1, 0xb4, 0xec, 0x44, 0x55, 0x66, 0x77 };
    msdis_result_t msdis_out{};
    BOOL success{};

    // Call MSDis to decode the instruction.
    if (success = msdis_disasm(msdis, instruction_address, instruction, sizeof(instruction), &msdis_out)) {

        // Process result.
        std::wcout << msdis_out.num_bytes_disassembled << " bytes have been decoded" << std::endl;
        std::wcout << msdis_out.str << std::endl;
    }

    // Free MSDis handle.
    msdis_free(msdis);
    return success ? 0 : 1;
}
```