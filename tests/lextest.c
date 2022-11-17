
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#include "lex.h"

tMorph Morph={0};

int main(int argc, void*argv[])
{
  initLex(argv[1]);
  do
  {
    lex();
    printf("Line%4d, Col%3d: ",Morph.posLine, Morph.posCol);
    switch(Morph.mc)
    {
       case mcSymb :
	    if (Morph.Val.Symb==tErg)   printf("Symbol,:=\n");    else
	    if (Morph.Val.Symb==tle )   printf("Symbol,<=\n");    else
	    if (Morph.Val.Symb==tge )   printf("Symbol,>=\n");    else
	    if (Morph.Val.Symb==tBGN)   printf("Symbol,_BEGIN\n");else
	    if (Morph.Val.Symb==tCLL)   printf("Symbol,_CALL\n"); else
	    if (Morph.Val.Symb==tCST)   printf("Symbol,_CONST\n");else
	    if (Morph.Val.Symb==tDO )   printf("Symbol,_DO\n");   else
	    if (Morph.Val.Symb==tEND)   printf("Symbol,_END\n");  else
	    if (Morph.Val.Symb==tIF )   printf("Symbol,_IF\n");   else
	    if (Morph.Val.Symb==tODD)   printf("Symbol,_ODD\n");  else
	    if (Morph.Val.Symb==tPRC)   printf("Symbol,_PROCEDURE\n");else
	    if (Morph.Val.Symb==tTHN)   printf("Symbol,_THEN\n"); else
	    if (Morph.Val.Symb==tVAR)   printf("Symbol,_VAR\n");  else
	    if (Morph.Val.Symb==tWHL)   printf("Symbol,_WHILE\n"); else 

            if (isprint(Morph.Val.Symb))
                printf("Symbol,%c\n",(char)Morph.Val.Symb);
            else 
                printf("undefined2: %d",Morph.mc);
				break;
       case mcNum :
            printf("Zahl  ,%ld\n",Morph.Val.Num);
				break;
       case mcIdent:
	    printf("Ident ,%s\n",(char*)Morph.Val.pStr);
				break;
       case mcEmpty:
                puts("war leer");
                break;
       default:
                printf("undefined: %d",Morph.mc);
    }
  }while (!(Morph.mc==mcSymb && Morph.Val.Symb==-1)) ;
  puts("");
  return 0;
}



