#ifndef BTOKEN_GEN_H
#define BTOKEN_GEN_H
#include <string>
#include <map>

int btoken_gen(std::string btaken_file, std::string btaken_class_name, std::string def_namespace,
                       std::map<std::string, std::string > nterm_class_var, std::string default_class_var,
                       std::map<std::string, int> aterm_val);

#endif // BTOKEN_GEN_H
