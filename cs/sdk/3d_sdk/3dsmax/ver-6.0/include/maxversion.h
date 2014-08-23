#ifndef _MAXVERSION_H_
#define _MAXVERSION_H_

#include "buildnumber.h" // defines VERSION_INT
#include "buildver.h" 

// VERSION_INT is defined in buildnumber.h and is written to by the builder, inserting the current build number.
// VERSION_INT is used for the fourth least siginifact version component eg. 4.1.1.[36]

// The product and file version could be different.
// For example, VIZ 4.0 works with max files version 4.2

#if defined(RENDER_VER)
	// JH 4/23/03 Incrementing the point version for Service pack 1

	// VIZ Render File version
	#ifndef MAX_VERSION_MAJOR
	#define MAX_VERSION_MAJOR 1
	#endif

	#ifndef MAX_VERSION_MINOR
	#define MAX_VERSION_MINOR 0
	#endif

	#ifndef MAX_VERSION_POINT
	#define MAX_VERSION_POINT 1
	#endif
  
	// VIZ Product version
	#ifndef MAX_PRODUCT_VERSION_MAJOR
	#define MAX_PRODUCT_VERSION_MAJOR 1
	#endif

	#ifndef MAX_PRODUCT_VERSION_MINOR
	#define MAX_PRODUCT_VERSION_MINOR 0
	#endif

	#ifndef MAX_PRODUCT_VERSION_POINT
	#define MAX_PRODUCT_VERSION_POINT 1
	#endif

	// aszabo|Mar.15.02|This should be blank for a non-service-pack release
	#ifndef MAX_PRODUCT_VERSION_SP
	#define MAX_PRODUCT_VERSION_SP "SP 1\0"
	#endif

	// MAX_RELEASE_EXTERNAL is an alternative for MAX_RELEASE (plugapi.h)
	// that can be used by UI when the internal version of the product
	// is different from the branded version number
	// JH 4/23/03 I'm incrementing this as well although I'm not 100% sure this is right
	#define MAX_RELEASE_EXTERNAL	1010

#elif defined(DESIGN_VER)

	// VIZ File version
	#ifndef MAX_VERSION_MAJOR
	#define MAX_VERSION_MAJOR 4
	#endif

	#ifndef MAX_VERSION_MINOR
	#define MAX_VERSION_MINOR 2
	#endif

	#ifndef MAX_VERSION_POINT
	#define MAX_VERSION_POINT 7
	#endif
  
	// VIZ Product version
	#ifndef MAX_PRODUCT_VERSION_MAJOR
	#define MAX_PRODUCT_VERSION_MAJOR 4
	#endif

	#ifndef MAX_PRODUCT_VERSION_MINOR
	#define MAX_PRODUCT_VERSION_MINOR 0
	#endif

	#ifndef MAX_PRODUCT_VERSION_POINT
	#define MAX_PRODUCT_VERSION_POINT 0
	#endif

	// aszabo|Mar.15.02|This should be blank for a non-service-pack release
	#ifndef MAX_PRODUCT_VERSION_SP
	#define MAX_PRODUCT_VERSION_SP "SP 1\0"
	#endif

#elif defined(WEBVERSION)

	// Mercury File version:
	#ifndef MAX_VERSION_MAJOR
	#define MAX_VERSION_MAJOR 4
	#endif

	#ifndef MAX_VERSION_MINOR
	#define MAX_VERSION_MINOR 2
	#endif

	#ifndef MAX_VERSION_POINT
	#define MAX_VERSION_POINT 0
	#endif

	// Mercury Product version
	#ifndef MAX_PRODUCT_VERSION_MAJOR
	#define MAX_PRODUCT_VERSION_MAJOR 1
	#endif

	#ifndef MAX_PRODUCT_VERSION_MINOR
	#define MAX_PRODUCT_VERSION_MINOR 0
	#endif

	#ifndef MAX_PRODUCT_VERSION_POINT
	#define MAX_PRODUCT_VERSION_POINT 0
	#endif

	// aszabo|May.13.02|This should be blank for a non-service-pack release
	#ifndef MAX_PRODUCT_VERSION_SP
	#define MAX_PRODUCT_VERSION_SP "\0"
	#endif

#else

	// MAX File version:
	#ifndef MAX_VERSION_MAJOR
	#define MAX_VERSION_MAJOR 6
	#endif

	#ifndef MAX_VERSION_MINOR
	#define MAX_VERSION_MINOR 0
	#endif

	#ifndef MAX_VERSION_POINT
	#define MAX_VERSION_POINT 0
	#endif

	// MAX Product version
	#ifndef MAX_PRODUCT_VERSION_MAJOR
	#define MAX_PRODUCT_VERSION_MAJOR 6
	#endif

	#ifndef MAX_PRODUCT_VERSION_MINOR
	#define MAX_PRODUCT_VERSION_MINOR 0
	#endif

	#ifndef MAX_PRODUCT_VERSION_POINT
	#define MAX_PRODUCT_VERSION_POINT 0
	#endif

	// aszabo|May.13.02|This should be blank for a non-service-pack release
	#ifndef MAX_PRODUCT_VERSION_SP
	#define MAX_PRODUCT_VERSION_SP "\0"
	#endif
#endif


#define _MAX_VERSION(a, b, c,d) a##b##c##d
#define MAX_VERSION _MAX_VERSION(MAX_VERSION_MAJOR, MAX_VERSION_MINOR, MAX_VERSION_POINT,VERSION_INT)


#endif
