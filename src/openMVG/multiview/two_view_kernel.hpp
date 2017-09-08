// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#ifndef ALICEVISION_MULTIVIEW_TWO_VIEW_KERNEL_H_
#define ALICEVISION_MULTIVIEW_TWO_VIEW_KERNEL_H_

#include <vector>
#include "aliceVision/multiview/conditioning.hpp"
#include "aliceVision/numeric/numeric.h"

namespace aliceVision {
namespace two_view {
namespace kernel {

using namespace std;

// This is one example (targeted at solvers that operate on correspondences
// between two views) that shows the "kernel" part of a robust fitting
// problem:
//
//   1. The model; Mat3 in the case of the F or H matrix.
//   2. The minimum number of samples needed to fit; 7 or 8 (or 4).
//   3. A way to convert samples to a model.
//   4. A way to convert a sample and a model to an error.
//
// Of particular note is that the kernel does not expose what the samples are.
// All the robust fitting algorithm sees is that there is some number of
// samples; it is able to fit subsets of them (via the kernel) and check their
// error, but can never access the samples themselves.
//
// The Kernel objects must follow the following concept so that the robust
// fitting alogrithm can fit this type of relation:
//
//   1. Kernel::MAX_MODELS
//   2. Kernel::MINIMUM_SAMPLES
//   3. Kernel::Fit(vector<size_t>, vector<Kernel::Model> *)
//   4. Kernel::Error(size_t, Model) -> error
//
// The fit routine must not clear existing entries in the vector of models; it
// should append new solutions to the end.
template<typename SolverArg,
         typename ErrorArg,
         typename ModelArg = Mat3>
class Kernel {
 public:
  Kernel(const Mat &x1, const Mat &x2) : x1_(x1), x2_(x2) {}
  typedef SolverArg Solver;
  typedef ModelArg  Model;
  /// The minimal number of point required for the model estimation
  enum { MINIMUM_SAMPLES = Solver::MINIMUM_SAMPLES };
  /// The number of models that the minimal solver could return.
  enum { MAX_MODELS = Solver::MAX_MODELS };

  /// Extract required sample and fit model(s) to the sample
  void Fit(const vector<size_t> &samples, vector<Model> *models) const {
    Mat x1 = ExtractColumns(x1_, samples),
        x2 = ExtractColumns(x2_, samples);
    Solver::Solve(x1, x2, models);
  }
  /// Return the error associated to the model and sample^nth point
  double Error(size_t sample, const Model &model) const {
    return ErrorArg::Error(model, x1_.col(sample), x2_.col(sample));
  }
  /// Number of putative point
  size_t NumSamples() const {
    return x1_.cols();
  }
  /// Compute a model on sampled point
  static void Solve(const Mat &x1, const Mat &x2, vector<Model> *models) {
    // By offering this, Kernel types can be passed to templates.
    Solver::Solve(x1, x2, models);
  }
 protected:
  const Mat & x1_, & x2_; // Left/Right corresponding point
};

// Analog Normalized version of the previous Kernel.
template<typename SolverArg,
  typename UnnormalizerArg,
  typename ModelArg = Mat3>
class NormalizedSolver {
public:
  enum { MINIMUM_SAMPLES = SolverArg::MINIMUM_SAMPLES };
  enum { MAX_MODELS = SolverArg::MAX_MODELS };

  static void Solve(const Mat &x1, const Mat &x2, vector<ModelArg> *models) {
    assert(2 == x1.rows());
    assert(MINIMUM_SAMPLES <= x1.cols());
    assert(x1.rows() == x2.rows());
    assert(x1.cols() == x2.cols());

    // Normalize the data.
    Mat3 T1, T2;
    Mat x1_normalized, x2_normalized;
    NormalizePoints(x1, &x1_normalized, &T1);
    NormalizePoints(x2, &x2_normalized, &T2);

    SolverArg::Solve(x1_normalized, x2_normalized, models);
    // Unormalize model from the computed conditioning.
    for (int i = 0; i < models->size(); ++i) {
      UnnormalizerArg::Unnormalize(T1, T2, &(*models)[i]);
    }
  }
};

}  // namespace kernel
}  // namespace two_view
}  // namespace aliceVision

#endif  // ALICEVISION_MULTIVIEW_TWO_VIEW_KERNEL_H_
