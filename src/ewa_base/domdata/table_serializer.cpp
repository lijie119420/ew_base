#include "ewa_base/domdata/table_serializer.h"
#include "ewa_base/domdata/symm.h"

EW_ENTER


TableSerializer::TableSerializer(int t,VariantTable& v):type(t),value(v)
{

}

void TableSerializer::link(const String& s,double& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<double>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,BitFlags& f,int m)
{
	int v=f.get(m)?1:0;
	link(s,v);
	if(type==READER)
	{
		f.set(m,v!=0);
	}
}

void TableSerializer::link(const String& s,int& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<int>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,String& v)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<String>::g(value.get(id).second);
		}
	}
}

void TableSerializer::link(const String& s,String& v,const String& d)
{
	if(type==WRITER)
	{
		value[s].reset(v);
	}
	if(type==READER)
	{
		int id=value.find1(s);
		if(id>=0)
		{
			v=pl_cast<String>::g(value.get(id).second);
		}
		else
		{
			v=d;
		}
	}
}

//void TableSerializer::link_vec3d(const String& s, vec3d& v)
//{
//	link(s+".x", v[0]);
//	link(s+".y", v[1]);
//	link(s+".z", v[2]);
//}
//
//void TableSerializer::link_vec2d(const String& s, vec2d& v)
//{
//	link(s + ".x", v[0]);
//	link(s + ".y", v[1]);
//}
//
//void TableSerializer::link_box3d(const String& s, box3d& v)
//{
//	link_vec3d(s + ".DL", v.hi);
//	link_vec3d(s + ".DH", v.lo);
//}



EW_LEAVE

