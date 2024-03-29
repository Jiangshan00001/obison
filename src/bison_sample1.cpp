#include <sstream>
#include <sstream>
#include <algorithm>
#include "replace.h"
#include "bison_sample1.h"
#include "lalr1.h"
#include "string_eval.h"
#include "iostream"

bison_sample1::bison_sample1()
{

}

std::string bison_sample1::render_def_header(const std::map<int, std::string> &aterm_nval, std::string namespace_to_add, std::string enum_name,std::vector<std::string> mterms)
{

    std::stringstream ss;

    ss<<"#ifndef "<<enum_name<<"_h\n";
    ss<<"#define "<<enum_name<<"_h\n";


    if(!namespace_to_add.empty())
    {
        ss<<"namespace "<<namespace_to_add<<"{\n";
    }
    ss<<"enum e_bison_head{\n";
    for(auto it=aterm_nval.begin();it!=aterm_nval.end();++it)
    {
        if (it->second[0]=='\'')
        {
            ss<<"//";
        }
        if(std::find(mterms.begin(), mterms.end(), it->second)==mterms.end())
        {
            //非终结符,添加前缀
            ss<<"NTK_";
        }
        ss<<""<< it->second<<"=" << it->first<<",\n";
    }
    ss<<"};\n";

    if(!namespace_to_add.empty())
    {
        ss<<"};//namespace "<<namespace_to_add<<"\n";
    }

    ss<<"#endif\n";
    return ss.str();
}




std::string bison_sample1::render_parser(std::string &class_name, std::vector<std::vector<std::string> > &mrules,
                                         std::map<std::string, int> &materm_val,
                                         std::string &def_code, std::string &last_code,
                                         const std::vector<std::string > &mactions,
                                         std::vector<Closure > &mclosures,
                                         std::vector< std::vector<int> > &maction_id,
                                         std::vector< std::vector<int> > &maction_type,
                                         std::vector<std::string> &maction_table_x_str,
                                         std::map<int, int > &mmiddle_action_len,
                                         std::map<int, std::set<int> > &mmiddle_action_state,
                                         std::map<int, int > &mbefore_action, std::map<int, int > &mafter_action,
                                         std::map<int, int > &mcompaction,
                                         std::string name_space
                                         , std::string btaken_file, std::string btaken_class_name,
                                         std::vector<std::string> nterms)
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
#include <deque>

#include "BTAKEN_FILE_NAME"

///


DEF_INCLUDE_CODE


namespace NAME_SPACE{

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
   CLASS_NAME(){

   }


   typedef  typename LEX_C::token lex_token_type;
   typedef class BTAKEN_CLASS_NAME syntax_token_type;

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

   BEFORE_AFTER_PROCESS

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

   TEMPLATE_CORE_POSITION


#endif

};


};//NAME_SPACE

#endif // CLASS_NAME_H

                       )AAA";

    replace(temp, "BTAKEN_CLASS_NAME",btaken_class_name);
    replace(temp, "BTAKEN_FILE_NAME",btaken_file);

    replace(temp, "CLASS_NAME", class_name);
    replace(temp, "NAME_SPACE", name_space);
    replace(temp, "DEF_INCLUDE_CODE", def_code);

   std::string action_codes = generate_actions_code(mactions, mbefore_action, mafter_action,mcompaction, mrules,nterms);
   replace(temp, "BEFORE_AFTER_PROCESS", action_codes);

   std::string core_codes = generate_rules(mrules, materm_val, maction_table_x_str);
   core_codes=core_codes+generate_action_table(mclosures.size(), materm_val.size(),maction_type, maction_id);
   core_codes=core_codes+generate_middle_action(mmiddle_action_len, mmiddle_action_state, mactions );
   core_codes=core_codes+ last_code ;

   replace(temp, "TEMPLATE_CORE_POSITION", core_codes);


   return temp;
   }

   std::string bison_sample1::generate_one_action_code(std::string func_name,
                                                       const std::map<int, int> &mbefore_action,
                                                       const std::vector< std::vector<std::string > > &mrules,
                                                       const std::vector<std::string > &mactions,
                                                       std::vector<std::string> &nterms, int is_shortcut)
   {
       ///TODO: 添加已用变量 auto ll lr0 lr1 lr... sl sr0 sr1 sr2... sr... 分别代表
       /// 词法分析的左端，右边第0个，第一个。。。 语法分析的左端，右端第0个，第1个。。。
       std::stringstream ss;


       ss<<"int "<<func_name<<"(lex_token_type &tk, syntax_token_type &stk)\n{\n";

       ss<<"auto &ll=tk;\n";
       ss<<"auto &lr = tk.m_children;\n";
//       ss<<"for(int i=0;i<tk.m_children.size();++i)\n";
//       ss<<""



       ss<<"switch(tk.m_rule_index)\n{\n";
       for(auto it=mbefore_action.begin();it!=mbefore_action.end();++it)
       {
           ss<<"case "<<it->first<<"://";
           //添加注释
           for(unsigned ri =0;ri<mrules[it->first].size();++ri)
           {
               ss<< mrules[it->first][ri];
               if(ri==0){ss<<"->" ;}else{ss<<" ";}
           }
           ss    <<"\n";



           ///此处添加大括号，方便放入新定义的变量
           ss<<"{\n";

           //ss<<"if(tk.m_children.size()<"<<mrules[it->first].size()-1<<")\n{\n";
           //ss<<"tk.m_children.resize("<<mrules[it->first].size()-1<<");\n}\n";
           //ss<<"if(stk.m_children.size()<"<<mrules[it->first].size()-1<<")\n{\n";
           //ss<<"stk.m_children.resize("<<mrules[it->first].size()-1<<");\n}\n";

           ss<<"auto &sl=stk."<<mrules[it->first][0]  <<";\n";
           ss<<"auto &sr=stk.m_children;\n";

           if( is_shortcut)
           {
               for(unsigned ri =1;ri<mrules[it->first].size();++ri)
               {
                   if(std::find(nterms.begin(), nterms.end(), mrules[it->first][ri])!=nterms.end()){
                       ss<<"auto &sr"<<ri-1<<"=stk.m_children["<<ri-1<< "]."<< mrules[it->first][ri]<<";\n";
                   }
               }
               for(unsigned ri =1;ri<mrules[it->first].size();++ri)
               {
                   if(EPS_SYM!=mrules[it->first][ri])
                   //if(std::find(nterms.begin(), nterms.end(), mrules[it->first][ri])!=nterms.end())
                   {
                        ss<<"auto &lr"<<ri-1<<"=tk.m_children["<<ri-1<< "];\n";
                   }
               }
           }



           std::string action_str = mactions[it->second];
           replace(action_str, "$1", "tk.m_children[0]");
           replace(action_str, "$2", "tk.m_children[1]");
           replace(action_str, "$3", "tk.m_children[2]");
           replace(action_str, "$4", "tk.m_children[3]");
           replace(action_str, "$5", "tk.m_children[4]");
           replace(action_str, "$6", "tk.m_children[5]");
           replace(action_str, "$7", "tk.m_children[6]");
           replace(action_str, "$$", "tk");

           replace(action_str, "yychar", "tk.m_ret");



           ss<<action_str<<"\n";
           ss<<"\n}\n";
           ss<<"\nbreak;\n";
       }
       ss<<"default:\n";
       ss<<"{if(tk.m_rule_index!=-1){std::cerr<<\"action error\"<<tk.m_rule_index<<\"\\n\";}}break;\n";

       ss<<"}\n";//finish switch
       ss<<"return 0;\n";
       ss<<"\n}\n";//func_name finish

       return ss.str();
   }



std::string bison_sample1::generate_actions_code(const std::vector<std::string > &mactions,
                                                 const std::map<int, int> &mbefore_action,
                                                 const std::map<int, int> &mafter_action,
                                                 const std::map<int, int> &mcompaction,
                                                 const std::vector< std::vector<std::string > > &mrules,
                                                 std::vector<std::string> &nterms)
{
    std::stringstream ss;

    ss<<generate_one_action_code("before_process_children",mbefore_action, mrules, mactions,nterms,1);
    ss<<generate_one_action_code("after_process_children",mafter_action, mrules, mactions,nterms,1);
    ss<<generate_one_action_code("comp_process_children",mcompaction, mrules, mactions,nterms);


    return ss.str();
}

std::string bison_sample1::generate_rules(const std::vector< std::vector<std::string > > &mrules, std::map<std::string, int> &materm_val,
                                          std::vector<std::string> &maction_table_x_str)
{
    std::stringstream ss;
    ss<<"std::vector< std::vector<std::string > > m_rules={\n";
    for(unsigned i=0;i<mrules.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<mrules[i].size();++j)
        {
            if(mrules[i][j]==EPS_SYM)continue;

            if (j!=0)ss<<",";
            ss<<"\""<<string_pack(mrules[i][j]) << "\"";
        }
        ss<<"},\n";
    }
    ss<<"};\n";

    {

        ss<<"std::vector< std::string > m_rule_name={\n";
        std::string last_name="";
        int rule_name_index=0;
        for(unsigned i=0;i<mrules.size();++i)
        {
            if(last_name!=mrules[i][0])
            {
                rule_name_index=0;
                last_name = mrules[i][0];
            }
            ss<<"\""<<mrules[i][0]<<":"<<rule_name_index<<"\",";

            rule_name_index++;
        }
        ss<<"};\n";
    }



    ss<<"std::vector< int > m_rule_reduce_ret={";
    for(unsigned i=0;i<mrules.size();++i)
    {
        if (i!=0)ss<<",";
        for(unsigned j=0;j<mrules[i].size();++j)
        {
            ss<<materm_val[mrules[i][0]];
            break;
        }
    }
    ss<<"};\n";


    ss<<"std::vector<int> m_char_vec={\n";
    for(unsigned char_idx=0;char_idx<maction_table_x_str.size();++char_idx)
    {
        ss<< materm_val[maction_table_x_str[char_idx]]<<",";
    }
    ss<<"};\n";

    ss<<"std::vector<std::string> m_char_str_vec={\n";
    for(unsigned char_idx=0;char_idx<maction_table_x_str.size();++char_idx)
    {
        ss<<"\""<< string_pack( maction_table_x_str[char_idx] )<<"\",";
    }
    ss<<"};\n";



    ss<<"std::map<int, int>  m_token_index={\n";
    for(unsigned char_idx=0;char_idx<maction_table_x_str.size();++char_idx)
    {
        ss<<"{"<<  materm_val[maction_table_x_str[char_idx]] <<", " << char_idx<<"},\n";
    }
    ss<<"};\n";



    ss<<"\n";
    ss<<"\n";
    return ss.str();
}

std::string bison_sample1::generate_action_table(int state_cnt, int terms_cnt, std::vector<std::vector<int> > &action_type, std::vector<std::vector<int> > &action_id)
{
    std::stringstream ss;

    ss << "int m_action_id["<< state_cnt<< "][" << terms_cnt<<  "]={\n";
    ///每个状态的跳转表
    ///
    for(unsigned i=0;i<action_id.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<action_id[i].size();++j)
        {
            if(j>0)ss<<",";
            ss<<action_id[i][j]<<" ";
        }
        ss<<"},\n";
    }
    ss<<"};\n";

    ss << "int m_action_type["<< state_cnt<< "][" <<terms_cnt<<  "]={\n";
    ///每个状态的跳转表
    ///
    for(unsigned i=0;i<action_type.size();++i)
    {
        ss<<"{";
        for(unsigned j=0;j<action_type[i].size();++j)
        {
            if(j>0)ss<<",";
            ss<<action_type[i][j]<<" ";
        }
        ss<<"},\n";
    }
    ss<<"};\n";

    return ss.str();
}

std::string bison_sample1::generate_middle_action(std::map<int, int > &mmiddle_action_len, const  std::map<int, std::set<int> > &mmiddle_action_state, const std::vector<std::string > &mactions)
{
    std::stringstream ss;


    ss<<"\nlex_token_type action_in_middle(int state_id, const std::deque<lex_token_type> token_stack)\n{\n";
    ss<<"lex_token_type _tk;\n";
    ss<< "_tk.m_ret="<<NULL_TOKEN<<";\n";

    ss<<"switch(state_id)\n{\n";//start of switch

    for(auto it=mmiddle_action_state.begin();it!=mmiddle_action_state.end();++it)
    {
        auto &state_list = it->second;
        for(auto it2=state_list.begin();it2!=state_list.end();++it2)
        {
            ss<<"case "<< *it2<<":\n";
        }
        ss<<"\n{\n";
        ss<<"std::vector<lex_token_type> v;\n";
        ss<<"v.assign(token_stack.end()-"<<mmiddle_action_len[it->first] <<", token_stack.end());\n";

        std::string action_str = mactions[it->first];
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












