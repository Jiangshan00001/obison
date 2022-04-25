#ifndef LALR1_H
#define LALR1_H

#include <string>
#include <vector>
#include <set>
#include <map>

enum action_type{
    E_ACTION_NULL=0,
    E_ACTION_JMP=1,
    E_ACTION_REDUCE=2,
    E_ACTION_ACCEPT=3,
};


class min_state
{
public:
    /// start: . cmd1
    /// m_rule={start, cmd1}
    /// m_curr_dot_index=0

    int  m_rule;
    int m_curr_dot_index;//index=0开始
    std::string m_next;

    bool operator<(const min_state& p2) const{
        const min_state &p1= *this;
        if(p1.m_rule!=p2.m_rule)
            return p1.m_rule> p2.m_rule;
        if(p1.m_curr_dot_index!=p2.m_curr_dot_index)
            return p1.m_curr_dot_index>p2.m_curr_dot_index;

        if(p1.m_next!=p2.m_next)
            return p1.m_next > p2.m_next;

        return false;
        }

    bool operator == (const min_state&p2) const{
        const min_state &p1= *this;
        if(p1.m_rule!=p2.m_rule)
            return false;
        if(p1.m_curr_dot_index!=p2.m_curr_dot_index)
            return false;

        if(p1.m_next!=p2.m_next)
            return false;

        return true;
    }

};


#define START_SYM "start_20220422_start"
#define EOF_SYM "EOF_20220422_EOF"

///TODO: 暂未解决空字符问题
/// 添加 START_20220422_START --6000 对应起始符号
/// 添加 EPS_20220422_EPS --6001 对应空字符？
/// 添加 END_20220422_END --6002 对应结束字符
///
class lalr1
{
public:
    lalr1();


    int generate_table(class bison_file_io* file_in);
    std::string get_def_file(std::string namespace_to_add="");
    std::string get_parser_file();



    ///语法规则。1个规则有多行，1行里有多列
    /// 规则存储格式：
    /// start :cmd1
    ///         | cmd2 cmd3
    ///     ;
    ///
    /// m_rules.size()=2
    /// m_rules[0][0]=="start";---保存第一条规则左
    /// m_rules[0][1]=="cmd1"; --保存第二条规则右
    /// m_rules[1][0]=="start";
    /// m_rules[1][1]=="cmd2";
    /// m_rules[1][2]=="cmd3";
    std::vector< std::vector<std::string > > m_rules;
    std::vector<std::string> m_terms;//所有终结符
    std::vector<std::string> m_nterms;//所有非终结符

    std::map<std::string, int>m_aterm_val;
    std::map<int,std::string>m_aterm_nval;

    ///每个非终结符的fist集。 first集的意思是，对于某个非终结符状态S，可以作为第一个起始终结符的集合。
    std::map<std::string, std::set<std::string> > m_first;
    ///每个非终结符的follow集，意思是，某个状态S，后面可以跟的终结符
    std::map<std::string, std::set<std::string> > m_follow;

    ///新的状态
    std::vector<std::set<min_state> > m_closures;
private:
    int generate_terms(const std::vector<std::string> &mtokens, const std::vector<std::string> &mleft, const std::vector<std::string> &mright);
    int generate_first();
    int generate_follow();
    int generate_closures(std::string mstart);
    int patch_accept(std::string mstart);
    std::string print_rules();
    std::string print_term_vals();
    std::string print_first();
    std::string print_follow();
    std::string print_closures();
    std::string print_jmp_table();
    //判断是终结符
    bool is_term(std::string mm);
    ///判断是非终结符
    bool is_nterm(std::string mm);
    ///判断是终结符或非终结符
    bool is_aterm(std::string mm);


    std::set<min_state> get_closure(std::set<min_state> state);
    int get_closure_next_token(const std::set<min_state> &state, std::map<std::string, std::set<min_state> > &shift_jmp, std::map<std::string, int > &reduce_jmp);
    std::set<min_state> go(std::set<min_state> &state, std::string tk);

    ///跳转表
    ///
    std::vector< std::map<std::string, int > > m_jmp;
    std::vector< std::map<std::string, int > > m_reduce;
    std::vector<int> m_is_accpetable;


};

#endif // LALR1_H
