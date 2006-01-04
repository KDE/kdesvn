#ifndef _KEYSTATUS_H
#define _KEYSTATUS_H

class KeyState
{
public:
  KeyState(){}
  ~KeyState(){}
  static void keystate(int*,int*,int*,int*,unsigned int*mask);

};

#endif

