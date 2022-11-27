#include <sstream>
#include <fstream>
#include "btoken_gen.h"
#include "replace.h"

int btoken_gen(std::string btaken_file, std::string btaken_class_name, std::string def_namespace,
                       std::map<std::string, std::string > nterm_class_var, std::string default_class_var,
                       std::map<std::string, int> aterm_val)
{
    std::stringstream ret;

    std::string guard_str = btaken_file;

    guard_str = replace(guard_str,".", "_");

    ret<<"#ifndef "<< guard_str<<"\n";
    ret<<"#define "<< guard_str<<"\n";

    ret<<"#include <vector>\n";
    ret <<"namespace "<< def_namespace<<"{\n";

    for(auto i=nterm_class_var.begin();i!=nterm_class_var.end();++i)
    {
        ret<<"class BToken_"<< i->first<<"\n";
        ret<< i->second;
        ret<<";\n";
    }

    ret<<"class "<< btaken_class_name<<"\n{\n";
    for(auto i=nterm_class_var.begin();i!=nterm_class_var.end();++i)
    {
        ret<<" BToken_"<< i->first<<" " << i->first<<";\n";
    }

    ret<<"public:\nstd::vector<"<<btaken_class_name <<"> m_children;\n";
    ret<<"};\n";

    ret<<"};// namespace "<<def_namespace<<" end\n";


    ret<<"#endif //"<< guard_str<<"\n";

    std::ofstream f;
    f.open(btaken_file);
    f<<ret.str();
    f.close();

    return 0;
}


