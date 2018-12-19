#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void load_actor(const char *file);
void compute(double *input,int input_s, double *output,int output_s);
#ifdef __cplusplus
}
#endif