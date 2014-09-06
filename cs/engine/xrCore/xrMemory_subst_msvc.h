template <typename TObject, typename ...Args>
IC TObject* xr_new(Args&&... args)
{
#ifdef DEBUG_MEMORY_NAME
    TObject* ptr = (TObject*)Memory.mem_alloc(sizeof(TObject), typeid(TObject).name());
#else
    TObject* ptr = (TObject*)Memory.mem_alloc(sizeof(TObject));
#endif
    return new(ptr) TObject(std::forward<Args>(args)...);
}

template <bool _is_pm, typename T>
struct xr_special_free
{
    IC void operator()(T*& ptr)
    {
        void* _real_ptr = dynamic_cast<void*>(ptr);
        ptr->~T();
        Memory.mem_free(_real_ptr);
    }
};

template <typename T>
struct xr_special_free<false, T>
{
    IC void operator()(T*& ptr)
    {
        ptr->~T();
        Memory.mem_free(ptr);
    }
};

template <class T>
IC void xr_delete(T*& ptr)
{
    if (ptr)
    {
        xr_special_free<is_polymorphic<T>::result, T>()(ptr);
        ptr = NULL;
    }
}

template <class T>
IC void xr_delete(T* const& ptr)
{
    if (ptr)
    {
        xr_special_free<is_polymorphic<T>::result, T>(ptr);
        const_cast<T*&>(ptr) = NULL;
    }
}

#ifdef DEBUG_MEMORY_MANAGER
    void XRCORE_API mem_alloc_gather_stats(const bool& value);
    void XRCORE_API mem_alloc_gather_stats_frequency(const float& value);
    void XRCORE_API mem_alloc_show_stats();
    void XRCORE_API mem_alloc_clear_stats();
#endif // DEBUG_MEMORY_MANAGER
