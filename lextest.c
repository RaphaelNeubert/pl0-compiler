
#include <stdio.h>
#include <ctype.h>

#include "lex.h"

tMorph Morph={0};
int main(int argc, char** argv)
{
    initLex(argv[1]);
    do {
        lex();
        printf("Line%4d, Col%3d: ",Morph.posLine, Morph.posCol);
        switch(Morph.mc)
        {
            case mcSymb :
                if (isprint(Morph.Val.Symb))
                    printf("Symbol,%c\n",(char)Morph.Val.Symb);
                break;
            case mcNum :
                printf("Zahl  ,%ld\n",Morph.Val.Num);
                break;
            case mcIdent:
                printf("Ident ,%s\n",(char*)Morph.Val.pStr);
                break;
            case mcEmpty:
                puts("its Empty");
                break;
        }
    } while (!(Morph.mc==mcSymb && Morph.Val.Symb==-1)) ;
    puts("");
    return 0;
}

//int main(int argc, void*argv[])
//{
//  initLex(argv[1]);
//  do
//  {
//    Lex();
//    printf("Line%4d, Col%3d: ",Morph.PosLine, Morph.PosCol);
//    switch(Morph.MC)
//    {
//       case mcSymb :
//	    if (Morph.Val.Symb==zErg)   printf("Symbol,:=\n");    else
//	    if (Morph.Val.Symb==zle )   printf("Symbol,<=\n");    else
//	    if (Morph.Val.Symb==zge )   printf("Symbol,>=\n");    else
//	    if (Morph.Val.Symb==zBGN)   printf("Symbol,_BEGIN\n");else
//	    if (Morph.Val.Symb==zCLL)   printf("Symbol,_CALL\n"); else
//	    if (Morph.Val.Symb==zCST)   printf("Symbol,_CONST\n");else
//	    if (Morph.Val.Symb==zDO )   printf("Symbol,_DO\n");   else
//	    if (Morph.Val.Symb==zEND)   printf("Symbol,_END\n");  else
//	    if (Morph.Val.Symb==zIF )   printf("Symbol,_IF\n");   else
//	    if (Morph.Val.Symb==zODD)   printf("Symbol,_ODD\n");  else
//	    if (Morph.Val.Symb==zPRC)   printf("Symbol,_PROCEDURE\n");else
//	    if (Morph.Val.Symb==zTHN)   printf("Symbol,_THEN\n"); else
//	    if (Morph.Val.Symb==zVAR)   printf("Symbol,_VAR\n");  else
//	    if (Morph.Val.Symb==zWHL)   printf("Symbol,_WHILE\n");
//
//            if (isprint(Morph.Val.Symb))printf("Symbol,%c\n",(char)Morph.Val.Symb);
//				break;
//       case mcNum :
//            printf("Zahl  ,%ld\n",Morph.Val.Numb);
//				break;
//       case mcIdent:
//	    printf("Ident ,%s\n",(char*)Morph.Val.pStr);
//				break;
//    }
//  }while (!(Morph.MC==mcSymb && Morph.Val.Symb==-1)) ;
//  puts("");
//  return 0;
//}



