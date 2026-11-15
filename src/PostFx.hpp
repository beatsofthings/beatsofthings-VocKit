#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace vockit {
// Allocation-free post-vocoder FX. Neutral settings are exact bypass.
class PostFx {
 static constexpr int Size=4096;
 std::array<float,Size> left_{},right_{};
 int cursor_=0;
 float phase_=0.f,sr_=44100.f;
 static float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
 static float tap(const std::array<float,Size>& data,float position){
   while(position<0.f)position+=Size;
   while(position>=Size)position-=Size;
   const int a=int(position),b=(a+1)&(Size-1);
   const float frac=position-float(a);
   return data[a]*(1.f-frac)+data[b]*frac;
 }
public:
 void prepare(float sr){sr_=std::max(8000.f,sr);left_.fill(0.f);right_.fill(0.f);cursor_=0;phase_=0.f;}
 void process(float& l,float& r,float saturation,float chorusMix,float rate,float depth){
   saturation=clamp(saturation,0.f,1.f);
   chorusMix=clamp(chorusMix,0.f,1.f);
   if(saturation>0.f){
     const float drive=1.f+5.f*saturation;
     const float norm=std::tanh(drive);
     l=std::tanh(l*drive)/norm;
     r=std::tanh(r*drive)/norm;
   }
   if(chorusMix<=0.f)return;
   rate=clamp(rate,.1f,5.f);
   depth=clamp(depth,0.f,1.f);
   left_[cursor_]=l;right_[cursor_]=r;
   phase_+=rate/sr_;
   if(phase_>=1.f)phase_-=1.f;
   const float center=.018f*sr_;
   const float swing=.006f*depth*sr_;
   const float dl=center+swing*std::sin(6.283185307f*phase_);
   const float dr=center+swing*std::sin(6.283185307f*(phase_+.25f));
   const float wetL=tap(left_,float(cursor_)-dl);
   const float wetR=tap(right_,float(cursor_)-dr);
   // Equal-gain blend: wet return stays below 50% to protect vocal clarity.
   const float amount=.45f*chorusMix;
   l=l*(1.f-amount)+wetL*amount;
   r=r*(1.f-amount)+wetR*amount;
   cursor_=(cursor_+1)&(Size-1);
 }
};
}
