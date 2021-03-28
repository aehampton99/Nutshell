%%
setenv|printenv|unsetenv|cd|alias|unalias|bye		return BUILTIN;
"."|".."|"\\"|"["|"]"|"~"					return META;
[A-Za-z]* 						return WORD;
-[A-Za-z]*						return FLAG;
" "|"\t"                  return WHITESPACE
.                               return UNDEFINED;
%%