#ifndef _STL_EXT_internal
#define _STL_EXT_internal

using std::swap;

#include "_type_traits.h"

#ifdef	__BORLANDC__
#define M_NOSTDCONTAINERS_EXT
#endif
#ifdef	_M_AMD64
#define M_DONTDEFERCLEAR_EXT
#endif

#define	M_DONTDEFERCLEAR_EXT		//. for mem-debug only

//--------	
#ifdef	M_NOSTDCONTAINERS_EXT

#define xr_list std::list
#define xr_deque std::deque
#define xr_stack std::stack
#define xr_set std::set
#define xr_multiset std::multiset
#define xr_map std::map
#define xr_hash_map std::hash_map
#define xr_multimap std::multimap
#define xr_string std::string

template <class T>
class xr_vector	: public std::vector<T> {
public: 
	typedef	size_t		size_type;
	typedef T&			reference;
	typedef const T&	const_reference;
public: 
			xr_vector			()								: std::vector<T>	()				{}
			xr_vector			(size_t _count, const T& _value): std::vector<T>	(_count,_value)	{}
	explicit xr_vector			(size_t _count)					: std::vector<T> 	(_count)		{}
	void	clear				()								{ erase(begin(),end());				} 
	void	clear_and_free		()								{ std::vector<T>::clear();			}
	void	clear_not_free		()								{ erase(begin(),end());	}
	ICF		const_reference	operator[]	(size_type _Pos) const	{ {VERIFY(_Pos<size());} return (*(begin() + _Pos)); }
	ICF		reference		operator[]	(size_type _Pos)		{ {VERIFY(_Pos<size());} return (*(begin() + _Pos)); }
};

template	<>												
class	xr_vector<bool>	: public std::vector<bool>{ 
	typedef	bool		T;
public: 
			xr_vector<T>		()								: std::vector<T>	()				{}
			xr_vector<T>		(size_t _count, const T& _value): std::vector<T>	(_count,_value)	{}
	explicit xr_vector<T>		(size_t _count)					: std::vector<T>	(_count)		{}
	u32		size() const										{ return (u32)std::vector<T>::size();	} 
	void	clear()												{ erase(begin(),end());				} 
};

#else

template <class T>
class	xalloc	{
public:
	typedef	size_t		size_type;
	typedef ptrdiff_t	difference_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T			value_type;

public:
	template<class _Other>	
	struct rebind			{	typedef xalloc<_Other> other;	};
public:
							pointer					address			(reference _Val) const					{	return (&_Val);	}
							const_pointer			address			(const_reference _Val) const			{	return (&_Val);	}
													xalloc			()										{	}
													xalloc			(const xalloc<T>&)						{	}
	template<class _Other>							xalloc			(const xalloc<_Other>&)					{	}
	template<class _Other>	xalloc<T>&				operator=		(const xalloc<_Other>&)					{	return (*this);	}
							pointer					allocate		(size_type n, const void* p=0) const	{	return xr_alloc<T>((u32)n);	}
							char*					_charalloc		(size_type n)							{	return (char*)allocate(n); }
							void					deallocate		(pointer p, size_type n) const			{	xr_free	(p);				}
							void					deallocate		(void* p, size_type n) const			{	xr_free	(p);				}
							void					construct		(pointer p, const T& _Val)				{	std::_Construct(p, _Val);	}
							void					destroy			(pointer p)								{	std::_Destroy(p);			}
							size_type				max_size		() const								{	size_type _Count = (size_type)(-1) / sizeof (T);	return (0 < _Count ? _Count : 1);	}
};

struct xr_allocator {
	template <typename T>
	struct helper {
		typedef xalloc<T>	result;
	};

	static	void	*alloc		(const u32 &n)	{	return xr_malloc((u32)n);	}
	template <typename T>
	static	void	dealloc		(T *&p)			{	xr_free(p);					}
};

template<class _Ty,	class _Other>	inline	bool operator==(const xalloc<_Ty>&, const xalloc<_Other>&)		{	return (true);							}
template<class _Ty, class _Other>	inline	bool operator!=(const xalloc<_Ty>&, const xalloc<_Other>&)		{	return (false);							}

namespace std
{
	template<class _Tp1, class _Tp2>	inline	xalloc<_Tp2>&	__stl_alloc_rebind(xalloc<_Tp1>& __a, const _Tp2*)	{	return (xalloc<_Tp2>&)(__a);	}
	template<class _Tp1, class _Tp2>	inline	xalloc<_Tp2>	__stl_alloc_create(xalloc<_Tp1>&, const _Tp2*)		{	return xalloc<_Tp2>();			}
};

// string(char)
typedef		std::basic_string<char, std::char_traits<char>, xalloc<char> >	xr_string;

// vector
template	<typename T, typename allocator = xalloc<T> >
class xr_vector : public std::vector<T,allocator> {
private:
	typedef std::vector<T,allocator>	inherited;

public:
	typedef allocator					allocator_type;

public:
			xr_vector			()									: inherited	()					{}
			xr_vector			(size_t _count, const T& _value)	: inherited	(_count,_value)		{}
	explicit xr_vector			(size_t _count)						: inherited (_count)			{}
	u32		size				() const							{ return (u32)inherited::size();} 

	void	clear_and_free		()									{ inherited::clear();			}
	void	clear_not_free		()									{ erase(begin(),end());			}
	void	clear_and_reserve	()									{ if ( capacity() <= (size()+size()/4) ) clear_not_free(); else { u32 old=size(); clear_and_free(); reserve(old); } }

#ifdef M_DONTDEFERCLEAR_EXT
	void	clear				()									{ clear_and_free	();			}
#else
	void	clear				()									{ clear_not_free	();			}
#endif

	const_reference operator[]	(size_type _Pos) const				{ {VERIFY2(_Pos<size(),make_string("index is out of range: index requested[%d], size of container[%d]", _Pos, size()).c_str());} return (*(begin() + _Pos)); }
	reference operator[]		(size_type _Pos)					{ {VERIFY2(_Pos<size(),make_string("index is out of range: index requested[%d], size of container[%d]", _Pos, size()).c_str());} return (*(begin() + _Pos)); }
};

// vector<bool>
template <>
class xr_vector<bool,xalloc<bool> >	: public std::vector<bool,xalloc<bool> > {
private:
	typedef std::vector<bool,xalloc<bool> > inherited;

public: 
	u32		size				() const							{ return (u32)inherited::size();} 
	void	clear				()									{ erase(begin(),end());			} 
};

template <typename allocator>
class xr_vector<bool,allocator>	: public std::vector<bool,allocator> {
private:
	typedef std::vector<bool,allocator> inherited;

public: 
	u32		size				() const							{ return (u32)inherited::size();} 
	void	clear				()									{ erase(begin(),end());			} 
};

// deque
template <typename T, typename allocator = xalloc<T> >
class xr_deque : public std::deque<T,allocator> {
public: 
	typedef typename allocator								allocator_type;
	typedef typename allocator_type::value_type				value_type;
	typedef typename allocator_type::size_type				size_type;
	u32		size				() const							{return (u32)__super::size();	} 
};

// stack
template <typename _Ty, class _C = xr_vector<_Ty> >
class xr_stack {
public:
	typedef typename _C::allocator_type						allocator_type;
	typedef typename allocator_type::value_type				value_type;
	typedef typename allocator_type::size_type				size_type;

	//explicit			stack(const allocator_type& _Al = allocator_type()) : c(_Al) {}
	allocator_type		get_allocator	() const							{return (c.get_allocator()); }
	bool				empty			() const							{return (c.empty()); }
	u32					size			() const							{return c.size(); } 
	value_type&			top				()									{return (c.back()); }
	const value_type&	top				() const							{return (c.back()); }
	void				push			(const value_type& _X)				{c.push_back(_X); }
	void				pop				()									{c.pop_back(); }
	bool				operator==		(const xr_stack<_Ty, _C>& _X) const	{return (c == _X.c); }
	bool				operator!=		(const xr_stack<_Ty, _C>& _X) const	{return (!(*this == _X)); }
	bool				operator<		(const xr_stack<_Ty, _C>& _X) const	{return (c < _X.c); }
	bool				operator>		(const xr_stack<_Ty, _C>& _X) const	{return (_X < *this); }
	bool				operator<=		(const xr_stack<_Ty, _C>& _X) const	{return (!(_X < *this)); }
	bool				operator>=		(const xr_stack<_Ty, _C>& _X) const	{return (!(*this < _X)); }

protected:
	_C c;
};

template	<typename T, typename allocator = xalloc<T> >									class	xr_list 		: public std::list<T,allocator>			{ public: u32 size() const {return (u32)__super::size(); } };
template	<typename K, class P=std::less<K>, typename allocator = xalloc<K> >				class	xr_set			: public std::set<K,P,allocator>		{ public: u32 size() const {return (u32)__super::size(); } };
template	<typename K, class P=std::less<K>, typename allocator = xalloc<K> >				class	xr_multiset		: public std::multiset<K,P,allocator>	{ public: u32 size() const {return (u32)__super::size(); } };
template	<typename K, class V, class P=std::less<K>, typename allocator = xalloc<std::pair<K,V> > >	class	xr_map 			: public std::map<K,V,P,allocator>		{ public: u32 size() const {return (u32)__super::size(); } };
template	<typename K, class V, class P=std::less<K>, typename allocator = xalloc<std::pair<K,V> > >	class	xr_multimap		: public std::multimap<K,V,P,allocator>	{ public: u32 size() const {return (u32)__super::size(); } };

#ifdef STLPORT
	template	<typename V, class _HashFcn=std::hash<V>, class _EqualKey=std::equal_to<V>, typename allocator = xalloc<V> >	class	xr_hash_set		: public std::hash_set<V,_HashFcn,_EqualKey,allocator>		{ public: u32 size() const {return (u32)__super::size(); } };
	template	<typename V, class _HashFcn=std::hash<V>, class _EqualKey=std::equal_to<V>, typename allocator = xalloc<V> >	class	xr_hash_multiset: public std::hash_multiset<V,_HashFcn,_EqualKey,allocator>	{ public: u32 size() const {return (u32)__super::size(); } };

	template	<typename K, class V, class _HashFcn=std::hash<K>, class _EqualKey=std::equal_to<K>, typename allocator = xalloc<std::pair<K,V> > >	class	xr_hash_map		: public std::hash_map<K,V,_HashFcn,_EqualKey,allocator>		{ public: u32 size() const {return (u32)__super::size(); } };
	template	<typename K, class V, class _HashFcn=std::hash<K>, class _EqualKey=std::equal_to<K>, typename allocator = xalloc<std::pair<K,V> > >	class	xr_hash_multimap: public std::hash_multimap<K,V,_HashFcn,_EqualKey,allocator>	{ public: u32 size() const {return (u32)__super::size(); } };
#else 
	template	<typename K, class V, class _Traits=stdext::hash_compare<K, stdext::less<K> >, typename allocator = xalloc<std::pair<K,V> > >
    class	xr_hash_map		: public stdext::hash_map<K,V,_Traits,allocator>
    {
        public: u32 size() const {return (u32)__super::size(); }
    };
#endif // #ifdef STLPORT

#endif

template	<class _Ty1, class _Ty2> inline	std::pair<_Ty1, _Ty2>		mk_pair		(_Ty1 _Val1, _Ty2 _Val2)	{	return (std::pair<_Ty1, _Ty2>(_Val1, _Val2));	}

struct pred_str		: public std::binary_function<char*, char*, bool>	{	
	IC bool operator()(const char* x, const char* y) const				{	return xr_strcmp(x,y)<0;	}
};
struct pred_stri	: public std::binary_function<char*, char*, bool>	{	
	IC bool operator()(const char* x, const char* y) const				{	return stricmp(x,y)<0;	}
};

#include "FixedVector.h"
#include "buffer_vector.h"

// auxilary definition
using boolVec = xr_vector<bool>;
using boolIt = boolVec::iterator;
using BOOLVec = xr_vector<BOOL>;
using BOOLIt = BOOLVec::iterator;
using LPBOOLVec = xr_vector<BOOL*>;
using LPBOOLIt = LPBOOLVec::iterator;
using FrectVec = xr_vector<Frect>;
using FrectIt = FrectVec::iterator;
using IrectVec = xr_vector<Irect>;
using IrectIt = IrectVec::iterator;
using PlaneVec = xr_vector<Fplane>;
using PlaneIt = PlaneVec::iterator;
using Fvector2Vec = xr_vector<Fvector2>;
using Fvector2It = Fvector2Vec::iterator;
using FvectorVec = xr_vector<Fvector>;
using FvectorIt = FvectorVec::iterator;
using LPFvectorVec = xr_vector<Fvector*>;
using LPFvectorIt = LPFvectorVec::iterator;
using FcolorVec = xr_vector<Fcolor>;
using FcolorIt = FcolorVec::iterator;
using LPFcolorVec = xr_vector<Fcolor*>;
using LPFcolorIt = LPFcolorVec::iterator;
using LPSTRVec = xr_vector<LPSTR>;
using LPSTRIt = LPSTRVec::iterator;
using LPCSTRVec = xr_vector<LPCSTR>;
using LPCSTRIt = LPCSTRVec::iterator;
using string64Vec = xr_vector<string64>;
using string64It = string64Vec::iterator;
using SStringVec = xr_vector<xr_string>;
using SStringVecIt = SStringVec::iterator;

using S8Vec = xr_vector<s8>;
using S8It = S8Vec::iterator;
using LPS8Vec = xr_vector<s8*>;
using LPS8It = LPS8Vec::iterator;
using S16Vec = xr_vector<s16>;
using S16It = S16Vec::iterator;
using LPS16Vec = xr_vector<s16*>;
using LPS16It = LPS16Vec::iterator;
using S32Vec = xr_vector<s32>;
using S32It = S32Vec::iterator;
using LPS32Vec = xr_vector<s32*>;
using LPS32It = LPS32Vec::iterator;
using U8Vec = xr_vector<u8>;
using U8It = U8Vec::iterator;
using LPU8Vec = xr_vector<u8*>;
using LPU8It = LPU8Vec::iterator;
using U16Vec = xr_vector<u16>;
using U16It = U16Vec::iterator;
using LPU16Vec = xr_vector<u16*>;
using LPU16It = LPU16Vec::iterator;
using U32Vec = xr_vector<u32>;
using U32It = U32Vec::iterator;
using LPU32Vec = xr_vector<u32*>;
using LPU32It = LPU32Vec::iterator;
using FloatVec = xr_vector<float>;
using FloatIt = FloatVec::iterator;
using LPFloatVec = xr_vector<float*>;
using LPFloatIt = LPFloatVec::iterator;
using IntVec = xr_vector<int>;
using IntIt = IntVec::iterator;
using LPIntVec = xr_vector<int*>;
using LPIntIt = LPIntVec::iterator;

#ifdef __BORLANDC__
using AStringVec = xr_vector<AnsiString>;
using AStringIt = AStringVec::iterator;
using LPAStringVec = xr_vector<AnsiString*>;
using LPAStringIt = LPAStringVec::iterator;
#endif

#endif