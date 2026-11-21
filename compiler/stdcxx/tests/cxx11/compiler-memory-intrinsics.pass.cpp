// Original regression: memory-reading builtins must observe preceding writes.
#include <cassert>
#include <cstring>
volatile unsigned memory_fill=0xcd;
volatile int memory_calls=3;
int main() {
    unsigned char x[4]={0xab,0xcd,0xcd,0xcd},y[4];
    std::memset(y,int(memory_fill),sizeof(y)); y[0]=0xab;
    assert(std::memcmp(x,y,sizeof(x))==0 && memory_calls==3);
    y[2]=0; assert(std::memcmp(x,y,sizeof(x))>0);
    std::memcpy(y,x,sizeof(y)); assert(std::memcmp(x,y,sizeof(x))==0);
    char text[16]; std::memset(text,0,sizeof(text));
    std::strcpy(text,"abc"); assert(std::strlen(text)==3 && std::strcmp(text,"abc")==0);
    text[1]=0; assert(std::strlen(text)==1 && std::strcmp(text,"a")==0);
    text[1]='b'; text[2]='d'; assert(std::strlen(text)==3 && std::strcmp(text,"abc")>0);
    std::strncpy(text,"x",4); assert(std::strlen(text)==1 && text[2]==0 && text[3]==0);
}
