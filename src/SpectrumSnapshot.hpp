#pragma once
#include <array>
#include <atomic>
#include <cstddef>
namespace vockit {
// Experimental, host-independent telemetry snapshot.
// Audio producer publishes at FFT hop boundaries. UI consumer may read without
// locking or touching VST parameters/preset serialization.
// Each bin is atomic to avoid C++ data races; snapshots can mix adjacent hops,
// which is acceptable for a visual spectrum and never changes audio output.
class SpectrumSnapshot {
public:
 static constexpr std::size_t Bands=24;
 void publish(const std::array<float,Bands>& values) noexcept {
  for(std::size_t i=0;i<Bands;++i)
   bins_[i].store(values[i],std::memory_order_relaxed);
  sequence_.fetch_add(1,std::memory_order_release);
 }
 std::array<float,Bands> read() const noexcept {
  std::array<float,Bands> result{};
  for(std::size_t i=0;i<Bands;++i)
   result[i]=bins_[i].load(std::memory_order_relaxed);
  return result;
 }
 unsigned sequence() const noexcept {
  return sequence_.load(std::memory_order_acquire);
 }
private:
 std::array<std::atomic<float>,Bands> bins_{};
 std::atomic<unsigned> sequence_{0};
};
}
