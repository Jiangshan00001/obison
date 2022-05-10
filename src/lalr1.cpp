///http://jsmachines.sourceforge.net/machines/lr1.html
///

#include <assert.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <set>
#include "lalr1.h"
#include "string_eval.h"
#include "obisonfile.h"
#include "replace.h"
using namespace std;

lalr1::lalr1()
{

}

int lalr1::generate_table(OBisonFile *file_in)
{
    {
        ///添加起始规则
        std::vector<std::string> one_rule;
        std::vector<int> one_rule_is_action;
        one_rule.push_back(START_SYM);
        one_rule.push_back(file_in->m_start);
        one_rule.push_back(EOF_SYM);
        for(int i=0;i<3;++i)one_rule_is_action.push_back(0);
        this->m_rules.push_back(one_rule);
        this->m_is_action.push_back(one_rule_is_action);
    }
    //规则格式转换
    this->m_rules.insert(this->m_rules.end(), file_in->m_rules.begin(), file_in->m_rules.end());
    this->m_is_action.insert(this->m_is_action.end(), file_in->m_is_action.begin(), file_in->m_is_action.end());

    ///对于空字符，添加符号表示
    rules_eps_patch();
    remove_action_from_rules();

    calc_skip_token();

    this->m_terms = file_in->m_terms;

    ///文件结束终结符。自动添加
    ///
    m_terms.push_back(EOF_SYM);
    m_terms.push_back(EPS_SYM);
    m_terms.push_back(NULL_TOKEN);



    add_squote_to_terms();

    ///others that are not term are nterms
    ///
    ///
    add_nterms();

    /// add value of the terms/nterms
    add_number_to_n_terms();



    generate_first(m_first, 0);
    generate_first(m_first_skip_eps, 1);

    generate_follow(m_follow, m_first, 0);
    generate_follow(m_follow_skip_eps, m_first_skip_eps, 1);

    generate_closures(START_SYM);
    calc_accept(START_SYM);

    calc_action_table();
    calc_middle_action_code();

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


    return 0;
}


int lalr1::remove_action_from_rules()
{

    std::vector<std::vector<int> > new_is_action;
    std::vector<std::vector<std::string> > new_rules;

    m_actions.push_back("{}");//action0 is nothing action


    ///对于action，从rules中移出到m_actions中
    /// 此处定义3个规则的含义：topdown之前，之后 和 bottom-up时
    for(unsigned i=0;i<m_is_action.size();++i)
    {
        std::vector<std::string> new_one_rule;
        std::vector<int> new_one_rule_is_act;
        std::vector<int> connected_act;
        for(unsigned j=0;j<m_is_action[i].size();++j)
        {
            if(!m_is_action[i][j])
            {
                new_one_rule.push_back(m_rules[i][j]);
                new_one_rule_is_act.push_back(0);
                connected_act.clear();
            }
            else
            {
                if(!new_one_rule_is_act[new_one_rule_is_act.size()-1])
                {
                    connected_act.clear();
                }

                m_actions.push_back(m_rules[i][j]);
                int action_index = m_actions.size()-1;
                new_one_rule_is_act[new_one_rule_is_act.size()-1]=action_index;
                connected_act.push_back(action_index);
            }
        }

        if(new_one_rule_is_act[new_one_rule_is_act.size()-1])
        {
            new_one_rule_is_act[new_one_rule_is_act.size()-1]=0;//最后的action是不在middle处处理，所以去掉标记
        }

        new_rules.push_back(new_one_rule);
        new_is_action.push_back(new_one_rule_is_act);

        m_before_action[i] = 0;
        m_after_action[i] = 0;
        m_comp_action[i] = 0;
        if(connected_act.size()>0)
        {
            m_before_action[i] = connected_act[0];
        }

        if(connected_act.size()>1)
        {
            m_before_action[i] = connected_act[0];
            m_after_action[i] = connected_act[1];
        }

        if(connected_act.size()>2)
        {
            m_before_action[i] = connected_act[0];
            m_after_action[i] = connected_act[1];
            m_comp_action[i] = connected_act[2];
        }
    }
    this->m_rules = new_rules;
    this->m_is_action = new_is_action;


    return 0;
}


const std::map<int, string> &lalr1::get_def_file()
{
    return this->m_aterm_nval;
}

int lalr1::calc_action_table()

{


    ///生成lalr的跳转表
    ///
    /// action表纵轴是状态。1代表初始态
    /// action表横轴是 终结符
    ///
    /// goto表 纵轴是状态
    /// goto表横轴是非终结符
    ///


    for(auto it=m_aterm_nval.begin();it!=m_aterm_nval.end();++it)
    {
        m_action_table_x_int.push_back(it->first);
        m_action_table_x_str.push_back(it->second);
    }

    for(unsigned state_idx=0;state_idx<m_closures.size();++state_idx)
    {
        ///此处需要处理shift和reduce的优先级规则
        ///
        ///
        ///
        vector<int> id_vec;
        vector<int> type_vec;
        for(unsigned char_idx=0;char_idx<m_action_table_x_int.size();++char_idx)
        {
            assert(m_jmp.size()>state_idx);
            std::string jmp_term = m_aterm_nval[m_action_table_x_int[char_idx]];
            ///此处需要有优先级规则等


            if(m_is_accpetable[state_idx])
            {
                type_vec.push_back(E_ACTION_ACCEPT);
                id_vec.push_back(-1);
            }
            else if(m_jmp[state_idx].find(jmp_term)!=m_jmp[state_idx].end())
            {
                ///有跳转
                type_vec.push_back(E_ACTION_SHIFT);
                id_vec.push_back(m_jmp[state_idx][jmp_term]);
            }
            else if(m_reduce[state_idx].find(jmp_term)!=m_reduce[state_idx].end())
            {
                ///有规约--只取第一个规则？？？
                type_vec.push_back(E_ACTION_REDUCE);
                if(m_reduce[state_idx][jmp_term].size()==1)
                    id_vec.push_back(m_reduce[state_idx][jmp_term][0]);
                else{

                    std::cerr<<"reduce state error:  state_idx="<< state_idx<<". term:"<< jmp_term<<"\n";
                    std::cerr<<"reduce size:"<< m_reduce[state_idx][jmp_term].size()<<"\n";
                    id_vec.push_back(m_reduce[state_idx][jmp_term][0]);
                }
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
    return 0;


#if 0
    stringstream ss;

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


    return ss.str();

#endif

}

int lalr1::calc_middle_action_code()
{
    ///std::map<int, std::string > m_middle_action;

    stringstream ss;

    ///每个rule1中，中间action的位置
    for(unsigned i=0;i<m_is_action.size();++i)
    {
        for(unsigned j=0;j<m_is_action[i].size();++j)
        {
            if(m_is_action[i][j])
            {
                int action_id = m_is_action[i][j];
                m_middle_action_len[action_id] = j;
            }
        }
    }


    ///每个state:middle action
    std::map<int, int> state_action;
    for(unsigned i=0;i<m_closures.size();++i)
    {
        std::set<min_state> &states = m_closures[i].m_states;
        for(auto it=states.begin();it!=states.end();++it)
        {
            const min_state & st = (*it);
            if(m_is_action[st.m_rule][st.m_curr_dot_index])
            {
                if(state_action.find(i)==state_action.end())
                {
                    state_action[i] = m_is_action[st.m_rule][st.m_curr_dot_index];
                }
                else
                {
                    if(state_action[i] != m_is_action[st.m_rule][st.m_curr_dot_index])
                    {
                        std::cerr<<"get_action_code: two actions in one state\n";
                        std::cerr<<"closures:"<<i<<". action index:"<<state_action[i]<<" and "<<m_is_action[st.m_rule][st.m_curr_dot_index]<<"\n";
                    }
                }
            }

        }
    }

    for(auto it=state_action.begin();it!=state_action.end();++it)
    {
        auto &state_list = m_middle_action_state[it->second];
        state_list.insert(it->first);
    }


    return 0;
}
#if 0
string lalr1::get_action_code()
{
    stringstream ss;

    std::map<int, int> action_len;
    for(unsigned i=0;i<m_is_action.size();++i)
    {
        for(unsigned j=0;j<m_is_action[i].size();++j)
        {
            if(m_is_action[i][j])
            {
                int action_id = m_is_action[i][j];
                action_len[action_id] = j;
            }
        }
    }



    std::map<int, int> state_action;
    for(unsigned i=0;i<m_closures.size();++i)
    {
        std::set<min_state> &states = m_closures[i];
        for(auto it=states.begin();it!=states.end();++it)
        {
            const min_state & st = (*it);
            if(m_is_action[st.m_rule][st.m_curr_dot_index])
            {
                if(state_action.find(i)==state_action.end())
                {
                    state_action[i] = m_is_action[st.m_rule][st.m_curr_dot_index];
                }
                else
                {
                    if(state_action[i] != m_is_action[st.m_rule][st.m_curr_dot_index])
                    {
                        std::cerr<<"get_action_code: two actions in one state\n";
                        std::cerr<<"closures:"<<i<<". action index:"<<state_action[i]<<" and "<<m_is_action[st.m_rule][st.m_curr_dot_index]<<"\n";
                    }
                }
            }

        }
    }


    std::map<int, std::set<int> > action_state;
    for(auto it=state_action.begin();it!=state_action.end();++it)
    {
        auto &state_list = action_state[it->second];
        state_list.insert(it->first);
    }



    ss<<"\nobison_token_type action_in_middle(int state_id, const std::deque<obison_token_type> token_stack)\n{\n";
    ss<<"obison_token_type _tk;\n";
    ss<< "_tk.m_ret="<<NULL_TOKEN<<";\n";

    ss<<"switch(state_id)\n{\n";//start of switch

    for(auto it=action_state.begin();it!=action_state.end();++it)
    {
        auto &state_list = it->second;
        for(auto it2=state_list.begin();it2!=state_list.end();++it2)
        {
            ss<<"case "<< *it2<<":\n";
        }
        ss<<"\n{\n";
        ss<<"std::vector<obison_token_type> v;\n";
        ss<<"v.assign(token_stack.end()-"<<action_len[it->first] <<", token_stack.end());\n";

        std::string action_str = m_actions[it->first];
        ///action replace
        replace(action_str, "$1", "v[0]");
        replace(action_str, "$2", "v[1]");
        replace(action_str, "$3", "v[2]");
        replace(action_str, "$4", "v[3]");
        replace(action_str, "$5", "v[4]");
        replace(action_str, "$6", "v[5]");
        replace(action_str, "$7", "v[6]");
        replace(action_str, "$$", "_tk");

        replace(action_str, "yychar", "_tk.m_ret");






        ss<< action_str;
        ss<<"\n}\n";
        ss<<"break;\n";
    }
    ss<<"default:\n";
    ss<< "_tk.m_ret="<<NULL_TOKEN<<";\n";
    ss<<"break;\n";


    ss<<"\n}\n";//end of switch
    ss<<"return _tk;\n";
    ss<<"\n}\n";

    return ss.str();
}
#endif

void lalr1::add_number_to_n_terms()
{
    this->m_aterm_val.clear();
    this->m_aterm_nval.clear();

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
}

void lalr1::add_nterms()
{
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
}

void lalr1::add_squote_to_terms()
{
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

    add_squote_to_terms();

    ///others that are not term are nterms
    ///
    ///
    add_nterms();

    /// add value of the terms/nterms
    add_number_to_n_terms();


    return 0;
}

int lalr1::generate_first(std::map<std::string, std::set<std::string> > &mfirst, int is_skip_eps)
{

    for(unsigned i=0;i<m_terms.size();++i)
    {
        std::set<std::string> s;
        s.insert(m_terms[i]);
        mfirst[m_terms[i]] =s;
    }


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
            //if((m_rules[i].size()==2)&&(m_rules[i][1]==EPS_SYM))
            //{
            //    ///空规则，去除?
            //    continue;
            //}

            std::string right_first = m_rules[i][1];

            std::set<std::string> &fset = mfirst[left];
            if(is_nterm(right_first))
            {//this is a nterm
                fset.insert(mfirst[right_first].begin(),mfirst[right_first].end());
            }
            else
            {
                fset.insert(right_first);
            }
            if(is_skip_eps)
            {
                if(can_be_skipped(right_first)&& (m_rules[i].size()>2))
                {
                    right_first = m_rules[i][2];
                    if(is_nterm(right_first))
                    {//this is a nterm
                        fset.insert(mfirst[right_first].begin(),mfirst[right_first].end());
                    }
                    else
                    {
                        fset.insert(right_first);
                    }
                }
            }


            curr_cnt += fset.size();
        }
    }while(curr_cnt>last_cnt);

    return 0;
}

int lalr1::generate_follow(std::map<std::string, std::set<std::string> > &mfollow, std::map<std::string, std::set<std::string> > &mfirst, int is_skip_eps)
{

    mfollow.clear();

    int last_cnt=0;
    int curr_cnt=0;
    do{

        last_cnt=curr_cnt;
        curr_cnt=0;
        for(unsigned i=0;i<m_rules.size();++i)
        {
            if(m_rules[i].size()<2)continue;

            ///https://wenku.baidu.com/view/6afe05906bec0975f465e2c7.html
            /// 1 UP fist(P) 放入follow(U)
            /// 2 U->...P 的产生式，把Follow(U)的全部内容传送到Follow(P)中
            ///
            ///
            std::string left = m_rules[i][0];

            std::string most_right = m_rules[i][m_rules[i].size()-1];
            if(is_nterm(most_right))
            {
                std::set<std::string> &fset = mfollow[most_right];
                fset.insert(mfollow[left].begin(),mfollow[left].end());
            }


            for(unsigned j=1;j<m_rules[i].size()-1;++j)
            {
                std::string right_first = m_rules[i][j];
                std::string right_follow = m_rules[i][j+1];
                if(is_term(right_first))
                {//right_first是终结符，不管
                    continue;
                }

                std::set<std::string> &fset = mfollow[right_first];

                if(is_nterm(right_follow))
                {//this is a nterm
                    fset.insert(mfirst[right_follow].begin(),mfirst[right_follow].end());
                }
                else
                {
                    fset.insert(right_follow);
                }

                if(is_skip_eps)
                {
                    unsigned kk = j+2;

                    while((kk<m_rules[i].size()))
                    {
                        if (!can_be_skipped(right_follow))break;
                        right_follow = m_rules[i][kk];
                        if(is_nterm(right_follow))
                        {//this is a nterm
                            fset.insert(mfirst[right_follow].begin(),mfirst[right_follow].end());
                        }
                        else
                        {
                            fset.insert(right_follow);
                        }
                        kk++;
                    }
                }


                curr_cnt += fset.size();
            }
        }
    }while(curr_cnt>last_cnt);

    return 0;
}

int lalr1::generate_closures(std::string mstart)
{
    Closure first_state;

    for(unsigned i=0;i<m_rules.size();++i)
    {
        if(m_rules[i][0]!=mstart)continue;

        min_state curr;
        curr.m_rule = i;
        curr.m_curr_dot_index=0;
        curr.m_next="";
        first_state.m_kernel.insert(curr);
    }
    first_state = get_closure(first_state);
    m_closures.push_back(first_state);


    for(unsigned i=0;i<m_closures.size();++i)
    {
        auto &onec = m_closures[i];
        ///如果已经处理过，则jmptable中已经有，则不再处理
        if(m_jmp.size()>i)continue;
        std::map<std::string, int > jmp_table;

        std::map<std::string, Closure > shift_jmp;
        std::map<std::string, std::vector<int> > reduce_jmp;
        int ret = get_closure_next_token(onec,shift_jmp, reduce_jmp);
        ///shift 表，可能有不存在的closure,所以需要创建
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


    return 0;
}

int lalr1::calc_accept(string mstart)
{
    m_is_accpetable.resize(m_closures.size());
    for(unsigned i=0;i<m_closures.size();++i)
    {
        m_is_accpetable[i]=0;
        auto curr_set = m_closures[i].m_states;
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

int lalr1::rules_eps_patch()
{
    ///此处将只有left规则的，在right添加EPS_SYM
    ///
    for(unsigned i=0;i<m_rules.size();++i)
    {
        if(m_rules[i].size()==1)
        {
            //只有左符号，无右边的符号
            m_rules[i].push_back(EPS_SYM);
            m_is_action[i].push_back(0);
        }
        else if((m_rules[i].size()==2)&&(m_is_action[i][1]))
        {
            //只有左符号，右边有一个action
            m_rules[i].insert(m_rules[i].begin()+1, EPS_SYM);
            m_is_action[i].insert(m_is_action[i].begin()+1, 0);
        }
    }


    return 0;
}

int lalr1::calc_skip_token()
{


    int eps_cnt=0;
    m_eps_tokens.clear();
    m_eps_tokens.push_back(EPS_SYM);
    while(eps_cnt<m_eps_tokens.size())
    {
        eps_cnt=m_eps_tokens.size();
        for(unsigned i=0;i<m_rules.size();++i)
        {
            if(std::find(m_eps_tokens.begin(), m_eps_tokens.end(), m_rules[i][0])!=m_eps_tokens.end())
            {
                //已经添加过了，直接下一个
                continue;
            }

            int can_skip=1;
            for(int j=1;j<m_rules[i].size();++j)
            {
                if(std::find(m_eps_tokens.begin(), m_eps_tokens.end(), m_rules[i][j])==m_eps_tokens.end())
                {
                    can_skip=0;
                    break;
                }
            }
            if(can_skip)
            {
                    m_eps_tokens.push_back(m_rules[i][0]);
                    m_eps_token_rule[m_rules[i][0] ] = i;
            }
        }
    }

    if(this->m_is_debug)
    {
        std::cout<<"eps_tokens:\n";
        for(unsigned i=0;i<m_eps_tokens.size();++i)
        {
            std::cout<<m_eps_tokens[i]<<", ";
        }
        std::cout<<"\n";
    }

    return 0;
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

int lalr1::add_one_production_to(std::string next_tk, std::string nn_tk, std::set<min_state> &state)
{
    ///添加规则到state，返回值是添加个数
    /// 如果已经存在，则不再添加，返回0
    /// 如果不存在，则添加，返回1

    int need_loop_next = 0;
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
    return need_loop_next;
}

std::string lalr1::get_nn_tk(int rule_index, int dot_index, std::string curr_nn)
{
    std::string nn_tk;
    auto curr_rule = m_rules[rule_index];

    if(dot_index+1==curr_rule.size()-1)
    {
        nn_tk=curr_nn;
    }
    else
    {
        nn_tk=curr_rule[dot_index+2];
    }
    return nn_tk;
}

int lalr1::can_be_skipped(string tk)
{
    if(std::find(m_eps_tokens.begin(), m_eps_tokens.end(), tk)!=m_eps_tokens.end())
    {
        return 1;
    }
    return 0;

    if(tk==EPS_SYM)
    {
        return 1;
    }
    for(unsigned i=0;i<m_rules.size();++i)
    {
        if(m_rules[i].size()!=2 )continue;
        if(m_rules[i][0]!=tk)continue;
        if(m_rules[i][1]!=EPS_SYM)continue;

        return 1;
    }
    return 0;

}

Closure lalr1::get_closure(Closure& cstate)
{
    ///从一个状态开始，推出所有的当前闭包中的production
    /// 因为state每次添加新项后，可能引起it的失效，所以每次插入一个，都需要break，重新遍历
    ///
    ///
    ///eg:
    /// S->A $END
    /// A->A+B
    /// A->a
    /// B->b
    ///
    /// 一开始，只有 S->.A $END
    /// 发现.后边是A，则添加
    /// A->.A+B $END
    /// A->.a $END
    /// 添加后，又发现.后边是A，则添加
    /// A->.A+B +
    /// A->.a +
    ///
    /// 完成I0
    ///
    ///
    ///
    ///
    ///
    ///

    auto &state = cstate.m_states;
    cstate.m_states.insert(cstate.m_kernel.begin(),cstate.m_kernel.end());

    int need_loop_next=0;
    do{
        need_loop_next=0;

        for(auto it=state.begin();it!=state.end();++it)
        {
            min_state st = (*it);
            const auto &curr_rule = m_rules[ st.m_rule];
            if((st.m_curr_dot_index+1)>=curr_rule.size())
            {
                ///此处已经是状态结束，需要规约的状态，无法再找到其他状态
                continue;
            }

            ///找到下一个 token
            std::string next_tk = curr_rule[st.m_curr_dot_index+1];

//            if(can_be_skipped(next_tk))
//            {
//                ///this next_tk is eps, can be skipped
//                /// just add next step
//                int dot_index2 = st.m_curr_dot_index+1;
//                if(dot_index2+1<curr_rule.size())
//                {
//                    min_state stnew;
//                    stnew.m_rule = (int)st.m_rule;
//                    stnew.m_curr_dot_index = dot_index2;
//                    stnew.m_next = st.m_next;
//                    if(state.find(stnew)==state.end())
//                    {
//                        state.insert(stnew);
//                        need_loop_next=1;
//                    }
//                    if (need_loop_next)break;
//                }
//            }


            if(is_term(next_tk))continue;

            ///下一个token是非终结符，找到下一个token的下一个token
            /// 如果下一个已经结束，则下一个token是本次的next。否则就是nn
            std::string nn_tk = get_nn_tk(st.m_rule, st.m_curr_dot_index, st.m_next);

            ///找到下一个token对应的规则
            need_loop_next = add_one_production_to(next_tk, nn_tk, state);
            //如果state被添加项，则it已经不能使用，需要重新循环
            if (need_loop_next)break;
        }
    }while(need_loop_next);

    return cstate;
}

std::set<min_state> lalr1::go(std::set<min_state> &state, string tk)
{
    return state;
}

int lalr1::find_eps_rule(string rule_left)
{
    for(unsigned i=0;i<m_rules.size();++i)
    {
        if(m_rules[i].size()!=2)continue;
        if(m_rules[i][0]!=rule_left)continue;
        if(m_rules[i][1]!=EPS_SYM)continue;
        return i;
    }
    return -1;
}

int lalr1::get_closure_next_token(const Closure &cstate,
                       std::map<std::string, Closure > &shift_jmp,
                       std::map<std::string, std::vector<int> > &reduce_jmp)
{
    shift_jmp.clear();
    reduce_jmp.clear();

    auto state = cstate.m_states;


    for(auto it=state.begin();it!=state.end();++it)
    {
        auto curr_rule = m_rules[it->m_rule];
        if((curr_rule.size()==2)&&(curr_rule[1]==EPS_SYM) || (it->m_curr_dot_index+1>=curr_rule.size()))
        {   //reduce
            std::vector<int> &rlist = reduce_jmp[it->m_next];
            rlist.push_back( it->m_rule);
            ///如果需要规约，则规则对应的所有first集，都需要规约
            auto reduce_to_chars = m_first_skip_eps[it->m_next];
            reduce_to_chars.insert(it->m_next);


            reduce_to_chars.erase(EPS_SYM);
            ///如果first集中有eps，则合并入follow集
            int red_cnt = 0;
            do
            {
                red_cnt = reduce_to_chars.size();
                for(auto redi=reduce_to_chars.begin();redi!=reduce_to_chars.end();++redi)
                {
                    if (can_be_skipped( *redi ))
                    {
                        auto red2 = m_follow_skip_eps[*redi];
                        reduce_to_chars.insert(red2.begin(), red2.end());
                        reduce_to_chars.erase(EPS_SYM);
                    }
                }
            }while(red_cnt<reduce_to_chars.size());



            for(auto it2=reduce_to_chars.begin();it2!=reduce_to_chars.end();++it2)
            {
                std::vector<int> &rlist = reduce_jmp[*it2];
                rlist.push_back(it->m_rule);
            }
        }
        else
        {
            ///shift
            /// 将下一个token放入，指向一个新的closure
            std::string tk = curr_rule[it->m_curr_dot_index+1];

            //生产新的状态
            Closure &nclosure = shift_jmp[tk] ;
            min_state nst = *it;
            nst.m_curr_dot_index++;
            nclosure.m_kernel.insert(nst);


            /// 如果tk是终结符，则无操作
            /// 如果tk不是终结符，则需要判断：
            /// 如果是可跳过，则在tk的follow集出现时跳过(reduce)
            /// 如果是不可跳过，则

            if(is_nterm(tk))
            {

                ///




                if(can_be_skipped(tk))
                {

                    auto reduce_to_chars = m_follow_skip_eps[tk];
                    for(auto it2=reduce_to_chars.begin();it2!=reduce_to_chars.end();++it2)
                    {
                        std::vector<int> &rlist = reduce_jmp[*it2];
                        rlist.push_back(m_eps_token_rule[tk]);
                    }
                }



            }




#if 0

            if(tk==EPS_SYM)
            {
                //DO NOT use eps jump??? or just jump to next pos???
                continue;
            }
            ///shift如果需要，则tk对应的first集，都要加入
            ///
            auto shift_to_chars = m_first[tk];


            shift_to_chars.erase(EPS_SYM);
            shift_to_chars.insert(tk);
            ///如果first集中有eps，则合并入follow集
            int red_cnt = 0;
            do
            {
                red_cnt = shift_to_chars.size();
                for(auto redi=shift_to_chars.begin();redi!=shift_to_chars.end();++redi)
                {
                    if (can_be_skipped( *redi ))
                    {
                        auto red2 = m_follow_skip_eps[*redi];
                        shift_to_chars.insert(red2.begin(), red2.end());
                        shift_to_chars.erase(EPS_SYM);
                    }
                }
            }while(red_cnt<shift_to_chars.size());


            //生产新的状态
            Closure nclosure;
            min_state nst = *it;
            nst.m_curr_dot_index++;
            nclosure.m_kernel.insert(nst);

            ///将新的状态放入设备
            for(auto redi=shift_to_chars.begin();redi!=shift_to_chars.end();++redi)
            {
                auto &closure = shift_jmp[*redi];
                closure=nclosure;
            }
#endif
        }
    }

#if 0
    for(auto it=state.begin();it!=state.end();++it)
    {
        auto curr_rule = m_rules[it->m_rule];
        if(it->m_curr_dot_index+1>=curr_rule.size())
        {   //reduce
            continue;
        }
        ///没有==2的？？？？
        if(curr_rule.size()==2)continue;

        std::string to_reduce_rule_left = curr_rule[it->m_curr_dot_index+1];
        if(can_be_skipped( to_reduce_rule_left))
        {
            /// reduce eps for some rule.
            ///
            ///
            ///

            vector<string> reduce_token;
            string next_nterms;
            if(it->m_curr_dot_index+2>=curr_rule.size())
            {
                ///后面没有其他字符了，需要reduce
                next_nterms = it->m_next;
            }
            else
            {
                next_nterms= curr_rule[it->m_curr_dot_index+2];
            }

            reduce_token.insert(reduce_token.end(), m_first[next_nterms].begin(),m_first[next_nterms].end());
            int rule_id = find_eps_rule(to_reduce_rule_left);
            if(rule_id<0)
            {
                ///no such rule. just skip???
                std::cerr<<"empty rule not find."<<to_reduce_rule_left<<"\n";
                continue;
            }

            for(auto it=reduce_token.begin();it!=reduce_token.end();++it)
            {
                auto & reduce_rule_list = reduce_jmp[*it];
                reduce_rule_list.push_back(rule_id);
            }
        }
    }
#endif
    ///规则重复的去除
    for(auto it=reduce_jmp.begin();it!=reduce_jmp.end();++it)
    {
        //it->first;
        //it->second;
        set<int> st(it->second.begin(), it->second.end());
        it->second.assign(st.begin(), st.end());
    }







    return 0;
}



std::string lalr1::print_action_goto_table()
{
    std::stringstream ss;
    std::vector<int> aterm_val;

    ss<<"state_id\t| ";
    for(auto it=m_aterm_nval.begin();it!=m_aterm_nval.end();++it)
    {
        aterm_val.push_back(it->first);
        ss<< it->second<<"\t| ";
    }
    ss<<"\n";

    for(unsigned state_idx=0;state_idx<m_closures.size();++state_idx)
    {
        ss<<state_idx<<"\t| ";

        vector<int> id_vec;
        vector<int> type_vec;
        for(unsigned char_idx=0;char_idx<aterm_val.size();++char_idx)
        {
            int act_cnt=0;


            std::string jmp_term = m_aterm_nval[aterm_val[char_idx]];
            if(m_is_accpetable[state_idx])
            {
            ss<<"acc";
            act_cnt++;
            }
            if(m_jmp[state_idx].find(jmp_term)!=m_jmp[state_idx].end())
            {
                ///有跳转
                ss<<"s"<<m_jmp[state_idx][jmp_term];
                act_cnt++;
            }
            if(m_reduce[state_idx].find(jmp_term)!=m_reduce[state_idx].end())
            {
                auto curr_reduce_rule = m_reduce[state_idx][jmp_term];
                for(unsigned rr1=0;rr1!=curr_reduce_rule.size();++rr1)
                {
                    if(rr1!=0){ss<<"/";}
                    ///有规约
                    ss<<"r"<<m_reduce[state_idx][jmp_term][rr1];
                    act_cnt++;
                }
            }
            ss<<"\t| ";
        }
        ss<<"\n";
    }
    return ss.str();
}

std::string lalr1::print_actions()
{
    std::stringstream ss;
    ss<<"[actions]\n";
    for(unsigned i=0;i<m_actions.size();++i)
    {
        ss<<i<<": "<<string_pack(m_actions[i])<<"\n";
    }
        return ss.str();
}

string lalr1::print_debug_info()
{
    std::stringstream ss;
    ss<<print_rules();
    ss<<print_term_vals();

    ss<< print_actions();


    //ss<<print_actions();




    ss<<print_first(m_first);
    ss<<print_first(m_first_skip_eps);
    ss<<print_follow(m_follow);
    ss<<print_follow(m_follow_skip_eps);

    ss<<print_skip_token();

    ss<<print_closures();
    ss<<print_jmp_table();

    ss<<print_action_goto_table();


    ss<<"[MIDACTIONS]\n";
    for(auto it=m_middle_action_state.begin();it!=m_middle_action_state.end();++it)
    {
        ss<<it->first<<":\n";

        for(auto it2=it->second.begin();it2!=it->second.end();++it2)
        {
            ss<<*it2<<", ";
        }
        ss<<"\n";
    }



    return ss.str();

}

std::string lalr1::print_skip_token()
{
    std::stringstream ss;
    ss<<"[eps]:\n";
    for(unsigned i=0;i<m_eps_tokens.size();++i)
    {
        ss<<m_eps_tokens[i]<<", ";
    }
    ss<<"\n";

    return ss.str();
}

std::string lalr1::print_rules()
{
    std::stringstream ss;
    ss<<"[rules]\n";
    for(unsigned i=0;i<this->m_rules.size();++i)
    {
        ss<<i<<". ";
        for(unsigned j=0;j<this->m_rules[i].size();++j)
        {
            if(j==0)
            {
            ss<<this->m_rules[i][j]<<" -> ";
            }
            else
            {
                {
                    ss<<this->m_rules[i][j]<<" ";
                }
                if(this->m_is_action[i][j])
                {
                    ss<<"<ACTION "<<this->m_is_action[i][j]<< ">. ";
                }
            }
        }
        ss<<"\n";
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

string lalr1::print_closures()
{
    stringstream ss;
    ss<<"//[CLOSURE]\n";
    for(unsigned i=0;i<m_closures.size();++i)
    {
        ss<<"//closure-I"<<i<<":\n";
        ss<<"//Kernel:\n";

        {
            auto &st = m_closures[i].m_kernel;
            for(auto it=st.begin();it!=st.end();++it)
            {
                ss<<print_one_rule(it->m_rule, it->m_curr_dot_index, it->m_next);
            }
        }

        ss<<"//States:\n";
        auto &st = m_closures[i].m_states;
        for(auto it=st.begin();it!=st.end();++it)
        {
            ss<<print_one_rule(it->m_rule, it->m_curr_dot_index, it->m_next);
        }
    }
    return ss.str();
}

string lalr1::print_one_rule(int rule_index, int dot_index, std::string next_tk)
{
    std::stringstream ss;

    auto curr_rule = m_rules[rule_index];
    auto curr_is_action= m_is_action[rule_index];
    ss<<"//"<<curr_rule[0]<<"->";
    for(unsigned j=1;j<curr_rule.size();++j)
    {
        if(dot_index+1==j)
        {
            ss<<".";
        }

        {
            ss<<curr_rule[j]<<" ";
        }
        if(curr_is_action[j]){
            ss<<"<ACTION "<< curr_is_action[j]<<"> ";
        }
    }
    if(dot_index+1==curr_rule.size())
    {
        ss<<".";
    }
    ss<<", "<< next_tk<<"\n";
    return ss.str();

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

std::string lalr1::print_first(std::map<std::string, std::set<std::string> > &mfirst)
{
    stringstream ss;
    ss<<"[FIRST]\n";
    for(auto it =mfirst.begin();it!=mfirst.end();++it)
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

string lalr1::print_follow(std::map<std::string, std::set<std::string> > &mfollow)
{
    stringstream ss;
    ss<<"[FOLLOW]\n";
    for(auto it =mfollow.begin();it!=mfollow.end();++it)
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
