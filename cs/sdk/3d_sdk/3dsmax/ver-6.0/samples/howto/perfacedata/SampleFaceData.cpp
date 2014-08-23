#include "PerFaceData.h"
#include "SampleFaceData.h"

// The following functions make sure the Pipeline are aware of us and the data flows
// up the stack correctly

IFaceDataChannel* SampleFaceData::CreateChannel( ) {
	return static_cast<IFaceDataChannel*>( new SampleFaceData() );
}

IFaceDataChannel* SampleFaceData::CloneChannel( ) {
	IFaceDataChannel* fdc = CreateChannel();
	if ( fdc != NULL ) fdc->AppendChannel( this );	// adds our data.
	return fdc;
}

BOOL SampleFaceData::AppendChannel( const IFaceDataChannel* fromChan ) {
	if ( fromChan == NULL || fromChan->DataChannelID() != DataChannelID())
		return false;
	const SampleFaceData* fromFDChan = dynamic_cast<const SampleFaceData*>(fromChan);
	int start = data.Count();
	data.Append (fromFDChan->data.Count(), fromFDChan->data.Addr(0));
	return true;
}

// Called when new faces were created at index 'at' in the object's list of faces.
BOOL SampleFaceData::FacesCreated( ULONG at, ULONG num ) {
	float val = 0.0f;	 // our default value
	if ( num < 1 ) return FALSE;

	if (at > (ULONG)data.Count()) at = data.Count();

	// Rather than use the Insert function "num" times,
	// here we set the desired count, move existing data if needed,
	// and initialize the new members with memcpy.

	// Set desired count:
	int oldCount = data.Count();
	data.SetCount (oldCount + num);

	// Move existing data if needed:
	int end = at+num;
	int numToMove = oldCount - at;
	if (numToMove) memmove (data.Addr (end), data.Addr (at), numToMove*sizeof(float));

	// initialize new data:
	for(int i=at; i<end; i++) memcpy (data.Addr(i), &val, sizeof(float));

	return TRUE;
}

// The following functions are called when the object has deleted some of its faces

BOOL SampleFaceData::FacesDeleted( BitArray& set )
{
	if ( set.NumberSet() == 0 ) return true;	// nothing to free

	int n = set.GetSize();
	if (n>data.Count()) n = data.Count();

	for ( int i = n-1; i >= 0; i-- ) {
		if (set[i]) data.Delete( i,1 );
	}

	return true;
}

BOOL SampleFaceData::FacesDeleted( ULONG from, ULONG num )
{
	if ((from >= (ULONG)data.Count()) || (num < 1)) return false;
	data.Delete(from,num);
	return TRUE;
}

void SampleFaceData::AllFacesDeleted() {
	data.ZeroCount();
}

// Called when the object has copied some of its faces 'from' 'to'

BOOL SampleFaceData::FaceCopied(ULONG from,ULONG to) {
	if ( from >= (ULONG)data.Count() ) return FALSE;
	if ( to >= (ULONG)data.Count() ) return FALSE;
	data[to] = data[from];
	return TRUE;
}

// Called when a new face has been created in the owner object based on
// data interpolated from other face
// NOTE: that targetFace may be one of the srcFaces!
BOOL SampleFaceData::FaceInterpolated(ULONG numSrc,ULONG *srcFaces,float *coeff,ULONG targetFace)
{
	// if params for interpolation are illegal, try simple copydata
	if ( numSrc < 1 || srcFaces == NULL || numSrc == data.Count()  || coeff == NULL )
		return FALSE;

	float result = data[srcFaces[0]]*coeff[0];

	for ( ULONG i = 1; i < numSrc; i++ )
		result += data[srcFaces[i]]*coeff[i];

	data[targetFace] = result;

	return TRUE;
}

// Called when the owner object has cloned some of its faces and appended
// them to its list of faces.

BOOL SampleFaceData::FacesClonedAndAppended( BitArray& set )
{
	if ( set.GetSize() != data.Count() ) set.SetSize( data.Count(), TRUE );
	
	if ( set.NumberSet() == 0 ) return TRUE;	// nothing to do.

	int n = set.GetSize();
	for ( int i = 0; i < n; i++ ) {
		if (set[i])	data.Append (1, data.Addr(i));
	}

	return TRUE;
}


// Simple Access to our data

BOOL SampleFaceData::GetValue( ULONG at, float& val ) const
{
	val = data[at];
	return TRUE;
}

void SampleFaceData::SetValue( ULONG at, const float& val )
{
	data[at] = val;
}
