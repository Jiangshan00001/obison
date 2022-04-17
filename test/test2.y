%token a b c d
%type SS S A B 
%start SS

%%
SS :S
    ;
S : A a
	| b A c
	| B c
	| b B a
	;
A : d
	;
B  : d
	;
