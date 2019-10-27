#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void load_actor(int id,const char *file, int in, int out);
void compute(int id, double *input, double *output);
#ifdef __cplusplus
}
#endif