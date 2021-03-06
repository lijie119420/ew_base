#ifndef __H_EW_PROCESS__
#define __H_EW_PROCESS__

#include "ewa_base/config.h"
#include "ewa_base/basic/object.h"
#include "ewa_base/basic/platform.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/pointer.h"
#include "ewa_base/basic/stream.h"

EW_ENTER

class ProcessImpl;

class DLLIMPEXP_EWA_BASE Process : public Object
{
public:
	enum
	{
		FLAG_ASYNC		=1<<0,
		FLAG_CONSOLE	=1<<1,
	};

	Process();

	bool Redirect();
	bool Redirect(KO_Handle<KO_Policy_handle> h);

	bool Execute(const String& cmd);

	Stream GetStream();

	bool Kill(int r=0);


	void Wait();

	bool WaitFor(int ms);

	void Close();

	bool GetExitCode(int* code=NULL);

protected:

	ProcessImpl& impl();

	DataPtrT<ObjectData> m_impl;
};


EW_LEAVE

#endif
