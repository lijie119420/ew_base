#ifndef __H_EW_DOMDATA_DTEXT__
#define __H_EW_DOMDATA_DTEXT__

#include "ewa_base/domdata/dobject.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE DText : public DObjectBox
{
public:

	DText(const String& name = "") :DObjectBox(name)
	{ 

	}

	vec3d m_v3Pos;
	vec3d m_v3Pxl;
	vec3d m_v3Shf;
	String m_sText;

	DECLARE_OBJECT_INFO(DText, DObjectInfo);

};


EW_LEAVE

#endif
