#ifndef OBISONFILE_H
#define OBISONFILE_H
#include <vector>
#include<string>
#include <map>

class OBisonFile
{
public:
    OBisonFile();
    int m_debug=0;
    std::string m_start;
    std::string m_def_includes;
    std::string m_def_union;
    std::string m_last_code;

    std::vector< std::vector<std::string > > m_rules;
    std::vector< std::vector<int > > m_is_action;

    /// left + -
    /// left * /
    std::vector<std::vector<  std::string> > m_defs_token;




    std::vector< std::string > m_terms;//所有终结符
    std::vector<std::string> m_nterms;//所有非终结符

    std::map<std::string, int>m_aterm_val;
    std::map<int,std::string>m_aterm_nval;

    /// action 是规则的一部分
    /// 规约时，需要确定规则号和规约深度. 因为有多个action的规则，则规约深度可以不同
    /// 每个action执行后，需要将一个token压入堆栈

    int print_all();

    std::vector<std::string> m_curr_rword;

    std::vector<std::string> m_curr_rule;
    std::string m_curr_rule_left;
    std::vector<int> m_curr_rule_is_action;


    ///每一行是 一个rule. 1个rule有3个规则，分别是当前bottom-up时的规则， top-down时的before和after.
    /// 如果用户没有写规则，则默认是{},保证每个规则都有3个action
    std::vector<std::vector<std::string> > m_before_after_actions;

};

#endif // OBISONFILE_H
