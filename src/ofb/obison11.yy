/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/yacc.html */
/* Grammar for the input to yacc. */
/* Basic entries. */
/* The following are recognized by the lexical analyzer. */

%{
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

%}


%token    IDENTIFIER      /* Includes identifiers and literals */
%token    NUMBER          /* [0-9][0-9]* */


/* Reserved words : %type=>TYPE %left=>LEFT, and so on */


%token    LEFT RIGHT NONASSOC TOKEN TYPE START UNION


%token    MARK  FAKE_FINISH_MARK          /* The %% mark. */
%token    LCURL           /* The %{ mark. */
%token    RCURL           /* The %} mark. */


/* 8-bit character literals stand for themselves; */
/* tokens have to be defined for multi-byte characters. */


%start    spec


%%


spec  : defs MARK rules tail {} {} {}
      ;
tail  : MARK {
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
            yychar = FAKE_FINISH_MARK;
            /* In this action, set up the rest of the file. */
            }   FAKE_FINISH_MARK  { } {}

      | /* Empty; the second MARK is optional. */
      ;
defs  : /* Empty. */
        |    defs def {} {}
      ;
def   : START IDENTIFIER  {m_file.m_start = tk.m_children[1].m_yytext;} {} {}
      |    UNION '{'
           {
          /* Copy action, translate $$, and so on. */
            std::string curr_act = get_one_action();
            m_file.m_default_class_var=curr_act;
          lex_token_type tk("act", curr_act);
          $$=tk;
          yychar='}';//add RCURL to stack???

        }   '}'
      |    LCURL
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
        //$$=$1;
        /* Copy C code to output file. */
        lex_token_type tk("def_inc_code", include_code);
        $$= tk;
        yychar=RCURL;//add RCURL to stack???

        m_file.m_def_includes=include_code;

      }
        RCURL  {} {}
        |    rword tag nlist   {m_file.m_curr_rword.clear();} {
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
      ;
rword : TOKEN
      | LEFT
      | RIGHT
      | NONASSOC
      | TYPE
      ;
tag   :  /* Empty: union tag ID optional. */
      | '<'    IDENTIFIER      '>'
      ;
nlist : nmno
      | nlist nmno
      ;
nmno  : IDENTIFIER     {m_file.m_curr_rword.push_back(tk.m_children[0].m_yytext);}
      | IDENTIFIER NUMBER  /* Note: invalid with % type. */
      ;


/* Rule section */


rules : rule
        | rules  rule {m_file.m_curr_rule.clear(); m_file.m_curr_rule_is_action.clear();} {}
      ;
r_head : IDENTIFIER ':' {
                            m_file.m_curr_rule.clear();
                            m_file.m_curr_rule_is_action.clear();
                            m_file.m_curr_rule_left = tk.m_children[0].m_yytext;
                            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
                            m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
                            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

                            m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
                            m_file.m_curr_rule_is_action.push_back(0);
                        }
                    | IDENTIFIER '{' {

                        m_file.m_curr_rule.clear();
                        m_file.m_curr_rule_is_action.clear();
                        m_file.m_curr_rule_left = v[0].m_yytext;
                        m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
                        m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
                        m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

                        m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
                        m_file.m_curr_rule_is_action.push_back(0);

                        /* Copy action, translate $$, and so on. */
                          std::string curr_act = get_one_action();
                        m_file.m_nterm_class_var[m_file.m_curr_rule_left] = curr_act;
                        lex_token_type tk("act", curr_act);
                        $$=tk;
                        yychar='}';//add RCURL to stack???
                    }  '}'
                    ;



rule : rulemulti prec
      ;
rulemulti :   rulesingleline
         | rulemulti rulesingleline
         ;


rulesingleline  : r_head rbody   {
            m_file.m_curr_rule_left = tk.m_children[0].m_yytext;
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);
            m_file.m_curr_rule_left = trim1(m_file.m_curr_rule_left,':');
            m_file.m_curr_rule_left = trim(m_file.m_curr_rule_left);

            m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
            m_file.m_curr_rule_is_action.push_back(0);
            } {
                    m_file.m_rules.push_back(m_file.m_curr_rule);
                    m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                    m_file.m_curr_rule.clear();
                    m_file.m_curr_rule_is_action.clear();
                }
            | '|' rbody   {
                    m_file.m_curr_rule.push_back(m_file.m_curr_rule_left);
                    m_file.m_curr_rule_is_action.push_back(0);
                    }
                    {
                        m_file.m_rules.push_back(m_file.m_curr_rule);
                        m_file.m_is_action.push_back(m_file.m_curr_rule_is_action);

                        m_file.m_curr_rule.clear();
                        m_file.m_curr_rule_is_action.clear();
                    }

      ;
rbody : /* empty */
        | rbody IDENTIFIER {} {
                                m_file.m_curr_rule.push_back(tk.m_children[1].m_yytext);
                                m_file.m_curr_rule_is_action.push_back(0);
                            }
        | rbody acts {} {
                                unpack_acts(m_file.m_curr_rule, tk.m_children[1]);
                                while(m_file.m_curr_rule_is_action.size()<m_file.m_curr_rule.size())
                                {
                                    m_file.m_curr_rule_is_action.push_back(1);
                                }
                        }
        | rbody '<'
        | rbody '>'
      ;


acts: act          {}
     | acts act {}
     ;

act   : '{'
        {
		
          /* Copy action, translate $$, and so on. */
            std::string curr_act = get_one_action();
            std::cerr<<"action finish here\n";
          lex_token_type tk("act", curr_act);
          $$=tk;
          yychar='}';//add RCURL to stack???

          }

          '}'
      ;
prec  : ';'
      | prec ';'
      ;
	  
%% 

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






