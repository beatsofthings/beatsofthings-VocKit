# VocKit DSP prototype

The DSP core is independent of MPC host APIs. Start with the tiny offline smoke-test harness below to establish a buildable C++ baseline. **It is not an MPC plugin yet**.

Build on desktop with a C++17 compiler:

```sh
c++ -std=c++17 -O2 -Wall -Wextra -pedantic prototype/smoke_test.cpp -o /tmp/vockit-smoke
/tmp/vockit-smoke
```

This intentionally does not include proprietary or unverified SDK interfaces. Implement true 24-band filters only after selecting coefficients and tests.
