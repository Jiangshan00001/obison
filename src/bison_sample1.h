#ifndef BISON_SAMPLE1_H
#define BISON_SAMPLE1_H
#include <string>

class bison_sample1
{
public:
    bison_sample1();
    std::string render_def_header(std::string core_txt);
    std::string render_parser(std::string class_name, std::string core_txt);
};

#endif // BISON_SAMPLE1_H
