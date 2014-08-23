
#include "painterInterface.h"

// Add a normal to the list if the smoothing group bits overlap, 
// otherwise create a new vertex normal in the list
void VNormal::AddNormal(Point3 &n,DWORD s) {
	if (!(s&smooth) && init) {
		if (next) next->AddNormal(n,s);
		else {
			next = new VNormal(n,s);
		}
	} 
	else {
		norm   += n;
		smooth |= s;
		init    = TRUE;
	}
}

// Retrieves a normal if the smoothing groups overlap or there is 
// only one in the list
Point3 &VNormal::GetNormal(DWORD s) {

if (smooth&s || !next) return norm;
	else return next->GetNormal(s);	
}

// Normalize each normal in the list
void VNormal::Normalize() {
	VNormal *ptr = next, *prev = this;
	while (ptr) {
		if (ptr->smooth&smooth) {
			norm += ptr->norm;
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		} 
		else {
			prev = ptr;
			ptr  = ptr->next;
		}
	}
	norm = ::Normalize(norm);
	if (next) next->Normalize();
}