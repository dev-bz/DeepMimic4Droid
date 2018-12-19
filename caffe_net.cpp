#ifdef USE_CAFFE
#include "caffe_net.hpp"
#include <caffe/caffe.hpp>
#include <caffe/net.hpp>
#include <caffe/sgd_solvers.hpp>
/*std::shared_ptr<caffe::Net<float>> local;
std::shared_ptr<caffe::Net<float>> tlocal;
std::shared_ptr<caffe::Solver<float>> slocal;*/

void flip(caffe::Net<float> *local) {
  auto &layer = local->layer_by_name("normal");
  auto &b = layer->blobs();
  if (b.size() > 1) {
    auto &blob = b[0];
    auto &base = b[1];
    auto count = blob->count();
    auto b_count = base->count();
    if (count > 0 && count == b_count) {
      auto data = blob->mutable_cpu_data();
      auto mean = base->mutable_cpu_data();
      for (int i = 0; i < count; ++i) {
        data[i] = 1 / data[i];
        mean[i] *= -data[i];
      }
    }
  }
}
int Net::GetBatchSize() {
  int batch_size = 0;
  {
    const auto &input_blobs = slocal->net()->input_blobs();
    const auto &input_blob = input_blobs[0];
    batch_size = input_blob->shape(0);
  }
  return batch_size;
}
void Net::SetTrainParam(const std::vector<float> &param) {
  int blobs = tlocal->input_blobs().size();
  if (blobs > 3 && param.size() == 4) {
    auto dt = param.data();
    auto &i = tlocal->input_blobs()[3];
    if (i->count() > 3) {
      float *td = i->mutable_cpu_data();
      for (int j = 0; j < 4; ++j) { td[j] = dt[j]; }
    }
  }
}
float Net::GetTrainResult() {
  int blobs = tlocal->input_blobs().size();
  if (blobs > 3) {
    auto &i = tlocal->input_blobs()[3];
    if (i->count() > 4) return i->cpu_data()[4];
  }
  return 0;
}
void Net::LoadTrainData(const std::vector<float> &X,
                        const std::vector<float> &Y,
                        const std::vector<float> &W) {
  int blobs = tlocal->input_blobs().size();
  {
    auto dt = X.data();
    int jc = Y.size();
    auto &i = tlocal->input_blobs()[0];
    int ct = i->count();
    float *td = i->mutable_cpu_data();
    for (int j = 0; j < ct; ++j) { td[j] = dt[j % jc]; }
  }
  if (blobs > 1) {
    auto dt = Y.data();
    int jc = Y.size();
    auto &i = tlocal->input_blobs()[1];
    int ct = i->count();
    float *td = i->mutable_cpu_data();
    for (int j = 0; j < ct; ++j) { td[j] = dt[j % jc]; }
  }
  if (blobs > 2 && W.size() == Y.size()) {
    auto dt = W.data();
    int jc = W.size();
    auto &i = tlocal->input_blobs()[2];
    int ct = i->count();
    float *td = i->mutable_cpu_data();
    for (int j = 0; j < ct; ++j) { td[j] = dt[j % jc]; }
  }
}
void Net::CopyParams(const std::vector<caffe::Blob<float> *> &src_params,
                     const std::vector<caffe::Blob<float> *> &dst_params) {
  int num_blobs = static_cast<int>(src_params.size());
  for (int b = 0; b < num_blobs; ++b) {
    auto src_blob = src_params[b];
    auto dst_blob = dst_params[b];
    int src_blob_count = src_blob->count();
    int dst_blob_count = dst_blob->count();
    assert(src_blob_count == dst_blob_count);
    dst_blob->CopyFrom(*src_blob);
  }
}
void Net::CopyModel(const caffe::Net<float> &src, caffe::Net<float> &dst) {
  const auto &src_params = src.learnable_params();
  const auto &dst_params = dst.learnable_params();
  CopyParams(src_params, dst_params);
}
void Net::syncNet(bool train, int flags) {
  if (tlocal != nullptr) {
    if (train) {
      CopyModel(*tlocal.get(), *local.get());
      if (flags & 1) CopyModel(*tlocal.get(), *batch.get());
      if (flags & 2) CopyModel(*tlocal.get(), *n256.get());
    } else {
      CopyModel(*local.get(), *tlocal.get());
      if (flags & 1) CopyModel(*local.get(), *batch.get());
      if (flags & 2) CopyModel(*local.get(), *n256.get());
    }
  }
}
void Net::makeNet(int &i, int &o, const char *net, const char *solver) {
  const char *SolverType;
  caffe::NetParameter p;
  caffe::SolverParameter param;
  if (solver) {
    caffe::ReadProtoFromTextFileOrDie(solver, &param);
    caffe::SolverParameter_SolverType type = param.solver_type();
    SolverType = param.type().c_str();
    if (strcmp("SGD", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_SGD;
    } else if (strcmp("Nesterov", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_NESTEROV;
    } else if (strcmp("AdaGrad", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_ADAGRAD;
    } else if (strcmp("RMSProp", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_RMSPROP;
    } else if (strcmp("AdaDelta", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_ADADELTA;
    } else if (strcmp("Adam", SolverType) == 0) {
      type = caffe::SolverParameter_SolverType_ADAM;
    }
    switch (type) {
    case caffe::SolverParameter_SolverType_SGD:
      slocal.reset(new caffe::SGDSolver<float>(param));
      break;
    case caffe::SolverParameter_SolverType_NESTEROV:
      slocal.reset(new caffe::NesterovSolver<float>(param));
      break;
    case caffe::SolverParameter_SolverType_ADAGRAD:
      slocal.reset(new caffe::AdaGradSolver<float>(param));
      break;
    case caffe::SolverParameter_SolverType_RMSPROP:
      slocal.reset(new caffe::RMSPropSolver<float>(param));
      break;
    case caffe::SolverParameter_SolverType_ADADELTA:
      slocal.reset(new caffe::AdaDeltaSolver<float>(param));
      break;
    case caffe::SolverParameter_SolverType_ADAM:
      slocal.reset(new caffe::AdamSolver<float>(param));
      break;
    default: LOG(FATAL) << "Unknown SolverType: " << type;
    }
    tlocal = slocal->net();
    int blobs = tlocal->input_blobs().size();
    if (blobs > 2) {
      auto &i = tlocal->input_blobs()[2];
      int ct = i->count();
      float *td = i->mutable_cpu_data();
      for (int j = 0; j < ct; ++j) td[j] = 1.0;
    }
    if (net == nullptr) net = param.net().c_str();
    SolverType = slocal->type();
  }
  if (net) {
    local.reset(new caffe::Net<float>(net, caffe::TEST));
    {
      batch.reset(new caffe::Net<float>(net, caffe::TEST));
      auto &blob = batch->blob_by_name("input");
      blob->Reshape(32 * 32, blob->channels(), blob->height(), blob->width());
    }
    {
      n256.reset(new caffe::Net<float>(net, caffe::TEST));
      auto &blob = n256->blob_by_name("input");
      blob->Reshape(256, blob->channels(), blob->height(), blob->width());
    }
  }
  i = local->blob_by_name("input")->count();
  o = local->blob_by_name("output")->count();
}
#define Dtype float
int step = 0;
int step_p = 0;
/*
494,1221
673,863
1024,512
1382,333
*/

void local_cpu_gemm(const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
                    const int M, const int N, const int K, const float alpha,
                    const float *A, const float *B, const float beta,
                    float *C) {
  int lda = (TransA == CblasNoTrans) ? K : M;
  int ldb = (TransB == CblasNoTrans) ? N : K;
  cblas_sgemm(CblasRowMajor, TransA, TransB, M, N, K, alpha, A, lda, B, ldb,
              beta, C, N);
}
const std::vector<caffe::Blob<float> *> &ForwardFromTo(caffe::Net<float> *net) {
  int start = 0;
  auto &layers_ = net->layers();
  auto &bottom_vecs_ = net->bottom_vecs();
  auto &top_vecs_ = net->top_vecs();
  int end = layers_.size();
  Dtype loss = 0;
  for (int i = start; i < end; ++i) {
    /*for (int c = 0; c < before_forward_.size(); ++c) {
      before_forward_[c]->run(i);
    }*/
    if (i != 4) {
      Dtype layer_loss = layers_[i]->Forward(bottom_vecs_[i], top_vecs_[i]);
      loss += layer_loss;
    } else {
      auto &top = top_vecs_[i];
      auto &bottom = bottom_vecs_[i];
      auto &blobs_ = layers_[i]->blobs();
      const bool transpose_ = true;
      const Dtype *bottom_data = bottom[0]->cpu_data();
      Dtype *top_data = top[0]->mutable_cpu_data();
      const Dtype *weight = blobs_[0]->cpu_data();
      int axis = 1;
      int N_  = top[0]->count(axis)>>1;    // 512
      int K_ = bottom[0]->count(axis);    // 1024
      int M_ = bottom[0]->count(0, axis); // 1

      auto bias = blobs_[1]->cpu_data();
      step = i;
      caffe::caffe_copy(N_+N_, bias, top_data);
      cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M_, N_, K_, 1.0,
                  bottom_data, K_, weight, N_ + N_, 1.0, top_data, N_);
      cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M_, N_, K_, 1.0,
                  bottom_data, K_, weight + N_, N_ + N_, 1.0, top_data + N_,
                  N_);
      /*cblas_sgemv(CblasRowMajor, CblasTrans, K_ / 2, N_, 1.0,
                  weight + N_ * K_ / 2, N_, bottom_data + K_ / 2, 1, 1.0,
                  top_data, 1);*/
      /*caffe::caffe_copy(N_, blobs_[1]->cpu_data(), top_data);
      local_cpu_gemm(CblasNoTrans, transpose_ ? CblasNoTrans : CblasTrans, M_,
                     N_, K_, (Dtype)1., bottom_data, weight, (Dtype)1.,
                     top_data);*/
    }
    /*if (debug_info_) { ForwardDebugInfo(i); }
    for (int c = 0; c < after_forward_.size(); ++c) {
      after_forward_[c]->run(i);
    }*/
  }
  return net->output_blobs();
}
void Net::getValue(const double *input, double *output) {
  auto &b = local->input_blobs();
  int ix = 0;
  for (auto &i : b) {
    int ct = i->count();
    float *td = i->mutable_cpu_data();
    for (int j = 0; j < ct; ++j) { td[j] = input[ix++]; }
  }
  auto &o = local->Forward();
      //ForwardFromTo(local.get());

  ix = 0;
  for (auto &i : o) {
    auto cnt = i->count();
    auto d = i->cpu_data();
    for (int j = 0; j < cnt; ++j, ++ix) { output[ix] = d[j]; }
  }
}
std::vector<float> &Net::getValue(const std::vector<float> &input) {
  auto &b = local->input_blobs();
  // for (int v = 0; v < ct; ++v)
  {
    auto dt = input.data();
    int ix = 0;
    for (auto &i : b) {
      int ct = i->count();
      float *td = i->mutable_cpu_data();
      for (int j = 0; j < ct; ++j) { td[j] = dt[ix++]; }
    }
    float loss = 0;
    local->Forward(&loss);
    auto &o = local->output_blobs();
    ix = 0;
    for (auto &i : o) {
      auto cnt = i->count();
      if (local_data.size() != cnt) local_data.resize(cnt);
      auto d = i->cpu_data();
      for (int j = 0; j < cnt; ++j, ++ix) { local_data[ix] = d[j]; }
    }
  }
  return local_data;
}
float Net::getValues(const std::vector<float> &input,
                     std::vector<float> &output, int batchSize) {
  auto nt = batchSize == 1024 ? batch.get() : n256.get();
  auto &b = nt->input_blobs();
  // for (int v = 0; v < ct; ++v)
  float loss = 0;
  {
    auto dt = input.data();
    int ix = 0;
    for (auto &i : b) {
      int ct = i->count();
      float *td = i->mutable_cpu_data();
      for (int j = 0; j < ct; ++j) { td[j] = dt[ix++]; }
    }
    nt->Forward(&loss);
    auto &o = nt->output_blobs();
    ix = 0;
    for (auto &i : o) {
      auto cnt = i->count();
      if (output.size() != cnt) output.resize(cnt);
      auto d = i->cpu_data();
      for (int j = 0; j < cnt; ++j, ++ix) { output[ix] = d[j]; }
    }
  }
  return loss;
}
float Net::trainNet(
    /*const std::vector<float> &input, const std::vector<float> &target, const std::vector<float> &w*/) {
  // LoadTrainData(input, target, w);
  float loss = 0;
  // for (int e = 0; e < 100; ++e) {
  slocal->Step(10);
  // CopyModel(*tlocal.get(), *local.get());
  auto &o = tlocal->output_blobs();
  int cnt = 0;
  for (auto &i : o) {
    auto d = i->cpu_data();
    for (int j = 0; j < i->count(); ++j) { loss += d[j]; }
    cnt += i->count();
  }
  loss /= cnt;
  // if (loss < 0.05) break;
  //}
  return loss;
}
//#include "android_file.hpp"
#include <fstream>
void Net::Load(const std::string &model_file) {
  if (model_file != "") {
    if (local != nullptr) {
      if (model_file.size() >= 4 &&
          model_file.compare(model_file.size() - 4, 4, ".bin") == 0) {
        std::ifstream buffer(model_file);
        if (buffer) {
          for (auto &layer : local->layers()) {
            auto &b = layer->blobs();
            if (b.size() > 0) {
              if (strcmp(layer->type(), "Scale") == 0) {
                for (int i = b.size() - 1; i >= 0; --i) {
                  auto &blob = b[i];
                  buffer.read((char *)blob->mutable_cpu_data(),
                              sizeof(float) * blob->count());
                }
              }
            }
          }
          for (auto &layer : local->layers()) {
            auto &b = layer->blobs();
            if (b.size() > 0) {
              if (strcmp(layer->type(), "Scale") != 0)
                for (auto &blob : b)
                  buffer.read((char *)blob->mutable_cpu_data(),
                              sizeof(float) * blob->count());
            }
          }
          flip(local.get());
          buffer.close();
        }
      }
      //** mNet->CopyTrainedLayersFromHDF5(model_file);
      // LoadScale(GetOffsetScaleFile(model_file));
      syncNet(false);
    } else {
      printf("Net structure has not been initialized\n");
      assert(false);
    }
  }
}
void Net::Save(const std::string &model_file) {
  if (model_file != "") {
    if (local != nullptr) {
      syncNet(true);
      if (model_file.size() >= 4 &&
          model_file.compare(model_file.size() - 4, 4, ".bin") == 0) {
        FILE *f = fopen(model_file.c_str(), "wb");
        if (f) {
          flip(local.get());
          for (auto &layer : local->layers()) {
            auto &b = layer->blobs();
            if (b.size() > 0) {
              if (strcmp(layer->type(), "Scale") == 0)
                for (int i = b.size() - 1; i >= 0; --i) {
                  auto &blob = b[i];
                  fwrite(blob->cpu_data(), sizeof(float) * blob->count(), 1, f);
                }
            }
          }
          flip(local.get());
          for (auto &layer : local->layers()) {
            auto &b = layer->blobs();
            if (b.size() > 0) {
              if (strcmp(layer->type(), "Scale") != 0)
                for (auto &blob : b)
                  fwrite(blob->cpu_data(), sizeof(float) * blob->count(), 1, f);
            }
          }
          fclose(f);
        }
      }
      //** mNet->CopyTrainedLayersFromHDF5(model_file);
      // LoadScale(GetOffsetScaleFile(model_file));
    } else {
      printf("Net structure has not been initialized\n");
      assert(false);
    }
  }
}
void Net::check(float weight_decay) {
  if (local != nullptr) {
    syncNet(true);
    float max = 0;
    int nan_count = 0;
    int nan_diff_count = 0;
    int nan_all_count = 0;
    float max_diff = 0;
    for (auto &layer : local->layers()) {
      auto &b = layer->blobs();
      for (auto &blob : b) {
        auto count = blob->count();
        nan_all_count += count;
        {
          auto data = blob->mutable_cpu_data();
          for (int i = 0; i < count; ++i) {
            max = fmaxf(fabsf(data[i]), max);
            if (isnanf(data[i])) {
              data[i] = drand48() * 0.001 - 0.0005;
              ++nan_count;
            }
            // data[i] *= weight_decay;
          }
          data[rand() % count] = drand48() * 0.001;
        }
        {
          auto data = blob->cpu_diff();
          for (int i = 0; i < count; ++i) {
            max_diff = fmaxf(fabsf(data[i]), max_diff);
            if (isnanf(data[i])) { ++nan_diff_count; }
          }
        }
      }
    }
    printf("check: %f, %f (%d, %d)/%d\n", max, max_diff, nan_count,
           nan_diff_count, nan_all_count);
    if (nan_count > 0) syncNet(false);
  }
  //** mNet->CopyTrainedLayersFromHDF5(model_file);
  // LoadScale(GetOffsetScaleFile(model_file));
  else {
    printf("Net structure has not been initialized\n");
    assert(false);
  }
}
void InitCaffe(char **argv) {
  if (argv != NULL) {
    FLAGS_log_dir = "log";
    FLAGS_v = 3;
    // FLAGS_logfile_mode = 0;
    // FLAGS_alsologtostderr = 0;
    int caffe_argc = 1; // hack
    caffe::GlobalInit(&caffe_argc, &argv);
  }
}
extern "C" void layer_name(const char *name);
#include "net.h"
Net local;
int input_count, output_count;
void compute(double *input, double *output) { local.getValue(input, output); }
void load_actor(const char *file) {
  if (!local.local) {
    char app[32] = "deepmimic";
    char *argv[1] = {app};
    InitCaffe(argv);
    local.makeNet(input_count, output_count,
                  "apkoverlay/assets/data/nets/1024_512.txt");
    for (auto &n : local.local->layer_names()) layer_name(n.c_str());
  }
  char path[256] = "apkoverlay/assets/";
  strcat(path, file);
  local.Load(path);
}
#endif