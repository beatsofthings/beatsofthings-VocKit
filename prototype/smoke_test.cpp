#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

namespace vockit {
// Starter signal safety / smoothing helper, not yet the 24-band vocoder.
class EnvelopeFollower {
public:
    void setSampleRate(float sampleRate) {
        sampleRate_ = (sampleRate > 1.0f) ? sampleRate : 48000.0f;
        setTimes(10.0f, 80.0f);
    }
    void setTimes(float attackMs, float releaseMs) {
        attack_ = coefficient(attackMs);
        release_ = coefficient(releaseMs);
    }
    float tick(float input) {
        const float rectified = std::isfinite(input) ? std::fabs(input) : 0.0f;
        const float coeff = rectified > value_ ? attack_ : release_;
        value_ += coeff * (rectified - value_);
        return value_;
    }
private:
    float coefficient(float ms) const {
        const float seconds = std::fmax(0.1f, ms) * 0.001f;
        return 1.0f - std::exp(-1.0f / (seconds * sampleRate_));
    }
    float sampleRate_ = 48000.0f;
    float attack_ = 0.0f;
    float release_ = 0.0f;
    float value_ = 0.0f;
};
}

int main() {
    vockit::EnvelopeFollower env;
    for (float rate : {44100.0f, 48000.0f}) {
        env = {};
        env.setSampleRate(rate);
        float peak = 0.0f;
        for (int i = 0; i < static_cast<int>(rate); ++i) {
            const float v = env.tick(i < static_cast<int>(rate / 2) ? 0.5f : 0.0f);
            assert(std::isfinite(v) && v >= 0.0f && v <= 0.500001f);
            peak = std::fmax(peak, v);
        }
        assert(peak > 0.45f);
    }
    std::puts("VocKit envelope smoke test passed");
    return 0;
}
