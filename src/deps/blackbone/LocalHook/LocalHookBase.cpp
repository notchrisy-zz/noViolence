#include "LocalHookBase.h"
#define BitTestAndResetT    _bittestandreset
#define NIP Eip

namespace blackbone
{
std::unordered_map<void*, DetourBase*> DetourBase::_breakpoints;
void* DetourBase::_vecHandler = nullptr;

DetourBase::DetourBase()
{
}

DetourBase::~DetourBase()
{
    VirtualFree( _buf, 0, MEM_RELEASE );
}

bool DetourBase::AllocateBuffer( uint8_t* nearest )
{
    if (_buf != nullptr)
        return true;

    MEMORY_BASIC_INFORMATION mbi = { 0 };
    for (SIZE_T addr = (SIZE_T)nearest; addr > (SIZE_T)nearest - 0x80000000; addr = (SIZE_T)mbi.BaseAddress - 1)
    {
        if (!VirtualQuery( (LPCVOID)addr, &mbi, sizeof( mbi ) ))
            break;

        if (mbi.State == MEM_FREE)
        {
            _buf = (uint8_t*)VirtualAlloc(
                (uint8_t*)mbi.BaseAddress + mbi.RegionSize - 0x1000, 0x1000,
                MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE
                );

            if (_buf)
                break;
        }
    }

    if (_buf == nullptr)
        _buf = (uint8_t*)VirtualAlloc( nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

    _origCode = _buf + 0x100;
    _origThunk = _buf + 0x200;
    _newCode  = _buf + 0x300;

    return _buf != nullptr;
}

bool DetourBase::DisableHook()
{
    WriteProcessMemory( GetCurrentProcess(), _original, _origCode, _origSize, NULL );

    return true;
}

bool DetourBase::EnableHook()
{
	WriteProcessMemory( GetCurrentProcess(), _original, _newCode, _origSize, NULL );

    return true;
}

LONG NTAPI DetourBase::VectoredHandler( PEXCEPTION_POINTERS excpt )
{
    switch (excpt->ExceptionRecord->ExceptionCode)
    {
        case static_cast<DWORD>(EXCEPTION_BREAKPOINT):
            return Int3Handler( excpt );

        case static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION):
            return AVHandler( excpt );

        case static_cast<DWORD>(EXCEPTION_SINGLE_STEP):
            return StepHandler( excpt );

        default:
            return EXCEPTION_CONTINUE_SEARCH;
    }
}

LONG NTAPI DetourBase::Int3Handler( PEXCEPTION_POINTERS excpt )
{
    if (_breakpoints.count( excpt->ExceptionRecord->ExceptionAddress ))
    {
        DetourBase* pInst = _breakpoints[excpt->ExceptionRecord->ExceptionAddress];

        ((_NT_TIB*)NtCurrentTeb())->ArbitraryUserPointer = (void*)pInst;
        excpt->ContextRecord->NIP = (uintptr_t)pInst->_internalHandler;

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

LONG NTAPI DetourBase::AVHandler( PEXCEPTION_POINTERS /*excpt*/ )
{
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG NTAPI DetourBase::StepHandler( PEXCEPTION_POINTERS excpt )
{
    DWORD index = 0;
    int found = _BitScanForward( &index, static_cast<DWORD>(excpt->ContextRecord->Dr6) );

    if (found != 0 && index < 4 && _breakpoints.count( excpt->ExceptionRecord->ExceptionAddress ))
    {
        DetourBase* pInst = _breakpoints[excpt->ExceptionRecord->ExceptionAddress];

        // Disable breakpoint at current index
        BitTestAndResetT( (LONG_PTR*)&excpt->ContextRecord->Dr7, 2 * index );

        ((_NT_TIB*)NtCurrentTeb())->ArbitraryUserPointer = (void*)pInst;
        excpt->ContextRecord->NIP = (uintptr_t)pInst->_internalHandler;

        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

}