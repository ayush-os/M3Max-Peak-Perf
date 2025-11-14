# M3Max-Peak-Perf

## Theoretical Peak FLOPs

* 4.05 GHz for P-Core
* How many FLOPs/cycle?
  * ARM P-core NEON does 2 FMAs (mult and add = 2 ops) / clock cycle -> 4 FP ops / clock cycle
  * 128-bit NEON register / 32 bit float = 4 lanes
  * 4 lanes * 4 FP ops / clock cycle = total of 16 FP ops / cycle
* 4.05 GHz * 16 FP ops / cycle = **64.8 GFLOPs for FP32** -> **129.6 GFLOPs for FP16**