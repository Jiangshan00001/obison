#include "btoken_gen.h"
#include "bison_header.h"
#include "lex_header.h"
#include "bison_parser.h"
#include "argv.h"
#include "lalr1.h"
#include "bison_sample1.h"
using namespace obison_space;


/**
* @defgroup 命令行使用说明
* 输入yy文件，输出2个文件，一个是term的定义文件，一个是解析代码文件。文件是c++格式，都在特定的命名空间中。
* usage: obison -i lex.yy -e def_file.h -p parser_file.h -n namespace -c class_name -b btaken_file -t btaken_class_name
* 参数说明 -i 输入的yy文件 -e 输出的terms定义文件 -p 解析代码文件 -n 命名空间 -c 解析代码类名称
*
*
*
* @defgroup 输入文件格式
* 输入文件格式：
* A { /** 此处是A需要解析的输出变量  / }
*  : A1 '+' A1
* ;
* A1
* ;


*/


int main(int argc, char* argv[])
{


    ArgsParser parse(argc, argv);



    if(parse.HaveOption('i'))
    {
        int is_debug = 0;
        std::string file_name = parse.GetOption('i');
        std::string def_file="bison_header.h";
        std::string parser_file="bison_parser.h";
        std::string def_namespace="bison_space";
        std::string class_name = "obison_sample";
        //20221127 添加新文件
        std::string btaken_file="bison_taken.h";
        std::string btaken_class_name = "BTaken";


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
        if(parse.HaveOption('b'))
        {
            btaken_file = parse.GetOption('b');
        }
        if(parse.HaveOption('t'))
        {
            btaken_class_name = parse.GetOption('t');
        }




        obison_sample<oflex_sample> m_bison;

        m_bison.m_oflex.m_is_debug = is_debug;
        m_bison.m_debug = is_debug;
        m_bison.set_file_name(file_name);

        auto parse_ret = m_bison.yyparse();

        //m_bison.m_file.print_all();
        std::cout<<"start process_top_down\n";
        obison_sample<oflex_sample>::syntax_taken_type stk;

        m_bison.process_top_down(parse_ret,stk);


        std::cout<<"start generate_table\n";
        lalr1 m_lalr1;
        m_lalr1.generate_table(&m_bison.m_file);

        bison_sample1 sample1;

        std::cout<<"generate_table finish!\n";

        std::ofstream ofile;
        ofile.open(def_file);
        auto aterm = m_lalr1.get_def_file();
        ofile<<sample1.render_def_header(aterm,def_namespace, "e_bison_head", m_lalr1.m_terms);
        ofile.close();

        std::string ret = sample1.render_parser(class_name,
                                                m_lalr1.m_rules, m_lalr1.m_aterm_val,
                                                m_bison.m_file.m_def_includes, m_bison.m_file.m_last_code,
                                                m_lalr1.m_actions, m_lalr1.m_closures,
                                                m_lalr1.m_action_id, m_lalr1.m_action_type,
                                                m_lalr1.m_action_table_x_str, m_lalr1.m_middle_action_len,
                                                m_lalr1.m_middle_action_state, m_lalr1.m_before_action, m_lalr1.m_after_action,
                                                m_lalr1.m_comp_action,def_namespace,
                                                btaken_file,btaken_class_name);

        ofile.open(parser_file);
        ofile<<ret;
        ofile.close();

        ofile.open("parser.dbg.txt");
        ofile<<m_lalr1.print_debug_info();
        ofile.close();


        btoken_gen(btaken_file, btaken_class_name,def_namespace,
                   m_bison.m_file.m_nterm_class_var, m_bison.m_file.m_default_class_var,
                   m_lalr1.m_aterm_val);

        exit(0);

    }

    std::cout<<"usage: prog -i lex.yy -e def_file.h -p parser_file.h -n namespace -c parser_class_name -t btaken_class_name -b btaken_file.h\n";

    return 0;
}
