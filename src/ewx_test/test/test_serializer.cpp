
#include "ewa_base/testing/test.h"
#include "ewa_base/threading.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/serialization.h"
#include "ewa_base/scripting.h"
#include "ewa_base/math/math_def.h"

using namespace ew;

template<typename T1,typename T2>
class serializable_data : public ObjectData
{
public:

	T1 val;
	T2 tmp;

	void Serialize(SerializerHelper sh)
	{
		Serializer& ar(sh.ref(0));
		ar & val & tmp;
	}

	DECLARE_OBJECT_INFO(serializable_data,ObjectInfo)
};

IMPLEMENT_OBJECT_INFO_T2(serializable_data,ObjectInfo)

class sample_data
{
public:
	bst_map<String,float64_t> aMaps;

	arr_1t<int32_t> aInts;
	String s1;
	DataPtrT<serializable_data<float,String> > dptr1;
	DataPtrT<ObjectData> dptr2;

	int32_t ivals[10];
	arr_xt<float64_t> arr;

	box3i bbox;

	void Serialize(Serializer& ar)
	{
		ar & bbox;
		ar & aInts;
		ar & aMaps;
		ar & s1 & ivals & arr;
		ar & dptr1 & dptr2;
	}

};


TEST_DEFINE(TEST_Serializer)
{

	SerializerStream stream;

	sample_data dat[2];
	dat[0].bbox.set_x(1, 3);
	dat[0].aInts.push_back(1);
	dat[0].aInts.push_back(5);
	dat[0].aMaps["hello"]=1.033;
	dat[0].aMaps["gg"]=1.034;
	dat[0].s1="sample_string";

	serializable_data<float,String>* p1=new serializable_data<float,String>;
	p1->val=1.324;
	p1->tmp="hello";

	dat[0].dptr1.reset(p1);
	dat[0].dptr2.reset(p1);

	dat[0].arr.resize(6,3,2);
	dat[0].arr(1,2,1)=3.2;
	dat[0].arr(0)=1.234;

	dat[0].ivals[3]=32;

	stream.openfile("test_serializer.dat",FLAG_FILE_WC|FLAG_FILE_TRUNCATE);
	try
	{
		stream.writer().handle_head();
		stream.writer() & dat[0];
		stream.writer().handle_tail();
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"serializer write file failed!");
	}
	stream.close();

	stream.openfile("test_serializer.dat");
	try
	{
		stream.reader().handle_head();
		stream.reader() & dat[1];
		stream.reader().handle_tail();
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"serializer read file failed!");
	}
	stream.close();
	TEST_ASSERT(dat[1].bbox==dat[0].bbox);
	TEST_ASSERT(dat[1].aInts==dat[0].aInts);
	TEST_ASSERT(dat[1].aMaps==dat[0].aMaps);
	TEST_ASSERT(dat[1].s1==dat[0].s1);

	TEST_ASSERT(dat[1].dptr1==dat[1].dptr2);
	TEST_ASSERT(dat[1].dptr1!=NULL);
	TEST_ASSERT(dat[1].dptr1->val==dat[0].dptr1->val);

	TEST_ASSERT(dat[1].arr.size(0)==dat[0].arr.size(0));
	TEST_ASSERT(dat[1].arr.size(1)==dat[0].arr.size(1));
	TEST_ASSERT(dat[1].arr.size(2)==dat[0].arr.size(2));

	if(dat[1].arr.size(0)==dat[0].arr.size(0))
	{
		for(size_t i=0; i<dat[1].arr.size(0); i++)
		{
			TEST_ASSERT(dat[1].arr(i)==dat[0].arr(i));
		}
	}

	for(int i=0; i<10; i++)
	{
		TEST_ASSERT(dat[1].ivals[i]==dat[0].ivals[i]);
	}


	SerializerStream sbuf;
	sbuf.assign(new IStreamBuffer);

	double v1[4]= {1.234,234.0,323,432};
	double v2[4];

	try
	{
		// write data to buffer
		sbuf.writer().handle_head();
		sbuf.writer() & v1;
		sbuf.writer().handle_tail();

		// read data from buffer
		sbuf.reader().handle_head();
		sbuf.reader() & v2;
		sbuf.reader().handle_tail();
	}
	catch(...)
	{
		TEST_ASSERT_MSG(false,"SerializerBuffer failed!");
	}

	TEST_ASSERT(memcmp(v1,v2,sizeof(double)*4)==0);
	//TEST_ASSERT(sbuf.skip()); // reader reach the end of buffer
}


TEST_DEFINE(TEST_SerializerSeek)
{
	typedef serializable_data<float,String> float_string;

	SerializerStream sf;

	if(sf.openfile("seekable.dat",FLAG_FILE_WC))
	{

		SerializerWriter &writer(sf.writer());

		DataPtrT<float_string> p1(new float_string);
		DataPtrT<float_string> p2(new float_string);
		DataPtrT<CallableTableEx> p3(new CallableTableEx);

		p3->value["abc"].reset("hello");
		p3->value["cde"].reset("world");

		p1->val=1.0;
		p2->val=2.0;

		writer.flags.add(Serializer::FLAG_OFFSET_TABLE);
		writer.handle_head();
		writer & p1 & p2 & p3;
		writer.handle_tail();

	}
	sf.close();

	if(sf.openfile("seekable.dat",FLAG_FILE_RD))
	{
		DataPtrT<float_string> p1;
		DataPtrT<float_string> p2;
		DataPtrT<CallableTableEx> p3;
		DataPtrT<ObjectData> p4;

		SerializerReader &reader(sf.reader());

		reader.handle_head();

		p3.reset(dynamic_cast<CallableTableEx*>(reader.read_object(3)));
		p1.reset(dynamic_cast<float_string*>(reader.read_object(1)));
		p2.reset(dynamic_cast<float_string*>(reader.read_object(2)));
		p4.reset(reader.read_object(4));


		TEST_ASSERT(p3);

		if(p3)
		{
			String& s1=p3->value["abc"].ref<String>();
			String& s2=p3->value["cde"].ref<String>();
			TEST_ASSERT(s1=="hello");
			TEST_ASSERT(s2=="world");		
		}

		TEST_ASSERT(p1 && p1->val==1.0);
		TEST_ASSERT(p2 && p2->val==2.0);

		TEST_ASSERT_EQ(reader.cached_objects().shrink(),0);
		p3.reset(NULL);
		TEST_ASSERT_EQ(reader.cached_objects().shrink(),2);

		DataPtrT<CallableTableEx> p5(new CallableTableEx);
		p5->value["abc"].ref<String>();
		p5->value["cde"].ref<String>();

		p3.reset(dynamic_cast<CallableTableEx*>(reader.read_object(3)));
		TEST_ASSERT_EQ(reader.cached_objects().shrink(),0);
		TEST_ASSERT(p3 && p4.get()==p3->value["abc"].kptr());
	}

	
}
