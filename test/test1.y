/*https://en.wikipedia.org/wiki/LR_parser*/



%start E
%%

E: B
	| E '+' B
	| E '*' B
	;
B : NUM
	;
NUM: '1'
	| '0'
	;