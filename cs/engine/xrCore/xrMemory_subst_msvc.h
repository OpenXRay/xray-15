template <typename T>
struct xr_special_delete
{
    IC void operator()(T*& ptr)
    {
        delete ptr;
    }
};

template <typename T>
IC void xr_delete(T*& ptr)
{
    if (ptr)
    {
        xr_special_delete<T>()(ptr);
        ptr = nullptr;
    }
}

template <typename T>
IC void xr_delete(T* const& ptr)
{
    if (ptr)
    {
        xr_special_delete<T>()(ptr);
        const_cast<T*&>(ptr) = nullptr;
    }
}

#ifdef DEBUG_MEMORY_MANAGER
    void XRCORE_API mem_alloc_gather_stats(const bool& value);
    void XRCORE_API mem_alloc_gather_stats_frequency(const float& value);
    void XRCORE_API mem_alloc_show_stats();
    void XRCORE_API mem_alloc_clear_stats();
#endif // DEBUG_MEMORY_MANAGER
