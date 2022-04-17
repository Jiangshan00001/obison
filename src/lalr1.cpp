#include <algorithm>
#include <sstream>
#include <iostream>
#include <set>
#include "lalr1.h"
#include "bison_file_io.h"

using namespace std;

lalr1::lalr1()
{

}

int lalr1::generate_table(bison_file_io *file_in)
{
    if(file_in==nullptr)return -1;

    //规则格式转换
    for(int i=0;i<file_in->m_rules.size();++i)
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
     
     generate_closures(file_in->m_start);
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

int lalr1::generate_terms(     const std::vector<std::string> &mtokens,
const std::vector<std::string> &mleft,
const std::vector<std::string> &mright)
{

    /// 找到所有终结符
    /// 找到所有非终结符

    std::stringstream iss;

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
                if ((rule_tk.size()==3) &&(rule_tk[0]=='\'')&&(rule_tk[2]=='\''))
                {
                    m_terms.push_back(rule_tk);
                }
        }
    }

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


    int val=256;
    for(unsigned i=0;i<m_terms.size();++i)
    {
        std::string t = m_terms[i];
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
                it->first;
                auto new_closure = get_closure( it->second);
                m_closures.push_back(new_closure);
                jmp_table[it->first] = m_closures.size()-1;
                //if(it->first)
            }
            m_jmp.push_back(jmp_table);
            m_reduce.push_back(reduce_jmp);
        }

    }while(0);







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
    ss<<"[CLOSURE]\n";
    for(unsigned i=0;i<m_closures.size();++i)
    {
        ss<<"closure-I"<<i<<":\n";
        auto &st = m_closures[i];
        for(auto it=st.begin();it!=st.end();++it)
        {
            auto curr_rule = m_rules[(unsigned)it->m_rule];
            ss<<curr_rule[0]<<"->";
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
    ss<<"[EDGE]\n";
    for(unsigned i=0;i<m_jmp.size();++i)
    {
        auto rjmp = m_jmp[i];
        for(auto it=rjmp.begin();it!=rjmp.end();++it)
        {
            ss<<"I"<<i<<"->"<<it->first<<"->I"<<it->second<<"\n";
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
        }
    }while(need_loop_next);

    return state;
}


int lalr1::get_closure_next_token(const std::set<min_state> &state,
                                                                     std::map<std::string, std::set<min_state> > &shift_jmp,
                                                                     std::map<std::string, int > &reduce_jmp)
{
    for(auto it=state.begin();it!=state.end();++it)
    {
        auto curr_rule = m_rules[it->m_rule];
        if(it->m_curr_dot_index+1>=curr_rule.size())
        {   //reduce
            reduce_jmp[it->m_next] = it->m_rule;
            continue;
        }
        else
        {
            //shift
            std::string tk = curr_rule[it->m_curr_dot_index+1];
            auto &closure = shift_jmp[tk];
            min_state nst = *it;
            nst.m_curr_dot_index++;
            closure.insert(nst);
        }
    }
    return 0;
}

std::set<min_state> lalr1::go(std::set<min_state> &state, string tk)
{
    return state;
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


