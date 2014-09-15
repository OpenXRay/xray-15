#ifndef PHDEFS_H
#define PHDEFS_H
class CPHElement;
class CPHJoint;
class CPhysicsShell;

//class CPHFracture;
class CShellSplitInfo;

typedef std::pair<CPhysicsShell*,u16>	shell_root;

using ELEMENT_STORAGE = xr_vector<CPHElement*>;
using ELEMENT_I = ELEMENT_STORAGE::iterator;
typedef		xr_vector<CPHElement*>::const_iterator	ELEMENT_CI;
using JOINT_STORAGE = xr_vector<CPHJoint*>;
using JOINT_I = JOINT_STORAGE::iterator;
using PHSHELL_PAIR_VECTOR = xr_vector<shell_root>;
using SHELL_PAIR_I = PHSHELL_PAIR_VECTOR::iterator;
typedef xr_vector<shell_root>::reverse_iterator SHELL_PAIR_RI;

typedef		xr_vector<CPHElement*>::reverse_iterator	ELEMENT_RI;

#endif