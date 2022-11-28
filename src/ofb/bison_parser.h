
#ifndef obison_sample_H
#define obison_sample_H
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <iostream>
#include <deque>

#include "bison_token.h"

///



#include "lex_token.h"
#define YYSTYPE lex_token
#include <stdio.h>
#include <iostream>
#include "lex_header.h"
#include "trim.h"
#include "obisonfile.h"


#include <vector>
#include <string>


std::string get_one_action();




namespace obison_space{

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


   typedef  typename LEX_C::token lex_token_type;
   typedef class BToken syntax_token_type;

   lex_token_type yyparse(){
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
       std::deque<lex_token_type> m_token_stack;
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

               std::deque<lex_token_type> state_vec;
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


   int print_token_tree(lex_token_type &tk, int depth=0, int show_rule_text=0)
   {
       for(unsigned i=0;i<depth;++i)std::cout<<">";

       if(tk.m_rule_index!=-1)
       {
           //rule
           std::cout<<"R"<< tk.m_rule_index<<", ";
           //left str
           if(show_rule_text)
            std::cout<< m_rules[tk.m_rule_index][0]<<"("<<tk.m_yytext<<  ")";
           else
            std::cout<< m_rules[tk.m_rule_index][0]<<" ";
           std::cout<<"->";
       }
       else
       {
           std::cout<<m_char_str_vec[m_token_index[tk.m_ret]] <<"("<<tk.m_yytext<<  ")";
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
           //<<"("<<cr<< ")"
        std::cout<< m_char_str_vec[tk_index]<<" ";
       }
       std::cout<<"\n";

       for(unsigned i=0;i<tk.m_children.size();++i)
       {
           print_token_tree(tk.m_children[i], depth+1);
       }

        return depth;
   }


private:
   int m_reduce_index=1;
   lex_token_type reduce_match_call(std::deque<lex_token_type> &state_vec, int rule_index)
   {
       lex_token_type tk;
        syntax_token_type stk;
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

       ///此处添加新的token
       tk.m_ret=m_rule_reduce_ret[rule_index];
       tk.m_rule_index = rule_index;
       tk.m_typestr = m_rule_name[rule_index];
       tk.m_reduce_index = m_reduce_index++;

       if(m_is_recoding)
       {
           tk.m_children.assign(state_vec.begin(), state_vec.end());
       }
       comp_process_children(tk,stk);

       return tk;
   }

#if 1
   ///def actions before and after
   ///
public:
   int process_top_down(lex_token_type &tk, syntax_token_type &stk)
   {
        process_one_token_top_down(tk, stk, 0);
        return 0;
   }

  int resize_children(lex_token_type &tk, syntax_token_type &stk)
   {
        stk.m_children.resize(tk.m_children.size());
        for(int i=0;i<tk.m_children.size();++i)
        {
            resize_children(tk.m_children[i], stk.m_children[i]);
        }
        return 0;
   }

   int resort_tk(lex_token_type &tk,syntax_token_type &stk,
                std::map<int, lex_token_type*> &tk_list,
                std::map<int, syntax_token_type*> &stk_list)
    {
       if(tk.m_reduce_index==0)return 0;
        if(tk_list.find(tk.m_reduce_index)!=tk_list.end())
        {
            std::cerr<<"reduce index error. index="<< tk.m_reduce_index<<". "<< tk<<"...VS..." <<
                      *tk_list[tk.m_reduce_index]<<"\n";

        }
       tk_list[tk.m_reduce_index] = &tk;
       stk_list[tk.m_reduce_index] = &stk;

        for(int i=0;i<tk.m_children.size();++i)
        {
            resort_tk(tk.m_children[i], stk.m_children[i], tk_list, stk_list);
        }

        return 0;
    }


   int process_down_top(lex_token_type &tk, syntax_token_type &stk)
   {
        /// 从底层到顶层。先将stk生成
        ///
        resize_children(tk, stk);
        std::map<int, lex_token_type*> tk_list;
        std::map<int, syntax_token_type*> stk_list;
        resort_tk(tk,stk, tk_list, stk_list);
        for(auto i=tk_list.begin();i!=tk_list.end();++i)
        {
          int k = i->first;
          lex_token_type &tki=*i->second;
          syntax_token_type &stki = * stk_list[k];
          before_process_children(tki, stki);
        }
        return 0;
   }





private:
   void process_one_token_top_down(lex_token_type &tk,syntax_token_type &stk,  int depth=0)
   {
       stk.m_children.resize(tk.m_children.size());
       before_process_children(tk, stk);
       if(m_debug)
       {
           for(unsigned i=0;i<depth;++i)
               std::cout<<" ";
           std::cout<<tk.m_typestr<<":"<<tk.m_yytext<<"\n";
       }

       for(unsigned i=0;i<tk.m_children.size();++i)
       {
           process_one_token_top_down(tk.m_children[i], stk.m_children[i], depth+1);
       }
       after_process_children(tk, stk);
   }

   int before_process_children(lex_token_type &tk, syntax_token_type &stk)
{
auto &ll=tk;
auto &lr = tk.m_children;
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{
auto &sl=stk.start_20220422_start;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].spec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 1://spec->defs MARK rules tail 
{
auto &sl=stk.spec;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].defs;
auto &sr2=stk.m_children[2].rules;
auto &sr3=stk.m_children[3].tail;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
auto &lr3=tk.m_children[3];
{}

}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{ }

}

break;
case 3://tail->EPS_20220422_EPS 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
{}

}

break;
case 4://defs->EPS_20220422_EPS 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
{}

}

break;
case 5://defs->defs def 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].defs;
auto &sr1=stk.m_children[1].def;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 6://def->START IDENTIFIER 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{m_file.m_start = tk.m_children[1].m_yytext;}

}

break;
case 7://def->UNION '{' '}' 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
{}

}

break;
case 8://def->LCURL RCURL 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 9://def->rword tag nlist 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rword;
auto &sr1=stk.m_children[1].tag;
auto &sr2=stk.m_children[2].nlist;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
{m_file.m_curr_rword.clear();}

}

break;
case 10://rword->TOKEN 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 11://rword->LEFT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 12://rword->RIGHT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 13://rword->NONASSOC 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 14://rword->TYPE 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 15://tag->EPS_20220422_EPS 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
{}

}

break;
case 16://tag->'<' IDENTIFIER '>' 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
{}

}

break;
case 17://nlist->nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].nmno;
auto &lr0=tk.m_children[0];
{}

}

break;
case 18://nlist->nlist nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].nlist;
auto &sr1=stk.m_children[1].nmno;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 19://nmno->IDENTIFIER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{m_file.m_curr_rword.push_back(tk.m_children[0].m_yytext);}

}

break;
case 20://nmno->IDENTIFIER NUMBER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 21://rules->rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rule;
auto &lr0=tk.m_children[0];
{}

}

break;
case 22://rules->rules rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rules;
auto &sr1=stk.m_children[1].rule;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{m_file.m_curr_rule.clear(); m_file.m_curr_rule_is_action.clear();}

}

break;
case 23://r_head->IDENTIFIER ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 24://r_head->IDENTIFIER '{' '}' ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
auto &lr3=tk.m_children[3];
{}

}

break;
case 25://rule->rulemulti prec 
{
auto &sl=stk.rule;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulemulti;
auto &sr1=stk.m_children[1].prec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 26://rulemulti->rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulesingleline;
auto &lr0=tk.m_children[0];
{}

}

break;
case 27://rulemulti->rulemulti rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulemulti;
auto &sr1=stk.m_children[1].rulesingleline;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 28://rulesingleline->r_head rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].r_head;
auto &sr1=stk.m_children[1].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
             if (tk.m_children[0].m_children.size()<=2)
             {
                 m_file.m_curr_rule_left = tk.m_children[0].m_yytext;
             }
             else
             {
                 m_file.m_curr_rule_left = tk.m_children[0].m_children[0].m_yytext;
             }
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
            m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

            m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
            m_file.m_curr_rule_is_action.push_back(0);
            }

}

break;
case 29://rulesingleline->'|' rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
auto &sr1=stk.m_children[1].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
                    m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
                    m_file.m_curr_rule_is_action.push_back(0);
                    }

}

break;
case 30://rbody->EPS_20220422_EPS 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 31://rbody->rbody IDENTIFIER 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 32://rbody->rbody acts 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &sr1=stk.m_children[1].acts;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 33://rbody->rbody '<' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 34://rbody->rbody '>' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 35://acts->act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].act;
auto &lr0=tk.m_children[0];
{}

}

break;
case 36://acts->acts act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].acts;
auto &sr1=stk.m_children[1].act;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 37://act->'{' '}' 
{
auto &sl=stk.act;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 38://prec->';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 39://prec->prec ';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].prec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
default:
{if(tk.m_rule_index!=-1){std::cerr<<"action error"<<tk.m_rule_index<<"\n";}}break;
}
return 0;

}
int after_process_children(lex_token_type &tk, syntax_token_type &stk)
{
auto &ll=tk;
auto &lr = tk.m_children;
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{
auto &sl=stk.start_20220422_start;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].spec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 1://spec->defs MARK rules tail 
{
auto &sl=stk.spec;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].defs;
auto &sr2=stk.m_children[2].rules;
auto &sr3=stk.m_children[3].tail;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
auto &lr3=tk.m_children[3];
{}

}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 3://tail->EPS_20220422_EPS 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
{}

}

break;
case 4://defs->EPS_20220422_EPS 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
{}

}

break;
case 5://defs->defs def 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].defs;
auto &sr1=stk.m_children[1].def;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 6://def->START IDENTIFIER 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 7://def->UNION '{' '}' 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
{}

}

break;
case 8://def->LCURL RCURL 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 9://def->rword tag nlist 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rword;
auto &sr1=stk.m_children[1].tag;
auto &sr2=stk.m_children[2].nlist;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
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

}

break;
case 10://rword->TOKEN 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 11://rword->LEFT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 12://rword->RIGHT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 13://rword->NONASSOC 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 14://rword->TYPE 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 15://tag->EPS_20220422_EPS 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
{}

}

break;
case 16://tag->'<' IDENTIFIER '>' 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
{}

}

break;
case 17://nlist->nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].nmno;
auto &lr0=tk.m_children[0];
{}

}

break;
case 18://nlist->nlist nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].nlist;
auto &sr1=stk.m_children[1].nmno;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 19://nmno->IDENTIFIER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 20://nmno->IDENTIFIER NUMBER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 21://rules->rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rule;
auto &lr0=tk.m_children[0];
{}

}

break;
case 22://rules->rules rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rules;
auto &sr1=stk.m_children[1].rule;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 23://r_head->IDENTIFIER ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 24://r_head->IDENTIFIER '{' '}' ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
auto &lr2=tk.m_children[2];
auto &lr3=tk.m_children[3];
{}

}

break;
case 25://rule->rulemulti prec 
{
auto &sl=stk.rule;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulemulti;
auto &sr1=stk.m_children[1].prec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 26://rulemulti->rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulesingleline;
auto &lr0=tk.m_children[0];
{}

}

break;
case 27://rulemulti->rulemulti rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rulemulti;
auto &sr1=stk.m_children[1].rulesingleline;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 28://rulesingleline->r_head rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].r_head;
auto &sr1=stk.m_children[1].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
                    m_file.m_rules.push_back(m_file.m_curr_rule);
                    m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                    m_file.m_curr_rule.clear();
                    m_file.m_curr_rule_is_action.clear();
                }

}

break;
case 29://rulesingleline->'|' rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
auto &sr1=stk.m_children[1].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
                        m_file.m_rules.push_back(m_file.m_curr_rule);
                        m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                        m_file.m_curr_rule.clear();
                        m_file.m_curr_rule_is_action.clear();
                    }

}

break;
case 30://rbody->EPS_20220422_EPS 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 31://rbody->rbody IDENTIFIER 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
                                m_file.m_curr_rule.push_back(tk.m_children[1].m_yytext);
                                m_file.m_curr_rule_is_action.push_back(0);
                            }

}

break;
case 32://rbody->rbody acts 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &sr1=stk.m_children[1].acts;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{
                                unpack_acts(m_file.m_curr_rule, tk.m_children[1]);
                                while(m_file.m_curr_rule_is_action.size()<m_file.m_curr_rule.size())
                                {
                                    m_file.m_curr_rule_is_action.push_back(1);
                                }
                        }

}

break;
case 33://rbody->rbody '<' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 34://rbody->rbody '>' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].rbody;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 35://acts->act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].act;
auto &lr0=tk.m_children[0];
{}

}

break;
case 36://acts->acts act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].acts;
auto &sr1=stk.m_children[1].act;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 37://act->'{' '}' 
{
auto &sl=stk.act;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
case 38://prec->';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
auto &lr0=tk.m_children[0];
{}

}

break;
case 39://prec->prec ';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
auto &sr0=stk.m_children[0].prec;
auto &lr0=tk.m_children[0];
auto &lr1=tk.m_children[1];
{}

}

break;
default:
{if(tk.m_rule_index!=-1){std::cerr<<"action error"<<tk.m_rule_index<<"\n";}}break;
}
return 0;

}
int comp_process_children(lex_token_type &tk, syntax_token_type &stk)
{
auto &ll=tk;
auto &lr = tk.m_children;
switch(tk.m_rule_index)
{
case 0://start_20220422_start->spec EOF_20220422_EOF 
{
auto &sl=stk.start_20220422_start;
auto &sr=stk.m_children;
{}

}

break;
case 1://spec->defs MARK rules tail 
{
auto &sl=stk.spec;
auto &sr=stk.m_children;
{}

}

break;
case 2://tail->MARK FAKE_FINISH_MARK 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
{}

}

break;
case 3://tail->EPS_20220422_EPS 
{
auto &sl=stk.tail;
auto &sr=stk.m_children;
{}

}

break;
case 4://defs->EPS_20220422_EPS 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
{}

}

break;
case 5://defs->defs def 
{
auto &sl=stk.defs;
auto &sr=stk.m_children;
{}

}

break;
case 6://def->START IDENTIFIER 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
{}

}

break;
case 7://def->UNION '{' '}' 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
{}

}

break;
case 8://def->LCURL RCURL 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
{}

}

break;
case 9://def->rword tag nlist 
{
auto &sl=stk.def;
auto &sr=stk.m_children;
{}

}

break;
case 10://rword->TOKEN 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
{}

}

break;
case 11://rword->LEFT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
{}

}

break;
case 12://rword->RIGHT 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
{}

}

break;
case 13://rword->NONASSOC 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
{}

}

break;
case 14://rword->TYPE 
{
auto &sl=stk.rword;
auto &sr=stk.m_children;
{}

}

break;
case 15://tag->EPS_20220422_EPS 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
{}

}

break;
case 16://tag->'<' IDENTIFIER '>' 
{
auto &sl=stk.tag;
auto &sr=stk.m_children;
{}

}

break;
case 17://nlist->nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
{}

}

break;
case 18://nlist->nlist nmno 
{
auto &sl=stk.nlist;
auto &sr=stk.m_children;
{}

}

break;
case 19://nmno->IDENTIFIER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
{}

}

break;
case 20://nmno->IDENTIFIER NUMBER 
{
auto &sl=stk.nmno;
auto &sr=stk.m_children;
{}

}

break;
case 21://rules->rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
{}

}

break;
case 22://rules->rules rule 
{
auto &sl=stk.rules;
auto &sr=stk.m_children;
{}

}

break;
case 23://r_head->IDENTIFIER ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
{}

}

break;
case 24://r_head->IDENTIFIER '{' '}' ':' 
{
auto &sl=stk.r_head;
auto &sr=stk.m_children;
{}

}

break;
case 25://rule->rulemulti prec 
{
auto &sl=stk.rule;
auto &sr=stk.m_children;
{}

}

break;
case 26://rulemulti->rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
{}

}

break;
case 27://rulemulti->rulemulti rulesingleline 
{
auto &sl=stk.rulemulti;
auto &sr=stk.m_children;
{}

}

break;
case 28://rulesingleline->r_head rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
{}

}

break;
case 29://rulesingleline->'|' rbody 
{
auto &sl=stk.rulesingleline;
auto &sr=stk.m_children;
{}

}

break;
case 30://rbody->EPS_20220422_EPS 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 31://rbody->rbody IDENTIFIER 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 32://rbody->rbody acts 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 33://rbody->rbody '<' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 34://rbody->rbody '>' 
{
auto &sl=stk.rbody;
auto &sr=stk.m_children;
{}

}

break;
case 35://acts->act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
{}

}

break;
case 36://acts->acts act 
{
auto &sl=stk.acts;
auto &sr=stk.m_children;
{}

}

break;
case 37://act->'{' '}' 
{
auto &sl=stk.act;
auto &sr=stk.m_children;
{}

}

break;
case 38://prec->';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
{}

}

break;
case 39://prec->prec ';' 
{
auto &sl=stk.prec;
auto &sr=stk.m_children;
{}

}

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
{"def","UNION","'{'","'}'"},
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
{"rules","rule"},
{"rules","rules","rule"},
{"r_head","IDENTIFIER","':'"},
{"r_head","IDENTIFIER","'{'","'}'","':'"},
{"rule","rulemulti","prec"},
{"rulemulti","rulesingleline"},
{"rulemulti","rulemulti","rulesingleline"},
{"rulesingleline","r_head","rbody"},
{"rulesingleline","'|'","rbody"},
{"rbody"},
{"rbody","rbody","IDENTIFIER"},
{"rbody","rbody","acts"},
{"rbody","rbody","'<'"},
{"rbody","rbody","'>'"},
{"acts","act"},
{"acts","acts","act"},
{"act","'{'","'}'"},
{"prec","';'"},
{"prec","prec","';'"},
};
std::vector< std::string > m_rule_name={
"start_20220422_start:0","spec:0","tail:0","tail:1","defs:0","defs:1","def:0","def:1","def:2","def:3","rword:0","rword:1","rword:2","rword:3","rword:4","tag:0","tag:1","nlist:0","nlist:1","nmno:0","nmno:1","rules:0","rules:1","r_head:0","r_head:1","rule:0","rulemulti:0","rulemulti:1","rulesingleline:0","rulesingleline:1","rbody:0","rbody:1","rbody:2","rbody:3","rbody:4","acts:0","acts:1","act:0","prec:0","prec:1",};
std::vector< int > m_rule_reduce_ret={316,315,318,318,304,304,303,303,303,303,314,314,314,314,314,317,317,305,305,306,306,312,312,308,308,310,311,311,313,313,309,309,309,309,309,302,302,301,307,307};
std::vector<int> m_char_vec={
58,59,60,62,123,124,125,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,};
std::vector<std::string> m_char_str_vec={
"':'","';'","'<'","'>'","'{'","'|'","'}'","IDENTIFIER","NUMBER","LEFT","RIGHT","NONASSOC","TOKEN","TYPE","START","UNION","MARK","FAKE_FINISH_MARK","LCURL","RCURL","EOF_20220422_EOF","EPS_20220422_EPS","NULL_20220422_NULL","act","acts","def","defs","nlist","nmno","prec","r_head","rbody","rule","rulemulti","rules","rulesingleline","rword","spec","start_20220422_start","tag","tail",};
std::map<int, int>  m_token_index={
{58, 0},
{59, 1},
{60, 2},
{62, 3},
{123, 4},
{124, 5},
{125, 6},
{257, 7},
{258, 8},
{259, 9},
{260, 10},
{261, 11},
{262, 12},
{263, 13},
{264, 14},
{265, 15},
{266, 16},
{267, 17},
{268, 18},
{269, 19},
{270, 20},
{271, 21},
{272, 22},
{301, 23},
{302, 24},
{303, 25},
{304, 26},
{305, 27},
{306, 28},
{307, 29},
{308, 30},
{309, 31},
{310, 32},
{311, 33},
{312, 34},
{313, 35},
{314, 36},
{315, 37},
{316, 38},
{317, 39},
{318, 40},
};


int m_action_id[57][41]={
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,4 ,-1 ,4 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,2 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,4 ,7 ,6 ,9 ,10 ,8 ,11 ,5 ,-1 ,3 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,11 ,-1 ,-1 ,-1 ,-1 ,11 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,11 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,19 ,20 ,21 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,13 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,13 ,-1 },
{-1 ,-1 ,12 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,12 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,10 ,-1 ,-1 ,-1 ,-1 ,10 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,10 ,-1 },
{-1 ,-1 ,14 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,14 ,-1 },
{-1 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,5 ,-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,15 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,8 ,-1 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,30 ,30 ,30 ,30 ,30 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,27 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 },
{28 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,30 ,30 ,30 ,30 ,30 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,30 ,-1 ,30 ,-1 ,-1 ,-1 ,30 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,21 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,21 },
{-1 ,31 ,-1 ,-1 ,-1 ,16 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,18 ,-1 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,3 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,35 ,20 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,36 },
{-1 ,26 ,-1 ,-1 ,-1 ,26 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 ,26 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,6 ,-1 ,6 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,6 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,37 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,38 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,39 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,40 ,41 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,29 ,42 ,43 ,44 ,29 ,-1 ,45 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,46 ,47 ,-1 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 ,29 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,23 ,23 ,23 ,23 ,23 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,23 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,48 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,28 ,42 ,43 ,44 ,28 ,-1 ,45 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,46 ,47 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 ,28 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,38 ,-1 ,-1 ,-1 ,38 ,-1 ,38 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,38 ,-1 ,-1 ,-1 ,38 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,38 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,38 },
{-1 ,49 ,-1 ,-1 ,-1 ,25 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,25 },
{-1 ,27 ,-1 ,-1 ,-1 ,27 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 ,27 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,50 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,22 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,22 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,-1 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,7 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,51 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,19 ,52 ,19 ,19 ,19 ,19 ,19 ,19 ,19 ,19 ,-1 ,19 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,19 ,-1 ,-1 ,19 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,39 ,-1 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,9 ,-1 ,9 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,9 ,-1 ,-1 ,53 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,17 ,-1 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,17 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,17 ,-1 ,-1 ,17 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,33 ,33 ,33 ,33 ,33 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 ,33 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,34 ,34 ,34 ,34 ,34 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,-1 ,34 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,54 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,31 ,31 ,31 ,31 ,31 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 ,31 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,35 ,35 ,35 ,35 ,35 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,35 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 ,35 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,32 ,32 ,32 ,44 ,32 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,55 ,32 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 ,32 ,-1 ,-1 ,-1 ,-1 ,-1 },
{56 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,39 ,-1 ,-1 ,-1 ,39 ,-1 ,39 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,39 ,-1 ,-1 ,-1 ,39 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,39 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,39 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,2 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,16 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,20 ,-1 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,20 ,-1 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,20 ,-1 ,-1 ,20 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,18 ,-1 ,18 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,18 ,-1 ,-1 ,18 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,37 ,37 ,37 ,37 ,37 ,-1 ,37 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,37 ,37 ,-1 ,-1 ,-1 ,-1 ,37 ,-1 ,-1 ,-1 ,-1 ,-1 ,37 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,36 ,36 ,36 ,36 ,36 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,36 ,36 ,-1 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 ,36 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,24 ,24 ,24 ,24 ,24 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,24 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
};
int m_action_type[57][41]={
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 },
{0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,1 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 },
{3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 },
{0 ,2 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,1 ,1 ,1 ,2 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,1 ,1 ,1 ,2 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,1 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,2 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,1 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,0 ,0 ,0 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,2 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 },
{0 ,2 ,2 ,2 ,2 ,2 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 },
};

lex_token_type action_in_middle(int state_id, const std::deque<lex_token_type> token_stack)
{
lex_token_type _tk;
_tk.m_ret=NULL_20220422_NULL;
switch(state_id)
{
case 34:

{
std::vector<lex_token_type> v;
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
            lex_token_type ttk("tail", last_code);
            m_file.m_last_code =last_code;
            if(m_debug){std::cout<<"tail eatup finish. size:"<< last_code.size()<<"\n";}
            _tk.m_ret = FAKE_FINISH_MARK;
            /* In this action, set up the rest of the file. */
            }
}
break;
case 24:

{
std::vector<lex_token_type> v;
v.assign(token_stack.end()-2, token_stack.end());
{
          /* Copy action, translate _tk, and so on. */
            std::string curr_act = get_one_action();
            m_file.m_default_class_var=curr_act;
          lex_token_type tk("act", curr_act);
          _tk=tk;
          _tk.m_ret='}';//add RCURL to stack???

        }
}
break;
case 3:

{
std::vector<lex_token_type> v;
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
        lex_token_type tk("def_inc_code", include_code);
        _tk= tk;
        _tk.m_ret=RCURL;//add RCURL to stack???

        m_file.m_def_includes=include_code;

      }
}
break;
case 29:

{
std::vector<lex_token_type> v;
v.assign(token_stack.end()-2, token_stack.end());
{
                        /* Copy action, translate _tk, and so on. */
                          std::string curr_act = get_one_action();
                        m_file.m_nterm_class_var[v[0].m_yytext] = curr_act;
                        lex_token_type tk("act", curr_act);
                        _tk=tk;
                        _tk.m_ret='}';//add RCURL to stack???
                    }
}
break;
case 44:

{
std::vector<lex_token_type> v;
v.assign(token_stack.end()-1, token_stack.end());
{
		
          /* Copy action, translate _tk, and so on. */
            std::string curr_act = get_one_action();
            std::cerr<<"action finish here\n";
          lex_token_type tk("act", curr_act);
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

int unpack_acts(std::vector<std::string> &rule, lex_token_type &tk)
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


};//obison_space

#endif // obison_sample_H

                       