#include <ProcessMemory.hpp>
#include <TlHelp32.h>
namespace igi {
ProcessMemory::~ProcessMemory(){ detach(); }
bool ProcessMemory::attach(const wchar_t* name){
    detach();
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0); if(snap==INVALID_HANDLE_VALUE)return false;
    PROCESSENTRY32W pe{sizeof(pe)};
    if(Process32FirstW(snap,&pe)){do{if(_wcsicmp(pe.szExeFile,name)==0){pid_=pe.th32ProcessID;break;}}while(Process32NextW(snap,&pe));}
    CloseHandle(snap); if(!pid_)return false;
    handle_=OpenProcess(PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION|PROCESS_QUERY_INFORMATION,FALSE,pid_);
    if(!handle_){pid_=0;return false;}
    snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid_); if(snap==INVALID_HANDLE_VALUE){detach();return false;}
    MODULEENTRY32W me{sizeof(me)};
    if(Module32FirstW(snap,&me)){do{if(_wcsicmp(me.szModule,name)==0){moduleBase_=reinterpret_cast<std::uintptr_t>(me.modBaseAddr);break;}}while(Module32NextW(snap,&me));}
    CloseHandle(snap); if(!moduleBase_){detach();return false;} return true;
}
void ProcessMemory::detach() noexcept{if(handle_)CloseHandle(handle_);handle_=nullptr;pid_=0;moduleBase_=0;}
bool ProcessMemory::attached() const noexcept{if(!handle_)return false;DWORD c{};return GetExitCodeProcess(handle_,&c)&&c==STILL_ACTIVE;}
}
