#include "../src/Vocoder.hpp"
#include <cmath>
#include <cstdio>
int main(){
 vockit::Vocoder v;v.prepare(48000.f);v.setGate(true);
 float max=0.f;
 for(int i=0;i<24000;++i){
  const float voice=.5f*std::sin(6.283185307f*200.f*i/48000.f);
  v.tick(voice);
 }
 for(float x:v.spectrum()){
  if(!std::isfinite(x)||x<0.f)return 1;
  max=std::max(max,x);
 }
 if(max<=0.f)return 2;
 std::printf("FFT spectrum 24 bins measured, peak %.5f\n",max);
}
