
#ifndef lex_token_h
#define lex_token_h
#include <vector>
#include <string>
#include <iostream>

namespace obison_space
{

class lex_token
{
public:
   lex_token()
   {
      m_rule_index=-1;
      m_ret= is_eof=m_line=m_column=0;
      m_state_id=0;
   }
   lex_token(int typ, std::string ytext)
   {
      m_rule_index=-1;
      m_yytext=ytext;
      m_ret= is_eof=m_line=m_column=0;
      m_ret=typ;
        m_state_id=0;
   }
   lex_token(std::string typ_str, std::string ytext)
   {
      m_rule_index=-1;
      m_ret= is_eof=m_line=m_column=0;
      m_yytext=ytext;
      m_typestr=typ_str;
        m_state_id=0;
   }
   friend std::ostream& operator<<(std::ostream&out, const lex_token & a)
   {
    out<<a.m_yytext<<"\n";
    return out;
   }

   std::string m_yytext;
   int m_ret;
   int m_rule_index;//only use when m_ret is non-term
   std::string m_typestr;
   int is_eof;
   int m_line;
   int m_column;

   int m_state_id;
   std::vector<lex_token> m_children;
   int m_val;
};

};//obison_space

#endif

                   