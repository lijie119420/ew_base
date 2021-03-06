
#include "ewc_base/wnd/wnd_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/mvc/mvc_book.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_maker.h"

EW_ENTER



class WndManagerImpl
{
public:

	WndManagerImpl()
	{
		plogptr.reset(new LogPtr());
	}

	App app;

	EvtManagerTop evtmgr;

	WndUpdator wup;

	PluginManager plugin;

	MvcBook book;

	DataPtrT<LogPtr> plogptr;

	static WndManagerImpl& current()
	{
		static WndManagerImpl gInstance;
		return gInstance;
	}
};


wxWindow* WndModelHolder::GetWindow()
{
	return m_refData?m_refData->GetWindow():NULL;
}

void WndModelHolder::Show(bool flag)
{
	if(!m_refData) return;
	m_refData->Show(flag);
}

void WndModelHolder::OnChildWindow(IWindowPtr w,int a)
{
	if(!m_refData) return;
	m_refData->OnChildWindow(w,a);
}

bool WndModelHolder::mu_set(const String& a)
{
	return m_refData && m_refData->mu_set(a);
}

bool WndModelHolder::Create()
{
	if(!m_refData) return false;
	if(!m_refData->Create()) return false;
	m_refData->SetLabel(m_sName);
	return true;
}




WndManager::WndManager()
	:evtmgr(WndManagerImpl::current().evtmgr)
	,wup(WndManagerImpl::current().wup)
	,book(WndManagerImpl::current().book)
	,plugin(WndManagerImpl::current().plugin)
	,app(WndManagerImpl::current().app)
	,logptr(*WndManagerImpl::current().plogptr)
	,lang(Language::current())
{
	Logger::def(&logptr);
}

WndManager::~WndManager()
{

}


void WndManager::StartFrame()
{

	evtmgr["StartFrame"].CmdExecuteEx(-1);
	
	UpdateTitle();
	model.Show(true);
}

bool WndManager::LoadPlugins()
{
	LockGuard<WndUpdator> lock(wup);

	model.SetData(new WndModelTop(*this));

	for(size_t i=0;i<plugin.plugin_map.size();i++)
	{
		Plugin* p=plugin.plugin_map.get(i).second.get();
		if(!plugin.AttachPlugin(p))
		{
			System::LogError("WndManager::LoadPlugins FAILED");
			return false;
		}
	}
	wup.update();

	model.Show(false);
	return true;
}

void WndManager::SetName(const String& s)
{
	model.m_sName=s;
	UpdateTitle();
}

String WndManager::GetTitle()
{
	return Translate(model.m_sName);
}

void WndManager::UpdateTitle()
{
	if(!model.m_refData) return;

	if(model.m_sLabel.empty())
	{
		model.m_refData->SetLabel(Translate(model.m_sName));
	}
	else
	{
		model.m_refData->SetLabel(model.m_sLabel);
	}
}

bool WndManager::LoadConfig()
{
	evtmgr["Config"].StdExecuteEx(+1);
	return true;
}

bool WndManager::SaveConfig(bool save_file)
{
	evtmgr["Config"].StdExecuteEx(-1);
	if (save_file)
	{
		return conf.Save();
	}
	return true;
}


EvtManager& EvtManager::current()
{
	return WndManager::current().evtmgr;
}

WndUpdator& WndUpdator::current()
{
	return WndManager::current().wup;
}

PluginManager& PluginManager::current()
{
	return WndManager::current().plugin;
}

MvcBook& MvcBook::current()
{
	return WndManager::current().book;
}


WndModel& WndModel::current()
{
	return *WndManager::current().model.GetData();
}




class EvtManagerGroupSetAccel : public CallableFunction
{
public:
	EvtManagerGroupSetAccel():CallableFunction("ui.evt.set_accel"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm>=2)
		{
			String val1;
			String val2;
			String val3;

			if(!ewsl.ci0.nbx[1].get(val1))
			{
				ewsl.kerror("invalid param");
			}
			if(!ewsl.ci0.nbx[2].get(val2))
			{
				ewsl.kerror("invalid param");
			}
			EvtCommand* p=EvtManager::current().get_command(val1);
			if(p)
			{
				p->m_sAccel=val2;
			}

			if (p && pm >= 3 && ewsl.ci0.nbx[3].get(val3))
			{
				p->m_sHotkey=val3;
			}


		}
		else
		{
			ewsl.kerror("invalid param");
		}
		return 0;
	};
};

class EvtManagerGroupSetHotkey : public CallableFunction
{
public:
	EvtManagerGroupSetHotkey():CallableFunction("ui.evt.set_hotkey"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm>=2)
		{
			String val1;
			String val2;
			String val3;

			if(!ewsl.ci0.nbx[1].get(val1))
			{
				ewsl.kerror("invalid param");
			}
			if(!ewsl.ci0.nbx[2].get(val2))
			{
				ewsl.kerror("invalid param");
			}
			EvtCommand* p=EvtManager::current().get_command(val1);
			if(p)
			{
				p->m_sHotkey=val2;
			}

			if (p && pm >= 3 && ewsl.ci0.nbx[3].get(val3))
			{
				p->m_sAccel = val3;
			}

		}
		else
		{
			ewsl.kerror("invalid param");
		}
		return 0;
	};
};

class EvtManagerGroupBeg : public CallableFunction
{
public:

	EvtManagerGroupBeg():CallableFunction("ui.evt.gp_beg"){}

	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==1)
		{		
			String val;
			if(!ewsl.ci0.nbx[1].get(val))
			{
				ewsl.kerror("invalid param");
			}
			EvtManager::current().gp_beg(val);
		}
		else if(pm==2)
		{
			String val1;
			String val2;
			if(!ewsl.ci0.nbx[1].get(val1))
			{
				ewsl.kerror("invalid param");
			}
			if(!ewsl.ci0.nbx[2].get(val2))
			{
				ewsl.kerror("invalid param");
			}
			EvtManager::current().gp_beg(val1,val2);

		}
		else
		{
			ewsl.kerror("invalid param");
		}
		return 0;
	};
};

class EvtManagerGroupNew : public CallableFunction
{
public:
	EvtManagerGroupNew():CallableFunction("ui.evt.gp_new"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==1)
		{		
			String val;
			if(!ewsl.ci0.nbx[1].get(val))
			{
				ewsl.kerror("invalid param");
			}
			EvtManager::current().gp_new(val);
		}
		else
		{
			ewsl.kerror("invalid param");
		}
		return 0;
	};
};



class EvtManagerGroupAdd : public CallableFunction
{
public:
	EvtManagerGroupAdd():CallableFunction("ui.evt.gp_add"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		if(pm==1)
		{
			String val;

			if(!ewsl.ci0.nbx[1].get(val))
			{
				ewsl.kerror("invalid param");
			}
			EvtManager::current().gp_add(val);
		}
		else if(pm==2)
		{
			String val;
			if(!ewsl.ci0.nbx[1].get(val))
			{
				ewsl.kerror("invalid param");
			}
			String sval;
			int64_t ival;
			if(ewsl.ci0.nbx[2].get(sval))
			{
				EvtManager::current().gp_add(val,sval);
			}
			else if(ewsl.ci0.nbx[2].get(ival))
			{
				EvtManager::current().gp_add(val,ival);
			}
			else
			{
				ewsl.kerror("invalid param");
			}
		}
		else
		{
			ewsl.kerror("invalid param");
		}
		return 0;
	};
};
class EvtManagerGroupEnd : public CallableFunction
{
public:
	EvtManagerGroupEnd():CallableFunction("ui.evt.gp_end"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this,pm,0);
		EvtManager::current().gp_end();
		return 0;
	};
};

bool WndManager::InitConfig(const String& fp)
{
	conf.s_file = fp;

	// 尝试从文件载入配置文件
	if (!conf.Load())
	{
		// 配置文件不存在，初始化配置信息
		conf.SetValue<String>("/basic/language", "Chinese");
	}

	String sLanguage;
	if (conf.GetValue<String>("/basic/language", sLanguage))
	{
		Language::current().SetLanguage(sLanguage);
	}

	return true;

}

bool WndManager::LoadScript(const String& fp)
{


	try
	{
		Executor ewsl;
		if(!ewsl.execute_file(fp))
		{
			Exception::XError("invalid script");
		}
	}
	catch(std::exception& e)
	{
		this_logger().LogError(e.what());
	}

	return true;
}




WndManager* g_pWndManagerInstance = NULL;

WndManager& WndManager::current()
{
	if (!g_pWndManagerInstance)
	{
		static WndManager gInstance;
		g_pWndManagerInstance = &gInstance;

		CG_GGVar &gi(CG_GGVar::current());
		gi.add(new EvtManagerGroupBeg);
		gi.add(new EvtManagerGroupAdd);
		gi.add(new EvtManagerGroupEnd);
		gi.add(new EvtManagerGroupNew);
		gi.add(new EvtManagerGroupSetAccel);
		gi.add(new EvtManagerGroupSetHotkey);
		gi.add(new CallableMaker, "ui.maker");
	}
	return *g_pWndManagerInstance;
}


EW_LEAVE
