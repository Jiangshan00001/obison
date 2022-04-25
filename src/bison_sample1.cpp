#include <sstream>
#include "bison_sample1.h"

bison_sample1::bison_sample1()
{

}

std::string bison_sample1::render_def_header(std::string core_txt)
{
    std::stringstream ss;

    return ss.str();
}

std::string bison_sample1::render_parser_header(std::string core_txt)
{
    std::stringstream ss;

    std::string temp = R"AAA(
#ifndef CLASS_NAME_H
#define CLASS_NAME_H
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <iostream>
///TODO: 添加bison的运行的代码
///


template<class LEX_C>
class CLASS_NAME
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


   typedef  class LEX_C::token obison_token_type;


   obison_token_type yyparse(){
       ///此处获取token，进行解析
       ///
       ///
       /// token = next_token()
       ///
       /// repeat forever
       ///    s = top of stack
       ///
       ///    if action[s, token] = “shift si” then
       ///       PUSH token
       ///       PUSH si
       ///       token = next_token()
       ///
       ///    else if action[s, token] = “reduce A::= β“ then
       ///       POP 2 * |β| symbols
       ///       s = top of stack
       ///       PUSH A
       ///       PUSH goto[s,A]
       ///
       ///    else if action[s, token] = “accept” then
       ///       return
       ///
       ///    else
       ///       error()
       ///

       auto tk = m_oflex.yylex();
       std::stack<int> m_state_stack;
       std::stack<obison_token_type> m_token_stack;
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

               m_token_stack.push(tk);
               m_state_stack.push(next_state);
               tk = m_oflex.yylex();
               if(m_debug)std::cout<<"get new tk:"<<m_char_str_vec[m_token_index[tk.m_ret]]<<"\n";
           }
           else if(action_type == OBISON_ACTION_TYPE_REDUCE)
           {
               if(m_debug)std::cout<<"reduce:\n";
               int reduce_state = m_action_id[s][m_token_index[tk.m_ret]];
               int rule_right_len = m_rules[reduce_state].size()-1;

               std::vector<obison_token_type> state_vec;
               while(rule_right_len>0)
               {
                   state_vec.push_back(m_token_stack.top());
                   m_token_stack.pop();
                   m_state_stack.pop();
                   --rule_right_len;
               }
               auto tkn = reduce_match_call(state_vec,reduce_state);

               m_token_stack.push(tkn);

               s = m_state_stack.top();
               m_state_stack.push(m_action_id[s][m_token_index[tkn.m_ret]]);
               //tk = m_oflex.yylex();
           }
           else if(action_type == OBISON_ACTION_TYPE_NOTHING)
           {
               std::cerr<<"action_type-nothing error\n";
               std::cerr<<"curr state:"<<s<<"\n";
               std::cerr<<"next token:"<<tk.m_ret <<". "<< m_char_str_vec[m_token_index[tk.m_ret]]<<". token index"<< m_token_index[tk.m_ret]<<" token str:"<< tk.m_yytext <<"\n";
               break;
           }
           else if(action_type == OBISON_ACTION_TYPE_ACCEPT)
           {
               if(m_debug)std::cerr<<"action_type-accept finish\n";
               break;
           }
       }
       return m_token_stack.top();
   }
   int set_file_name(std::string file_name){
       return m_oflex.set_file_name(file_name);
   }

   LEX_C m_oflex;

private:
   obison_token_type reduce_match_call(std::vector<obison_token_type> &state_vec, int rule_index)
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

       if(m_is_recoding)
       {
           tk.m_children.assign(state_vec.begin(), state_vec.end());
       }


       return tk;
   }

#if 1
   ///std::vector< std::vector<std::string > > m_rules
   /// std::vector< int > m_rule_reduce_ret
   /// std::vector<int> m_char_vec
   /// std::vector<std::string> m_char_str_vec
   /// std::map<int, int>  m_token_index
   /// int m_action_id[31][12]
   /// int m_action_type[31][12]
   ///

   TEMPLATE_CORE_POSITION


#endif

   /// state:0-9
   /// term_token:0-14
   std::vector< std::vector<std::string > > m_rules={
   {"start_20220422_start","S","EOF_20220422_EOF"},
   {"S","E","'\n'"},
   {"S","S","E","'\n'"},
   {"E","E","'+'","E"},
   {"E","E","'-'","E"},
   {"E","E","'*'","E"},
   {"E","E","'/'","E"},
   {"E","T_NUM"},
   {"E","'('","E","')'"},
   };
   std::vector< int > m_rule_reduce_ret={303,302,302,301,301,301,301,301,301};



   std::vector<int> m_char_vec={
   10,40,41,42,43,45,47,257,258,301,302,303,};
   std::vector<std::string> m_char_str_vec={
   "'\n'","'('","')'","'*'","'+'","'-'","'/'","EOF_20220422_EOF","T_NUM","E","S","start_20220422_start",};
   std::map<int, int>  m_token_index={
   {10, 0},
   {40, 1},
   {41, 2},
   {42, 3},
   {43, 4},
   {45, 5},
   {47, 6},
   {257, 7},
   {258, 8},
   {301, 9},
   {302, 10},
   {303, 11},
   };
   int m_action_id[31][12]={
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,2 ,3 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,6 ,-1 ,-1 },
   {12 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,14 ,4 ,13 ,-1 ,-1 },
   {7 ,-1 ,-1 ,7 ,7 ,7 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,15 ,-1 ,-1 },
   {-1 ,-1 ,16 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,7 ,7 ,7 ,7 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,21 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,22 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,23 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,24 ,-1 ,-1 },
   {-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,1 ,1 ,1 ,-1 ,-1 },
   {25 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,26 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {8 ,-1 ,-1 ,8 ,8 ,8 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,27 ,-1 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,28 ,-1 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,29 ,-1 ,-1 },
   {-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,30 ,-1 ,-1 },
   {5 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {3 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {4 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {6 ,-1 ,-1 ,8 ,9 ,10 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,2 ,-1 ,-1 ,-1 ,-1 ,-1 ,2 ,2 ,2 ,-1 ,-1 },
   {-1 ,-1 ,8 ,8 ,8 ,8 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,5 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,3 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,4 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   {-1 ,-1 ,6 ,17 ,18 ,19 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 },
   };
   int m_action_type[31][12]={
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,0 },
   {2 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,0 ,0 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,0 ,2 ,2 ,2 ,2 ,2 ,0 ,0 ,0 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,0 ,0 },
   {1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 },
   {0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {2 ,0 ,0 ,2 ,2 ,2 ,2 ,0 ,0 ,0 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 },
   {2 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {2 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {2 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {2 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,0 ,0 },
   {0 ,0 ,2 ,2 ,2 ,2 ,2 ,0 ,0 ,0 ,0 ,0 },
   {0 ,0 ,2 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,0 ,2 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,0 ,2 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   {0 ,0 ,2 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
   };




};

#endif // CALC_BISON_H

                       )AAA";

    return ss.str();

}
