#include <assert.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <set>
#include "lalr1.h"
#include "bison_file_io.h"
#include "string_eval.h"

using namespace std;

lalr1::lalr1()
{

}

int lalr1::generate_table(bison_file_io *file_in)
{
    if(file_in==nullptr)return -1;

    {
        ///添加起始规则
        std::vector<std::string> one_rule;
        one_rule.push_back(START_SYM);
        one_rule.push_back(file_in->m_start);
        one_rule.push_back(EOF_SYM);
        this->m_rules.push_back(one_rule);
    }

    //规则格式转换
    for(unsigned i=0;i<file_in->m_rules.size();++i)
    {
        if(file_in->m_rules[i].size()==0)continue;
        std::string left = file_in->m_rules[i][0][0];
        for(unsigned j=1;j<file_in->m_rules[i].size();++j)
        {
            std::vector<std::string> one_rule;
            one_rule.push_back(left);
            one_rule.insert(one_rule.end(),file_in->m_rules[i][j].begin(),file_in->m_rules[i][j].end());
            this->m_rules.push_back(one_rule);
        }
    }

     std::cout<<print_rules();


     generate_terms(file_in->m_tokens, file_in->m_left, file_in->m_right);

     std::cout<<print_term_vals();
     
     generate_first();
     std::cout<<print_first();

     generate_follow();
     std::cout<<print_follow();
     
     generate_closures(START_SYM);
     patch_accept(START_SYM);
     std::cout<<print_closures();
     
     std::cout<<print_jmp_table();
     
     
    ///

     /// 利用符号优先级来解决冲突
     /// https://pandolia.net/tinyc/ch12_buttom_up_parse_b.html
     /// 大部分情况下， LR(1) 解析过程的 shift/reduce 冲突可以通过引入符号的优先级来解决。具体方法为：
     ///（1） 定义某些符号的优先级以及结合方式；
     ///（2） 当构造 LR(1) 的过程中出现了 shift/reduce 冲突时，即某个状态 I 中同时还有 [ A -> u.aw , c ] 和 [ B -> v. , a ] ，若已定义符号 a 的优先级，且符号串 v 中至少有一个已定义优先级的符号，则可通过以下原则确定 M[I, a] 的动作：
     ///（2.1） 找到 v 中最右边的、已定义优先级的符号（也就是 v 中离 a 最近的一个已定义优先级的符号），假设为 b ；
     ///（2.2） 若 a 的优先级 低于 b 的优先级，则： M[I, a] = reduce B -> v ；
     ///（2.3） 若 a 的优先级 高于 b 的优先级，则： M[I, a] = shift NEXT(I, a) ；
     ///（2.4） 若 a 的优先级 等于 b 的优先级，则根据 a 和 b 的结合方式：
     ///（2.4.1） 若 a 和 b 都为左结合，则 M[I, a] = shift NEXT(I, a) ；
     ///（2.4.2） 若 a 和 b 都为右结合，则 M[I, a] = reduce B -> v 。
     ///
     ///
     ///



     ///生成lalr的跳转表
     ///


     std::vector<std::vector<int> > action_table;

     std::vector<std::vector<int> > goto_table;

     /// action表纵轴是状态。1代表初始态
     /// action表横轴是 终结符
     ///
     /// goto表 纵轴是状态
     /// goto表横轴是非终结符
     ///




     return 0;


}

string lalr1::get_def_file(std::string namespace_to_add)
{
    stringstream ss;
    if(!namespace_to_add.empty())
    {
        ss<<"namespace "<<namespace_to_add<<"{\n";
    }
    ss<<"enum e_bison_head{\n";
    for(auto it=m_aterm_nval.begin();it!=m_aterm_nval.end();++it)
    {
        if (it->second[0]=='\'')
        {
            ss<<"//";
        }
        ss<<""<< it->second<<"=" << it->first<<",\n";
    }
    ss<<"};\n";

    if(!namespace_to_add.empty())
    {
        ss<<"};//namespace "<<namespace_to_add<<"\n";
    }

    return ss.str();

}

string lalr1::get_parser_file()
{
    stringstream ss;

    ss<<"std::vector< std::vector<std::string > > m_rules={\n";
    for(unsigned i=0;i<m_rules.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<m_rules[i].size();++j)
        {
            if (j!=0)ss<<",";
            ss<<"\""<<string_pack(m_rules[i][j]) << "\"";
        }
        ss<<"},\n";
    }
    ss<<"};\n";

    ss<<"std::vector< int > m_rule_reduce_ret={";
    for(unsigned i=0;i<m_rules.size();++i)
    {
        if (i!=0)ss<<",";
        for(unsigned j=0;j<m_rules[i].size();++j)
        {
            ss<<m_aterm_val[m_rules[i][0]];
            break;
        }
    }
    ss<<"};\n";



    ss<<print_closures();
    ss<<"\n";
    ss<<print_jmp_table();
    ss<<"\n";

    ///每个token对应的action表列号
    //m_aterm_nval

    //生成action/goto表
    std::vector< std::vector<int> > m_action_id;
    std::vector< std::vector<int> > m_action_type;//0--无 1--jmp 2--

    std::vector<int> aterm_val;
    for(auto it=m_aterm_nval.begin();it!=m_aterm_nval.end();++it)
    {
        aterm_val.push_back(it->first);
    }

    ss<<"std::vector<int> m_char_vec={\n";
    for(unsigned char_idx=0;char_idx<aterm_val.size();++char_idx)
    {
        ss<< aterm_val[char_idx]<<",";
    }
    ss<<"};\n";
    ss<<"std::vector<std::string> m_char_str_vec={\n";
    for(unsigned char_idx=0;char_idx<aterm_val.size();++char_idx)
    {
        ss<<"\""<< string_pack( m_aterm_nval[aterm_val[char_idx] ] )<<"\",";
    }
    ss<<"};\n";



    ss<<"std::map<int, int>  m_token_index={\n";
    for(unsigned char_idx=0;char_idx<aterm_val.size();++char_idx)
    {
        ss<<"{"<<aterm_val[char_idx]<<", " << char_idx<<"},\n";
    }
    ss<<"};\n";




    for(unsigned state_idx=0;state_idx<m_closures.size();++state_idx)
    {
        vector<int> id_vec;
        vector<int> type_vec;
        for(unsigned char_idx=0;char_idx<aterm_val.size();++char_idx)
        {
            assert(m_jmp.size()>state_idx);
            std::string jmp_term = m_aterm_nval[aterm_val[char_idx]];
            if(m_is_accpetable[state_idx])
            {
                type_vec.push_back(E_ACTION_ACCEPT);
                id_vec.push_back(-1);
            }
            else if(m_jmp[state_idx].find(jmp_term)!=m_jmp[state_idx].end())
            {
                ///有跳转
                type_vec.push_back(E_ACTION_JMP);
                id_vec.push_back(m_jmp[state_idx][jmp_term]);
            }
            else if(m_reduce[state_idx].find(jmp_term)!=m_reduce[state_idx].end())
            {
                ///有规约
                type_vec.push_back(E_ACTION_REDUCE);
                id_vec.push_back(m_reduce[state_idx][jmp_term]);
            }
            else
            {
                type_vec.push_back(E_ACTION_NULL);
                id_vec.push_back(-1);
            }


        }
        m_action_id.push_back(id_vec);
        m_action_type.push_back(type_vec);
    }

    ss << "int m_action_id["<< m_closures.size()<< "][" << m_aterm_nval.size()<<  "]={\n";
    ///每个状态的跳转表
    ///
    for(unsigned i=0;i<m_action_id.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<m_action_id[i].size();++j)
        {
            if(j>0)ss<<",";
            ss<<m_action_id[i][j]<<" ";
        }
        ss<<"},\n";
    }
    ss<<"};\n";

    ss << "int m_action_type["<< m_closures.size()<< "][" << m_aterm_nval.size()<<  "]={\n";
    ///每个状态的跳转表
    ///
    for(unsigned i=0;i<m_action_type.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<m_action_type[i].size();++j)
        {
            if(j>0)ss<<",";
            ss<<m_action_type[i][j]<<" ";
        }
        ss<<"},\n";
    }
    ss<<"};\n";


    return ss.str();
}

int lalr1::generate_terms(     const std::vector<std::string> &mtokens,
const std::vector<std::string> &mleft,
const std::vector<std::string> &mright)
{

    /// 找到所有终结符
    /// 找到所有非终结符

    std::stringstream iss;

    ///文件结束终结符。自动添加
    ///
    m_terms.push_back(EOF_SYM);

    /// add terms first
    ///
    for(unsigned i=0;i<mtokens.size();++i)
    {
        m_terms.push_back(mtokens[i]);
    }
    for(unsigned i=0;i<mleft.size();++i)
    {
        m_terms.push_back(mleft[i]);
    }
    for(unsigned i=0;i<mright.size();++i)
    {
        m_terms.push_back(mright[i]);
    }
    for(unsigned i=0;i<m_rules.size();++i)
    {
        for(unsigned j=0;j<m_rules[i].size();j++)
        {
                std::string rule_tk = m_rules[i][j];
                if (((rule_tk.size()==3) &&(rule_tk[0]=='\'')&&(rule_tk[2]=='\''))||
                        ((rule_tk.size()==4) &&(rule_tk[0]=='\'')&&(rule_tk[3]=='\'')&&(rule_tk[1]=='\\'))
                        )
                {
                    m_terms.push_back(rule_tk);
                }
        }
    }

    ///others that are not term are nterms
    ///
    ///
    std::set<std::string> nterms_set;
    for(unsigned i=0;i<m_rules.size();++i)
    {
        for(unsigned j=0;j<m_rules[i].size();j++)
        {
                std::string rule_tk = m_rules[i][j];
                if (std::find(m_terms.begin(), m_terms.end(),rule_tk)==m_terms.end())
                {
                    nterms_set.insert(rule_tk);
                }
        }
    }
    m_nterms.assign(nterms_set.begin(), nterms_set.end());

    /// add value of the terms/nterms
    int val=256;
    for(unsigned i=0;i<m_terms.size();++i)
    {
        std::string t = m_terms[i];
        int v = 0;
        if((t.size()==3)&&(t[0]=='\'')&&(t[2]=='\''))
        {
            v = t[1];
        }
        else if((t.size()==4)&&(t[0]=='\'')&&(t[3]=='\'')&&(t[1]=='\\')&&(t[2]=='n'))
        {
            v = '\n';///FIXME: 此处只处理了\n. 还有其他字符未处理
        }
        else if((t.size()==4)&&(t[0]=='\'')&&(t[3]=='\'')&&(t[1]=='\\')&&(t[2]=='t'))
        {
            v = '\t';///FIXME: 此处只处理了\n. 还有其他字符未处理
        }
        else if((t.size()==4)&&(t[0]=='\'')&&(t[3]=='\'')&&(t[1]=='\\')&&(t[2]=='v'))
        {
            v = '\v';///FIXME: 此处只处理了\n. 还有其他字符未处理
        }
        else
        {
            v = ++val;
        }

        m_aterm_val[t]=v;
        m_aterm_nval[v]=t;
    }
    val =val/100 * 100 + 100;
    for(unsigned i=0;i<m_nterms.size();++i)
    {
        std::string t = m_nterms[i];
        int v = 0;
        if((t.size()==3)&&(t[0]=='\'')&&(t[2]=='\''))
        {
            v = t[1];
        }
        else
        {
            v = ++val;
        }

        m_aterm_val[t]=v;
        m_aterm_nval[v]=t;
    }


    return 0;
}

int lalr1::generate_first()
{
    ///生成非终结符的first集
    ///

    int last_cnt=0;
    int curr_cnt=0;
    do{
        last_cnt=curr_cnt;
        curr_cnt=0;
        for(unsigned i=0;i<m_rules.size();++i)
        {
            std::string left = m_rules[i][0];
            
            if (m_rules[i].size() < 2) {
                /// 空规则。first集如何处理？？？FIXME:
                continue;
            }

            std::string right_first = m_rules[i][1];
            std::set<std::string> &fset = m_first[left];
            if(is_nterm(right_first))
            {//this is a nterm
                fset.insert(m_first[right_first].begin(),m_first[right_first].end());
            }
            else
            {
                fset.insert(right_first);
            }
            curr_cnt += fset.size();
        }
    }while(curr_cnt>last_cnt);

    return 0;
}

int lalr1::generate_follow()
{

    int last_cnt=0;
    int curr_cnt=0;
    do{
        last_cnt=curr_cnt;
        curr_cnt=0;
        for(unsigned i=0;i<m_rules.size();++i)
        {
            if(m_rules[i].size()<3)continue;

            std::string left = m_rules[i][0];
            for(unsigned j=1;j<m_rules[i].size()-1;++j)
            {
                std::string right_first = m_rules[i][j];
                std::string right_follow = m_rules[i][j+1];
                if(is_term(right_first))
                {//right_first是终结符，不管
                    continue;
                }


                std::set<std::string> &fset = m_follow[right_first];

                if(is_nterm(right_follow))
                {//this is a nterm
                    fset.insert(m_first[right_follow].begin(),m_first[right_follow].end());
                }
                else
                {
                    fset.insert(right_follow);
                }
                curr_cnt += fset.size();
            }
        }
    }while(curr_cnt>last_cnt);

    return 0;
}

int lalr1::generate_closures(std::string mstart)
{
    set<min_state> first_state;

    for(unsigned i=0;i<m_rules.size();++i)
    {
        if(m_rules[i][0]!=mstart)continue;

        min_state curr;
        curr.m_rule = i;
        curr.m_curr_dot_index=0;
        curr.m_next="";
        first_state.insert(curr);
    }
    first_state = get_closure(first_state);
    m_closures.push_back(first_state);

    do{
        for(unsigned i=0;i<m_closures.size();++i)
        {
            auto &onec = m_closures[i];
            ///如果已经处理过，则jmptable中已经有，则不再处理
            if(m_jmp.size()>i)continue;
            std::map<std::string, int > jmp_table;

            std::map<std::string, std::set<min_state> > shift_jmp;
            std::map<std::string, int > reduce_jmp;
            int ret = get_closure_next_token(onec,shift_jmp, reduce_jmp);
            for(auto it=shift_jmp.begin();it!=shift_jmp.end();++it)
            {
                //it->first;
                auto new_closure = get_closure( it->second);
                int clo_index = 0;
                auto clo_it = std::find(m_closures.begin(), m_closures.end(), new_closure);
                if (clo_it == m_closures.end())
                {
                    m_closures.push_back(new_closure);
                    clo_index = m_closures.size() - 1;
                }
                else
                {
                    clo_index = clo_it - m_closures.begin();
                }
                
                jmp_table[it->first] = clo_index;
            }
            m_jmp.push_back(jmp_table);
            m_reduce.push_back(reduce_jmp);
        }

    }while(0);







    return 0;
}

int lalr1::patch_accept(string mstart)
{
    m_is_accpetable.resize(m_closures.size());
    for(unsigned i=0;i<m_closures.size();++i)
    {
        m_is_accpetable[i]=0;
        auto curr_set = m_closures[i];
        for(auto it = curr_set.begin();it!=curr_set.end();++it)
        {
            auto statei = *it;
            if(m_rules[statei.m_rule].size()==0)continue;
            if(m_rules[statei.m_rule][0]!=mstart)continue;
            if(statei.m_curr_dot_index+1!=m_rules[statei.m_rule].size())continue;
            ///当前规则，所有都是可接受
            m_is_accpetable[i]=1;


        }
    }
    return 0;
}

std::string lalr1::print_term_vals()
{
    std::stringstream ss;
    ss<<"[terms]\n";
    for(auto it=m_aterm_nval.begin();it!=m_aterm_nval.end();++it)
    {
        ss<<"["<< it->second <<"]"<<"->"<<it->first<<"\n";
    }
    return ss.str();
}

std::string lalr1::print_first()
{
    stringstream ss;
    ss<<"[FIRST]\n";
    for(auto it =m_first.begin();it!=m_first.end();++it)
    {
        ss << "FIRST(" << it->first.c_str() << ")={";
        set<std::string> & temp = it->second;
        auto it1 = temp.begin();
        bool flag = false;
        for (; it1 != temp.end() ; it1++)
        {
            if (flag)
                ss << ",";
            ss << *it1;
            flag = true;
        }
        ss << "}" << endl;
    }



    return ss.str();
}

string lalr1::print_follow()
{
    stringstream ss;
    ss<<"[FOLLOW]\n";
    for(auto it =m_follow.begin();it!=m_follow.end();++it)
    {
        ss << "FOLLOW(" << it->first.c_str() << ")={";
        set<std::string> & temp = it->second;
        auto it1 = temp.begin();
        bool flag = false;
        for (; it1 != temp.end() ; it1++)
        {
            if (flag)
                ss << ",";
            ss << *it1;
            flag = true;
        }
        ss << "}" << endl;
    }



    return ss.str();
}

string lalr1::print_closures()
{
    stringstream ss;
    ss<<"//[CLOSURE]\n";
    for(unsigned i=0;i<m_closures.size();++i)
    {
        ss<<"//closure-I"<<i<<":\n";
        auto &st = m_closures[i];
        for(auto it=st.begin();it!=st.end();++it)
        {
            auto curr_rule = m_rules[(unsigned)it->m_rule];
            ss<<"//"<<curr_rule[0]<<"->";
            for(unsigned j=1;j<curr_rule.size();++j)
            {
                if(it->m_curr_dot_index+1==j)
                {
                    ss<<".";
                }
                ss<<curr_rule[j]<<" ";
            }
            ss<<", "<< it->m_next<<"\n";
        }
    }
    return ss.str();
}

string lalr1::print_jmp_table()
{
    stringstream ss;
    ss<<"//[EDGE]\n";
    for(unsigned i=0;i<m_jmp.size();++i)
    {
        auto rjmp = m_jmp[i];
        for(auto it=rjmp.begin();it!=rjmp.end();++it)
        {
            ss<<"//I"<<i<<"->"<<it->first<<"->I"<<it->second<<"\n";
        }
    }
    ss<<"\n";
    return ss.str();
}

bool lalr1::is_term(string mm)
{
    if(std::find(m_terms.begin(), m_terms.end(), mm)==m_terms.end())
    {
        return false;
    }
    return true;
}

bool lalr1::is_nterm(string mm)
{
    if(std::find(m_nterms.begin(), m_nterms.end(), mm)==m_nterms.end())
    {
        return false;
    }
    return true;

}

bool lalr1::is_aterm(string mm)
{
    if(m_aterm_val.find(mm)!=m_aterm_val.end())
    {
        return true;
    }
    return false;

}

std::set<min_state> lalr1::get_closure(std::set<min_state> state)
{
    int need_loop_next=0;
    do{
        need_loop_next=0;
        for(auto it=state.begin();it!=state.end();++it)
        {
            min_state st = (*it);
            const auto &curr_rule = m_rules[ st.m_rule];
            if((st.m_curr_dot_index+1)>=curr_rule.size())
                continue;

            ///找到下一个 token
            std::string next_tk = curr_rule[st.m_curr_dot_index+1];
            if(is_term(next_tk))continue;
            ///下一个token是非终结符，找到下一个token的下一个token
            /// 如果下一个已经结束，则下一个token是本次的next。否则就是nn
            std::string nn_tk;
            if(st.m_curr_dot_index+1==curr_rule.size()-1)
            {
                nn_tk=st.m_next;
            }
            else
            {
                nn_tk=curr_rule[st.m_curr_dot_index+2];
            }

            ///找到下一个token对应的规则
            for(unsigned i=0;i<this->m_rules.size();++i)
            {
                if(this->m_rules[i][0]!=next_tk)
                    continue;

                min_state stnew;
                stnew.m_rule = (int)i;
                stnew.m_curr_dot_index = 0;
                stnew.m_next = nn_tk;
                if(state.find(stnew)==state.end())
                {
                    state.insert(stnew);
                    need_loop_next=1;
                }
            }
            //如果state被添加项，则it已经不能使用，需要重新循环
            if (need_loop_next)break;
        }
    }while(need_loop_next);

    return state;
}

std::set<min_state> lalr1::go(std::set<min_state> &state, string tk)
{
    return state;
}

int lalr1::get_closure_next_token(const std::set<min_state> &state,
                       std::map<std::string, std::set<min_state> > &shift_jmp,
                       std::map<std::string, int > &reduce_jmp)
{
    shift_jmp.clear();
    reduce_jmp.clear();
    for(auto it=state.begin();it!=state.end();++it)
    {
        auto curr_rule = m_rules[it->m_rule];
        if(it->m_curr_dot_index+1>=curr_rule.size())
        {   //reduce
            reduce_jmp[it->m_next] = it->m_rule;
            ///如果需要规约，则规则对应的所有first集，都需要规约
            auto reduce_to_chars = m_first[it->m_next];
            for(auto it2=reduce_to_chars.begin();it2!=reduce_to_chars.end();++it2)
            {
                reduce_jmp[*it2] = it->m_rule;
            }
            continue;
        }
        else
        {
            //shift
            std::string tk = curr_rule[it->m_curr_dot_index+1];
            ///shift如果需要，则tk对应的first集，都要加入
            ///
            auto shift_to_chars = m_first[tk];

            auto &closure = shift_jmp[tk];
            min_state nst = *it;
            nst.m_curr_dot_index++;
            closure.insert(nst);

            //for(auto it2=shift_to_chars.begin();it2!=shift_to_chars.end();++it2)
            //{
            //    auto &closure = shift_jmp[*it2];
            //    min_state nst = *it;
            //    nst.m_curr_dot_index++;
            //    closure.insert(nst);
            //}


        }
    }
    return 0;
}


std::string lalr1::print_rules()
{
    std::stringstream ss;
    ss<<"[rules]\n";
    for(unsigned i=0;i<this->m_rules.size();++i)
    {
        for(unsigned j=0;j<this->m_rules[i].size();++j)
        {
            if(j==0)
            {
            ss<<this->m_rules[i][j]<<"->";
            }
            else
            {
            ss<<this->m_rules[i][j]<<" ";
            }
        }
        ss<<"\n";
    }

    return ss.str();
}


