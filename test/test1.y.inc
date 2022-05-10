
#ifndef obison_sample_H
#define obison_sample_H
#include <string>
#include <stack>
#include <map>
#include <vector>
#include <iostream>
#include <deque>
///



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


   typedef  class LEX_C::token obison_token_type;


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

                ///FIXME: 此处，action如果执行，则需要2个返回值：tk.m_ret对应下一个token，如果已经被内部解析则需要天际。 tk 当前action的tk
                /// 此时需要多一个tk。此tk是action的返回值。 reduce时
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
case 0://start_20220422_start->E EOF_20220422_EOF 
{}

break;
case 1://E->B 
{}

break;
case 2://E->E '+' B 
{}

break;
case 3://E->E '*' B 
{}

break;
case 4://B->NUM 
{}

break;
case 5://NUM->'1' 
{}

break;
case 6://NUM->'0' 
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
case 0://start_20220422_start->E EOF_20220422_EOF 
{}

break;
case 1://E->B 
{}

break;
case 2://E->E '+' B 
{}

break;
case 3://E->E '*' B 
{}

break;
case 4://B->NUM 
{}

break;
case 5://NUM->'1' 
{}

break;
case 6://NUM->'0' 
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
{"start_20220422_start","E","EOF_20220422_EOF"},
{"E","B"},
{"E","E","'+'","B"},
{"E","E","'*'","B"},
{"B","NUM"},
{"NUM","'1'"},
{"NUM","'0'"},
};
std::vector< std::string > m_rule_name={
"start_20220422_start:0","E:0","E:1","E:2","B:0","NUM:0","NUM:1",};
std::vector< int > m_rule_reduce_ret={304,302,302,302,301,303,303};
std::vector<int> m_char_vec={
42,43,48,49,257,258,259,301,302,303,304,};
std::vector<std::string> m_char_str_vec={
"'*'","'+'","'0'","'1'","EOF_20220422_EOF","EPS_20220422_EPS","NULL_20220422_NULL","B","E","NUM","start_20220422_start",};
std::map<int, int>  m_token_index={
{42, 0},
{43, 1},
{48, 2},
{49, 3},
{257, 4},
{258, 5},
{259, 6},
{301, 7},
{302, 8},
{303, 9},
{304, 10},
};


int m_action_id[11][11]={
{-1 ,-1 ,1 ,2 ,-1 ,-1 ,-1 ,3 ,4 ,5 ,-1 },
{6 ,6 ,-1 ,-1 ,6 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{5 ,5 ,-1 ,-1 ,5 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{1 ,1 ,-1 ,-1 ,1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{6 ,7 ,-1 ,-1 ,8 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{4 ,4 ,-1 ,-1 ,4 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{-1 ,-1 ,1 ,2 ,-1 ,-1 ,-1 ,9 ,-1 ,5 ,-1 },
{-1 ,-1 ,1 ,2 ,-1 ,-1 ,-1 ,10 ,-1 ,5 ,-1 },
{-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{3 ,3 ,-1 ,-1 ,3 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
{2 ,2 ,-1 ,-1 ,2 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 },
};
int m_action_type[11][11]={
{0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
{1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
{0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 },
{0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 },
{3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 ,3 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
{2 ,2 ,0 ,0 ,2 ,0 ,0 ,0 ,0 ,0 ,0 },
};

obison_token_type action_in_middle(int state_id, const std::deque<obison_token_type> token_stack)
{
obison_token_type _tk;
_tk.m_ret=NULL_20220422_NULL;
switch(state_id)
{
default:
_tk.m_ret=NULL_20220422_NULL;
break;

}
return _tk;

}



#endif

};

#endif // obison_sample_H

                       