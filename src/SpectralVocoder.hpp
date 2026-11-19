#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace vockit {
// Streaming 512-point STFT vocoder, 128-sample hop, 24 logarithmic spectral bands.
// Fixed memory; no allocations or locks in the audio callback.
class SpectralVocoder {
 static constexpr int N=512,H=128,B=24,R=2048;
 struct C {float r=0,i=0;};
 std::array<float,N> voice_{},carrier_{},window_{};
 std::array<float,R> ola_{};
 std::array<C,N> a_{},b_{};
 std::array<float,B> smooth_{},spectrum_{};
 std::array<int,N/2+1> bins_{};
 int cursor_=0,clock_=0;
 float sr_=44100.f,attack_=8.f,release_=90.f,formantShift_=0.f;
 float sensitivity_=1.f,smoothing_=0.f,spread_=0.f,tilt_=0.f,brightness_=0.f;
 bool freeze_=false;
 std::array<float,B> frozen_{};
 float lowTime_=1.f,highTime_=1.f,range_=1.f;
 static void fft(std::array<C,N>& x,bool inverse){
  for(int i=1,j=0;i<N;++i){
   int bit=N>>1;
   for(;j&bit;bit>>=1)j^=bit;
   j^=bit;
   if(i<j)std::swap(x[i],x[j]);
  }
  for(int len=2;len<=N;len<<=1){
   const float angle=(inverse?2.f:-2.f)*3.14159265358979323846f/len;
   const float wr=std::cos(angle),wi=std::sin(angle);
   for(int start=0;start<N;start+=len){
    float ur=1.f,ui=0.f;
    for(int j=0;j<len/2;++j){
     C& u=x[start+j];C& v=x[start+j+len/2];
     const float vr=v.r*ur-v.i*ui,vi=v.r*ui+v.i*ur;
     const float ar=u.r,ai=u.i;
     u.r=ar+vr;u.i=ai+vi;v.r=ar-vr;v.i=ai-vi;
     const float next=ur*wr-ui*wi;
     ui=ur*wi+ui*wr;ur=next;
    }
   }
  }
  if(inverse)for(auto& z:x){z.r/=N;z.i/=N;}
 }
 void frame(const std::array<float,B>& bandLevels){
  // Frame ending at cursor_; windowed input is centered 256 samples in past.
  for(int j=0;j<N;++j){
   const int idx=(cursor_+j)&(N-1);
   a_[j]={voice_[idx]*window_[j],0.f};
   b_[j]={carrier_[idx]*window_[j],0.f};
  }
  fft(a_,false);fft(b_,false);
  std::array<float,B> va{},ca{};
  std::array<int,B> counts{};
  for(int k=1;k<N/2;++k){
   const int band=bins_[k];
   va[band]+=a_[k].r*a_[k].r+a_[k].i*a_[k].i;
   ca[band]+=b_[k].r*b_[k].r+b_[k].i*b_[k].i;
   ++counts[band];
  }
  std::array<float,B> gain{};
  std::array<float,B> envelope{};
  for(int j=0;j<B;++j){
   const float v=std::sqrt(va[j]/std::max(1,counts[j]));
   const float c=std::sqrt(ca[j]/std::max(1,counts[j]));
   const float timeScale=j<8?lowTime_:(j>=16?highTime_:1.f);
   const float atk=std::exp(-float(H)*1000.f/(sr_*std::max(.5f,attack_*timeScale*(1.f+smoothing_*3.f))));
   const float rel=std::exp(-float(H)*1000.f/(sr_*std::max(2.f,release_*timeScale*(1.f+smoothing_*3.f))));
   const float coeff=v>smooth_[j]?atk:rel;
   smooth_[j]=coeff*smooth_[j]+(1.f-coeff)*v;
   // Measured per-band voice energy, no host parameter updates.
   spectrum_[j]=smooth_[j];
   if(!freeze_)frozen_[j]=smooth_[j];
   envelope[j]=freeze_?frozen_[j]:smooth_[j];
   // Zero shift follows the original gain path bit-for-bit.
   gain[j]=std::min(12.f,envelope[j]/std::max(.002f,c))*
           std::max(0.f,bandLevels[j]);
  }
  if(spread_>0.f){
   const auto original=envelope;
   for(int j=0;j<B;++j){
    const float near=(original[std::max(0,j-1)]+original[j]+original[std::min(B-1,j+1)])/3.f;
    envelope[j]=original[j]*(1.f-spread_)+near*spread_;
   }
  }
  if(formantShift_!=0.f){
   // Resample the 24-band speech envelope on its logarithmic frequency axis.
   // Keep carrier normalisation and band mixer at the *destination* band.
   const float offset=formantShift_*B*std::log(2.f)/
       (12.f*std::log(10000.f/80.f));
   for(int j=0;j<B;++j){
    const float src=std::max(0.f,std::min(float(B-1),float(j)-offset));
    const int lo=int(src),hi=std::min(B-1,lo+1);
    const float t=src-float(lo);
    const float shifted=envelope[lo]*(1.f-t)+envelope[hi]*t;
    const float carrierRms=std::sqrt(ca[j]/std::max(1,counts[j]));
    gain[j]=std::min(12.f,shifted/std::max(.002f,carrierRms))*
            std::max(0.f,bandLevels[j]);
   }
  }
  if(sensitivity_!=1.f||range_!=1.f||tilt_!=0.f||brightness_!=0.f||spread_>0.f){
   for(int j=0;j<B;++j){
    const float c=std::sqrt(ca[j]/std::max(1,counts[j]));
    const float target=std::min(12.f,envelope[j]*sensitivity_/std::max(.002f,c));
    if(formantShift_==0.f)gain[j]=target*std::max(0.f,bandLevels[j]);
    gain[j]=std::pow(std::max(0.f,gain[j]),range_);
    const float x=float(j)/float(B-1)-.5f;
    gain[j]*=std::pow(2.f,x*(tilt_+brightness_));
   }
  }
  b_[0]={0,0};b_[N/2]={0,0};
  for(int k=1;k<N/2;++k){
   const float g=gain[bins_[k]];
   b_[k].r*=g;b_[k].i*=g;
   b_[N-k].r*=g;b_[N-k].i*=g;
  }
  fft(b_,true);
  // Four Hann overlaps; synthesis energy normalization 2/3.
  // Output is delayed one FFT frame to ensure causal overlap-add.
  for(int j=0;j<N;++j){
   const int pos=(clock_+j+N)&(R-1);
   ola_[pos]+=b_[j].r*window_[j]*(2.f/3.f);
  }
 }
public:
 void prepare(float sr){
  sr_=std::max(8000.f,sr);voice_.fill(0);carrier_.fill(0);
  ola_.fill(0);smooth_.fill(0);spectrum_.fill(0);frozen_.fill(0);freeze_=false;cursor_=clock_=0;
  for(int j=0;j<N;++j)window_[j]=.5f-.5f*std::cos(2.f*3.14159265358979323846f*j/N);
  for(int k=0;k<=N/2;++k){
   const float hz=std::max(80.f,float(k)*sr_/N);
   const float position=24.f*std::log(hz/80.f)/std::log(10000.f/80.f);
   bins_[k]=std::max(0,std::min(B-1,int(position)));
  }
 }
 const std::array<float,B>& spectrum()const noexcept{return spectrum_;}
 void setAttackRelease(float a,float r){attack_=a;release_=r;}
 void setVoiceShape(float sensitivity,float smoothing,float spread,float tilt,float brightness,float lowTime,float highTime,float range){
  sensitivity_=std::max(.25f,std::min(4.f,sensitivity));smoothing_=std::max(0.f,std::min(1.f,smoothing));
  spread_=std::max(0.f,std::min(1.f,spread));tilt_=std::max(-2.f,std::min(2.f,tilt));
  brightness_=std::max(-2.f,std::min(2.f,brightness));lowTime_=std::max(.25f,std::min(4.f,lowTime));
  highTime_=std::max(.25f,std::min(4.f,highTime));range_=std::max(.5f,std::min(1.5f,range));
 }
 void setFreeze(bool enabled){freeze_=enabled;}
 void setFormantShift(float semitones){formantShift_=std::max(-12.f,std::min(12.f,semitones));}
 float tick(float voice,float carrier,const std::array<float,B>& levels){
  const float output=ola_[clock_&(R-1)];
  ola_[clock_&(R-1)]=0.f;
  voice_[cursor_]=std::isfinite(voice)?voice:0.f;
  carrier_[cursor_]=std::isfinite(carrier)?carrier:0.f;
  cursor_=(cursor_+1)&(N-1);
  ++clock_;
  if((clock_&(H-1))==0)frame(levels);
  return std::tanh(output*2.5f);
 }
 static constexpr int latencySamples(){return N;}
};
}
