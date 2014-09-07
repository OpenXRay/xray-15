template <typename TObject, typename ...Args>
IC TObject* xr_new(Args&&... args)
{
    return new TObject(std::forward<Args>(args)...);
}

template <class T>
IC void xr_delete (T*& ptr)
{
    if (ptr)
    {
        delete ptr;
        const_cast<T*&>(ptr) = nullptr;
    }
}
