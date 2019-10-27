#ifndef USE_CAFFE
#include "net.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_CBLAS
#include <cblas.h>
#endif
#include <vector>
typedef float net_type;
struct actor {
	net_type *s_mean; //[INPUT_SIZE];
	net_type *s_std;  //[INPUT_SIZE];
	net_type *a_mean; //[OUTPUT_SIZE];
	net_type *a_std;  //[OUTPUT_SIZE];

	net_type *w; //[INPUT_SIZE][1024];
	net_type *b; //[1024];

	net_type *w0; //[1024][512];
	net_type *b0; //[512];

	net_type *w1; //[512][OUTPUT_SIZE];
	net_type *b1; //[OUTPUT_SIZE];

	net_type *ret_nn;   //[INPUT_SIZE];
	net_type *ret_n;	//[1024];
	net_type *ret_n0;   //[512];
	net_type *ret_nnn;  //[OUTPUT_SIZE];
	net_type *bias_nn;  //[INPUT_SIZE];
	net_type *bias_n;   //[1024];
	net_type *bias_n0;  //[512];
	net_type *bias_nnn; //[OUTPUT_SIZE];

	size_t input, output, length;
	std::vector<net_type> params;
	int ready(const long target, const int in, const int out) {
		length = in + in + out + out + in * 1024 + 1024 + 1024 * 512 + 512 +
				 512 * out + out;
		input = in;
		output = out;
		size_t buf = (in + out + 1024 + 512) * 2;
		params.resize(length + buf);

		auto ptr = params.data();
		s_mean = ptr;
		s_std = s_mean + input;
		a_mean = s_std + input;
		a_std = a_mean + output;
		w = a_std + output;
		b = w + input * 1024;
		w0 = b + 1024;
		b0 = w0 + 1024 * 512;
		w1 = b0 + 512;
		b1 = w1 + 512 * output;

		ret_nn = b1 + output;
		ret_n = ret_nn + input;
		ret_n0 = ret_n + 1024;
		ret_nnn = ret_n0 + 512;
		bias_nn = ret_nnn + output;
		bias_n = bias_nn + input;
		bias_n0 = bias_n + 1024;
		bias_nnn = bias_n0 + 512;
		return sizeof(net_type) * length == target;
	}
};
static std::vector<std::shared_ptr<struct actor>> actor_nets;
extern "C" int get_asset_data(const char *path, const int size, void **out);
void load_actor(int id, const char *file, int in, int out) {
#if 1
	while (actor_nets.size() <= id)
		actor_nets.push_back(std::shared_ptr<struct actor>(new actor));
	auto &actor_net = *actor_nets[id];
	actor_net.ready(-1, in, out);
	if (file == NULL)return;
	void *data = actor_net.params.data();
	if (get_asset_data(file, sizeof(net_type) * actor_net.length, &data)) {
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
			printf("file size %lu,struct size: %lu\n", len, sizeof(struct
		  actor));
		  }
		  fclose(f);
		  for (int i = 0; i < 197; ++i) {
			actor_net.s_std[i] = (1 / actor_net.s_std[i]);
			actor_net.s_mean[i] *= -actor_net.s_std[i];
		  }
		  */
	} else {
		memcpy(actor_net.bias_n, actor_net.b, 1024 * sizeof(net_type));
		memcpy(actor_net.bias_n0, actor_net.b0, 512 * sizeof(net_type));
		memcpy(actor_net.bias_nnn, actor_net.b1,
			   actor_net.output * sizeof(net_type));
		for (int i = 0; i < actor_net.input; ++i)
			if (actor_net.s_std[i] != 0.0f) {
				actor_net.s_std[i] = 1 / actor_net.s_std[i];
				actor_net.s_mean[i] *= -actor_net.s_std[i];
			}
	}
#endif
}
#ifdef USE_CBLAS
void compute(int id, double *input, double *output) {
	register int i;
	if (id >= actor_nets.size())
		return;
	auto &actor_net = *actor_nets[id];
	const auto &OUTPUT_SIZE = actor_net.output;
	const auto &INPUT_SIZE = actor_net.input;
	memcpy(actor_net.ret_n, actor_net.bias_n,
		   (1024 + 512 + OUTPUT_SIZE) * sizeof(net_type));
	for (int i = 0; i < INPUT_SIZE; ++i)
		actor_net.ret_nn[i] =
			net_type(input[i] * actor_net.s_std[i]) + actor_net.s_mean[i];
	cblas_sgemv(CblasRowMajor, CblasTrans, INPUT_SIZE, 1024, 1.0f, actor_net.w,
				1024, actor_net.ret_nn, 1, 1.0, actor_net.ret_n, 1);
	for (int i = 0; i < 1024; ++i)
		actor_net.ret_n[i] = fmaxf(0.0, actor_net.ret_n[i]);
	cblas_sgemv(CblasRowMajor, CblasTrans, 1024, 512, 1.0f, actor_net.w0, 512,
				actor_net.ret_n, 1, 1.0, actor_net.ret_n0, 1);
	for (int i = 0; i < 512; ++i)
		actor_net.ret_n0[i] = fmaxf(0.0, actor_net.ret_n0[i]);
	cblas_sgemv(CblasRowMajor, CblasTrans, 512, OUTPUT_SIZE, 1.0f, actor_net.w1,
				OUTPUT_SIZE, actor_net.ret_n0, 1, 1.0, actor_net.ret_nnn, 1);
	for (int i = 0; i < OUTPUT_SIZE; ++i)
		output[i] = net_type(actor_net.ret_nnn[i] * actor_net.a_std[i] +
							 actor_net.a_mean[i]);
}
#else
void compute(int id, double *input, double *output) {
	register float ret;
	register int i, j;
	auto &actor_net = *actor_nets[id];
#pragma omp parallel for num_threads(2)
	for (j = 0; j < actor_net.input; ++j)
		actor_ret.nn[j] = input[j] * actor_net.s_std[j] + actor_net.s_mean[j];
#pragma omp parallel for num_threads(4)
	for (j = 0; j < 1024; ++j) {
		ret = actor_net.b[j];
		for (i = 0; i < 197; ++i)
			ret += actor_ret.nn[i] * actor_net.w[i][j];
		actor_ret.n[j] = fmaxf(ret, 0.0f);
	}
#pragma omp parallel for num_threads(4)
	for (j = 0; j < 512; ++j) {
		ret = actor_net.b0[j];
		for (i = 0; i < 1024; ++i)
			ret += actor_ret.n[i] * actor_net.w0[i][j];
		actor_ret.n0[j] = fmaxf(ret, 0.0f);
	}
#pragma omp parallel for num_threads(2)
	for (j = 0; j < 36; ++j) {
		ret = actor_net.b1[j];
		for (i = 0; i < 512; ++i)
			ret += actor_ret.n0[i] * actor_net.w1[i][j];
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