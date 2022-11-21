# obison
oh, this is another bison implementation try using pure c++, so that it could be compile and use in windows/linux/macos smoothly.




http://gokcehan.github.io/notes/recursive-descent-parsing.html



https://tomassetti.me/why-you-should-not-use-flex-yacc-and-bison/

1.5 Writing GLR Parsers
https://www.gnu.org/software/bison/manual/bison.html#GLR-Parsers


https://www.tutorialspoint.com/compiler_design/compiler_design_bottom_up_parser.htm

```

token = next_token()

repeat forever
   s = top of stack
   
   if action[s, token] = “shift si” then
      PUSH token
      PUSH si 
      token = next_token()
      
   else if action[s, token] = “reduce A::= β“ then 
      POP 2 * |β| symbols
      s = top of stack
      PUSH A
      PUSH goto[s,A]
      
   else if action[s, token] = “accept” then
      return
      
   else
      error()
```




https://github.com/peter-winter/ctpg#lr1-parser

```
bool parse(input, sr_table[states_count][terms_count], goto_table[states_count][nterms_count])
   state = 0
   states.push(state)
   needs_term = true;

   while (true)
      if (needs_term)
         term_nr = get_next_term(input)
      entry = sr_table[state, term_nr]
      kind = entry.kind

      if (kind == success)
         return true

      else if (kind == shift)
         needs_term = true;
         state = entry.next
         states.push(state)
         continue

      else if (kind == reduce)
         states.pop_n(entry.rule_length)
         state = states.top()
         state = goto_table[state, entry.nterm_nr]
         continue

      else
         return false
		 
```



Create LR(1) parsers in your browser and see how they work.
https://soroushj.github.io/lr1-parser-vis/



https://blog.csdn.net/C666Lover/article/details/109258558

https://blog.csdn.net/killian0213/article/details/108478863



http://www.cs.ecu.edu/karl/5220/spr16/Notes/Bottom-up/lr1.html



项目集族

先构造LR（0）项目，再为为其配上搜索符

搜索符生成方法：上一项目集传播获得的


First集 --终结符的FIRST集是本身。非终结符，则包含直接的终结符。
Follow集
CLOSURE(I) -- 表示和I中项目可以识别同样活前缀的所有项目的集合。
GO(I,X)  --- 


miniyacc:
https://c9x.me/yacc/