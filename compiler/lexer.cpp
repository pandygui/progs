#include <cstdio>
#include <string>
#include <cctype>

#include <iostream>
#include <vector>
#include <deque>

#include "mytype.h"
#include "lexer.h"

using namespace std;

//#define DEBUG_LEXER_MSG

// printable ascii, but not (, )
static inline int isgraph_ex(int c) 
{
#if 1
  if (c == '(')
    return 0;
  if (c == ')')
    return 0;
#endif
  return isgraph(c);
}

int la=-1; // look ahead

int getchar_la()
{
  if (la != -1)
  {
    int tmp=la;
    la = -1;
    return tmp;
  }
  else
    return getchar();
}

int get_token(string &token)
{
  int c;

  do
  {
    c = getchar_la();
  }while(isspace(c));

  if (c == EOF)
    return EOF;
  else if (isdigit(c))
       {
         do
         {
           token.push_back(c); 
           c = getchar_la();
         }while(isdigit(c));
       }
       else if (isalpha(c))
            {
              do
              {
                token.push_back(c); 
                c = getchar_la();
              } while(isalnum(c));
            }
            else if (c == '=')
                 {
                   c = getchar_la();
                   if (c == '=')
                   {
                     token = "==";
                   }
                   else
                   {
                     la = c;
                     token = "=";
                   }
                   return OK;
                 }
                 else
                 {
                   return ERR;
                 }
  if (c != EOF)
    la = c;
  return OK;
}

int get_se_token(string &token)
{
  int c;

  do
  {
    c = getchar_la();
  }while(isspace(c));

  if (c == EOF)
    return EOF;
  else if (c == '(')
       {
         token = '(';
         return OK;
       }
       else if (c == ')')
            {
              token = ')';
              return OK;
            }
            else if (isgraph_ex(c)) // printable ascii, but not (, )
                 {
                   do
                   {
                     token.push_back(c); 
                     c = getchar_la();
                   }while(isgraph_ex(c));
                 }
                 else
                 {
                   return ERR;
                 }


  if (c != EOF)
    la = c;
  return OK;
}

// 1+2
int get_token(Token &token)
{
  int c;

  do
  {
    c = getchar_la();
#ifdef GET_EOL
  }while(c == ' ');
#else
  }while(isspace(c));
#endif

  if (c == EOF)
    return EOF;
  else if (isdigit(c))
       {
         do
         {
           token.str_.push_back(c);
           c = getchar_la();
         }while(isdigit(c));
         token.type_ = NUMBER;
       }
       else if (isalpha(c))
            {
              do
              {
                token.str_.push_back(c); 
                c = getchar_la();
              } while(isalnum(c));
              token.type_ = NAME;
            }
            else
       {
         switch (c)
         {
           case '<':
           {
             token.str_ = "<";
             token.type_ = LESS;
             break;
           }
           case '>':
           {
             token.str_ = ">";
             token.type_ = GREAT;
             break;
           }
           case '+':
           {
             token.str_ = "+";
             token.type_ = ADD;
             break;
           }
           case '-':
           {
             token.str_ = "-";
             token.type_ = MIN;
             break;
           }
           case '*':
           {
             token.str_ = "*";
             token.type_ = MUL;
             break;
           }
           case '/':
           {
             token.str_ = "/";
             token.type_ = DIV;
             break;
           }
#ifdef GET_EOL
           case '\n':
           {
             token.str_ = "\n";
             token.type_ = EOL;
             break;
           }
#endif
           case ',':
           {
             token.str_ = ",";
             token.type_ = SEP;
             break;
           }
           case ';':
           {
             token.str_ = ";";
             token.type_ = SEP;
             break;
           }
           case '{':
           {
             token.str_ = "{";
             token.type_ = SEP;
             break;
           }
           case '}':
           {
             token.str_ = "}";
             token.type_ = SEP;
             break;
           }
           case '(':
           {
             token.str_ = "(";
             token.type_ = SEP;
             break;
           }
           case ')':
           {
             token.str_ = ")";
             token.type_ = SEP;
             break;
           }
           case '=':
           {
             c = getchar_la();
             if (c == '=')
             {
               token.str_ = "==";
               token.type_ = EQUAL;
             }
             else
             {
               la = c;
               token.str_ = "=";
               token.type_ = ASSIGN;
             }
             break;
           }
           default:
           {
             token.str_.push_back(c); 
             return ERR;
           }
         }
         return OK;
       }

  if (c != EOF)
    la = c;
  return OK;
}

std::deque <Token> tokens;

int lexer()
{
  while(1)
  {
    //string token;
    Token token;

    int ret = get_token(token);
    //int ret = get_se_token(token);
    if (ret == EOF)
    {
      break;
    }
    if (ret == OK)
    {
      tokens.push_back(token);
#ifdef DEBUG_LEXER_MSG
      if (token.str_ == "\n")
        cout << "token: eol" << endl;
      else
        cout << "token: " << token.str_ << endl;
#endif
    }
    else
    {
      if (token.str_ == "\n")
        cout << "error token: eol" << endl;
      else
      cout << "error token: " << token.str_ << endl;
    }
    // token.str_.clear();
  }

  return 0;
}

#ifdef DEBUG_LEXER

int main(int argc, char *argv[])
{
  //int a,b;
  //a++ + ++b;
  lexer();
  return 0;
}
#endif
