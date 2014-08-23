/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM d:/BUILDS/tinderbox/Mozilla1.7/WINNT_5.0_Clobber/mozilla/extensions/wallet/public/nsIPassword.idl
 */

#ifndef __gen_nsIPassword_h__
#define __gen_nsIPassword_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIPassword */
#define NS_IPASSWORD_IID_STR "cf39c2b0-1e4b-11d5-a549-0010a401eb10"

#define NS_IPASSWORD_IID \
  {0xcf39c2b0, 0x1e4b, 0x11d5, \
    { 0xa5, 0x49, 0x00, 0x10, 0xa4, 0x01, 0xeb, 0x10 }}

/** 
 * An optional interface for clients wishing to access a
 * password object
 * 
 * @status FROZEN
 */
class NS_NO_VTABLE nsIPassword : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_IPASSWORD_IID)

  /**
     * the name of the host corresponding to the login being saved
     *
     * The form of the host depends on how the nsIPassword object was created
     *
     * - if it was created as a result of submitting a form to a site, then the
     *   host is the url of the site, as obtained from a call to GetSpec
     *
     * - if it was created as a result of another app (e.g., mailnews) calling a
     *   prompt routine such at PromptUsernameAndPassword, then the host is whatever
     *   arbitrary string the app decided to pass in.
     *
     * Whatever form it is in, it will be used by the password manager to uniquely
     * identify the login realm, so that "newsserver:119" is not the same thing as
     * "newsserver".
     */
  /* readonly attribute AUTF8String host; */
  NS_IMETHOD GetHost(nsACString & aHost) = 0;

  /**
     * the user name portion of the login
     */
  /* readonly attribute AString user; */
  NS_IMETHOD GetUser(nsAString & aUser) = 0;

  /**
     * the password portion of the login
     */
  /* readonly attribute AString password; */
  NS_IMETHOD GetPassword(nsAString & aPassword) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPASSWORD \
  NS_IMETHOD GetHost(nsACString & aHost); \
  NS_IMETHOD GetUser(nsAString & aUser); \
  NS_IMETHOD GetPassword(nsAString & aPassword); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPASSWORD(_to) \
  NS_IMETHOD GetHost(nsACString & aHost) { return _to GetHost(aHost); } \
  NS_IMETHOD GetUser(nsAString & aUser) { return _to GetUser(aUser); } \
  NS_IMETHOD GetPassword(nsAString & aPassword) { return _to GetPassword(aPassword); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPASSWORD(_to) \
  NS_IMETHOD GetHost(nsACString & aHost) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHost(aHost); } \
  NS_IMETHOD GetUser(nsAString & aUser) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUser(aUser); } \
  NS_IMETHOD GetPassword(nsAString & aPassword) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPassword(aPassword); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPassword : public nsIPassword
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPASSWORD

  nsPassword();

private:
  ~nsPassword();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPassword, nsIPassword)

nsPassword::nsPassword()
{
  /* member initializers and constructor code */
}

nsPassword::~nsPassword()
{
  /* destructor code */
}

/* readonly attribute AUTF8String host; */
NS_IMETHODIMP nsPassword::GetHost(nsACString & aHost)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AString user; */
NS_IMETHODIMP nsPassword::GetUser(nsAString & aUser)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AString password; */
NS_IMETHODIMP nsPassword::GetPassword(nsAString & aPassword)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPassword_h__ */
