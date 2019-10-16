#include "TraceHook.h"
#include <Windows.h>
#include <algorithm>

#define BEA_USE_STDCALL

#define NAX Eax
#define NIP Eip
#define NDI Edi
#define NSP Esp


#define HIGHEST_BIT_SET     (1LL << (sizeof(void*) * 8 - 1))
#define HIGHEST_BIT_UNSET  ~HIGHEST_BIT_SET
#define SingleStep          0x100

#define ADDR_MASK 0xFFFFF000

namespace blackbone
{

TraceHook::TraceHook()
{
}

TraceHook::~TraceHook()
{
    if (_pExptHandler != nullptr)
        RemoveVectoredExceptionHandler( _pExptHandler );
}

TraceHook& TraceHook::Instance()
{
    static TraceHook instance;  
    return instance;
}

bool TraceHook::ApplyHook( void* targetFunc, 
                           void* hookFunc, 
                           void* ptrAddress, 
                           const HookContext::vecState& tracePath /*= HookContext::vecState()*/,
                           void* chekcIP /*= 0*/)
{
    if (_contexts.count( (uintptr_t)ptrAddress ))
    {
        HookContext& ctx = _contexts[(uintptr_t)ptrAddress];

        // Already hooked
        if (ctx.hooks.count( (uintptr_t)targetFunc ))
            return false;
        else
            ctx.hooks.emplace( (uintptr_t)targetFunc, std::make_pair( (uintptr_t)hookFunc, false ) );
    }
    else
    {
        HookContext ctx;

        // Setup context
        ctx.targetPtr = (uintptr_t)ptrAddress;
        ctx.checkIP = (uintptr_t)chekcIP;
        ctx.origPtrVal = *(uintptr_t*)ptrAddress;
        ctx.breakValue = _breakPtr;
        ctx.tracePath = tracePath;

        ctx.hooks.emplace( (uintptr_t)targetFunc, std::make_pair( (uintptr_t)hookFunc, false ) );
        _contexts.emplace( (uintptr_t)ptrAddress, std::move( ctx ) );

        if (_pExptHandler == nullptr)
            _pExptHandler = AddVectoredExceptionHandler( 0, &TraceHook::VecHandler );

        // Setup exception
        *(uintptr_t*)ptrAddress = ctx.breakValue;
        _breakPtr += 0x10;
    }

    return true;
}

bool TraceHook::RemoveHook( void* targetFunc )
{
    auto findfn = [targetFunc]( const mapContext::value_type& val ) { 
        return val.second.hooks.count( (uintptr_t)targetFunc );
    };

    auto iter = std::find_if( _contexts.begin(), _contexts.end(), findfn );
                              
    if (iter != _contexts.end())
    {
        auto& ctx = iter->second;

        // Remove function from list
        ctx.hooks.erase( (uintptr_t)targetFunc );

        if (ctx.hooks.empty())
        {
            // Remove hook itself
            *(uintptr_t*)ctx.targetPtr = ctx.origPtrVal;
            _contexts.erase( iter );
        }

        Sleep( 10 );

        // Remove exception handler
        if (_contexts.empty() && _pExptHandler != nullptr)
        {
            RemoveVectoredExceptionHandler( _pExptHandler );
            _pExptHandler = nullptr;
        }

        return true;
    }

    return false;
}

LONG __stdcall TraceHook::VecHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
    return Instance().VecHandlerP( ExceptionInfo );
}

LONG TraceHook::VecHandlerP( PEXCEPTION_POINTERS ExceptionInfo )
{
    auto exptContex = ExceptionInfo->ContextRecord;
    auto exptRecord = ExceptionInfo->ExceptionRecord;
    auto exptCode   = exptRecord->ExceptionCode;

    HookContext* ctx = &_contexts.begin()->second;

    if (exptCode != EXCEPTION_SINGLE_STEP && exptCode != EXCEPTION_ACCESS_VIOLATION)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else if (exptCode == EXCEPTION_ACCESS_VIOLATION && (ctx->state == TS_Step || ctx->state == TS_StepInto))
    {
        if ((exptRecord->ExceptionInformation[1] & ADDR_MASK) != (ctx->breakValue & ADDR_MASK))
            return EXCEPTION_CONTINUE_SEARCH;

        if (ctx->checkIP != 0 && exptContex->NIP != ctx->checkIP)
        {
            exptContex->EFlags |= SingleStep;

            RestorePtr( *ctx, ExceptionInfo );
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    
    switch (ctx->state)
    {
        case TS_Start:
            {
                ctx->state = ctx->tracePath[ctx->stateIdx].action;

                RestorePtr( *ctx, ExceptionInfo );
                return VecHandlerP( ExceptionInfo );
            }
            break;

        case TS_Step:
            {
                if (CheckBranching( *ctx, exptContex->NIP, exptContex->NSP ))
                {
                    if (ctx->hooks.count( exptContex->NIP ))
                    {
                        HandleBranch( *ctx, exptContex );
                        return EXCEPTION_CONTINUE_EXECUTION;
                    }
                    else
                    {
                        ctx->state = TS_WaitReturn;
                        BreakOnReturn( exptContex->NSP );
                    }
                }
                else
                    exptContex->EFlags |= SingleStep;
            }
            break;

        case TS_StepOut:
            {
                // Get current stack frame
                vecStackFrames frames;
                StackBacktrace( exptContex->NIP, exptContex->NSP, frames, 1 );

                if (frames.size() > 1)
                {
                    ctx->stateIdx++;
                    ctx->state = TS_WaitReturn;
                    BreakOnReturn( frames.back().first );
                }
            }
            break;

        case TS_StepInto:
            {
                // Check if step into path function has occurred
                if (CheckBranching( *ctx, exptContex->NIP, exptContex->NSP ))
                {
                    if (exptContex->NIP == ctx->tracePath[ctx->stateIdx].arg)
                    {
                        ctx->stateIdx++;
                        ctx->state = ctx->tracePath[ctx->stateIdx].action;
                    }
                }

                exptContex->EFlags |= SingleStep;
            }
            break;

        case TS_WaitReturn:
            {
                exptContex->NIP &= HIGHEST_BIT_UNSET;

                exptContex->EFlags |= SingleStep;
                ctx->state = ctx->tracePath[ctx->stateIdx].action;
            }
            break;

        default:
            break;
    }

    ctx->lastIP = exptContex->NIP;
    ctx->lastSP = exptContex->NSP;
    
    return EXCEPTION_CONTINUE_EXECUTION;
}

bool TraceHook::CheckBranching( const HookContext& ctx, uintptr_t ip, uintptr_t sp )
{
    // Not yet initialized
    if (ctx.lastIP == 0 || ctx.lastSP == 0)
        return false;

    if (ip - ctx.lastIP >= 8 && sp != ctx.lastSP)
    {
        //DISASM info = { 0 };
        //info.EIP = ctx.lastIP;
    }

    return false;
}

void TraceHook::HandleBranch( HookContext& ctx, PCONTEXT exptContex )
{
    ctx.hooks[exptContex->NIP].second = true;

    auto iter = std::find_if( ctx.hooks.begin(), ctx.hooks.end(),
                              []( const decltype(ctx.hooks)::value_type& val ){ return (val.second.second == false); } );

    if (iter != ctx.hooks.end())
    {
        ctx.state = TS_WaitReturn;
        BreakOnReturn( exptContex->NSP );
    }
    else
        ctx.reset();

    exptContex->NIP = ctx.hooks[exptContex->NIP].first;
}

inline void TraceHook::BreakOnReturn( uintptr_t sp )
{
    *(DWORD_PTR*)sp |= HIGHEST_BIT_SET;
}

bool TraceHook::RestorePtr( const HookContext& ctx, PEXCEPTION_POINTERS ExceptionInfo )
{
    bool found = false;
    auto expCtx = ExceptionInfo->ContextRecord;

    if (ExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 8)
    {
        expCtx->NIP = ctx.origPtrVal;
        return true;
    }

    for (DWORD_PTR* pRegVal = &expCtx->NDI; pRegVal <= &expCtx->NAX; pRegVal++)  
    {
        // Compare high address parts
        if ((*pRegVal & ADDR_MASK) == (ExceptionInfo->ExceptionRecord->ExceptionInformation[1] & ADDR_MASK))
        {
            *pRegVal = ctx.origPtrVal;
            found = true;
        }
    }

    return found;
}

size_t TraceHook::StackBacktrace( uintptr_t ip, uintptr_t sp, vecStackFrames& results, uintptr_t depth /*= 10 */ )
{
    SYSTEM_INFO sysinfo = {};
    uintptr_t stack_base = (uintptr_t)((PNT_TIB)NtCurrentTeb())->StackBase;

    GetNativeSystemInfo( &sysinfo );

    results.emplace_back( 0, ip );

    for (uintptr_t stackPtr = sp; stackPtr < stack_base && results.size() <= depth; stackPtr += sizeof(void*))
    {
        uintptr_t stack_val = *(uintptr_t*)stackPtr;
        MEMORY_BASIC_INFORMATION meminfo = { 0 };

        uintptr_t original = stack_val & HIGHEST_BIT_UNSET;

        if ( original < (uintptr_t)sysinfo.lpMinimumApplicationAddress ||
             original > (uintptr_t)sysinfo.lpMaximumApplicationAddress)
        {
            continue;
        }

        if (VirtualQuery( (LPVOID)original, &meminfo, sizeof(meminfo) ) != sizeof(meminfo))
            continue;

        if ( meminfo.Protect != PAGE_EXECUTE_READ &&
             meminfo.Protect != PAGE_EXECUTE_WRITECOPY &&
             meminfo.Protect != PAGE_EXECUTE_READWRITE)
        {
            continue;
        }

        for (uintptr_t j = 1; j < 8; j++)
        {
            //DISASM info = { 0 };
            //info.EIP = original - j;
        }
    }

    return results.size();
}
}