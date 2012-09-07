#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>

using namespace std;

class Queue
{
  public:
    explicit Queue(int len);
    bool push(char ch);
    bool pop(char &ch);
    int len() const {return len_;}
    int begin() const {return begin_;}
    int end() const {return end_;}
    bool print()
    {
      if (ready() == false) return false;
      cout << "-- ";
      int e = end();
      for (int i=e ; i != begin() ; )
      {
        cout << *(q_ + i) << " ";
        i = (i + 1) % len_;
      }
      cout << " --" << endl;
      return true;
    }
    bool ready() const
    {
      //cout << "len_:" << len_ << endl;
      if (len_ > 1)
        return true;
      else
        return false;
    }
  private:
    bool can_push()
    {
      int next_begin = ((begin() + 1) % len_);
      if (next_begin == end())
        return false;
      else
        return true;
    }
    bool can_pop()
    {
      if (begin() == end())
        return false;
      else
        return true;
    }
    char *q_;
    int len_;
    int begin_, end_;
};

#endif
