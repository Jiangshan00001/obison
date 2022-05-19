#include "otoken.h"
#include "bison_header.h"
#include "lex_header.h"
#include "bison_parser.h"
#include "argv.h"
#include "lalr1.h"
#include "bison_sample1.h"

int main(int argc, char* argv[])
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


        obison_sample<oflex_sample> m_bison;

        m_bison.m_oflex.m_is_debug = is_debug;
        m_bison.m_debug = is_debug;
        m_bison.set_file_name(file_name);

        auto parse_ret = m_bison.yyparse();

        //m_bison.m_file.print_all();
        std::cout<<"start process_top_down\n";
        m_bison.process_top_down(parse_ret);


        std::cout<<"start generate_table\n";
        lalr1 m_lalr1;
        m_lalr1.generate_table(&m_bison.m_file);

        bison_sample1 sample1;

        std::cout<<"generate_table finish!\n";

        std::ofstream ofile;
        ofile.open(def_file);
        auto aterm = m_lalr1.get_def_file();
        ofile<<sample1.render_def_header(aterm,def_namespace, "e_bison_head");
        ofile.close();

        std::string ret = sample1.render_parser(class_name,
                                                m_lalr1.m_rules, m_lalr1.m_aterm_val,
                                                m_bison.m_file.m_def_includes, m_bison.m_file.m_last_code,
                                                m_lalr1.m_actions, m_lalr1.m_closures,
                                                m_lalr1.m_action_id, m_lalr1.m_action_type,
                                                m_lalr1.m_action_table_x_str, m_lalr1.m_middle_action_len,
                                                m_lalr1.m_middle_action_state, m_lalr1.m_before_action, m_lalr1.m_after_action,
                                                m_lalr1.m_comp_action);
        ofile.open(parser_file);
        ofile<<ret;
        ofile.close();

        ofile.open("parser.dbg.txt");
        ofile<<m_lalr1.print_debug_info();
        ofile.close();


        exit(0);

    }

    std::cout<<"usage: prog -i lex.yy -e def_file.h -p parser_file.h -n namespace -c class_name\n";




    //OBisonFile m_file;
    //m_file.m_debug = 1;
    //m_file.read(file_name);

   // lalr1 m_lalr1;
    //m_lalr1.generate_table(&m_file);

    //std::ofstream ofile;
    //ofile.open("parser.dbg.txt");
    //ofile<<m_lalr1.print_debug_info();
    //ofile.close();
    return 0;
}
