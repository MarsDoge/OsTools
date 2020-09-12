

#ifndef __DEBUG_H_
#define __DEBUG_H_

//#define RELEASE

#ifdef  RELEASE
  #define printfQ(x)
#else
  #define printfQ(format,...) printf(format, ##__VA_ARGS__)
#endif



#endif
