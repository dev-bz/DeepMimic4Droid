#ifndef USE_CAFFE
#include "net.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_CBLAS
#include <cblas.h>
#endif
typedef float net_type;
struct actor {
  net_type s_mean[197];
  net_type s_std[197];
  net_type a_mean[36];
  net_type a_std[36];
  net_type w[197][1024];
  net_type b[1024];
  net_type w0[1024][512];
  net_type b0[512];
  net_type w1[512][36];
  net_type b1[36];
};
struct actor_ {
  net_type nn[197];
  net_type n[1024];
  net_type n0[512];
  net_type nnn[36];
};
static struct actor actor_net;
static struct actor_ actor_ret, actor_bais;
int get_asset_data(const char *path, const int size, void **out);
void load_actor(const char *file) {
#if 1

  void *data = &actor_net;
  if (get_asset_data(file, sizeof(struct actor), &data)) {
    /*FILE *f = fopen(file, "rb");
      if (!f) {
        char path[256] = "apkoverlay/assets/";
        strcat(path, file);
        f = fopen(path, "rb");
      }
      if (!f) { return; }
      fseek(f, 0, SEEK_END);
      long len = ftell(f);
      if (len == sizeof(struct actor)) {
        fseek(f, 0, SEEK_SET);
        int ret = fread(data, sizeof(struct actor), 1, f);
        printf("read return %d, size: %lu\n", ret, sizeof(struct actor));
      } else {
        printf("file size %lu,struct size: %lu\n", len, sizeof(struct actor));
      }
      fclose(f);
      for (int i = 0; i < 197; ++i) {
        actor_net.s_std[i] = (1 / actor_net.s_std[i]);
        actor_net.s_mean[i] *= -actor_net.s_std[i];
      }
      */
  } else {
    memcpy(actor_bais.n, actor_net.b, 1024 * sizeof(net_type));
    memcpy(actor_bais.n0, actor_net.b0, 512 * sizeof(net_type));
    memcpy(actor_bais.nnn, actor_net.b1, 36 * sizeof(net_type));
    for (int i = 0; i < 197; ++i)
      if (actor_net.s_std[i] != 0.0f) {
        actor_net.s_std[i] = 1 / actor_net.s_std[i];
        actor_net.s_mean[i] *= -actor_net.s_std[i];
      }
  }
#endif
}
#ifdef USE_CBLAS
void compute(double *input, int input_s, double *output, int output_s) {
  register int i, j;
  memcpy(actor_ret.n, actor_bais.n, (1024 + 512 + 36) * sizeof(net_type));
  for (j = 0; j < input_s; ++j)
    actor_ret.nn[j] = input[j] * actor_net.s_std[j] + actor_net.s_mean[j];

  cblas_sgemv(CblasRowMajor, CblasTrans, 197, 1024, 1.0, &actor_net.w[0][0],
              1024, actor_ret.nn, 1, 1.0, actor_ret.n, 1);
  for (j = 0; j < 1024; ++j) actor_ret.n[j] = fmaxf(actor_ret.n[j], 0.0);
  cblas_sgemv(CblasRowMajor, CblasTrans, 1024, 512, 1.0, &actor_net.w0[0][0],
              512, actor_ret.n, 1, 1.0, actor_ret.n0, 1);
  for (j = 0; j < 512; ++j) actor_ret.n0[j] = fmaxf(actor_ret.n0[j], 0.0);
  cblas_sgemv(CblasRowMajor, CblasTrans, 512, 36, 1.0, &actor_net.w1[0][0], 36,
              actor_ret.n0, 1, 1.0, actor_ret.nnn, 1);
  for (j = 0; j < output_s; ++j)
    output[j] = actor_ret.nnn[j] * actor_net.a_std[j] + actor_net.a_mean[j];
}
#else
void compute(double *input, double *output) {
  register float ret;
  register int i, j;
#pragma omp parallel for num_threads(2)
  for (j = 0; j < 197; ++j)
    actor_ret.nn[j] = input[j] * actor_net.s_std[j] + actor_net.s_mean[j];
#pragma omp parallel for num_threads(4)
  for (j = 0; j < 1024; ++j) {
    ret = actor_net.b[j];
    for (i = 0; i < 197; ++i) ret += actor_ret.nn[i] * actor_net.w[i][j];
    actor_ret.n[j] = fmaxf(ret, 0.0f);
  }
#pragma omp parallel for num_threads(4)
  for (j = 0; j < 512; ++j) {
    ret = actor_net.b0[j];
    for (i = 0; i < 1024; ++i) ret += actor_ret.n[i] * actor_net.w0[i][j];
    actor_ret.n0[j] = fmaxf(ret, 0.0f);
  }
#pragma omp parallel for num_threads(2)
  for (j = 0; j < 36; ++j) {
    ret = actor_net.b1[j];
    for (i = 0; i < 512; ++i) ret += actor_ret.n0[i] * actor_net.w1[i][j];
    output[j] = ret * actor_net.a_std[j] + actor_net.a_mean[j];
  }
}
#endif
/*#include <time.h>
int main() {
  load_actor("data/policies/humanoid3d/humanoid3d_spinkick_float.bin");
  double input[197];
  for (int i = 0; i < 197; ++i) input[i] = 0.25;
  double output[36];
  clock_t t = clock();
  int j = 0;
  while (1) {
    ++j;
    compute(input, output);
    clock_t n = clock();
    if (n - t > CLOCKS_PER_SEC) {
      printf("%d ==.== %f\n", j, n / (double)(CLOCKS_PER_SEC));
      j = 0;
      // for (int i = 0; i < 36; ++i) printf("%f\n", output[i]);
      t = clock();
    }
  }
}*/
#endif