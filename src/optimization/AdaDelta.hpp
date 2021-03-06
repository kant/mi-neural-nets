/*!
 * Copyright (C) tkornuta, IBM Corporation 2015-2019
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*!
 * @file: AdaDelta.hpp
 * @Author: Tomasz Kornuta <tkornut@us.ibm.com>
 * @Date:   Nov 18, 2016
 *
 * Copyright (c) 2016, IBM Corporation. All rights reserved.
 *
 */

#ifndef ADADELTA_HPP_
#define ADADELTA_HPP_

#include <optimization/OptimizationFunction.hpp>

namespace mic {
namespace neural_nets {
namespace optimization {

/*!
 * \brief Update using AdaDelta - adaptive gradient descent with running average E[g^2] and  E[d^2].
 * \author tkornuta
 */
template <typename eT=float>
class AdaDelta : public OptimizationFunction<eT> {
public:

	/*!
	 * Constructor. Sets dimensions, values of decay (default=0.9) and eps (default=1e-8).
	 * @param rows_ Number of rows of the updated matrix/its gradient.
	 * @param cols_ Number of columns of the updated matrix/its gradient.
	 */
	AdaDelta(size_t rows_, size_t cols_, eT decay_ = 0.9, eT eps_ = 1e-8) : decay(decay_), eps(eps_) {
		EG = MAKE_MATRIX_PTR(eT, rows_, cols_);
		EG->zeros();

		ED = MAKE_MATRIX_PTR(eT, rows_, cols_);
		ED->zeros();

		// Allocate and reset delta.
		delta = MAKE_MATRIX_PTR(eT, rows_, cols_);
		delta->zeros();
	}

	/*!
	 * Calculates the update according to the AdaDelta update rule.
	 * @param x_ Pointer to the current matrix.
	 * @param dx_ Pointer to current gradient of that matrix.
	 * @param learning_rate_ Learning rate (default=0.001). NOT USED!
	 */
	mic::types::MatrixPtr<eT> calculateUpdate(mic::types::MatrixPtr<eT> x_, mic::types::MatrixPtr<eT> dx_, eT learning_rate_) {
		assert(x_->size() == dx_->size());
		assert(x_->size() == EG->size());

/*		for (size_t i=0; i<x_->size(); i++) {
			std::cout << "(*x)["<< i <<"] = " << (*x_)[i] <<std::endl;
		}
		for (size_t i=0; i<x_->size(); i++) {
			std::cout << "(*dx_)["<< i <<"] = " << (*dx_)[i] <<std::endl;
		}*/

		// Update decaying sum of squares of gradients - up to time t.
		for (size_t i=0; i<(size_t)x_->size(); i++) {
			(*EG)[i] = decay *(*EG)[i] + (1.0 - decay) * (*dx_)[i] * (*dx_)[i];
//			std::cout << "(*EG)["<< i <<"] = " << (*EG)[i] <<std::endl;
			assert(std::isfinite((*EG)[i]));
		}

		// Update decaying sum of squares of updates - up to time t-1.
		for (size_t i=0; i<(size_t)x_->size(); i++)
			(*ED)[i] = decay *(*ED)[i] + (1 - decay) * (*delta)[i] * (*delta)[i];

		// Calculate updates - and store as previous (already) = - RMS(ED)/(RMS(G) * dx
		for (size_t i=0; i<(size_t)x_->size(); i++){
//			(*prev_d)[i] = (0.1 / std::sqrt((*EG)[i] + eps)) * (*dx_)[i];
			(*delta)[i] = (std::sqrt((*ED)[i] + eps) / std::sqrt((*EG)[i] + eps)) * (*dx_)[i];
//			std::cout << "(*prev_d)["<< i <<"] = " << (*prev_d)[i] <<std::endl;
			assert(std::isfinite((*delta)[i]));
		}

		// Return the update.
		return delta;
	}

protected:
	/// Decay ratio, similar to momentum.
	eT decay;

	/// Smoothing term that avoids division by zero.
	eT eps;

	/// Decaying average of the squares of gradients up to time t ("diagonal matrix") - E[g^2].
	mic::types::MatrixPtr<eT> EG;

	/// Decaying average of the squares of updates up to time t ("diagonal matrix") - E[delta Theta^2].
	mic::types::MatrixPtr<eT> ED;

	/// Calculated update.
	mic::types::MatrixPtr<eT> delta;
};


} //: optimization
} //: neural_nets
} //: mic

#endif /* ADADELTA_HPP_ */
