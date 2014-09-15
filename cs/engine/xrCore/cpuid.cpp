#include "stdafx.h"

#include "CPUID.hpp"
#include <windows.h>
#include <intrin.h>

using namespace XRay;

#define add_feature_generic(feature, container, bit)  featuresGeneric.insert({feature, FeatureInfo(container, bit)})
#define add_feature_intel(feature, container, bit)    featuresIntel.insert({feature, FeatureInfo(container, bit)})
#define add_feature_intel_ex(feature, container, bit) featuresIntelExtended.insert({feature, FeatureInfo(container, bit)})
#define add_feature_amd(feature, container, bit)      featuresAmd.insert({feature, FeatureInfo(container, bit)})
#define add_feature_amd_ex(feature, container, bit)   featuresAmdExtended.insert({feature, FeatureInfo(container, bit)})

CPUID::Vendor CPUID::vendor;
int CPUID::family;
int CPUID::model;
int CPUID::stepping;
int CPUID::cpuidLevel;
int CPUID::exIds1;
int CPUID::exIds2;
char CPUID::longName[80];
char CPUID::shortName[13];
int CPUID::flagsECX;
int CPUID::flagsEDX;
int CPUID::eflagsECX;
int CPUID::eflagsEDX;
int CPUID::speed;
int CPUID::logicalCores;
bool CPUID::featuresInitialized = false;

std::unordered_map<CPUID::Feature, CPUID::FeatureInfo> CPUID::featuresGeneric;
std::unordered_map<CPUID::Feature, CPUID::FeatureInfo> CPUID::featuresIntel, CPUID::featuresIntelExtended;
std::unordered_map<CPUID::Feature, CPUID::FeatureInfo> CPUID::featuresAmd, CPUID::featuresAmdExtended;

static void CleanDups(char* s, char c = ' ')
{
    if (*s == 0)
        return;
    char* dst = s;
    char* src = s + 1;
    while (*src != 0)
    {
        if (*src == c && *dst == c)
            ++src;
        else
            *++dst = *src++;
    }
    *++dst = 0;
}

bool IsCpuidSupported()
{
    __asm
    {
        pushfd                  // push the flags onto the stack
        pop     eax             // pop them back out, into EAX
        mov     ebx, eax        // keep original
        xor     eax, 00200000h  // turn bit 21 on
        push    eax             // put altered EAX on stack
        popfd                   // pop stack into flags
        pushfd                  // push flags back onto stack
        pop     eax             // put them back into EAX
        cmp     eax, ebx
        jz      not_supported
    }
    return true;
not_supported:
    return false;
}

void CPUID::DetectShortName()
{
    char* name = shortName;
    __asm
    {
        mov     ebx, name
        mov     cl, 0
    clear_id:
        inc     cl
        mov     dword ptr[ebx], 0
        cmp     cl, 13
        jb      clear_id
        xor     eax, eax    // set request type to the string ID
        cpuid               // send request
        xchg    eax, ebx    // swap so address register can be used
        mov     ebx, name   // get destination string offset
        mov     [ebx], eax  // save first four letters of string
        add     ebx, 4      // go up anther four bytes			
        // repeat for next two registers
        mov     [ebx], edx
        add     ebx, 4
        mov     [ebx], ecx
    }
}

void CPUID::DetectLongName()
{
    memset(longName, '\0', sizeof(longName));
    int cpuInfo[4] = { -1 };
    __cpuid(cpuInfo, 0x80000000);
    uint nExIds = cpuInfo[0];
    // Get the information associated with each extended ID
    for (uint i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(cpuInfo, i);
        // Interpret CPU brand string and cache information
        if (i == 0x80000002)
        {
            memcpy(longName, cpuInfo, sizeof(cpuInfo));
        }
        else if (i == 0x80000003)
        {
            memcpy(longName + 16, cpuInfo, sizeof(cpuInfo));
        }
        else if (i == 0x80000004)
        {
            memcpy(longName + 32, cpuInfo, sizeof(cpuInfo));
        }
    }
    CleanDups(longName);
}

void CPUID::DetectFeatures()
{
    CpuIdData id;
    __cpuid(id.data, 0x00000001);
    flagsECX = id.ecx;
    flagsEDX = id.edx;
    if (exIds1 >= 0x80000001)
    {
        __cpuid(id.data, 0x80000001);
        eflagsECX = id.ecx;
        eflagsEDX = id.edx;
    }
}

bool CPUID::Detect()
{
    if (!IsCpuidSupported())
    {
        return false;
    }
    if (!featuresInitialized)
    {
        #pragma region [ initialize features ]
        // --- Generic ---
        // flagsEDX
        add_feature_generic(Feature::FPU, &flagsEDX, 0);
        add_feature_generic(Feature::VME, &flagsEDX, 1);
        add_feature_generic(Feature::DE, &flagsEDX, 2);
        add_feature_generic(Feature::PSE, &flagsEDX, 3);
        add_feature_generic(Feature::TSC, &flagsEDX, 4);
        add_feature_generic(Feature::MSR, &flagsEDX, 5);
        add_feature_generic(Feature::PAE, &flagsEDX, 6);
        add_feature_generic(Feature::MCE, &flagsEDX, 7);
        add_feature_generic(Feature::CX8, &flagsEDX, 8);
        add_feature_generic(Feature::APIC, &flagsEDX, 9);
        add_feature_generic(Feature::SEP, &flagsEDX, 11);
        add_feature_generic(Feature::MTRR, &flagsEDX, 12);
        add_feature_generic(Feature::PGE, &flagsEDX, 13);
        add_feature_generic(Feature::MCA, &flagsEDX, 14);
        add_feature_generic(Feature::CMOV, &flagsEDX, 15);
        add_feature_generic(Feature::PAT, &flagsEDX, 16);
        add_feature_generic(Feature::PSE36, &flagsEDX, 17);
        add_feature_generic(Feature::PSN, &flagsEDX, 18);
        add_feature_generic(Feature::CLFLSH, &flagsEDX, 19);
        add_feature_generic(Feature::DS, &flagsEDX, 21);
        add_feature_generic(Feature::ACPI, &flagsEDX, 22);
        add_feature_generic(Feature::MMX, &flagsEDX, 23);
        add_feature_generic(Feature::FXSR, &flagsEDX, 24);
        add_feature_generic(Feature::SSE, &flagsEDX, 25);
        add_feature_generic(Feature::SSE2, &flagsEDX, 26);
        add_feature_generic(Feature::SS, &flagsEDX, 27);
        add_feature_generic(Feature::HT, &flagsEDX, 28);
        add_feature_generic(Feature::TM, &flagsEDX, 29);
        add_feature_generic(Feature::PBE, &flagsEDX, 31);
        // --- Intel ---
        // flagsECX
        add_feature_intel(Feature::SSE3, &flagsECX, 0);
        add_feature_intel(Feature::PCLMULDQ, &flagsECX, 1);
        add_feature_intel(Feature::DTES64, &flagsECX, 2);
        add_feature_intel(Feature::MONITOR_MWAIT, &flagsECX, 3);
        add_feature_intel(Feature::DS_CPL, &flagsECX, 4);
        add_feature_intel(Feature::VMX, &flagsECX, 5);
        add_feature_intel(Feature::SMX, &flagsECX, 6);
        add_feature_intel(Feature::EST, &flagsECX, 7);
        add_feature_intel(Feature::TM2, &flagsECX, 8);
        add_feature_intel(Feature::SSSE3, &flagsECX, 9);
        add_feature_intel(Feature::CID, &flagsECX, 10);
        add_feature_intel(Feature::FMA, &flagsECX, 12);
        add_feature_intel(Feature::CX16, &flagsECX, 13);
        add_feature_intel(Feature::XTPR, &flagsECX, 14);
        add_feature_intel(Feature::PDCM, &flagsECX, 15);
        add_feature_intel(Feature::PCID, &flagsECX, 17);
        add_feature_intel(Feature::DCA, &flagsECX, 18);
        add_feature_intel(Feature::SSE4_1, &flagsECX, 19);
        add_feature_intel(Feature::SSE4_2, &flagsECX, 20);
        add_feature_intel(Feature::X2APIC, &flagsECX, 21);
        add_feature_intel(Feature::MOVBE, &flagsECX, 22);
        add_feature_intel(Feature::POPCNT, &flagsECX, 23);
        add_feature_intel(Feature::TSC_DEADLINE, &flagsECX, 24);
        add_feature_intel(Feature::AES, &flagsECX, 25);
        add_feature_intel(Feature::XSAVE, &flagsECX, 26);
        add_feature_intel(Feature::OSXSAVE, &flagsECX, 27);
        add_feature_intel(Feature::AVX, &flagsECX, 28);
        // eflagsEDX
        add_feature_intel_ex(Feature::SYSCALL, &eflagsEDX, 11);
        add_feature_intel_ex(Feature::XD, &eflagsEDX, 20);
        add_feature_intel_ex(Feature::PDPE1GB, &eflagsEDX, 26);
        add_feature_intel_ex(Feature::RDTSCP, &eflagsEDX, 27);
        add_feature_intel_ex(Feature::EM64T, &eflagsEDX, 29);
        // --- AMD ---
        // flagsECX
        add_feature_amd(Feature::SSE3, &flagsECX, 0);
        add_feature_amd(Feature::PCLMULQDQ, &flagsECX, 1);
        add_feature_amd(Feature::MONITOR_MWAIT, &flagsECX, 3);
        add_feature_amd(Feature::SSSE3, &flagsECX, 9);
        add_feature_amd(Feature::FMA, &flagsECX, 12);
        add_feature_amd(Feature::CMPXCHG16B, &flagsECX, 13);
        add_feature_amd(Feature::SSE4_1, &flagsECX, 19);
        add_feature_amd(Feature::SSE4_2, &flagsECX, 20);
        add_feature_amd(Feature::POPCNT, &flagsECX, 23);
        add_feature_amd(Feature::AES, &flagsECX, 25);
        add_feature_amd(Feature::XSAVE, &flagsECX, 26);
        add_feature_amd(Feature::OSXSAVE, &flagsECX, 27);
        add_feature_amd(Feature::AVX, &flagsECX, 28);
        add_feature_amd(Feature::F16C, &flagsECX, 29);
        // eflagsEDX
        add_feature_amd_ex(Feature::FPU, &eflagsEDX, 0);
        add_feature_amd_ex(Feature::VME, &eflagsEDX, 1);
        add_feature_amd_ex(Feature::DE, &eflagsEDX, 2);
        add_feature_amd_ex(Feature::PSE, &eflagsEDX, 3);
        add_feature_amd_ex(Feature::TSC, &eflagsEDX, 4);
        add_feature_amd_ex(Feature::MSR, &eflagsEDX, 5);
        add_feature_amd_ex(Feature::PAE, &eflagsEDX, 6);
        add_feature_amd_ex(Feature::MCE, &eflagsEDX, 7);
        add_feature_amd_ex(Feature::CX8, &eflagsEDX, 8);
        add_feature_amd_ex(Feature::APIC, &eflagsEDX, 9);
        add_feature_amd_ex(Feature::SEP, &eflagsEDX, 11);
        add_feature_amd_ex(Feature::MTRR, &eflagsEDX, 12);
        add_feature_amd_ex(Feature::PGE, &eflagsEDX, 13);
        add_feature_amd_ex(Feature::MCA, &eflagsEDX, 14);
        add_feature_amd_ex(Feature::CMOV, &eflagsEDX, 15);
        add_feature_amd_ex(Feature::PAT, &eflagsEDX, 16);
        add_feature_amd_ex(Feature::PSE36, &eflagsEDX, 17);
        add_feature_amd_ex(Feature::MP, &eflagsEDX, 19);
        add_feature_amd_ex(Feature::NX, &eflagsEDX, 20);
        add_feature_amd_ex(Feature::MMXEXT, &eflagsEDX, 22);
        add_feature_amd_ex(Feature::MMX, &eflagsEDX, 23);
        add_feature_amd_ex(Feature::FXSR, &eflagsEDX, 24);
        add_feature_amd_ex(Feature::FFXSR, &eflagsEDX, 25);
        add_feature_amd_ex(Feature::PAGE1GB, &eflagsEDX, 26);
        add_feature_amd_ex(Feature::RDTSCP, &eflagsEDX, 27);
        add_feature_amd_ex(Feature::LM, &eflagsEDX, 29);
        add_feature_amd_ex(Feature::_3DNowExt, &eflagsEDX, 30);
        add_feature_amd_ex(Feature::_3DNow, &eflagsEDX, 31);
        // eflagsECX
        add_feature_amd_ex(Feature::lahf_sahf, &eflagsECX, 0);
        add_feature_amd_ex(Feature::CmpLegacy, &eflagsECX, 1);
        add_feature_amd_ex(Feature::svm, &eflagsECX, 2);
        add_feature_amd_ex(Feature::ExtApicSpace, &eflagsECX, 3);
        add_feature_amd_ex(Feature::LockMovCr0, &eflagsECX, 4);
        add_feature_amd_ex(Feature::abm, &eflagsECX, 5);
        add_feature_amd_ex(Feature::SSE4a, &eflagsECX, 6);
        add_feature_amd_ex(Feature::misalignsse, &eflagsECX, 7);
        add_feature_amd_ex(Feature::_3DNowPref, &eflagsECX, 8);
        add_feature_amd_ex(Feature::osvw, &eflagsECX, 9);
        add_feature_amd_ex(Feature::ibs, &eflagsECX, 10);
        add_feature_amd_ex(Feature::xop, &eflagsECX, 11);
        add_feature_amd_ex(Feature::skinit, &eflagsECX, 12);
        add_feature_amd_ex(Feature::wdt, &eflagsECX, 13);
        add_feature_amd_ex(Feature::lwp, &eflagsECX, 15);
        add_feature_amd_ex(Feature::fma4, &eflagsECX, 16);
        add_feature_amd_ex(Feature::tce, &eflagsECX, 17);
        add_feature_amd_ex(Feature::NodeId, &eflagsECX, 19);
        add_feature_amd_ex(Feature::tbm, &eflagsECX, 21);
        add_feature_amd_ex(Feature::TopoExt, &eflagsECX, 22);
        add_feature_amd_ex(Feature::PerfCtrExtCore, &eflagsECX, 23);
        add_feature_amd_ex(Feature::PerfCtrExtNB, &eflagsECX, 24);
        #pragma endregion
        featuresInitialized = true;
    }
    CpuIdData id;
    __cpuid(id.data, 0);
    cpuidLevel = id.eax & 0xFFFF;
    uint vendorId = id.ebx;
    if (cpuidLevel < 1)
    {
        return false;
    }
    __cpuid(id.data, 1);
    stepping = (id.eax >> 0) & 0xF;
    model = (id.eax >> 4) & 0xF;
    family = (id.eax >> 8) & 0xF;
    logicalCores = (id.ebx >> 16) & 0xFF;
    __cpuid(id.data, 0x80000000);
    exIds1 = id.eax;
    __cpuid(id.data, 0xC0000000);
    exIds2 = id.eax;
    switch (vendorId)
    {
    case 0x756e6547:
        vendor = Vendor::Intel;
        break;
    case 0x68747541:
        vendor = Vendor::AMD;
        break;
    default:
        vendor = Vendor::Unknown;
        break;
    }
    DetectFeatures();
    DetectShortName();
    DetectLongName();
    return true;
}

CPUID::Vendor CPUID::GetVendor()
{
    return vendor;
}

int CPUID::GetFamily()
{
    return family;
}

int CPUID::GetModel()
{
    return model;
}

int CPUID::GetStepping()
{
    return stepping;
}

char const* CPUID::GetShortName()
{
    return shortName;
}

char const* CPUID::GetLongName()
{
    return longName;
}

int CPUID::GetLogicalCoreCount()
{
    return logicalCores;
}

double CPUID::GetSpeed()
{
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    LARGE_INTEGER qwWait, qwStart, qwCurrent;
    QueryPerformanceCounter(&qwStart);
    QueryPerformanceFrequency(&qwWait);
    qwWait.QuadPart >>= 5;
    uint64 start = __rdtsc();
    do
    {
        QueryPerformanceCounter(&qwCurrent);
    } while (qwCurrent.QuadPart - qwStart.QuadPart < qwWait.QuadPart);
    double speed = ((__rdtsc() - start) << 5) / 1000000.0;
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    return speed;
}

bool CPUID::IsFeaturePresent(Feature feature)
{
    auto i = featuresGeneric.find(feature);
    bool found = i != featuresGeneric.end() && i->second.Check();
    if (!found)
    {
        // Vendor specific extensions
        switch (vendor)
        {
        case Vendor::Intel:
            i = featuresIntel.find(feature);
            found = i != featuresIntel.end() && i->second.Check();
            if (exIds1 < 0x80000001)
            {
                break;
            }
            if (!found)
            {
                i = featuresIntelExtended.find(feature);
                found = i != featuresIntelExtended.end() && i->second.Check();
            }
            break;

        case Vendor::AMD:
            i = featuresAmd.find(feature);
            found = i != featuresAmd.end() && i->second.Check();
            if (exIds1 < 0x80000001)
            {
                break;
            }
            if (!found)
            {
                i = featuresAmdExtended.find(feature);
                found = i != featuresAmdExtended.end() && i->second.Check();
            }
            break;
        }
    }
    return found;
}
