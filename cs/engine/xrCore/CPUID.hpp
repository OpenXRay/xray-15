#pragma once

#include <unordered_map>
#include "xrCore.h"
#include "Common.hpp"

namespace XRay
{
class XRCORE_API CPUID
{
public:
    enum class Vendor
    {
        Unknown = 0,
        AMD,
        Intel
    };

    enum class Feature
    {
        FPU,
        VME,
        DE,
        PSE,
        TSC,
        MSR,
        PAE,
        MCE,
        CX8,
        APIC,
        SEP,
        MTRR,
        PGE,
        MCA,
        CMOV,
        PAT,
        PSE36,
        PSN,
        CLFLSH,
        DS,
        ACPI,
        MMX,
        FXSR,
        SSE,
        SSE2,
        SS,
        HT,
        TM,
        PBE,
        SSE3,
        PCLMULDQ,
        DTES64,
        MONITOR_MWAIT,
        DS_CPL,
        VMX,
        SMX,
        EST,
        TM2,
        SSSE3,
        CID,
        FMA,
        CX16,
        XTPR,
        PDCM,
        PCID,
        DCA,
        SSE4_1,
        SSE4_2,
        X2APIC,
        MOVBE,
        POPCNT,
        TSC_DEADLINE,
        AES,
        XSAVE,
        OSXSAVE,
        AVX,
        SYSCALL,
        XD,
        PDPE1GB,
        RDTSCP,
        ULL,
        EM64T,
        PCLMULQDQ,
        CMPXCHG16B,
        F16C,
        MP,
        NX,
        MMXEXT,
        FFXSR,
        PAGE1GB,
        LM,
        _3DNowExt,
        _3DNow,
        EPS,
        lahf_sahf,
        CmpLegacy,
        svm,
        ExtApicSpace,
        LockMovCr0,
        abm,
        SSE4a,
        misalignsse,
        _3DNowPref,
        osvw,
        ibs,
        xop,
        skinit,
        wdt,
        lwp,
        fma4,
        tce,
        NodeId,
        tbm,
        TopoExt,
        PerfCtrExtCore,
        PerfCtrExtNB
    };

private:
    struct FeatureInfo
    {
        int* Container;
        byte Bit;

        FeatureInfo(int* container, byte bit)
            : Container(container), Bit(bit)
        {}

        bool Check() const
        {
            return (*Container & (1 << Bit)) != 0;
        }
    };

    static Vendor vendor;
    static int family;
    static int model;
    static int stepping;
    static int cpuidLevel;
    static int exIds1;
    static int exIds2;
    static char longName[80];
    static char shortName[13];
    static int flagsECX;
    static int flagsEDX;
    static int eflagsECX;
    static int eflagsEDX;
    static int speed;
    static int logicalCores;
    static std::unordered_map<Feature, FeatureInfo> featuresGeneric;
    static std::unordered_map<Feature, FeatureInfo> featuresIntel;
    static std::unordered_map<Feature, FeatureInfo> featuresIntelExtended;
    static std::unordered_map<Feature, FeatureInfo> featuresAmd;
    static std::unordered_map<Feature, FeatureInfo> featuresAmdExtended;
    static bool featuresInitialized;

    union CpuIdData
    {
        struct
        {
            int eax, ebx, ecx, edx;
        };
        int data[4];
    };

private:
    CPUID();
    static void DetectShortName();
    static void DetectLongName();
    static void DetectFeatures();

public:
    static bool Detect();
    static Vendor GetVendor();
    static int GetFamily();
    static int GetModel();
    static int GetStepping();
    static char const* GetShortName();
    static char const* GetLongName();
    static int GetLogicalCoreCount();
    static double GetSpeed();
    static bool IsFeaturePresent(Feature feature);
};
} // namespace XRay
