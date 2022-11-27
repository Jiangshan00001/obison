
all:
	../../../obison/build/src/obison.exe -i ./obison11.yy -p bison_parser.h -e bison_header.h -c obison_sample -n obison_space -b bison_token.h -t BToken
	../../../oflex/build/src/oflex.exe  -i obison11.l  -o lex_header.h -t lex_token.h -p oflex_sample -k lex_token -n obison_space
