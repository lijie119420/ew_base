#ifndef __UI_IWND_TEXTCTRL__
#define __UI_IWND_TEXTCTRL__

#include "wx/textctrl.h"
#include "wx/srchctrl.h"
#include "ewc_base/wnd/impl_wx/window.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE IWnd_textctrl : public wxTextCtrl
{
public:
	IWnd_textctrl(wxWindow* p,const WndPropertyEx& h,int w=0);
	ValidatorHolderT<Validator> m_pVald;
};

class DLLIMPEXP_EWC_BASE IWnd_password : public IWnd_textctrl
{
public:
	IWnd_password(wxWindow* p,const WndPropertyEx& h);
};


class DLLIMPEXP_EWC_BASE IWnd_search : public wxSearchCtrl
{
public:
	IWnd_search();
	IWnd_search(wxWindow* p,const WndPropertyEx& h);
	ValidatorHolderT<Validator> m_pVald;
};



class DLLIMPEXP_EWC_BASE ICmdProcTextEntryCtrl : public ICmdProcTextEntry
{
public:
	typedef ICmdProcTextEntry basetype;

	IWnd_textctrl& Target;
	IDat_search& data;

	ICmdProcTextEntryCtrl(IWnd_textctrl& t);

	bool TestSelection();
	bool DoFind(ICmdParam& cmd);
	bool DoReplace(ICmdParam& cmd);
	bool DoReplaceAll(ICmdParam& cmd);
	bool DoExecId(ICmdParam& cmd);
	bool DoTestId(ICmdParam& cmd);

};


EW_LEAVE

#endif
