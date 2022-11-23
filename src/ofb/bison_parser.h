
#ifndef obison_sample_H
#define obison_sample_H
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <iostream>
#include <deque>
///


#include "otoken.h"
#define YYSTYPE OToken
#include <stdio.h>
#include <iostream>
#include "lex_header.h"
#include "trim.h"
#include "obisonfile.h"


#include <vector>
#include <string>


std::string get_one_action();



template<class LEX_C>
class obison_sample
{
public:

#define OBISON_ACTION_TYPE_NOTHING 0
#define OBISON_ACTION_TYPE_SHIFT 1
#define OBISON_ACTION_TYPE_REDUCE 2
#define OBISON_ACTION_TYPE_ACCEPT 3
   int m_debug=0;
   int m_is_recoding=1;
   obison_sample(){

   }


   typedef  typename LEX_C::token obison_token_type;


   obison_token_type yyparse(){
       ///get token, parse the token with shift/reduce
       ///
       ///
       /// token = next_token()
       ///
       /// repeat forever
       ///    s = top of stack
       ///
       ///    if action[s, token] = "shift si" then
       ///       PUSH token
       ///       PUSH si
       ///       token = next_token()
       ///
       ///    else if action[s, token] = "reduce A::= β" then
       ///       POP 2 * |β| symbols
       ///       s = top of stack
       ///       PUSH A
       ///       PUSH goto[s,A]
       ///
       ///    else if action[s, token] = "accept" then
       ///       return
       ///
       ///    else
       ///       error()
       ///

       auto tk = m_oflex.yylex();
       std::stack<int> m_state_stack;
       std::deque<obison_token_type> m_token_stack;
       m_state_stack.push(0);

       while(true)
       {
           int s = m_state_stack.top();
           if(m_debug){
               std::cout<<"loop_curr_state:"<<s<<"\n";
               std::cout<<"tk:"<< m_char_str_vec[m_token_index[tk.m_ret]]<<"\n";
           }

           int action_type = m_action_type[s][m_token_index[tk.m_ret] ];
           if(action_type == OBISON_ACTION_TYPE_SHIFT)
           {
               if(m_debug)std::cout<<"shift:\n";
               int next_state = m_action_id[s][m_token_index[tk.m_ret]];

               m_token_stack.push_back(tk);
               m_state_stack.push(next_state);

                ///
                ///FIXME: 此处，action如果执行，则需要2个返回值：tk.m_ret对应下一个token，如果已经被内部解析则需要天际。 tk 当前action的tk
                /// 此时需要多一个tk。此tk是action的返回值。 reduce时
                ///
                ///
               tk = action_in_middle(next_state, m_token_stack);
               if(tk.m_ret==NULL_20220422_NULL)
               {
                   tk = m_oflex.yylex();
               }
               if(m_debug)std::cout<<"get new tk:"<<m_char_str_vec[m_token_index[tk.m_ret]]<<"\n";
           }
           else if(action_type == OBISON_ACTION_TYPE_REDUCE)
           {
               if(m_debug)std::cout<<"reduce:\n";
               int reduce_state = m_action_id[s][m_token_index[tk.m_ret]];
               int rule_right_len = m_rules[reduce_state].size()-1;

               std::deque<obison_token_type> state_vec;
               while(rule_right_len>0)
               {
                   state_vec.push_front(m_token_stack.back());
                   m_token_stack.pop_back();
                   m_state_stack.pop();
                   --rule_right_len;
               }
               auto tkn = reduce_match_call(state_vec,reduce_state);

               m_token_stack.push_back(tkn);

               s = m_state_stack.top();
               m_state_stack.push(m_action_id[s][m_token_index[tkn.m_ret]]);
           }
           else if(action_type == OBISON_ACTION_TYPE_NOTHING)
           {
               std::cerr<<"action_type-nothing error\n";
               std::cerr<<"curr state:"<<s<<"\n";
               std::cerr<<"next token:"<<tk.m_ret <<". "<< m_char_str_vec[m_token_index[tk.m_ret]]<<". token index"<< m_token_index[tk.m_ret]<<" token str:"<< tk.m_yytext <<"\n";
               std::cerr<<"LINE/COLUMN:"<<m_oflex.m_line<<"/"<< m_oflex.m_column<<"\n";
               break;
           }
           else if(action_type == OBISON_ACTION_TYPE_ACCEPT)
           {
               if(m_debug)std::cerr<<"action_type-accept finish\n";
               break;
           }
       }
       //return m_token_stack.top();
        if(!m_token_stack.empty())
            tk = m_token_stack.front();
        else
        {
            std::cerr<<"error tk empty:\n";
        }
        return tk;
   }

   int set_file_name(std::string file_name){
       return m_oflex.set_file_name(file_name);
   }

   LEX_C m_oflex;

   int yyinput()
   {
       return m_oflex.input();
   }
   int yyunput()
   {
       return m_oflex.unput();
   }
   int yypeek(int i)
   {
       return m_oflex.peek(i);
   }


   int print_token_tree(obison_token_type &tk, int depth=0)
   {
       for(unsigned i=0;i<depth;++i)std::cout<<" ";

       if(tk.m_rule_index!=-1)
       {
           //rule
           std::cout<<"R"<< tk.m_rule_index<<", ";
           //left str
           std::cout<< m_rules[tk.m_rule_index][0]<<"("<<tk.m_yytext<<  ")";
           std::cout<<"->";
       }


       for(unsigned i=0;i<tk.m_children.size();++i)
       {
           std::string cr;
           int tk_index = 0;
           if(i<tk.m_children.size())
           {
               cr=tk.m_children[i].m_yytext;
               tk_index = m_token_index[tk.m_children[i].m_ret];
           }
        std::cout<< m_char_str_vec[tk_index]<<"("<<cr<< ")";
       }
       std::cout<<"\n";

       for(unsigned i=0;i<depth;++i) std::cout<<" ";
       std::cout<<"CHILD\n";
       for(unsigned i=0;i<tk.m_children.size();++i)
       {
           print_token_tree(tk.m_children[i], depth+1);
       }

       for(unsigned i=0;i<depth;++i) std::cout<<" ";
       std::cout<<"CHILDEND\n";

        return depth;
   }


private:
   obison_token_type reduce_match_call(std::deque<obison_token_type> &state_vec, int rule_index)
   {
       obison_token_type tk;
       if(m_debug)
       {
           std::cout<<"match:\n";
           for(int i=0;i<m_rules[rule_index].size();++i)
           {
               std::cout<<m_rules[rule_index][i]<<" ";
           }
           std::cout<<"\n";
           for(int i=0;i<state_vec.size();++i)
           {
               std::cout<<state_vec[i].m_yytext<<"(" <<state_vec[i].m_ret<<") ";
           }
           std::cout<<"\n";
       }
       tk.m_yytext = "";
       for(int i=0;i<state_vec.size();++i)
       {
           tk.m_yytext = tk.m_yytext + " " + state_vec[i].m_yytext;
       }

       tk.m_ret=m_rule_reduce_ret[rule_index];
       tk.m_rule_index = rule_index;
       tk.m_typestr = m_rule_name[rule_index];
       if(m_is_recoding)
       {
           tk.m_children.assign(state_vec.begin(), state_vec.end());
       }
       comp_process_children(tk);

       return tk;
   }

#if 1
   ///def actions before and after
   ///
public:
   int process_top_down(obison_token_type &tk)
   {
        process_one_token(tk, 0);
        return 0;
   }
private:
   void process_one_token(obison_token_type &tk, int depth=0)
   {
       before_process_children(tk);
       if(m_debug)
       {
           for(unsigned i=0;i<depth;++i)
               std::cout<<" ";
           std::cout<<tk.m_typestr<<":"<<tk.m_yytext<<"\n";
       }

       for(unsigned i=0;i<tk.m_children.size();++i)
       {
           process_one_token(tk.m_children[i], depth+1);
       }
       after_process_children(tk);
   }

   int before_process_children(obison_token_type &tk)
{
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{}

break;
case 1://spec->defs MARK rules tail 
{}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{ }

break;
case 3://tail->EPS_20220422_EPS 
{}

break;
case 4://defs->EPS_20220422_EPS 
{}

break;
case 5://defs->defs def 
{}

break;
case 6://def->START IDENTIFIER 
{m_file.m_start = tk.m_children[1].m_yytext;}

break;
case 7://def->UNION 
{
        /* Copy union definition to output. */
		printf("here should start union definition\n");
      }

break;
case 8://def->LCURL RCURL 
{}

break;
case 9://def->rword tag nlist 
{m_file.m_curr_rword.clear();}

break;
case 10://rword->TOKEN 
{}

break;
case 11://rword->LEFT 
{}

break;
case 12://rword->RIGHT 
{}

break;
case 13://rword->NONASSOC 
{}

break;
case 14://rword->TYPE 
{}

break;
case 15://tag->EPS_20220422_EPS 
{}

break;
case 16://tag->'<' IDENTIFIER '>' 
{}

break;
case 17://nlist->nmno 
{}

break;
case 18://nlist->nlist nmno 
{}

break;
case 19://nmno->IDENTIFIER 
{m_file.m_curr_rword.push_back(tk.m_children[0].m_yytext);}

break;
case 20://nmno->IDENTIFIER NUMBER 
{}

break;
case 21://rules->C_IDENTIFIER rbody prec 
{
            m_file.m_curr_rule_left = tk.m_children[0].m_yytext;
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
            m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

            m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
            m_file.m_curr_rule_is_action.push_back(0);
            }

break;
case 22://rules->rules rule 
{m_file.m_curr_rule.clear(); m_file.m_curr_rule_is_action.clear();}

break;
case 23://rule->C_IDENTIFIER rbody prec 
{
            m_file.m_curr_rule_left = tk.m_children[0].m_yytext;
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
            m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

            m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
            m_file.m_curr_rule_is_action.push_back(0);
            }

break;
case 24://rule->'|' rbody prec 
{
                    m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
                    m_file.m_curr_rule_is_action.push_back(0);
                    }

break;
case 25://rbody->EPS_20220422_EPS 
{}

break;
case 26://rbody->rbody IDENTIFIER 
{}

break;
case 27://rbody->rbody acts 
{}

break;
case 28://rbody->rbody '<' 
{}

break;
case 29://rbody->rbody '>' 
{}

break;
case 30://acts->act 
{}

break;
case 31://acts->acts act 
{}

break;
case 32://act->'{' '}' 
{}

break;
case 33://prec->EPS_20220422_EPS 
{}

break;
case 34://prec->PREC IDENTIFIER 
{}

break;
case 35://prec->PREC IDENTIFIER act 
{}

break;
case 36://prec->prec ';' 
{}

break;
default:
{if(tk.m_rule_index!=-1){std::cerr<<"action error"<<tk.m_rule_index<<"\n";}}break;
}
return 0;

}
int after_process_children(obison_token_type &tk)
{
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{}

break;
case 1://spec->defs MARK rules tail 
{}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{}

break;
case 3://tail->EPS_20220422_EPS 
{}

break;
case 4://defs->EPS_20220422_EPS 
{}

break;
case 5://defs->defs def 
{}

break;
case 6://def->START IDENTIFIER 
{}

break;
case 7://def->UNION 
{}

break;
case 8://def->LCURL RCURL 
{}

break;
case 9://def->rword tag nlist 
{
                                    std::vector<std::string> def_tks;
                                    def_tks.push_back(tk.m_children[0].m_yytext);
                                    for(unsigned i=0;i<m_file.m_curr_rword.size();++i)
                                    {
                                        def_tks.push_back(m_file.m_curr_rword[i]);
                                        if(tk.m_children[0].m_yytext!="type"){
                                            m_file.m_terms.push_back(m_file.m_curr_rword[i]);
                                        }
                                    }
                                    m_file.m_defs_token.push_back(def_tks);
                                    m_file.m_curr_rword.clear();
                                }

break;
case 10://rword->TOKEN 
{}

break;
case 11://rword->LEFT 
{}

break;
case 12://rword->RIGHT 
{}

break;
case 13://rword->NONASSOC 
{}

break;
case 14://rword->TYPE 
{}

break;
case 15://tag->EPS_20220422_EPS 
{}

break;
case 16://tag->'<' IDENTIFIER '>' 
{}

break;
case 17://nlist->nmno 
{}

break;
case 18://nlist->nlist nmno 
{}

break;
case 19://nmno->IDENTIFIER 
{}

break;
case 20://nmno->IDENTIFIER NUMBER 
{}

break;
case 21://rules->C_IDENTIFIER rbody prec 
{
                    m_file.m_rules.push_back(m_file.m_curr_rule);
                    m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                    m_file.m_curr_rule.clear();
                    m_file.m_curr_rule_is_action.clear();
                }

break;
case 22://rules->rules rule 
{}

break;
case 23://rule->C_IDENTIFIER rbody prec 
{
                    m_file.m_rules.push_back(m_file.m_curr_rule);
                    m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                    m_file.m_curr_rule.clear();
                    m_file.m_curr_rule_is_action.clear();
                }

break;
case 24://rule->'|' rbody prec 
{
                        m_file.m_rules.push_back(m_file.m_curr_rule);
                        m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                        m_file.m_curr_rule.clear();
                        m_file.m_curr_rule_is_action.clear();
                    }

break;
case 25://rbody->EPS_20220422_EPS 
{}

break;
case 26://rbody->rbody IDENTIFIER 
{
                                m_file.m_curr_rule.push_back(tk.m_children[1].m_yytext);
                                m_file.m_curr_rule_is_action.push_back(0);
                            }

break;
case 27://rbody->rbody acts 
{
                                unpack_acts(m_file.m_curr_rule, tk.m_children[1]);
                                while(m_file.m_curr_rule_is_action.size()<m_file.m_curr_rule.size())
                                {
                                    m_file.m_curr_rule_is_action.push_back(1);
                                }
                        }

break;
case 28://rbody->rbody '<' 
{}

break;
case 29://rbody->rbody '>' 
{}

break;
case 30://acts->act 
{}

break;
case 31://acts->acts act 
{}

break;
case 32://act->'{' '}' 
{}

break;
case 33://prec->EPS_20220422_EPS 
{}

break;
case 34://prec->PREC IDENTIFIER 
{}

break;
case 35://prec->PREC IDENTIFIER act 
{}

break;
case 36://prec->prec ';' 
{}

break;
default:
{if(tk.m_rule_index!=-1){std::cerr<<"action error"<<tk.m_rule_index<<"\n";}}break;
}
return 0;

}
int comp_process_children(obison_token_type &tk)
{
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{}

break;
case 1://spec->defs MARK rules tail 
{}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{}

break;
case 3://tail->EPS_20220422_EPS 
{}

break;
case 4://defs->EPS_20220422_EPS 
{}

break;
case 5://defs->defs def 
{}

break;
case 6://def->START IDENTIFIER 
{}

break;
case 7://def->UNION 
{}

break;
case 8://def->LCURL RCURL 
{}

break;
case 9://def->rword tag nlist 
{}

break;
case 10://rword->TOKEN 
{}

break;
case 11://rword->LEFT 
{}

break;
case 12://rword->RIGHT 
{}

break;
case 13://rword->NONASSOC 
{}

break;
case 14://rword->TYPE 
{}

break;
case 15://tag->EPS_20220422_EPS 
{}

break;
case 16://tag->'<' IDENTIFIER '>' 
{}

break;
case 17://nlist->nmno 
{}

break;
case 18://nlist->nlist nmno 
{}

break;
case 19://nmno->IDENTIFIER 
{}

break;
case 20://nmno->IDENTIFIER NUMBER 
{}

break;
case 21://rules->C_IDENTIFIER rbody prec 
{}

break;
case 22://rules->rules rule 
{}

break;
case 23://rule->C_IDENTIFIER rbody prec 
{}

break;
case 24://rule->'|' rbody prec 
{}

break;
case 25://rbody->EPS_20220422_EPS 
{}

break;
case 26://rbody->rbody IDENTIFIER 
{}

break;
case 27://rbody->rbody acts 
{}

break;
case 28://rbody->rbody '<' 
{}

break;
case 29://rbody->rbody '>' 
{}

break;
case 30://acts->act 
{}

break;
case 31://acts->acts act 
{}

break;
case 32://act->'{' '}' 
{}

break;
case 33://prec->EPS_20220422_EPS 
{}

break;
case 34://prec->PREC IDENTIFIER 
{}

break;
case 35://prec->PREC IDENTIFIER act 
{}

break;
case 36://prec->prec ';' 
{}

break;
default:
{if(tk.m_rule_index!=-1){std::cerr<<"action error"<<tk.m_rule_index<<"\n";}}break;
}
return 0;

}


#endif

#if 1



   ///std::vector< std::vector<std::string > > m_rules
   /// std::vector< int > m_rule_reduce_ret
   /// std::vector<int> m_char_vec
   /// std::vector<std::string> m_char_str_vec
   /// std::map<int, int>  m_token_index
   /// int m_action_id[31][12]
   /// int m_action_type[31][12]
   ///
public:

   std::vector< std::vector<std::string > > m_rules={
{"start_20220422_start","spec","EOF_20220422_EOF"},
{"spec","defs","MARK","rules","tail"},
{"tail","MARK","FAKE_FINISH_MARK"},
{"tail"},
{"defs"},
{"defs","defs","def"},
{"def","START","IDENTIFIER"},
{"def","UNION"},
{"def","LCURL","RCURL"},
{"def","rword","tag","nlist"},
{"rword","TOKEN"},
{"rword","LEFT"},
{"rword","RIGHT"},
{"rword","NONASSOC"},
{"rword","TYPE"},
{"tag"},
{"tag","'<'","IDENTIFIER","'>'"},
{"nlist","nmno"},
{"nlist","nlist","nmno"},
{"nmno","IDENTIFIER"},
{"nmno","IDENTIFIER","NUMBER"},
{"rules","C_IDENTIFIER","rbody","prec"},
{"rules","rules","rule"},
{"rule","C_IDENTIFIER","rbody","prec"},
{"rule","'|'","rbody","prec"},
{"rbody"},
{"rbody","rbody","IDENTIFIER"},
{"rbody","rbody","acts"},
{"rbody","rbody","'<'"},
{"rbody","rbody","'>'"},
{"acts","act"},
{"acts","acts","act"},
{"act","'{'","'}'"},
{"prec"},
{"prec","PREC","IDENTIFIER"},
{"prec","PREC","IDENTIFIER","act"},
{"prec","prec","';'"},
};
std::vector< std::string > m_rule_name={
"start_20220422_start:0","spec:0","tail:0","tail:1","defs:0","defs:1","def:0","def:1","def:2","def:3","rword:0","rword:1","rword:2","rword:3","rword:4","tag:0","tag:1","nlist:0","nlist:1","nmno:0","nmno:1","rules:0","rules:1","rule:0","rule:1","rbody:0","rbody:1","rbody:2","rbody:3","rbody:4","acts:0","acts:1","act:0","prec:0","prec:1","prec:2","prec:3",};
std::vector< int > m_rule_reduce_ret={313,312,315,315,304,304,303,303,303,303,311,311,311,311,311,314,314,305,305,306,306,310,310,309,309,308,308,308,308,308,302,302,301,307,307,307,307};
std::vector<int> m_char_vec={
59,60,62,123,124,125,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,};
std::vector<std::string> m_char_str_vec={
"';'","'<'","'>'","'{'","'|'","'}'","IDENTIFIER","C_IDENTIFIER","NUMBER","LEFT","RIGHT","NONASSOC","TOKEN","PREC","TYPE","START","UNION","MARK","FAKE_FINISH_MARK","LCURL","RCURL","EOF_20220422_EOF","EPS_20220422_EPS","NULL_20220422_NULL","act","acts","def","defs","nlist","nmno","prec","rbody","rule","rules","rword","spec","start_20220422_start","tag","tail",};
std::map<int, int>  m_token_index={
{59, 0},
{60, 1},
{62, 2},
{123, 3},
{124, 4},
{125, 5},
{257, 6},
{258, 7},
{259, 8},
{260, 9},
{261, 10},
{262, 11},
{263, 12},
{264, 13},
{265, 14},
{266, 15},
{267, 16},
{268, 17},
{269, 18},
{270, 19},
{271, 20},
{272, 21},
{273, 22},
{274, 23},
{301, 24},
{302, 25},
{303, 26},
{304, 27},
{305, 28},
{306, 29},
{307, 30},
{308, 31},
{309, 32},
{310, 33},
{311, 34},
{312, 35},
{313, 36},
{314, 37},
{315, 38},
};


int m_action_id[54][39]={
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,4 ,4 ,4 ,-1 ,4 ,4 ,4 ,4 ,-1 ,4 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,2 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,7 ,6 ,9 ,-1 ,10 ,8 ,11 ,5 ,-1 ,3 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,11 ,-1 ,-1 ,-1 ,-1 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,11 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,13 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 },
{-1 ,12 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,10 ,-1 ,-1 ,-1 ,-1 ,10 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,10 ,-1 },
{-1 ,14 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,7 ,7 ,7 ,-1 ,7 ,7 ,7 ,7 ,-1 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,5 ,5 ,5 ,5 ,-1 ,5 ,5 ,5 ,5 ,-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,19 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,20 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,8 ,8 ,8 ,8 ,-1 ,8 ,8 ,8 ,8 ,-1 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{25 ,25 ,25 ,25 ,25 ,-1 ,25 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,3 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,6 ,6 ,6 ,6 ,-1 ,6 ,6 ,6 ,6 ,-1 ,6 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,6 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,29 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{33 ,31 ,32 ,33 ,33 ,-1 ,34 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,33 ,33 ,-1 ,36 ,37 ,-1 ,-1 ,-1 ,-1 ,38 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,33 },
{25 ,25 ,25 ,25 ,25 ,-1 ,25 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,39 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{25 ,25 ,25 ,25 ,25 ,-1 ,25 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,25 ,40 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,41 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,42 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,19 ,-1 ,43 ,19 ,19 ,19 ,19 ,-1 ,19 ,19 ,19 ,19 ,-1 ,19 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,19 ,-1 ,-1 ,19 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,9 ,9 ,9 ,9 ,-1 ,9 ,9 ,9 ,9 ,-1 ,9 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,9 ,-1 ,-1 ,44 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,17 ,-1 ,-1 ,17 ,17 ,17 ,17 ,-1 ,17 ,17 ,17 ,17 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,17 ,-1 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{28 ,28 ,28 ,28 ,28 ,-1 ,28 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{29 ,29 ,29 ,29 ,29 ,-1 ,29 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,45 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{26 ,26 ,26 ,26 ,26 ,-1 ,26 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,46 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{30 ,30 ,30 ,30 ,30 ,-1 ,30 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,30 ,30 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{27 ,27 ,27 ,33 ,27 ,-1 ,27 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,47 ,27 ,-1 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{48 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 },
{33 ,31 ,32 ,33 ,33 ,-1 ,34 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,33 ,33 ,-1 ,36 ,37 ,-1 ,-1 ,-1 ,-1 ,49 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,33 },
{33 ,31 ,32 ,33 ,33 ,-1 ,34 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,33 ,33 ,-1 ,36 ,37 ,-1 ,-1 ,-1 ,-1 ,50 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,33 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,2 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,20 ,-1 ,-1 ,20 ,20 ,20 ,20 ,-1 ,20 ,20 ,20 ,20 ,-1 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,20 ,-1 ,-1 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,-1 ,18 ,18 ,18 ,18 ,-1 ,18 ,18 ,18 ,18 ,-1 ,18 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,-1 ,18 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{32 ,32 ,32 ,32 ,32 ,-1 ,32 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,32 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{34 ,-1 ,-1 ,51 ,34 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,52 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 },
{31 ,31 ,31 ,31 ,31 ,-1 ,31 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,31 ,31 ,-1 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{36 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 ,36 },
{48 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,24 },
{48 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,23 },
{-1 ,-1 ,-1 ,-1 ,-1 ,53 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{35 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 },
{32 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 },
};
int m_action_type[54][39]={
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,1 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 },
{3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,1 ,1 ,1 ,2 ,0 ,1 ,2 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,2 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,1 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,1 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{1 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{2 ,1 ,1 ,1 ,2 ,0 ,1 ,2 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,2 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{2 ,1 ,1 ,1 ,2 ,0 ,1 ,2 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,2 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,0 ,0 ,1 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{2 ,2 ,2 ,2 ,2 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{1 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{1 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
{2 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 },
};

obison_token_type action_in_middle(int state_id, const std::deque<obison_token_type> token_stack)
{
obison_token_type _tk;
_tk.m_ret=NULL_20220422_NULL;
switch(state_id)
{
case 24:

{
std::vector<obison_token_type> v;
v.assign(token_stack.end()-1, token_stack.end());
{
            if(m_debug){std::cout<<"tail eatup start\n";}
            std::string last_code;
            int one_char =yyinput();
            while(-1!=one_char)
            {
                last_code.insert(last_code.end(), (char)one_char);
                one_char=yyinput();
            }
            OToken ttk("tail", last_code);
            m_file.m_last_code =last_code;
            if(m_debug){std::cout<<"tail eatup finish. size:"<< last_code.size()<<"\n";}
            _tk.m_ret = FAKE_FINISH_MARK;
            /* In this action, set up the rest of the file. */
            }
}
break;
case 3:

{
std::vector<obison_token_type> v;
v.assign(token_stack.end()-1, token_stack.end());
{
		printf("here should finish include definition\n");
        //m_file.m_include_code="";
        std::string include_code;
        int one_char = yypeek(0);
        int one_char2 = yypeek(1);

        while((0!=one_char)&&(0!=one_char2)&&(('%'!=one_char)||('}'!=one_char2) ))
        {
            one_char = yyinput();
            include_code.insert(include_code.end(), char(one_char));
            one_char = yypeek(0);
            one_char2 = yypeek(1);
        }

        //skipRCURL
        one_char = yyinput();
        one_char = yyinput();
        //_tk=v[0];
        /* Copy C code to output file. */
        OToken tk("def_inc_code", include_code);
        _tk= tk;
        _tk.m_ret=RCURL;//add RCURL to stack???

        m_file.m_def_includes=include_code;

      }
}
break;
case 33:
case 51:

{
std::vector<obison_token_type> v;
v.assign(token_stack.end()-1, token_stack.end());
{
		
          /* Copy action, translate _tk, and so on. */
            std::string curr_act = get_one_action();
            std::cerr<<"action finish here\n";
          OToken tk("act", curr_act);
          _tk=tk;
          _tk.m_ret='}';//add RCURL to stack???

          }
}
break;
default:
_tk.m_ret=NULL_20220422_NULL;
break;

}
return _tk;

}
 

OBisonFile m_file;

int unpack_acts(std::vector<std::string> &rule, OToken &tk)
{
    if(tk.m_typestr=="act:0")
    {
        rule.push_back(tk.m_children[1].m_yytext);
    }
    else if(tk.m_typestr=="acts:0")
    {
        unpack_acts(rule, tk.m_children[0]);
    }
    else if(tk.m_typestr=="acts:1")
    {
        unpack_acts(rule, tk.m_children[0]);
        unpack_acts(rule, tk.m_children[1]);
    }
    return 0;
}

int yyerror(const char*ss)
{
    return 0;
}

int yyprint(){
    return 0;
}


std::string get_one_action()
{
    std::cerr<<"action should do here\n";
    yyprint();
    std::string curr_act="{";
    int one_char = 0;
    int depth = 1;
    do{
      one_char = yyinput();

      ///去除注释
      if(one_char=='/' && yypeek(0)=='/')
      {
          std::cerr<<"comment remove start\n";
          yyprint();
          while(one_char!='\n')
          {
              curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
              one_char=yyinput();
          }
          std::cerr<<"comment remove end\n";
          yyprint();
      }

      ///去除字符串
      if(one_char=='\'')
      {
          std::cerr<<"single remove start\n";
          yyprint();
          //skip one char
          do
          {
              curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
              one_char=yyinput();
              if(one_char=='\\')///去除转义字符
              {
                  curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
                  one_char=yyinput();
                  curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
                  one_char=yyinput();
              }
          }while(one_char!='\'');
          std::cerr<<"single remove finish\n";
          yyprint();
      }
      if(one_char=='\"')
      {
          std::cerr<<"dquote remove start\n";
          yyprint();
          //skip one char
          do
          {
              curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
              one_char=yyinput();
              if(one_char=='\\')///去除转义字符
              {
                  curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
                  one_char=yyinput();
                  curr_act.insert(curr_act.end(), (char)one_char);// = curr_act + yytext;
                  one_char=yyinput();
              }
          }while(one_char!='\"');
          std::cerr<<"dquote remove finish\n";
          yyprint();

      }

      curr_act.insert(curr_act.end(), (char)one_char);
      if((char)one_char=='{'){ depth++;std::cerr<<"action parser+:"<<depth<<"\n"; yyprint();}
      if(((char)one_char=='}')&&(depth>0)){depth--;std::cerr<<"action parser-:"<<depth<<"\n"; yyprint();};

    }while(!(('}'==one_char)&&(depth==0)));

    return curr_act;
}









#endif

};

#endif // obison_sample_H

                       
