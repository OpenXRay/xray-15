#include "IDataChannel.h"
#include "ifacedatamgr.h"

#ifndef __RANDOMFACEDATA__H
#define __RANDOMFACEDATA__H

#define FACE_MAXSAMPLEUSE_CLSID Class_ID(0x5a0523ac, 0x63b701e2)

class SampleFaceData : public IFaceDataChannel  
{
	public:
		// Our Storage Container

		Tab<float> data;
		SampleFaceData( ) 
		{ 
		};

		~SampleFaceData( ) 
		{
		};
		
		// From IFaceDataChannel

		virtual BOOL FacesCreated( ULONG at, ULONG num ) ;
		BOOL FacesClonedAndAppended( BitArray& set ) ;
		BOOL FacesDeleted( BitArray& set ) ;
		BOOL FacesDeleted( ULONG from, ULONG num ) ;
		void AllFacesDeleted() ;
		BOOL FaceCopied( ULONG from, ULONG to ) ;
		BOOL FaceInterpolated( ULONG numSrc, ULONG* srcFaces,float* coeff, ULONG targetFace ); 


		IFaceDataChannel* CreateChannel( ) ;
		IFaceDataChannel* CloneChannel( ) ;
		BOOL AppendChannel( const IFaceDataChannel* fromChan ) ;

		//from IDataChannel
		
		Class_ID DataChannelID()const { return FACE_MAXSAMPLEUSE_CLSID; }; 
		ULONG Count() const { return data.Count(); };

		void DeleteThis(){delete this;}; 

		// Simple data access
		BOOL GetValue( ULONG at, float& val ) const;
		void SetValue( ULONG at, const float& val );
};

#endif