/*******************************************************************************
 *
 * $Id: snet.l 3371 2012-02-13 15:32:29Z mvn $
 *
 * Author: Kari Keinanen, VTT Technical Research Centre of Finland
 * -------
 *
 * Date:   05.01.2007
 * -----
 *
 * Description:
 * ------------
 *
 * Scanner of S-NET compiler.
 *
 *******************************************************************************/

%{

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "node_basic.h"
#include "str.h"
#include "free.h"
#include "memory.h"
#include "snet.tab.h"
#include "globals.h"
#include "ctinfo.h"
#include "bool.h"

#define LINEBUF_SIZE 256
#define YY_NO_INPUT

extern int yyerror(char *error);
extern void YYparseError(const char *f, ...);
extern int linenum; /* token location */
extern int charpos; /* token location */

/* scanner position, which will be after token's because of look-ahead */
static int mycharpos = 1;
static int old_linenum = 0;
static char* old_pathname = NULL;
static char* old_filename = NULL;

#define STATE_METADATA_ALLOWED            0
#define STATE_WAIT_SEMICOLON              1
#define STATE_WAIT_SEMICOLON_OR_RBRACE    2
#define STATE_WAIT_SEMICOLON_OR_LBRACE    3
#define STATE_IN_FILTER                   5
#define STATE_IN_PARENTHESIS              6
#define STATE_IN_STAGE_ID                 7

/* State variable for telling when METADATA-elements are allowed in the code */
static unsigned int state = STATE_METADATA_ALLOWED;

/* Used to decide if metadata value can be parsed from between two metadata tags*/
static bool accept_metadata_value = FALSE;

/* for str and cint token types */
#define FILTER(token){ \
    charpos = mycharpos; \
    mycharpos += yyleng; \
    return token; \
  }

/* for loc token type */
#define FILTER2(token){ \
    yylval.loc.line = linenum; \
    yylval.loc.col  = mycharpos; \
    FILTER(token); \
  }

%}
   /* set positions to 6000 */
%p 6000
   /* set nodes to 1100 */
%e 1100
%option noyywrap
%option stack
%option noyy_top_state
   /* Suppress compilation warning yyunput defined but not used */
%option nounput
%option reject

  /* Lexer states */

%x IN_SNET_BOXLANG
    /* Inside multiline comment (/ * ... * /) */
%x IN_SNET_COMMENT
    /* Inside single line comment (// ... \n) */
%x IN_SNET_COMMENTLINE
    /* Inside SNet box code (Ends before ">>>") */
%x IN_SNET_BOXCODE
    /* Inside metadata definition */
%x IN_METADATA
    /* Inside metadata value */
%x IN_MD_QUOTED_VALUE
%x IN_MD_DATA_VALUE

%%


<IN_SNET_COMMENT>{
 "*/"         |
 "*"+"/"      {
                mycharpos += yyleng;
                yy_pop_state();
              }

 [^/*\n]*      |
 "*"+[^*/\n]* {
                mycharpos += yyleng;
              }
 "/"+[^*\n]*  {
                mycharpos += yyleng;
              }
 "/*" {
                yy_push_state(IN_SNET_COMMENT);
                mycharpos += yyleng;
              }

 [\n]         {
                mycharpos = 1;
                linenum++;
              }
}


<IN_SNET_COMMENTLINE>{
 [^\n] {
         mycharpos += yyleng;
       }

 [\n]  {
         mycharpos = 1;
         linenum++;
         yy_pop_state();
       }
}


<IN_SNET_BOXCODE>{
 ">>>"       {
               yy_pop_state();
               yyless(yyleng - 3);
               yylval.str = STRcpy(yytext);
               mycharpos = (strlen(yylval.str) - (strrchr(yylval.str, '\n') + 1 - yylval.str));
               charpos = mycharpos;
               return CODE;
             }

 [^>\n]*     |
 ">"[^>\n]*  |
 ">>"[^>\n]* {
               yymore();
             }

 [\n]        {
               mycharpos = 1;
               linenum++;
               yymore();
             }
}


<IN_SNET_BOXLANG>"|" {
                 BEGIN(IN_SNET_BOXCODE);
                 FILTER2(VERTICAL_BAR);
               }


<*>{
 [ \t] {
         mycharpos += yyleng;
       }

 [\n]  {
         mycharpos = 1;
         linenum++;
       }
}

<IN_METADATA>{
 "</metadata>"   {
                 yy_pop_state();
                 FILTER2(METADATA_END);
                }

 "<boxdefault>" {
            FILTER2(METADATA_BOXDEFAULT_BEGIN);
          }

 "<netdefault>" {
            FILTER2(METADATA_NETDEFAULT_BEGIN);
          }

 "<box " {
         FILTER2(METADATA_BOX_BEGIN);
      }

 "<net " {
         FILTER2(METADATA_NET_BEGIN);
      }

 "<default>" {
            FILTER2(METADATA_DEFAULT_BEGIN);
          }

  "</"  {
         accept_metadata_value = FALSE;
         FILTER2(METADATA_ENDTAG_START);
      }

 "</boxdefault>" {
            FILTER2(METADATA_BOXDEFAULT_END);
          }

 "</netdefault>" {
            FILTER2(METADATA_NETDEFAULT_END);
          }

 "</box>" {
         FILTER2(METADATA_BOX_END);
      }

 "</net>" {
         FILTER2(METADATA_NET_END);
      }

 "</default>" {
            FILTER2(METADATA_DEFAULT_END);
        }

 "<"   {
         accept_metadata_value = TRUE;
         FILTER2(METADATA_TAG_START);
      }

 ">"   {
         if(accept_metadata_value == TRUE) {
	   yy_push_state(IN_MD_DATA_VALUE);
         }
         FILTER2(METADATA_TAG_END);
      }

 "/>"  {
         accept_metadata_value = FALSE;
         FILTER2(METADATA_TAG_SHORT_END);
      }

 "name" {
         FILTER2(NAME);
       }

 "value" {
         FILTER2(VALUE);
        }

 "=" {
      FILTER2(EQUAL_SIGN);
    }

 "\"" {
       yy_push_state(IN_MD_QUOTED_VALUE);
       FILTER2(DOUBLE_QUOTE);
     }

 "/" {
        FILTER2(DIVISION);
     }
}

<IN_MD_DATA_VALUE>{

[^< \t\n\"]|([^< \t\n\"][^<]*[^< \t\n\"]) {
          yylval.str = STRcpy(yytext);
	  yy_pop_state();
	  accept_metadata_value = FALSE;
          FILTER(MD_VALUE);
       }

"</"  {	
         yy_pop_state();
	 accept_metadata_value = FALSE;
         FILTER2(METADATA_ENDTAG_START);
      }

}

<IN_MD_QUOTED_VALUE>{
[^\"<]+ {
          yylval.str = STRcpy(yytext);
          FILTER(MD_VALUE);
       }

 "\""  {
          yy_pop_state();
          FILTER2(DOUBLE_QUOTE);
       }
}

<INITIAL>{

"<metadata>"    {
          if(state == STATE_METADATA_ALLOWED) {
            yy_push_state(IN_METADATA);
            FILTER2(METADATA_BEGIN);
          } else {
	    REJECT;
	  }
        }

"<"     {
          FILTER2(LESS_THAN);
        }

#\ *[0-9]+\ \".*\"\n  {
          /* This rule only removes C preprocessor directives.
           * The same could be done with gcc argument -P, but for
           * some unknown reason this adds newlines at the end of
           * the file, which affect line numbers registered by snetc.
           */
          }

"//#INCLUDE_BEGIN "[^\n\t ]+"\n" {
         mycharpos = 1;

         /* This code part changes global names, so that node made from code parts
          * included from different files have right position information.
          */

         old_pathname = global.pathname;
         old_linenum = linenum;
         old_filename = global.filename;

         char *c = strchr(yytext, ' ');
         if(c != NULL) {
           global.pathname = STRcpy(++c);
           *(global.pathname + strlen(global.pathname) - 1) = '\0';
         }

         global.filename = strrchr(global.pathname, '/');
         if (global.filename == NULL) {
           global.filename = global.pathname;
         }
         else {
           global.filename += 1;
         }

         linenum = 1;
      }

"//#INCLUDE_END\n" {
             mycharpos = 1;
       int i = 0;

       /* These need to be stored for error reporting purposes */

       while(i < global.mdfc + 1) {
         if(global.mdfv[i] != NULL) {
           i++;
         }
         else {
           global.mdfv[i] = global.pathname;
         break;
         }
       }

       linenum = old_linenum;
       global.pathname = old_pathname;
       global.filename = old_filename;
  }


 "/*"   {
          mycharpos += yyleng;
          yy_push_state(IN_SNET_COMMENT);
        }

 "//!"  {
          state = STATE_IN_STAGE_ID;
          FILTER2(SNET_COMMENT);
        }

 "//"   {
          mycharpos += yyleng;
          yy_push_state(IN_SNET_COMMENTLINE);
        }

 "{" {
        if(state == STATE_WAIT_SEMICOLON_OR_LBRACE) {
    state = STATE_METADATA_ALLOWED;
  }
  FILTER2(OPEN_BRACE);
     }

 ";" {
       if(state == STATE_WAIT_SEMICOLON
    || state == STATE_WAIT_SEMICOLON_OR_RBRACE
    || state == STATE_WAIT_SEMICOLON_OR_LBRACE) {
         state = STATE_METADATA_ALLOWED;
       }
       FILTER2(SEMI_COLON);
     }

 "}"   {
          if(state == STATE_WAIT_SEMICOLON_OR_RBRACE) {
           state = STATE_METADATA_ALLOWED;
    }
    if(state == STATE_METADATA_ALLOWED) {
      state = STATE_WAIT_SEMICOLON;
    }
    FILTER2(CLOSE_BRACE);
       }

 "<<<" {
         yy_push_state(IN_SNET_BOXLANG);
         FILTER2(TRIPLE_LESS_THAN);
       }

 ">>>" {
         FILTER2(TRIPLE_GREATER_THAN);
       }

 "("  {
        if(state == STATE_WAIT_SEMICOLON_OR_LBRACE) {
          state = STATE_IN_PARENTHESIS;
        }
        FILTER2(OPEN_PARENTHESIS);
      }

 ")"  {
        if(state == STATE_IN_PARENTHESIS) {
          state = STATE_WAIT_SEMICOLON_OR_LBRACE;//LBRACE
        }
        FILTER2(CLOSE_PARENTHESIS);
      }

 "|]" {
        FILTER2(VERTICAL_BAR_CLOSE_BRACKET);
      }

 "|"  {
        FILTER2(VERTICAL_BAR);
      }

 ","  {
        FILTER2(COMMA);
      }

 ">"  {
        FILTER2(GREATER_THAN);
      }

 "#"  {
        FILTER2(NUMBER_SIGN);
      }

 "\\" {
        FILTER2(BACKSLASH);
      }

 "->" {
        FILTER2(RIGHT_ARROW);
      }

 "::" {
        FILTER2(DOUBLE_COLON);
      }

 "snet"    {
             FILTER2(SNET);
           }

 "disp"    {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(DISP);
             }
             else {
               yylval.str = STRcpy("disp");
               FILTER(IDENTIFIER);
             }
           }

 "dcr"     {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(DCR);
             }
             else {
               yylval.str = STRcpy("dcr");
               FILTER(IDENTIFIER);
             }
           }

 "tcheck"  {
           if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(TCHECK);
             }
             else {
               yylval.str = STRcpy("tcheck");
               FILTER(IDENTIFIER);
             }
           }

 "boxex"   {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(BOXEX);
             }
             else {
               yylval.str = STRcpy("boxex");
               FILTER(IDENTIFIER);
             }
           }

 "tres"    {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(TRES);
             }
             else {
               yylval.str = STRcpy("tres");
               FILTER(IDENTIFIER);
             }
           }

 "core"    {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(CORE);
             }
             else {
               yylval.str = STRcpy("core");
               FILTER(IDENTIFIER);
             }
           }

 "flat"    {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(FLAT);
             }
             else {
               yylval.str = STRcpy("flat");
               FILTER(IDENTIFIER);
             }
           }

 "siginfd"  {
                if (state == STATE_IN_STAGE_ID) {
                  state = STATE_METADATA_ALLOWED;
                  FILTER2(SIGINFD);
                }
                else {
                  yylval.str = STRcpy("siginfd");
                  FILTER(IDENTIFIER);
                }
              }

 "routed"   {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(ROUTED);
             }
             else {
               yylval.str = STRcpy("routed");
               FILTER(IDENTIFIER);
             }
           }

 "mulinfd" {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(MULINFD);
             }
             else {
               yylval.str = STRcpy("mulinfd");
               FILTER(IDENTIFIER);
             }
           }

 "opt"     {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(OPT);
             }
             else {
               yylval.str = STRcpy("opt");
               FILTER(IDENTIFIER);
             }
           }

 "tclned"  {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(TCLNED);
             }
             else {
               yylval.str = STRcpy("tclned");
               FILTER(IDENTIFIER);
             }
           }

 "ptran"   {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(PTRAN);
             }
             else {
               yylval.str = STRcpy("ptran");
               FILTER(IDENTIFIER);
             }
           }

 "disam"   {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(DISAM);
             }
             else {
               yylval.str = STRcpy("disam");
               FILTER(IDENTIFIER);
             }
           }

 "ploc"    {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(PLOC);
             }
             else {
               yylval.str = STRcpy("ploc");
               FILTER(IDENTIFIER);
             }
           }

 "netren"  {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(NETREN);
             }
             else {
               yylval.str = STRcpy("netren");
               FILTER(IDENTIFIER);
             }
           }

 "final"   {
             if (state == STATE_IN_STAGE_ID) {
               state = STATE_METADATA_ALLOWED;
               FILTER2(FINAL);
             }
             else {
               yylval.str = STRcpy("final");
               FILTER(IDENTIFIER);
             }
           }

 "box"     {
             state = STATE_WAIT_SEMICOLON_OR_RBRACE;
             FILTER2(BOX);
           }

 "net"     {
             state = STATE_WAIT_SEMICOLON_OR_LBRACE;
             FILTER2(NET);
           }

 "type"    {
             state = STATE_WAIT_SEMICOLON;
             FILTER2(TYPE);
           }

 "typesig" {
             state = STATE_WAIT_SEMICOLON;
             FILTER2(TYPESIG);
           }

"connect" {
             state = STATE_WAIT_SEMICOLON;
             FILTER2(CONNECT);
         }

 "["    {
          if(state == STATE_WAIT_SEMICOLON)
            state = STATE_IN_FILTER;
          FILTER2(OPEN_BRACKET);
        }

 "]"    {
          if(state == STATE_IN_FILTER)
             state = STATE_WAIT_SEMICOLON;
          FILTER2(CLOSE_BRACKET);
        }

 ":"    {
          FILTER2(COLON);
        }

 "="    {
          FILTER2(EQUAL_SIGN);
        }

 "*"    {
          FILTER2(ASTERISK);
        }

 "!"    {
          FILTER2(EXLAMATION_POINT);
        }

 "[|"   {
          FILTER2(OPEN_BRACKET_VERTICAL_BAR);
        }

 ".."   {
          FILTER2(DOUBLE_PERIOD);
        }

 "..."  {
          FILTER2(TRIPLE_PERIOD);
        }

 "**"   {
          FILTER2(DOUBLE_ASTERISK);
        }

 "||"   {
          FILTER2(DOUBLE_VERTICAL_BAR);
        }

 "!!"   {
          FILTER2(DOUBLE_EXLAMATION_POINT);
        }

 "if"   {
          FILTER2(IF);
        }

 "else" {
          FILTER2(ELSE);
        }

 "then" {
          FILTER2(THEN);
        }

 "abs"  {
          FILTER2(ABSOLUTE_VALUE);
        }

 "/"    {
          FILTER2(DIVISION);
        }

 "%"    {
          FILTER2(REMAINDER);
        }

 "+"    {
          FILTER2(ADDITION);
        }

 "-"    {
          FILTER2(SUBTRACTION);
        }

 "min"  {
          FILTER2(MINIMUM);
        }

 "max"  {
          FILTER2(MAXIMUM);
        }

 "=="   {
          FILTER2(DOUBLE_EQUAL);
        }

 "!="   {
          FILTER2(NOT_EQUAL);
        }

 "<="   {
          FILTER2(LESS_THAN_OR_EQUAL);
        }

 ">="   {
          FILTER2(GREATER_THAN_OR_EQUAL);
        }

 "&&"   {
          FILTER2(DOUBLE_AND);
        }

 "?"    {
          FILTER2(QUESTION_POINT);
        }

 "@"    {
          FILTER2(AT);
        }

[1-9][0-9]*                { char *str;
                             sscanf( yytext, "%i", &yylval.cint);
                             str = STRitoa( yylval.cint);
                             if (!STReqci( str, yytext)) {
			       YYparseError("Decimal constant not representable as integer: %s",
					    yytext);
                             }
                             MEMfree( str);
                             FILTER( INTEGER);
                           }

"0"                        { yylval.cint = 0;
                             FILTER( INTEGER);
                           }

[0][1-7][0-7]*             { char *str;
                             sscanf( yytext, "%i", &yylval.cint);
                             str = STRitoa_oct( yylval.cint);
                             if (!STReqci( str, yytext)) {
			       YYparseError("Octal constant not representable as integer: %s",
					    yytext);
                             }
                             MEMfree( str);
                             FILTER( INTEGER);
                           }

0[xX][0-9a-fA-F]+          { char *str;
                             sscanf( yytext, "%i", &yylval.cint);
                             str = STRitoa_hex( yylval.cint);
                             if (!STReqci( str, yytext)) {
			       YYparseError("Hexadecimal constant not representable as integer: %s",
					    yytext);
                             }
                             MEMfree( str);
                             FILTER( INTEGER);
                           }

}


<INITIAL,IN_SNET_BOXLANG,IN_METADATA>{

[A-Za-z](_?[A-Za-z0-9])*_? {
                            yylval.str = STRcpy(yytext);
                            FILTER(IDENTIFIER);
                          }

[_a-zA-Z][_a-zA-Z0-9]*  {
                          if (global.compiler_phase!=PH_parse) {
                            yylval.str = STRcpy(yytext);
                            FILTER(IDENTIFIER);
                          }
                          else {
                            YYparseError("Illegal identifier %s", yytext);
                          }
                        }
}

<*>.      {
              YYparseError("Invalid character %s", yytext);
          }

%%
