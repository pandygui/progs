#include "type.h"

#include <stdio.h>

using namespace std;

#include "guint.h"

//using namespace DS;

int main(int argc, char *argv[])
{
  string s1("165535");
  string s2("1");
  Guint guint1(s1);
  Guint guint2(s2);

  cout << "s1: " << s1 << endl;
  cout << "s2: " << s2 << endl;
  cout << "guint1: " << guint1 << endl;
  cout << "guint2: " << guint2 << endl;

#if 0
  u16 a = 0xffff;
  //s16 a1 = 0xffff;
  s16 a1 = 0x1;

  if (a1 >= 0)
  {
    printf("a1 >= 0\n");
  }
  else
  {
    printf("a1 < 0\n");
  }

  if (~a1 >= 0)
  {
    printf("~a1 >= 0\n");
  }
  else
  {
    printf("~a1 < 0\n");
  }
  printf("a1: %d\n", a1);
  printf("~a1: %d\n", ~a1);


  u16 b = 1;

  u32 c = a+b;
  printf("a: %d\n", a);
  printf("a1: %d\n", a1);
  printf("%d\n", b);
  printf("%d\n", c);
#endif
  return 0;
}