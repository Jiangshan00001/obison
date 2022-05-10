#include <iostream>
#include "obisonfile.h"

OBisonFile::OBisonFile()
{

}

int OBisonFile::print_all()
{
    std::cout<<"m_def_includes:\n";
    std::cout<<m_def_includes<<"\n";
    std::cout<<"m_last_code:\n";
    std::cout<<m_last_code<<"\n";
    std::cout<<"m_start:\n";
    std::cout<<m_start<<"\n";

    std::cout<<"m_terms:\n";
    for(unsigned i=0;i<m_terms.size();++i)
    {
        std::cout<<m_terms[i]<<"\n";
    }
    std::cout<<"m_nterms:\n";
    for(unsigned i=0;i<m_nterms.size();++i)
    {
        std::cout<<m_nterms[i]<<"\n";
    }

    std::cout<<"rules:\n";
    for(unsigned i=0;i<m_rules.size();++i)
    {

        for(unsigned j=0;j<m_rules[i].size();++j)
        {

            if(m_is_action[i][j])
            {
                std::cout<<"<ACTION>"<< " ";
            }
            else
            {
                std::cout<<m_rules[i][j]<< " ";
            }
            if(j==0){
                std::cout<<"--->";
            }

        }
        std::cout<<"\n";
    }

    return 0;
}
