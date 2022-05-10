#ifndef LALR1_H
#define LALR1_H

#include <string>
#include <vector>
#include <set>
#include <map>

enum action_type{
    E_ACTION_NULL=0,
    E_ACTION_SHIFT=1,
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

    bool operator == (const min_state& p2) const{
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


class Closure{
public:
    std::set<min_state> m_kernel;
    std::set<min_state> m_states;

    bool operator == (const Closure& p2) const{
        return (this->m_kernel==p2.m_kernel);
    }

};

#define START_SYM "start_20220422_start"
#define EOF_SYM "EOF_20220422_EOF"
#define EPS_SYM "EPS_20220422_EPS"
#define NULL_TOKEN "NULL_20220422_NULL"

///TODO: 暂未解决空字符问题
/// 添加 START_20220422_START --6000 对应起始符号
/// 添加 EPS_20220422_EPS --6001 对应空字符？
/// 添加 END_20220422_END --6002 对应结束字符
///
class lalr1
{
public:
    lalr1();


    int generate_table(class OBisonFile* file_in);
    //int generate_table(class bison_file_io* file_in);
    const std::map<int,std::string> & get_def_file();


private:
    int calc_action_table();
    int calc_middle_action_code();
    int remove_action_from_rules();
public:



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
    std::vector< std::vector<int > > m_is_action;
    std::vector<std::string> m_actions;

    int m_is_debug=1;

    std::vector<std::string> m_terms;//所有终结符
    std::vector<std::string> m_nterms;//所有非终结符

    std::map<std::string, int> m_aterm_val;
    std::map<int,std::string> m_aterm_nval;

    ///每个非终结符的fist集。 first集的意思是，对于某个非终结符状态S，可以作为第一个起始终结符的集合。
    std::map<std::string, std::set<std::string> > m_first;
    ///每个非终结符的follow集，意思是，某个状态S，后面可以跟的终结符
    std::map<std::string, std::set<std::string> > m_follow;
    ///follow集有两种表示形式，上面是见到eps停止，这个是见到eps跳跃过去
    std::map<std::string, std::set<std::string> > m_follow_skip_eps;
    std::map<std::string, std::set<std::string> > m_first_skip_eps;

    ///新的状态
    //std::vector<std::set<min_state> > m_closures;
    std::vector<Closure > m_closures;
    void add_number_to_n_terms();

    void add_nterms();

    void add_squote_to_terms();

    int add_one_production_to(std::string next_tk, std::string nn_tk, std::set<min_state> &state);

    std::string get_nn_tk(int rule_index, int dot_index, std::string curr_nn);
    int can_be_skipped(std::string tk);

    std::string print_debug_info();

    std::string print_action_goto_table();

    std::string print_actions();

private:
    int generate_terms(const std::vector<std::string> &mtokens, const std::vector<std::string> &mleft, const std::vector<std::string> &mright);
    int generate_first(std::map<std::string, std::set<std::string> > &mfirst, int is_skip_eps=0);
    int generate_follow(std::map<std::string, std::set<std::string> > &mfollow, std::map<std::string, std::set<std::string> > &mfirst, int is_skip_eps=0);
    int generate_closures(std::string mstart);
    int calc_accept(std::string mstart);
    int rules_eps_patch();
    int calc_skip_token();
    std::string print_rules();
    std::string print_skip_token();
    std::string print_term_vals();
    std::string print_first(std::map<std::string, std::set<std::string> > &mfirst);
    std::string print_follow(std::map<std::string, std::set<std::string> > &mfollow);
    std::string print_closures();
    std::string print_one_rule(int rule_index, int dot_index, std::string next_tk);
    std::string print_jmp_table();
    //判断是终结符
    bool is_term(std::string mm);
    ///判断是非终结符
    bool is_nterm(std::string mm);
    ///判断是终结符或非终结符
    bool is_aterm(std::string mm);


    Closure get_closure(Closure &cstate);
    int get_closure_next_token(const Closure &state, std::map<std::string, Closure> &shift_jmp, std::map<std::string, std::vector<int> > &reduce_jmp);
    std::set<min_state> go(std::set<min_state> &state, std::string tk);

    int find_eps_rule(std::string rule_left);


    ///跳转表
    ///
    std::vector< std::map<std::string, int > > m_jmp;
    std::vector< std::map<std::string, std::vector<int> > > m_reduce;
    std::vector<int> m_is_accpetable;
    std::vector<std::string> m_eps_tokens;
    std::map<std::string, int> m_eps_token_rule;///每个可跳过的term/nterm的rule
public:

    //生成action/goto表

    std::vector< std::vector<int> > m_action_id;
    std::vector< std::vector<int> > m_action_type;//0--无 1--jmp 2--
    ///action表的x轴
    std::vector<int> m_action_table_x_int;
    std::vector<std::string> m_action_table_x_str;

    /// 状态(closure/state): 对应的action
    std::map<int, std::set<int> > m_middle_action_state;
    std::map<int, int > m_middle_action_len;
    /// rule: 对应的action
    std::map<int, int > m_before_action;
    std::map<int, int > m_after_action;
    std::map<int, int > m_comp_action;



public:
    ///每一行是 一个rule. 1个rule有3个规则，分别是当前bottom-up时的规则， top-down时的before和after.
    /// 如果用户没有写规则，则默认是{},保证每个规则都有3个action
    std::vector<std::vector<std::string> > m_before_after_actions;

};

#endif // LALR1_H
