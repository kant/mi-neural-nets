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
 * @file: GradPID.hpp
 * @Author: Tomasz Kornuta <tkornut@us.ibm.com>
 * @Date:   Nov 18, 2016
 *
 * Copyright (c) 2016, IBM Corporation. All rights reserved.
 *
 */

#ifndef GRADPID_HPP_
#define GRADPID_HPP_

#include <optimization/OptimizationFunction.hpp>


namespace mic {
namespace neural_nets {
namespace optimization {

/*!
 * \brief GradPID - adaptive gradient descent with proportional, integral and derivative coefficients.
 * \author tkornuta
 */
template <typename eT=float>
class GradPID : public OptimizationFunction<eT> {
public:

	/*!
	 * Constructor. Sets dimensions, values of decay (default=0.9) and eps (default=1e-8).
	 * @param rows_ Number of rows of the updated matrix/its gradient.
	 * @param cols_ Number of columns of the updated matrix/its gradient.
	 */
	GradPID(size_t rows_, size_t cols_, eT decay_ = 0.9, eT eps_ = 1e-8) : decay(decay_), eps(eps_) {

		Edx = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		Edx->zeros();

		dx_prev = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		dx_prev->zeros();

		deltaP = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaP->zeros();

		deltaI = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaI->zeros();

		deltaD = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaD->zeros();

		delta = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		delta->zeros();

	}

	/*!
	 * Calculates the update according to the GradPID update rule.
	 * @param x_ Pointer to the current matrix.
	 * @param dx_ Pointer to current gradient of that matrix.
	 * @param learning_rate_ Learning rate (default=0.001).
	 */
	mic::types::MatrixPtr<eT> calculateUpdate(mic::types::MatrixPtr<eT> x_, mic::types::MatrixPtr<eT> dx_, eT learning_rate_ = 0.001) {
		assert(x_->size() == dx_->size());
		assert(x_->size() == Edx->size());

		// Initialize ratios and variables.
		p_rate = learning_rate_ * learning_rate_ * learning_rate_ * learning_rate_ ;
		i_rate = learning_rate_;
		d_rate = learning_rate_ * learning_rate_ * learning_rate_ ;

		// Update decaying sum of gradients - up to time t.
		for (size_t i=0; i< (size_t)Edx->size(); i++) {
			(*Edx)[i] = decay *(*Edx)[i] + (1.0 - decay) * (*dx_)[i];
			assert(std::isfinite((*Edx)[i]));
		}

		// DEBUG
/*		for(size_t i=0; i< delta->size(); i++){
			std::cout<< "(*x_)[" << i << "]=" << (*x_)[i] << std::endl;
		}
		for(size_t i=0; i< delta->size(); i++){
			std::cout<< "(*dx_)[" << i << "]=" << (*dx_)[i] << std::endl;
		}*/

		// Calculate gradients updates.
		// Proportional.
		for(size_t i=0; i< (size_t)delta->size(); i++){
			(*deltaP)[i] = p_rate * (*dx_)[i];
//			std::cout<< "(*deltaP)[" << i << "]=" << (*deltaP)[i] << std::endl;
		}

		// Integral.
		for(size_t i=0; i< (size_t)delta->size(); i++){
			(*deltaI)[i] = i_rate * (*Edx)[i];
//			std::cout<< "(*deltaI)[" << i << "]=" << (*deltaI)[i] << std::endl;
		}

		// Derivative.
		for(size_t i=0; i< (size_t)delta->size(); i++){
			(*deltaD)[i] = d_rate * ((*dx_)[i] - (*dx_prev)[i]);
//			std::cout<< "(*deltaD)[" << i << "]=" << (*deltaD)[i] << std::endl;
		}

		// Calculate the update.
		for(size_t i=0; i< (size_t)delta->size(); i++) {
			(*delta)[i] = (*deltaP)[i] + (*deltaI)[i] + (*deltaD)[i];
//			std::cout<< "(*delta)[" << i << "]=" << (*delta)[i] << std::endl;
			assert(std::isfinite((*delta)[i]));
		}

		// Store past gradients.
		for (size_t i=0; i< (size_t)dx_->size(); i++) {
			(*dx_prev)[i] = (*dx_)[i];
		}

//		std::cout << std::endl;

		// Return the update.
		return delta;
	}

protected:

	/// Decay ratio, similar to momentum.
	eT decay;

	/// Smoothing term that avoids division by zero.
	eT eps;

	/// Adaptive proportional factor (learning rate).
	eT p_rate;

	/// Adaptive integral factor (learning rate).
	eT i_rate;

	/// Adaptive proportional factor (learning rate).
	eT d_rate;

	/// Decaying average of gradients up to time t - E[g].
	mic::types::MatrixPtr<eT> Edx;

	/// Previous value of gradients.
	mic::types::MatrixPtr<eT> dx_prev;

	/// Proportional update.
	mic::types::MatrixPtr<eT> deltaP;

	/// Integral update.
	mic::types::MatrixPtr<eT> deltaI;

	/// Derivative update.
	mic::types::MatrixPtr<eT> deltaD;

	/// Calculated update.
	mic::types::MatrixPtr<eT> delta;
};




/*!
 * \brief AdaGradPID - adaptive gradient descent with proportional, integral and derivative coefficients.
 * \author tkornuta
 */
template <typename eT=float>
class AdaGradPID : public OptimizationFunction<eT> {
public:

	/*!
	 * Constructor. Sets dimensions, values of decay (default=0.9) and eps (default=1e-8).
	 * @param rows_ Number of rows of the updated matrix/its gradient.
	 * @param cols_ Number of columns of the updated matrix/its gradient.
	 */
	AdaGradPID(size_t rows_, size_t cols_, eT decay_ = 0.9, eT eps_ = 1e-8) : decay(decay_), eps(eps_) {
		Edx = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		Edx->zeros();

		dx_prev = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		dx_prev->zeros();

		deltaP = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaP->zeros();

		deltaI = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaI->zeros();

		deltaD = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		deltaD->zeros();

		delta = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		delta->zeros();

		// Initialize ratios and variables.
		p_rate = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		p_rate->zeros();

		// Initialize ratios and variables.
		i_rate = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		i_rate->zeros();

		// Initialize ratios and variables.
		d_rate = MAKE_MATRIX_PTR(eT, rows_,  cols_);
		d_rate->zeros();

	}

	/*!
	 * Calculates the update according to the AdaGradPID update rule.
	 * @param x_ Pointer to the current matrix.
	 * @param dx_ Pointer to current gradient of that matrix.
	 * @param learning_rate_ Learning rate (default=0.001).
	 */
	mic::types::MatrixPtr<eT> calculateUpdate(mic::types::MatrixPtr<eT> x_, mic::types::MatrixPtr<eT> dx_, eT learning_rate_ = 0.001) {
		assert(x_->size() == dx_->size());
		assert(x_->size() == Edx->size());

		// Update decaying sum of gradients - up to time t.
/*		for (size_t i=0; i<Edx->size(); i++) {
			(*Edx)[i] = decay *(*Edx)[i] + (1.0 - decay) * (*dx_)[i];
			assert(std::isfinite((*Edx)[i]));
		}*/

		// DEBUG
/*		std::cout << "surprisal = " << surprisal << std::endl;
		std::cout << "p_rate = " << p_rate << std::endl;
		std::cout << "i_rate = " << i_rate << std::endl;
		std::cout << "d_rate = " << d_rate << std::endl;*/
/*		std::cout<< " x: ";
		for(size_t i=0; i< delta->size(); i++)
			std::cout<< (*x_)[i] << " | ";
		std::cout << std::endl;

		std::cout<< " dx: ";
		for(size_t i=0; i< delta->size(); i++)
			std::cout<<  (*dx_)[i] << " | ";
		std::cout << std::endl << std::endl;*/

		// Calculate gradients updates.
		// Proportional.
//		std::cout<< " deltaP: ";
		for(size_t i=0; i< (size_t)delta->size(); i++){
			// Calculate surprisal.
//			double surp_p = logistic(calculateSigmoidSurprisalMod((*deltaP)[i], (*dx_)[i]), 1.0);
//			(*deltaP)[i] = (1-surp_p) * (*deltaP)[i] + (*p_rate)[i] * surp_p * (*dx_)[i];
//			std::cout<< (*deltaP)[i] << " | ";
		}
//		std::cout << std::endl ;

		// Integral.
//		std::cout<< " deltaI: ";
		for(size_t i=0; i< (size_t)delta->size(); i++){
//			double surp_i = logistic(calculateSigmoidSurprisalMod((*deltaI)[i], (decay *(*deltaI)[i] + (1.0 - decay) * (*dx_)[i])), 1.0);
//			(*deltaI)[i] = (1-surp_i) * (*deltaI)[i] + (*i_rate)[i] * surp_i * (decay *(*deltaI)[i] + (1.0 - decay) * (*dx_)[i]);
//			std::cout<< (*deltaI)[i] << " | ";
		}
//		std::cout << std::endl ;

		// Derivative.
//		std::cout<< " deltaD: ";
		for(size_t i=0; i< (size_t)delta->size(); i++){
//			double surp_d = logistic(calculateSigmoidSurprisalMod((*deltaD)[i], ((*dx_)[i] - (*dx_prev)[i])), 1.0);
//			(*deltaD)[i] = (1-surp_d) * (*deltaD)[i] + (*d_rate)[i] * surp_d * ((*dx_)[i] - (*dx_prev)[i]);
//			std::cout<< (*deltaD)[i] << " | ";
		}
//		std::cout << std::endl ;

		// Adaptive rate update.
/*		// Update P ratio.
		eT up;
		std::cout<< " p_rate: ";
		for (size_t i=0; i<delta->size(); i++){
//			up = std::abs((*deltaP)[i] / ((*dx_)[i] + eps)); // softsign
//			std::cout << "up = " << up << std::endl;
//			std::cout << "tanh(up) = " << tanh(up) << std::endl;
			//decay * (*p_rate)[i] + (1 - decay) * tanh(up);//(logistic<eT>(up ,5) / 5);
			std::cout << (*p_rate)[i] << " | ";
		}
		std::cout << std::endl;

		// Update I ratio.
		eT ui;
		std::cout<< " i_rate: ";
		for (size_t i=0; i<delta->size(); i++){
//			ui = std::abs((*deltaI)[i] / ((*dx_)[i] + eps)); // softsign
//			(*i_rate)[i] = decay * (*i_rate)[i] + (1 - decay) * calculateSigmoidSurprisal((*i_rate)[i], ui);
			//decay * (*i_rate)[i] + (1 - decay) * tanh(ui);//(logistic<eT>(ui ,5) / 5);
			std::cout << (*i_rate)[i] << " | ";
		}
		std::cout << std::endl ;

		// Update D ratio.
		eT ud;
/		std::cout<< " d_rate: ";
		for (size_t i=0; i<delta->size(); i++){
//			ud = std::abs((*deltaD)[i] / ((*dx_)[i] + eps)); // softsign
//			(*d_rate)[i] = decay * (*d_rate)[i] + (1 - decay) * calculateSigmoidSurprisal((*d_rate)[i], ud);
			//decay * (*d_rate)[i] + (1 - decay) * tanh(ud);//(logistic<eT>(ud, 5) / 5);
			std::cout << (*d_rate)[i] << " | ";
		}
		std::cout << std::endl ;*/

		// Calculate update.
//		std::cout<< " delta: ";
		for(size_t i=0; i< (size_t)delta->size(); i++) {
			(*delta)[i] = (*deltaP)[i] + (*deltaI)[i] + (*deltaD)[i];
	//		std::cout<< (*delta)[i] << " | ";
			assert(std::isfinite((*delta)[i]));
		}
	//	std::cout << std::endl << std::endl;

		// Store past gradiens.
		for (size_t i=0; i< (size_t)dx_->size(); i++) {
			(*dx_prev)[i] = (*dx_)[i];
		}

		// Return the update.
		return delta;
//		std::cout << "-------------------" <<  std::endl;
	}

protected:		// Initialize ratios and variables.

	/// Decay ratio, similar to momentum.
	eT decay;

	/// Smoothing term that avoids division by zero.
	eT eps;

	/// Adaptive proportional factor (learning rate).
	mic::types::MatrixPtr<eT> p_rate;

	/// Adaptive integral factor (learning rate).
	mic::types::MatrixPtr<eT> i_rate;

	/// Adaptive proportional factor (learning rate).
	mic::types::MatrixPtr<eT> d_rate;

	/// Surprisal - for feed forward nets it is based on the difference between the prediction and target.
//	eT surprisal;

	/// Decaying average of gradients up to time t - E[g].
	mic::types::MatrixPtr<eT> Edx;

	/// Previous value of gradients.
	mic::types::MatrixPtr<eT> dx_prev;

	/// Proportional update.
	mic::types::MatrixPtr<eT> deltaP;

	/// Integral update.
	mic::types::MatrixPtr<eT> deltaI;

	/// Derivative update.
	mic::types::MatrixPtr<eT> deltaD;

	/// Calculated update.
	mic::types::MatrixPtr<eT> delta;
};

} //: optimization
} //: neural_nets
} //: mic

#endif /* GRADPID_HPP_ */
