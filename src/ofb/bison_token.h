#ifndef bison_token_h
#define bison_token_h
#include <vector>
namespace obison_space{
//default class
class BToken_act
{};
class BToken_acts
{};
class BToken_def
{};
class BToken_defs
{};
class BToken_nlist
{};
class BToken_nmno
{};
class BToken_prec
{};
class BToken_r_head
{};
class BToken_rbody
{};
class BToken_rule
{};
class BToken_rulemulti
{};
class BToken_rules
{};
class BToken_rulesingleline
{};
class BToken_rword
{};
class BToken_spec
{};
class BToken_start_20220422_start
{};
class BToken_tag
{};
class BToken_tail
{};
class BToken
{
public:
 BToken_act act;
 BToken_acts acts;
 BToken_def def;
 BToken_defs defs;
 BToken_nlist nlist;
 BToken_nmno nmno;
 BToken_prec prec;
 BToken_r_head r_head;
 BToken_rbody rbody;
 BToken_rule rule;
 BToken_rulemulti rulemulti;
 BToken_rules rules;
 BToken_rulesingleline rulesingleline;
 BToken_rword rword;
 BToken_spec spec;
 BToken_start_20220422_start start_20220422_start;
 BToken_tag tag;
 BToken_tail tail;

std::vector<BToken> m_children;
};
};// namespace obison_space end
#endif //bison_token_h
