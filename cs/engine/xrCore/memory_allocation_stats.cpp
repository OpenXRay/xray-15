#include "stdafx.h"

using namespace XRay;

#ifndef DEBUG_MEMORY_MANAGER
    #pragma warning(push)
    #pragma warning(disable:4995)
    #include <malloc.h>
    #pragma warning(pop)
    #include <boost/crc.hpp>

    static bool g_mem_alloc_gather_stats = false;
    static float g_mem_alloc_gather_stats_frequency = 0.f;

    struct StatsItem
    {
        char* StackTrace;
        uint Calls;
    };

    static std::multimap<uint, StatsItem> stats;

    void mem_alloc_gather_stats(const bool& value)
    {
        g_mem_alloc_gather_stats = value;
    }

    void mem_alloc_gather_stats_frequency(const float& value)
    {
        g_mem_alloc_gather_stats_frequency = value;
    }

    void mem_alloc_show_stats()
    {
        uint size = (uint)stats.size();
        auto strings = (StatsItem*)_alloca(size*sizeof(StatsItem));
        uint accumulator = 0, i = 0;
        for (auto& pair : stats)
        {
            strings[i] = pair.second;
            i++;
            accumulator += pair.second.Calls;
        }
        struct predicate
        {
            static inline bool compare(const StatsItem& a, const StatsItem& b)
            {
                return a.Calls < b.Calls;
            }
        };
        std::sort(strings, strings+size, predicate::compare);
        for (i = 0; i < size; i++)
        {
            auto& item = strings[i];
            Msg("%d(%d)-----------------%d[%d]:%5.2f%%------------------", 
                i, size, item.Calls, accumulator, (item.Calls * 100) / float(accumulator));
            Log(item.StackTrace);
        }
    }

    void mem_alloc_clear_stats()
    {
        for (auto& item : stats)
            free(item.second.StackTrace);
        stats.clear();
    }

    static struct StackTraceInfo
    {
        static const int Capacity = 100;
        static const int LineCapacity = 256;
        static char Frames[Capacity*LineCapacity];
        static int Count;

        static IC char* GetFrame(int i) { return Frames + i*LineCapacity; }
    } StackTrace;

    char StackTraceInfo::Frames[Capacity*LineCapacity];
    int StackTraceInfo::Count;

    __declspec(noinline) void save_stack_trace()
    {
        if (!g_mem_alloc_gather_stats)
            return;
        if (::Random.randF() >= g_mem_alloc_gather_stats_frequency)
            return;
        StackTrace.Count = xrDebug::BuildStackTrace(StackTrace.Frames, StackTrace.Capacity, StackTrace.LineCapacity);
        const int skipFrames = 2;
        if (StackTrace.Count <= skipFrames)
            return;
        int frameCount = StackTrace.Count - skipFrames;
        int totalSize = 0;
        int* lengths = (int*)_alloca(frameCount*sizeof(int));
        for (int i = 0; i < frameCount; i++)
        {
            lengths[i] = strlen(StackTrace.GetFrame(i + skipFrames));
            totalSize += lengths[i]+1;
        }
        char* stackTrace = (char*)malloc(totalSize);
        {
            auto ptr = stackTrace;
            for (int i = 0; i < frameCount; i++)
            {
                memcpy(ptr, StackTrace.GetFrame(i), lengths[i]);
                ptr += lengths[i];
                *ptr = '\n';
            }
            *ptr = 0;
        }
        boost::crc_32_type temp;
        temp.process_block(stackTrace, stackTrace+totalSize);
        uint crc = temp.checksum();
        for (auto it = stats.find(crc); it != stats.end(); it++)
        {
            auto& pair = *it;
            if (pair.first != crc)
                break;
            if (strcmp(pair.second.StackTrace, stackTrace))
                continue;
            pair.second.Calls++;
            return;
        }
        stats.insert({crc, {stackTrace, 1}});
    }
#endif // DEBUG
