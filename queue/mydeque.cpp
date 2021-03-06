#include "mydeque.h"

// init: 
// begin_ = end_ = 0;
// q leave one empty space when full
MyDeque::MyDeque(int len)
{
  len_ = len;
  if (len_ > 1)
  {
    len_ = len;
    q_ = new char[len_];
    begin_ = end_ = 0;
  }
}

bool MyDeque::push_back(char ch)
{
  if (ready() == false) 
    return false;
// copy ch to begin point
// ++begin

  bool ret = true;
  if (can_push())
  {
    *(q_ + begin_) = ch;
    begin_ = ((begin_ + 1) % len_);
  }
  else
    ret = false;
  return ret;
}

bool MyDeque::push_front(char ch)
{
  if (ready() == false) 
    return false;
  bool ret = true;

  // has space to push
  if (can_push())
  {
    end_ = ((end_ + len_ - 1) % len_);
    *(q_ + end_) = ch;
  }
  else
    ret = false;
  return ret;
}

bool MyDeque::pop_front(char &ch)
{
  if (ready() == false) 
    return false;
// return end point value, ++end
  bool ret = true;
  if (can_pop())
  {
    ch = *(q_ + end_);
    end_ = ((end_ + 1) % len_);
  }
  else
  {
    ret = false;
  }
  return ret;
}

bool MyDeque::pop_back(char &ch)
{
  if (ready() == false) 
    return false;

  bool ret = true;
  if (can_pop())
  {
    begin_ = ((begin_ - 1) % len_);
    ch = *(q_ + begin_);
  }
  else
  {
    ret = false;
  }
  return ret;
}
