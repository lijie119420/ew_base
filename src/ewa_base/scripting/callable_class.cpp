#include "ewa_base/scripting/callable_class.h"
#include "ewa_base/scripting/callable_array.h"
#include "ewa_base/scripting/parser_nodes.h"
#include "ewa_base/scripting/executor.h"
#include "ewa_base/scripting/callable_iterator.h"
#include "ewa_base/util/strlib.h"

EW_ENTER


int CallableMetatable::__metatable_call1(Executor&,const String&)
{
	return 0;
}

int CallableMetatable::__metatable_call2(Executor& ewsl,const String& op)
{
	typedef bool (CallableMetatable::*funcall2)(Variant&, Variant&, Variant&);

	static class metatable_op_map : public indexer_map<String,funcall2>
	{
	public:
		metatable_op_map()
		{
			indexer_map<String,funcall2>& mp(*this);

			mp["math.add"]=&CallableMetatable::__add;
			mp["math.sub"]=&CallableMetatable::__sub;
			mp["math.mul"]=&CallableMetatable::__mul;
			mp["math.div"]=&CallableMetatable::__div;

			mp["math.dot_mul"]=&CallableMetatable::__dot_mul;
			mp["math.dot_div"]=&CallableMetatable::__dot_div;

			mp["rel.eq"]=&CallableMetatable::__eq;
			mp["rel.ne"]=&CallableMetatable::__ne;
			mp["rel.gt"]=&CallableMetatable::__gt;
			mp["rel.ge"]=&CallableMetatable::__ge;
			mp["rel.lt"]=&CallableMetatable::__lt;
			mp["rel.le"]=&CallableMetatable::__le;

		}

	}op_map;

	int id=op_map.find1(op);
	if(id<0) return 0;

	Variant& v1(ewsl.ci1.nsp[-1]);
	Variant& v2(ewsl.ci1.nsp[-0]);

	funcall2 fun=op_map.get(id).second;

	if((this->*fun)(v1,v1,v2))
	{
		--ewsl.ci1.nsp;
		return 1;
	}

	return 0;
}

template<int M>
class DLLIMPEXP_EWA_BASE CallableDataIteratorClassT : public CallableDataIterator1
{
public:

	static const int N=M>0?M:-M;

	DataPtrT<CallableClass> obj;
	size_t it1;
	size_t it2;
	int ret;

	CallableDataIteratorClassT(CallableClass* p):obj(p)
	{
		it1=0;
		it2=obj->value.size();
	}


	int __fun_call(Executor& ewsl,int)
	{
		if(it1==it2)
		{
			ewsl.push(false);
			if(N>1) ewsl.push();
			ewsl.push();
		}
		else
		{
			ewsl.push(true);
			if(M==-2) ewsl.push(it1);
			if(M==+2) ewsl.push(obj->metax->table_self.get(it1).first);
			ewsl.push(obj->value[it1]);
			it1++;
		}
		return N+1;
	}
};

void CallableClass::__get_iterator(Executor& ewsl,int nd)
{
	if(nd==1)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIteratorClassT<1>(this));
	}
	else if(nd==2)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIteratorClassT<2>(this));
	}
	else if(nd==-2)
	{
		ewsl.ci1.nsp[0].kptr(new CallableDataIteratorClassT<-2>(this));
	}
	else
	{
		CallableData::__get_iterator(ewsl,nd);
	}
}

CallableMetatable::CallableMetatable(const String& name):table_meta(value),m_sClassName(name){}

void CallableMetatable::Serialize(Serializer& ar)
{
	ar & m_sClassName;
	ar & table_self;
	ar & table_meta;
}

bool CallableMetatable::ToValue(String& v,int) const
{
	v.Printf("metatable:%s(0x%p)",m_sClassName,this);
	return true;
}


HelpData* CallableMetatable::__get_helpdata()
{ 
	if (!m_pHelp)
	{
		m_pHelp.reset(new HelpData);

		StringBuffer<char> sb;
		if(sb.load("scripting/help/"+m_sClassName+".txt"))
		{
			m_pHelp->parse(m_sClassName, sb);
		}
	}
	return m_pHelp.get(); 
}

int CallableMetatable::__fun_call(Executor& ewsl,int pm)
{
	ewsl.check_pmc(this,pm,0);
	DataPtrT<CallableClass> p=new CallableClass(this);
	ewsl.push(p);
	return 1;
}

bool CallableClass::ToValue(String& v,int) const
{
	v.Printf("class:%s(0x%p)",metax->m_sClassName,this);
	return true;
}

bool CallableModule::ToValue(String& v,int) const
{
	v.Printf("module:%s(0x%p)",m_sClassName,this);
	return true;
}

void CallableClass::reset(CallableMetatable* p)
{
	metax.reset(p);

	if (metax)
	{
		size_t n = metax->table_self.size();
		value.resize(n);

		for (size_t i = 0; i<n; i++)
		{
			value[i] = metax->table_self.get(i).second;
		}
	}
	else
	{
		value.clear();
	}

}

int CallableClass::__getindex(Executor& ewsl,const String& si)
{
	ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr(this);
	{
		VariantTable& tb(metax->table_self);
		int id=tb.find1(si);
		if(id>=0)
		{
			(*ewsl.ci1.nsp)=value[id];
			return STACK_BALANCED;
		}
	}
	{
		VariantTable& tb(metax->table_meta);
		int id=tb.find1(si);
		if(id>=0)
		{
			(*ewsl.ci1.nsp)=tb.get(id).second;
			return STACK_BALANCED;
		}
	}

	ewsl.kerror(String::Format("getindex FAILED! Class has no index:%s",si));
	return INVALID_CALL;
}

int CallableClass::__setindex(Executor& ewsl,const String& si)
{

	{
		VariantTable& tb(metax->table_self);
		int id=tb.find1(si);
		if(id>=0)
		{
			--ewsl.ci1.nsp;
			ewsl.popq(value[id]);
			return 0;
		}
	}
	{
		VariantTable& tb(metax->table_meta);
		int id=tb.find1(si);
		if(id>=0)
		{
			--ewsl.ci1.nsp;
			ewsl.popq(tb.get(id).second);
			return 0;
		}
	}

	ewsl.kerror(String::Format("setindex FAILED! Class has no index:%s",si));
	return INVALID_CALL;
}

void CallableClass::Serialize(Serializer& ar)
{
	ar & metax;
	ar & value;
}

int CallableMetatable::__setindex(Executor& ewsl,const String&)
{
	ewsl.kerror("readonly");
	return -1;
}

int CallableMetatable::__setarray(Executor& ewsl,int pm)
{
	ewsl.kerror("readonly");
	return -1;
}


IMPLEMENT_OBJECT_INFO(CallableModule,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableMetatable,ObjectInfo);
IMPLEMENT_OBJECT_INFO(CallableClass,ObjectInfo);

EW_LEAVE
