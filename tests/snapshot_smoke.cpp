#include "../src/SpectrumSnapshot.hpp"
#include <array>
#include <cmath>
#include <cstdio>
int main(){
 vockit::SpectrumSnapshot s;
 if(s.sequence()!=0)return 1;
 std::array<float,24> input{};
 for(int frame=1;frame<=1000;++frame){
  for(int i=0;i<24;++i)input[i]=float(frame)*.001f+float(i)*.01f;
  s.publish(input);
  auto output=s.read();
  for(int i=0;i<24;++i)
   if(!std::isfinite(output[i])||std::fabs(output[i]-input[i])>1e-6f)return 2;
 }
 if(s.sequence()!=1000)return 3;
 std::puts("24-band non-persistent snapshot read/write passed");
}
