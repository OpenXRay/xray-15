///////////////////////////////////////////////////////////////////////////////
// Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
// Germany. All rights reserved.
///////////////////////////////////////////////////////////////////////////////
// Author:	kjs
// Created:	09.01.06
// Module:	api
// Purpose:	mental ray C++ shader interface extensions
///////////////////////////////////////////////////////////////////////////////

/// \file mi_shader_if.h
/// mental ray C++ shader interface extensions.
///
/// This is a new C++ interface for shaders which extends the existing C-style
/// shader interface to mental ray. It is implemented as abstract interface
/// classes which do not require symbol lookups across DLL boundaries.
///
/// The indirection through abstract interface classes and virtual member
/// functions is needed in order to avoid symbol lookups across dynamically
/// loaded libraries (like for example shaders) by using virtual function
/// tables. It also makes it possible to provide new, incompatible versions of
/// the interface later.
///
/// mi_shader_if.h is included from shader.h if the shader is compiled as C++
/// code; a shader does not need to include mi_shader_if.h directly.

#ifndef MI_SHADER_IF_H
#define MI_SHADER_IF_H

#if !defined(SHADER_H)
#include <mi_raylib.h>
#include <mi_lib.h>
#endif

#ifndef __cplusplus
#error mi_shader_if.h requires C++ compilation
#endif


/// \brief version number of the interface
static const int mi_ray_interface_version = 1;


// forward declarations
namespace mi {
    namespace shader {
        struct Interface;
    }
}


/// \brief Acquire an instance of the mental ray C++ shader interface extensions.
///
/// \param version is the version number of the requested interface class
/// and should usually be left at the default value.
/// The passed version argument is used to support multiple different interface 
/// versions and should usually be the value of the variable
/// mi_ray_interface_version in the defining header file. A future version of
/// mental ray may optionally return a pointer to a newer version of the 
/// interface of a different type or in a different namespace, identified by a
/// different version number.
///
/// \return The returned pointer points to an object in mental ray which
/// contains the interface functions as virtual methods (hence it is possible to
/// call the interface routines from a dynamically loaded library without
/// resolving the routine in a symbol table). The caller may not attempt to
/// modify or delete the returned object but should call the
/// mi::shader::Interface::release() method when done.

extern "C" mi::shader::Interface *mi_get_shader_interface(int version = mi_ray_interface_version);


//! \brief Top level namespace of mental ray
namespace mi {


/// \brief Namespace containing mental ray C++ shader interface extensions
///
/// The mental ray C++ interface is completely contained in the shader namespace.
/// \note This name may change later in order to support multiple versions of
/// the interface.
namespace shader {


/// forward declarations
struct Options;
class LightList;


/// \brief Top level C++ mental ray interface extensions.
///
/// This class is the top level access to the C++ shader interface extensions.
/// All other C++ extensions like mi::shader::Options and
/// mi::shader::LightIterator are accessible through this class.
///
/// An instance of the Interface must be acquired by calling
/// mi_get_shader_interface() or the static method Interface::get().
/// When the interface is no more needed, it must be released by calling
/// Interface::release():
/// \code
/// mi::shader::Interface *iface = mi::shader::Interface::get();
/// // do something with it...
/// iface->release();
/// \endcode
/// 
/// The C++ interface extensions are implemented as virtual functions in this
/// interface struct in order to avoid linking and symbol lookup problems.
/// The interface is defined in the header file mi_shader_if.h.

struct Interface {
    /// \brief acquire an instance of the interface
    ///
    /// This static method is equivalent to the function
    /// mi_get_shader_interface(), see there fore more information.
    /// This static function can be used as follows:
    /// \code
    /// mi::shader::Interface *iface = mi::shader::Interface::get();
    /// \endcode
    /// \param version is the version number of the requested interface class,
    /// usually no version argument needs to be passed.
    /// \return The returned pointer points to an object in mental ray which
    /// contains the interface functions as virtual methods (hence it is 
    /// possible to call the interface routines from a dynamically loaded 
    /// library without resolving the routine in a symbol table). The caller may
    /// not attempt to modify or delete the returned object but should call the
    /// mi::shader::Interface::release() method when done.
    static inline Interface* get(int version = mi_ray_interface_version)
        { return mi_get_shader_interface(version); }

    /// \brief Access to string options.
    ///
    /// This may later be extended to also access all other options from the
    /// miOptions structure. This function can be used as follows:
    /// \code
    /// Options *stringOptions = interface->getOptions(options->string_options);
    /// \endcode
    /// \param string_options is the tag of the string options, taken from
    /// the string_options field of the miOptions structure to be read or
    /// modified.
    /// It must be valid throughout the use of the Options instance.
    /// \return A pointer to an interface class. The Options::release() method
    /// should be called when done.
    virtual Options* getOptions(miTag string_options);

    /// \brief Used internally by LightIterator to create a light list.
    ///
    /// This may be used to generate light iterators. This method is needed
    /// by the LightIterator::LightIterator() constructor. Usually there is no need to invoke
    /// this method directly.
    /// \param state provided the method with the current rendering state. From
    /// the state the current instance light list may be deduced.
    /// \param slist is an optional list of light tags. If provided, this list
    /// will be used instead of the default instance light list.
    /// \param n gives the number of light tags in the optional light list.
    /// \return the method returns a pointer to a LightList.
    virtual LightList* createLightList(miState* state, miTag* slist=0, int n=0);

    /// \brief release (delete) the instance of the interface
    ///
    /// An interface acquired with mi_get_shader_interface() or 
    /// mi::shader::Interface::get() must be released with this call when 
    /// done. The call may delete the object, and the interface may no longer be
    /// used afterwards.
    virtual void release();

    virtual ~Interface() {}
};


/// \brief Access to string options
///
/// Up to version 3.4, options are hardcoded in the struct miOptions in
/// shader.h. New options are implemented as arbitrary name - value pairs,
/// where the name of the option is an arbitrary string, and the value can be a
/// boolean, string, integration, float, 3 floats, or 4 floats.
///
/// A pointer to string options must be obtained with Interface::getOptions().
/// When the pointer is no longer needed then the Options::release() method
/// must be called, like for example:
/// \code
/// mi::shader::Interface *iface = mi_get_shader_interface();
/// mi::shader::Options *opt = iface->getOptions(string_options_tag);
/// iface->release();
/// opt->set("favorite color", "blue");
/// opt->release();
/// \endcode
/// 
/// <h3>Setting options</h3>
/// Set functions set the value of an option of a given name, overwriting any
/// previous value. Previous values may be overwritten by values of a different
/// type.
///
/// \note Options should only be set before rendering starts. It is undefined
/// which value will be used if an option is set during rendering.
///
/// <h3>Getting options</h3>
/// All get functions return true and set the value if a matching option is
/// found, or returns false leave the value unmodified if no matching option is
/// found.
///
/// <h3>Strings and memory management</h3>
/// Strings passed as arguments are completely controlled by the caller; mental
/// ray uses the strings briefly, or makes copies of the passed strings.
///
/// Strings returned by these functions are read-only and controlled by mental
/// ray. The caller may use these only for a short time and may not delete them.
/// Make a copy if the value is needed later.

struct Options {
    /// \{

    /// \brief set a boolean option
    /// \param name is the name of the option to set.
    /// \param value is the new value of the option.
    virtual void set(const char *name, bool value) = 0;

    /// \brief set a string option
    /// \param name is the name of the option to set.
    /// \param value mental ray will make a copy of the passed string \a value,
    /// the passed argument is under control
    /// of the caller.
    virtual void set(const char *name, const char *value) = 0;

    /// \brief set an integer option
    ///\note Integer options may also be used as floating point values.
    /// \param name is the name of the option to set.
    /// \param value is the new value of the option.
    virtual void set(const char *name, int value) = 0;

    /// \brief set a floating point option
    /// \param name is the name of the option to set.
    /// \param value is the new value of the option.
    virtual void set(const char *name, float value) = 0;

    /// \brief set a float triple option
    /// \param name is the name of the option to set.
    /// \param value1 is the first component of the triple.
    /// \param value2 is the second component of the triple.
    /// \param value3 is the third component of the triple.
    virtual void set(const char *name, float value1, float value2, float value3) = 0;

    /// \brief set a float quadruple option
    /// \param name is the name of the option to set.
    /// \param value1 is the first component of the quadruple.
    /// \param value2 is the second component of the quadruple.
    /// \param value3 is the third component of the quadruple.
    /// \param value4 is the third component of the quadruple.
    virtual void set(const char *name, float value1, float value2, float value3, float value4) = 0;

    /// \brief get a boolean option
    /// \param name is the name of the option to look up
    /// \param value will be set on success, and left unchanged otherwise.
    /// \return true if an option of the given name and type is found, false
    /// otherwise.
    virtual bool get(const char *name, bool *value) = 0;

    /// \brief get a string option
    /// \param name is the name of the option to look up
    /// \param value The returned string \a value is only value for a short
    /// time, and may not be modified or deleted by the caller. The caller
    /// should make a copy of the string if needed.
    /// \return true if an option of the given name and type is found, false
    /// otherwise.
    virtual bool get(const char *name, const char **value) = 0;

    /// \brief get an integer option
    /// \param name is the name of the option to look up
    /// \param value will be set on success, and left unchanged otherwise.
    /// \return true if an option of the given name and type is found, false
    /// otherwise.
    virtual bool get(const char *name, int *value) = 0;

    /// \brief get a floating point option
    ///
    /// If the value of the named option is an integer then the integer is
    /// converted to a floating point number and returned in \a value.
    /// \param name is the name of the option to look up
    /// \param value will be set on success, and left unchanged otherwise.
    /// \return true if an option of the given name and type is found, false
    /// otherwise.
    virtual bool get(const char *name, float *value) = 0;

    /// \brief get a floating point triple option
    ///
    /// This can be used for RGB colors or 3 dimensional vectors.
    /// \param name is the name of the option to look up
    /// \param value1 will be set to the first component on success, and left
    /// unchanged otherwise.
    /// \param value2 will be set to the second component on success, and left
    /// unchanged otherwise.
    /// \param value3 will be set to the third component on success, and left
    /// unchanged otherwise.
    /// \return true if an option of the given name and type is found, false
    /// otherwise.
    virtual bool get(const char *name, float *value1, float *value2, float *value3) = 0;

    /// \brief get a floating point quadruple option
    ///
    /// This can be used for RGBA colors or 4 dimensional homogenous vectors.
    /// \param name is the name of the option to look up
    /// \param value1 will be set to the first component on success, and left 
    /// unchanged otherwise.
    /// \param value2 will be set to the second component on success, and left 
    /// unchanged otherwise.
    /// \param value3 will be set to the third component on success, and left 
    /// unchanged otherwise.
    /// \param value4 will be set to the fourth component on success, and left 
    /// unchanged otherwise.
    /// \return true if an option of the given name and type is found, false 
    /// otherwise.
    virtual bool get(const char *name, float *value1, float *value2, float *value3, float *value4) = 0;

    /// \brief release (delete) the interface
    ///
    /// This should be called when done. It may release the Options object.
    virtual void release() = 0;

    virtual ~Options() {}
};


/// \brief Iterate over shader or instance light lists.
/// 
/// mental ray 3.5 introduces the concept of instance light lists. These lists are not explicitely
/// passed to the shader as a parameter. Instead mental ray makes them available through LightIterators.
/// The LightIterator class allows shaders to iterate over lights for the purpose of sampling.
/// A typical use would look like
/// \code
/// for (mi::shader::LightIterator iter(state); !iter.at_end(); ++iter) {
///     miColor col = {0,0,0,1};
///     while (iter->sample()) {
///         miColor light_color;
///         iter->get_contribution(&light_color);
///         col.r += light_color.r;
///         col.g += light_color.g;
///         col.b += light_color.b;
///     }
///     const int n_samples = iter->get_number_of_samples();
///     if (n_samples > 1) {
///         col.r /= n_samples;
///         col.g /= n_samples;
///         col.b /= n_samples;
///     }
/// }
/// \endcode
///
class LightIterator {
public:
    /// \brief the contructor for the LightIterator
    ///
    /// Note that two different light lists may be assigned to instances: a light 
    /// list and a shadow light list. During regular rendering the iterator uses 
    /// the light list, but when either rendering shadow maps or tracing shadow rays, 
    /// then the iterator prefers the shadow light list over the light list, that 
    /// is, in these cases the light list will only be used if no shadow light list 
    /// is present. It is also possible to instanciate the iterator with an explicit 
    /// light list provided by the shader.
    /// \param state provides the iterator with information about the rendering state, 
    /// especially where to find the lights for the iteration.
    /// \param shader_light_list allows to provide a custom list of lights for the 
    /// iteration. This optional list overrides the instance light lists used by default.
    /// \param n_shader_lights tells the iterator the number of custom lights passed in.
    /// \param version allows to specify a specific interface version. By default this
    /// is set to the current interface version.
    LightIterator(				      //!< use to create iter within shader
	miState* state,				      //!< the state
	miTag*	 shader_light_list = 0,		      //!< optional array of light tags
	int	 n_shader_lights = 0,		      //!< number of lights in above array
        int      version = mi_ray_interface_version); //!< the ray interface version

    /// \brief the copy constructor.
    ///
    /// creates a copy of a given LightIterator. This constructor is needed
    /// to return LightIterators from methods.
    /// \param liter the LightIterator to be copied. 
    LightIterator(const LightIterator& liter);	    //!< needed to return LightIterators

    /// \brief the destructor
    ///
    /// The destructor for a LightIterator instance is called when it goes out of scope.
    /// It releases all resources associated with this instance.
    ~LightIterator();				    //!< delete light iterator

    /// \brief obtain the tag for the current light
    ///
    /// dereferencing a LightIterator with the unary prefix operator '*' yields 
    /// the tag of the current light. Note that this operation is distinctly 
    /// different from dereferencing a LightIterator with the postfix operator '->'.
    /// \return the tag of the current light in the light list.
    miTag operator*() const;			    //!< associated light tag

    /// \brief invoke a LightList method
    ///
    /// the postfix dereference operator '->' allows to invoke public methods of 
    /// the LightList class as if they would be methods of the LightIterator class.
    /// \return a pointer to the LightList associated with the LightIterator. Note
    /// that the LightList pointer is not available for direct use. Instead C++
    /// will immediately use it to invoke the LightList method written to the right
    /// of the '->' operator. For example, 
    /// \code
    /// LightIterator iter(state)->sample();
    /// \endcode
    /// will invoke the sample method of the LightList class.
    LightList* operator->() const;		    //!< access light list methods

    /// \brief pre-increment operator
    ///
    /// The operator advances the LightIterator to the next light in the light
    /// list.
    /// \return a constant reference to the incremented light iterator.
    ///
    /// The pre-increment operator should be prefered over the post-increment 
    /// operator, since it avoids the generation of a temporary LightIterator.
    const LightIterator& operator++();		    //!< pre-increment

    /// \brief post-increment operator
    ///
    /// The increment operator advances the LightIterator to the next light
    /// in the light list.
    /// \return a copy of the LightIterator as it was before it was advanced to 
    /// the next light.
    ///
    /// The post-increment operator is more expensive than the pre-increment
    /// operator, since it need to construct an extra  copy of the iterator. 
    LightIterator operator++(int);		    //!< post-increment

    /// \brief check if iteration should continue.
    ///
    /// \return true if the end of the light list has been reached,
    /// false otherwise.
    bool at_end() const;			    //!< check if iteration end has been reached

    /// \brief LightIterator assignment
    /// 
    /// assigns a LightIterator to another one.
    const LightIterator& operator=(const LightIterator& iter); //!< assign iterators 

    /// \brief check if two LightIterators are equal
    ///
    /// Two LightIterators are considered equal if they refer to the same light list
    /// and have the same current light within the list.
    /// \return true if the two iterators are equal, false else.
    bool operator==(const LightIterator& iter) const; //!< compare iterators

    /// \brief check if two LightIterators are unequal.
    ///
    /// Two LightIterators are unequal if they do not refer to the same LightList or
    /// have two different current lights within the list.
    /// \return true if the two iterators are unequal, false else.
    bool operator!=(const LightIterator& iter) const; //!< compare iterators

private:
    /// \brief the list of lights to be iterated
    ///
    /// The iteration light list is only referenced here to keep the iterator small.
    LightList* m_list;				    //!< list of light tags is here

    /// \brief the current light
    ///
    /// Keep the index of the current light within the iteration LightList.
    size_t m_current;				    //!< current index into light list 
};

/// \brief Light lists, used internally by LightIterator.
///
/// The LightList class manages the iteration over light sources. The class
/// is only used internally by the LightIterator class. The class is
/// reference counted to allow several iterators to refer to the same
/// light list. However, the class maintains only one cache for the results
/// of light sampling. Therefore, if there are several LightIterators
/// referencing the same light list, then most of the class "get" methods will
/// return results obtained from the most recent call of the sample method.
class LightList {
public:

    /// \brief sets the current light in the light list
    ///
    /// The method is used by light iterators to set the light list to the 
    /// wanted light. It is used when dereferencing a light iterator to 
    /// specify which light should be sampled. 
    /// \param current identifies the light to be made the current one.
    /// \return the current light index
    virtual size_t set_current(size_t current)		= 0; //!< set current light

    /// \brief get the index of the current light
    ///
    /// The method is used by light iterators to obtain the current light index
    /// \return the current light index
    virtual size_t get_current() const                  = 0; //!< get current light index

    /// \brief sample the current light source
    ///
    /// The method samples the current light and caches the obtained results
    /// \return true if a sample was obtained and further samples should be taken,
    /// false if no further samples are required.
    virtual bool sample()				= 0; //!< sample light source

    /// \brief return dot product between light direction and surface normal
    ///
    /// \return the dot product between the light direction and the surface normal,
    /// as obtained and chached during the last sampling.
    virtual miScalar get_dot_nl() const			= 0; //!< dot prod. light dir, normal

    /// \brief return the direction to the light source
    ///
    /// \return the direction vector to the light source, as obtained and cached 
    /// during the last sampling.
    virtual const miVector& get_direction() const	= 0; //!< get direction of sample

    /// \brief get the color contribution of the light source
    ///
    /// The method gets the cached color contribution obtained from the light 
    /// source during the last sampling. 
    /// \param c will on return contain the color contribution of the light 
    /// source. 
    virtual void get_contribution(miColor* c) const	= 0; //!< get color contribution

    /// \brief get the spectrum contribution of the light source
    ///
    /// The method gets the cached spectrum contribution obtained from the light
    /// source during the last sampling. 
    /// \param s will on return contain the spectrum contribution of the 
    /// light source. 
    virtual void get_contribution(miSpectrum* s) const	= 0; //!< get spectrum contribution

    /// \brief returns the number of light samples taken so far.
    ///
    /// The method should be invoked after the sample method returned false. 
    /// It allows to weight the color/spectrum contributions of a light source 
    /// with the number of samples taken.
    /// \return the number of samples taken for the current light source.
    virtual int get_number_of_samples() const		= 0; //!< get current number of samples

    /// \brief get the requested light tag
    ///
    /// The method obtains the light tag at the current'th position in 
    /// the light list.
    /// \param current gives the index of the light in the list.
    /// \return the light tag at the requested position in the list.
    virtual miTag get_light_tag(size_t current) const	= 0; //!< get current light tag

    /// \brief obtain the number of lights in the list
    ///
    /// \return the number of lights in the light list.
    virtual size_t get_number_of_lights() const         = 0; //!< get number of lights in list

    /// \brief increase reference count
    ///
    /// Tell the light list that one more object (most likely a LightIterator) 
    /// has connected to the list. As long as there are objects connected, the
    /// light list is not deleted.
    virtual void connect() 				= 0; //!< used for reference counting

    /// \brief decrease reference count
    ///
    /// Tell the light list that an object (most likely a LightIterator)
    /// releases its connection to the list. If there are no more connections
    /// left, then the LightList will be deleted.
    virtual void release() 				= 0; //!< release resource if not ref'ed
    virtual ~LightList() {}

}; 

//-----------------------------------------------------------------------------
// inline implementation for LightIterator methods
//-----------------------------------------------------------------------------

/// \brief construct a LightIterator
///
/// Construct a LightIterator from state and an optional shader light list.
/// \param state provides the current render state, especially access to the
/// default instance light lists.
/// \param shader_light_list is an optionally provided light list to be used 
/// for the iterationss
/// \param n_shader_lights gives the number of lights in the optional light list
/// provided to the constructor
/// \param version gives the version of the interface class to be used.
inline LightIterator::LightIterator(
    miState* state, 
    miTag*   shader_light_list, 
    int      n_shader_lights,
    int      version)
: m_list(0)
, m_current(0)
{
    Interface* iface = Interface::get(version);
    m_list = iface->createLightList(state, shader_light_list, n_shader_lights);
    iface->release();
}

/// \brief copy constructor 
///
/// The copy constructor is needed to return LightIterators from methods
/// \param iter is the LightIterator to be copied.
inline LightIterator::LightIterator(const LightIterator& iter) 
: m_list(iter.m_list)
, m_current(iter.m_current)
{ 
    if(m_list) m_list->connect();   //!< maintain reference count 
}

/// \brief destructor
///
/// delete the LightIterator. Make sure to release the associated LightList.
inline LightIterator::~LightIterator() 
{
    if(m_list) m_list->release();  //!< maintain reference count
}

/// \brief assign LightIterator
///
/// Assign a LightIterator to this LightIterator.
/// If the two iterators refer to different light lists, make
/// sure that proper reference counting is performed on the two lists.
/// \param iter gives the LightIterator to be assigned
/// \return a constant reference to the LightIterator that has been
/// assigned to.
inline const LightIterator& LightIterator::operator=(const LightIterator& iter) 
{
    if(m_list != iter.m_list) {
        // need the compare above, otherwise the release might delete the list
        // and the connect below would be invalid.
        m_list->release();
        m_list = iter.m_list;
        m_list->connect();
    }
    m_current = iter.m_current;
    return *this;
}

/// \brief obtain current light tag
///
/// Here the LightIterator acts like pointer. Obtain the
/// current light tag by dereferencing this pointer.
/// \return the tag of the current light in the list.
inline miTag LightIterator::operator*() const 
{
    return m_list->get_light_tag(m_current);
}

/// \brief access to light list from iterator
///
/// The postfix dereference operator allows the invokation
/// of LightList class methods from a LightList.
/// \return a pointer to the associated LightList
inline LightList* LightIterator::operator->() const
{
    m_list->set_current(m_current);
    return m_list;
}

/// \brief pre-increment LightIterator
///
/// advances the iterator to the next light in the list.
/// \return a reference to the iterator after it has been advanced
inline const LightIterator& LightIterator::operator++() 
{
    ++m_current;
    return *this;
}

/// \brief post-increment LightIterator
///
/// advances the itertor to the next light in the list
/// \return a copy of the iterator before it was advanced.
inline LightIterator LightIterator::operator++(int)
{
    LightIterator res(*this);
    ++m_current;
    return res;
}

/// \brief check if iterator has reached end
///
/// \return true if end of light list has been reached, false else.
inline bool LightIterator::at_end() const
{
    return m_current == m_list->get_number_of_lights();
}

/// \brief compare LightIterators for equality
///
/// \return true if the two iterators are equal, false else 
inline bool LightIterator::operator==(const LightIterator& iter) const 
{
    return m_list == iter.m_list && m_current == iter.m_current;
}

/// \brief compare LightIterators for inequality
///
/// \return true if the two iterators are unequal, false else.
inline bool LightIterator::operator!=(const LightIterator& iter) const 
{
    return !this->operator==(iter);
}

} // namespace shader
} // namespace mi


#endif // MI_SHADER_IF_H
