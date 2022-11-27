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
    /// 起始标识符
    std::string m_start;
    ///前面的包含定义
    std::string m_def_includes;
    /// token的union定义，
    std::string m_def_union;

    ///最后的代码，会放到生成的类中
    std::string m_last_code;

    ///所有规则
    std::vector< std::vector<std::string > > m_rules;
    ///和m_rules大小一致，代表m_rules里哪个是否是action
    std::vector< std::vector<int > > m_is_action;


    /// left + -
    /// left * /
    std::vector<std::vector<  std::string> > m_defs_token;



    ///所有终结符
    std::vector< std::string > m_terms;
    ///所有非终结符
    std::vector<std::string> m_nterms;

    /// 所有终结符和非终结符  及其对应的编号
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
    ///当前产生式需要的类的变量
    std::map<std::string, std::string> m_nterm_class_var;
    std::string m_default_class_var;

    ///每一行是 一个rule. 1个rule有3个规则，分别是当前bottom-up时的规则， top-down时的before和after.
    /// 如果用户没有写规则，则默认是{},保证每个规则都有3个action
    std::vector<std::vector<std::string> > m_before_after_actions;

};

#endif // OBISONFILE_H
