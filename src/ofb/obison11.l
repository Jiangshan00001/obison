D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{
#include "lex_token.h"
#define YYSTYPE lex_token

//https://lists.gnu.org/archive/html/help-bison/2009-08/msg00015.html
#include <stdio.h>
#include "bison_header.h"

void comment(void);
//int yyinput();
//int yylex();
////{L}({L}|{D})*[ \t\r\n]*":"		{ return(C_IDENTIFIER); }

%}

%%
"/*"			{ comment(); }
"//"[^\n]*              { /* consume //-comment */ }


"%token"			{ return(TOKEN); }
"%left"			{ return(LEFT); }
"%right"			{ return(RIGHT); }
"%type"			{ return(TYPE); }
"%start"			{ return(START); }
"%union"           { return(UNION); }
"%{"			{ return(LCURL); }
"%}"			{ return(RCURL); }
"%%"			{  return(MARK); }
[0-9][0-9]*			{ return(NUMBER); }


":" {return ':';}
"|" {return '|';}

"{" {return '{';}
"}" {return '}';}
";" {return ';';}
"<" {return '<';}
">" {return '>';}

{L}({L}|{D})*		{ return(IDENTIFIER); }


L?\"(\\.|[^\\"\n])*\"	{ return(IDENTIFIER); }
\'(\\.|[^\\"\n])(\\.|[^\\"\n])?\'	{ return(IDENTIFIER); }

"\'"."\'" {return IDENTIFIER;}

"\n"  {/* unmatched character \n*/ }

.			{ /* Add code to complain about unmatched characters */ }

%%

int yywrap(void)
{
	return 1;
}


void comment(void)
{
    int c, prev = 0;
  
    ;
    while ((c=input())!=0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
        //printf("comment:%c\n", c);
		prev = c;
	}
	yyerror("unterminated comment");
}


int column = 0;

