#include "regex_impl.h"
#include "ewa_base/util/regex.h"
EW_ENTER

template<typename X>
bool regex_impl<X>::search(regex_item_seq* seq,iterator t1,iterator t2)
{
	if(!seq) return false;

	it_beg=it_cur=t1;
	it_end=t2;

	regex_item_repeat repeat;

	repeat.child.reset(new regex_item_char_any);
	repeat.repeat_end.match_as_much_as_possible=false;
	repeat.child->sibling=&repeat.repeat_end;
	repeat.update(seq);

	if(!match_real(it_cur,&repeat))
	{
		return false;
	}
	return true;
}

template<typename X>
bool regex_impl<X>::match(regex_item_seq* seq,iterator t1,iterator t2)
{
	if(!seq) return false;

	it_beg=it_cur=t1;
	it_end=t2;

	if(!match_real(it_cur,seq))
	{
		return false;
	}
	return it_cur==it_end;
}

template<typename X>
bool regex_impl<X>::fallback(regex_state& state)
{
	if(arrStates.empty())
	{
		return false;
	}

	state=arrStates.back();
	stkRepeat.resize(state.n_pos_repeat);
	stkSeqpos.resize(state.n_pos_seqpos);
	arrStates.pop_back();
		
	return true;
}

template<typename X>
bool regex_impl<X>::match_real(iterator& it,regex_item* sq)
{

	arrStates.clear();
	stkRepeat.clear();
	stkSeqpos.clear();

	regex_state state;

	state.curp=sq;
	state.ipos=it;
	state.n_pos_repeat=0;
	state.n_pos_seqpos=0;
	
	while(state.curp)
	{
		switch(state.curp->type)
		{
		case regex_item::ITEM_CHAR_STR_NOCASE:
			{
				typedef lookup_table<lkt2lowercase> lk;

				regex_item_char_str& item(*static_cast<regex_item_char_str*>(state.curp));
				const char* p1=item.value.c_str();

				while(state.ipos !=it_end && *p1 && lk::cmap[unsigned char(*state.ipos)]==lk::cmap[unsigned char(*p1)])
				{
					state.ipos++;p1++;
				}

				if(*p1)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_STR:
			{
				regex_item_char_str& item(*static_cast<regex_item_char_str*>(state.curp));
				const char* p1=item.value.c_str();

				while(state.ipos !=it_end && *p1 && *state.ipos==*p1)
				{
					state.ipos++;p1++;
				}

				if(*p1)
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_ANY:
			{
				regex_item_char_any& item(*static_cast<regex_item_char_any*>(state.curp));
				if(state.ipos!=it_end)
				{
					++state.ipos;						
				}
				else
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_CHAR_MAP:
			{
				regex_item_char_map& item(*static_cast<regex_item_char_map*>(state.curp));
				if(item.get(*state.ipos))
				{
					++state.ipos;
				}
				else
				{
					if(!fallback(state))
					{
						return false;
					}
					continue;
				}
			}
			break;
		case regex_item::ITEM_REPEAT_NEXT:
			{
				regex_item_repeat_next& item(*static_cast<regex_item_repeat_next*>(state.curp));

				int n=stkRepeat.back().num++;
				if(n<item.node.nmin)
				{
						
				}
				else if(n==item.node.nmax||state.ipos==it_end)
				{							
					state.curp=item.node.real_sibling;
					continue;
				}
				else if(item.match_as_much_as_possible)
				{
					if(n==item.node.nmin||state.ipos!=stkRepeat.back().pos)
					{
						regex_state state2(state);
						state2.curp=item.node.real_sibling;
						state2.n_pos_repeat=state.n_pos_repeat-1;
						arrStates.push_back(state2);					
					}
					else
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}						
				}
				else
				{
					if(n==item.node.nmin||state.ipos!=stkRepeat.back().pos)
					{
						regex_state state2(state);
						state2.curp=state.curp->sibling;
						arrStates.push_back(state2);
						state.curp=item.node.real_sibling;
						continue;
					}
					else
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}				
				}

				stkRepeat.back().pos=state.ipos;
			}
			break;
		case regex_item::ITEM_REPEAT:
			{
				stkRepeat.push_back(repeat_pos_and_num(state.ipos,0));
				state.n_pos_repeat=stkRepeat.size();
			}
			break;
		case regex_item::ITEM_OR:
			{
				regex_state state2(state);
				state2.curp=static_cast<regex_item_or*>(state.curp)->child2.get();
				arrStates.push_back(state2);		
			}
			break;
		case regex_item::ITEM_LINE:
			{
				char ch=static_cast<regex_item_line*>(state.curp)->value;
				if(ch=='^')
				{
					iterator ipos(state.ipos);
					if(ipos!=it_beg&&*--ipos!='\n')
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}
				}
				else
				{
					iterator ipos(state.ipos);

					for(;ipos!=it_end && *ipos=='\r';ipos++);
							
					if(ipos!=it_end && *ipos++!='\n')
					{
						if(!fallback(state))
						{
							return false;
						}
						continue;
					}

					state.ipos=ipos;
				}
			
			}
				
				break;
		case regex_item::ITEM_SEQ:
			{
				state.n_pos_seqpos++;
				stkSeqpos.push_back(repeat_pos_and_num(state.ipos, static_cast<regex_item_seq*>(state.curp)->index));
			}
			break;
		case regex_item::ITEM_SEQ_END:
			{
				state.n_pos_seqpos++;
				stkSeqpos.push_back(repeat_pos_and_num(state.ipos,-1));
			}
			break;
		default:
			return false;				
		}

		state.curp=state.curp->sibling;
	};

	it=state.ipos;
	return true;
		
}



template<typename X>
void regex_impl<X>::update_match_results(Match& res)
{

	class item
	{
	public:

		item(){}
		item(const char* p,int n):str(p,p),num(n){}

		Match::item str;
		int num;
	};

	 arr_1t<item> q;

	for(size_t i=0;i<stkSeqpos.size();i++)
	{
		int n=stkSeqpos[i].num;
		if(n>=0)
		{
			q.push_back(item(stkSeqpos[i].pos,stkSeqpos[i].num));
		}
		else if(n<0)
		{
			q.back().str.it_end=stkSeqpos[i].pos;
			if(q.back().num>=(int)res.matchs.size())
			{
				res.matchs.resize(q.back().num+1);
			}
			res.matchs[q.back().num].push_back(q.back().str);
			q.pop_back();
		}
	}
}

template class regex_impl<const char*>;

EW_LEAVE