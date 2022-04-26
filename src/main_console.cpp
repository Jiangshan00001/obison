#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "bison_file_io.h"
#include "lalr1.h"
#include "argv.h"
#include "bison_sample1.h"

#ifndef QT_WID

int main(int argc, char *argv[])
#else
int main222(int argc, char *argv[])
#endif
{

    ArgsParser parse(argc, argv);



    if(parse.HaveOption('i'))
    {
        int is_debug = 0;
        std::string file_name = parse.GetOption('i');
        std::string def_file="bison_header.h";
        std::string parser_file="bison_parser.h";
        std::string def_namespace="";
        std::string class_name = "obison_sample";

        if(parse.HaveOption('d'))
        {
            is_debug=1;
        }
        if(parse.HaveOption('e'))
        {
            def_file=parse.GetOption('e');
        }
        if(parse.HaveOption('p'))
        {
            parser_file=parse.GetOption('p');
        }
        if(parse.HaveOption('n'))
        {
            def_namespace = parse.GetOption('n');
        }
        if(parse.HaveOption('c'))
        {
            class_name = parse.GetOption('c');
        }

        bison_file_io m_file;
        m_file.m_debug = is_debug;
        m_file.read(file_name);

        lalr1 m_lalr1;
        m_lalr1.generate_table(&m_file);

        std::ofstream ofile;
        ofile.open(def_file);
        ofile<<m_lalr1.get_def_file(def_namespace);
        ofile.close();

        bison_sample1 sample1;
        std::string ret = sample1.render_parser(class_name, m_lalr1.get_parser_file());
        ofile.open(parser_file);
        ofile<<ret;
        ofile.close();


        exit(0);

    }

    std::cout<<"usage: prog -i lex.l -e def_file.h -p parser_file.h -n namespace -c class_name\n";

    return 0;
}
