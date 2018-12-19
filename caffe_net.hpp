#pragma once
#include <memory>
#include <vector>
namespace caffe {
template <typename Dtype> class Net;
template <typename Dtype> class Solver;
template <typename Dtype> class Blob;
} // namespace caffe
struct Net {
	std::shared_ptr<caffe::Net<float>> local;
	std::shared_ptr<caffe::Net<float>> batch, n256;
	std::shared_ptr<caffe::Net<float>> tlocal;
	std::shared_ptr<caffe::Solver<float>> slocal;
	int GetBatchSize();
	void LoadTrainData(const std::vector<float> &X, const std::vector<float> &Y, const std::vector<float> &W);
	void SetTrainParam(const std::vector<float> &param);
	float GetTrainResult();
	static void CopyParams(const std::vector<caffe::Blob<float> *> &src_params,
												 const std::vector<caffe::Blob<float> *> &dst_params);
	static void CopyModel(const caffe::Net<float> &src, caffe::Net<float> &dst);
	void makeNet(int &i, int &o, const char *net, const char *solver = nullptr);
        void syncNet(bool train = true, int flags = 0);
	float trainNet();
	std::vector<float> &getValue(const std::vector<float> &input);
	void getValue(const double*input, double*output);
	float getValues(const std::vector<float> &input, std::vector<float> &output, int batchSize = 1024);
	void Save(const std::string &model_file);
	void Load(const std::string &model_file);
	void check(float weight_decay = 1);
	std::vector<float> local_data;
	std::vector<float> batch_data;
};
void InitCaffe(char **argv);