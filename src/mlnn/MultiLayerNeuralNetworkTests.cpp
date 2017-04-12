/*!
 * @file: MultiLayerNeuralNetworkTests.cpp
 * @Author: Tomasz Kornuta <tkornut@us.ibm.com>
 * @Date:   Apr 10, 2017
 *
 * Copyright (c) 2017, Tomasz Kornuta, IBM Corporation. All rights reserved.
 *
 */

#include "MultiLayerNeuralNetworkTests.hpp"



/*!
 * Tests the dimensionality of nn.
 */
TEST_F(Simple2LayerRegressionNN, Dimensions) {

	// Number of layers.
	ASSERT_EQ(nn.layers.size(), 4);
	// First fully connected dimensions.
	ASSERT_EQ(nn.layers[0]->inputSize(), 10);
	ASSERT_EQ(nn.layers[0]->batchSize(), 1);
	ASSERT_EQ(nn.layers[0]->outputSize(), 20);

	// Second fully connected dimensions.
	ASSERT_EQ(nn.layers[2]->inputSize(), 20);
	ASSERT_EQ(nn.layers[2]->batchSize(), 1);
	ASSERT_EQ(nn.layers[2]->outputSize(), 4);

}

/*!
 * Tests the batch resizing.
 */
TEST_F(Simple2LayerRegressionNN, BatchResize) {

	// Check original batch size.
	for (size_t i=0; i< nn.layers.size(); i++)
		ASSERT_EQ(nn.layers[0]->batchSize(), 1);

	// Resize.
	nn.resizeBatch(5);
	// Check new size.
	for (size_t i=0; i< nn.layers.size(); i++)
		ASSERT_EQ(nn.layers[0]->batchSize(), 5);
}


/*!
 * Tests squared error loss function on vectors with four floats.
 */
TEST_F(Simple2LayerRegressionNN, Serialization) {

	// Save network to file.
	const char* fileName = "saved.txt";
	nn.save(fileName);
	//std::cout << "Saved network: \n" << nn;

	// Load network from file.
	mic::mlnn::MultiLayerNeuralNetwork<double, mic::neural_nets::loss::SquaredErrorLoss<double> > restored_nn("simple_linear_network_loaded");
	restored_nn.load(fileName);
	//std::cout << "Restored network: \n" << restored_nn;

	// Compare number of layers.
	ASSERT_EQ(nn.layers.size(), restored_nn.layers.size());

	// Compare layers.
	for (size_t i=0; i< nn.layers.size(); i++) {
		// Compare sizes and types.
		ASSERT_EQ(nn.layers[i]->batchSize(), restored_nn.layers[i]->batchSize());
		ASSERT_EQ(nn.layers[i]->inputSize(), restored_nn.layers[i]->inputSize());
		ASSERT_EQ(nn.layers[i]->outputSize(), restored_nn.layers[i]->outputSize());
		ASSERT_EQ(nn.layers[i]->layer_type, restored_nn.layers[i]->layer_type);
	}//: for

}


/*!
 * Tests a single iteration of a backpropagation algorithm.
 */
TEST_F(Tutorial2LayerNN, Backpropagation) {
	double eps = 1e-5;

	// Forward pass.
	nn.forward(input_x);

	// Lin1 layer output.
	ASSERT_LE( abs( (*nn.layers[0]->s["y"])[0] - (*ffpass1_lin1_y)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->s["y"])[1] - (*ffpass1_lin1_y)[1]), eps);
	// Sig1 layer output.
	ASSERT_LE( abs( (*nn.layers[1]->s["y"])[0] - (*ffpass1_sig1_y)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[1]->s["y"])[1] - (*ffpass1_sig1_y)[1]), eps);
	// Lin1 layer output.
	ASSERT_LE( abs( (*nn.layers[2]->s["y"])[0] - (*ffpass1_lin2_y)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->s["y"])[1] - (*ffpass1_lin2_y)[1]), eps);
	// Sig1 layer output.
	ASSERT_LE( abs( (*nn.layers[3]->s["y"])[0] - (*ffpass1_sig2_y)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[3]->s["y"])[1] - (*ffpass1_sig2_y)[1]), eps);

	// Calculate loss.
	double loss = nn.loss.calculateLoss(target_y, nn.getPredictions());
	ASSERT_LE( abs( loss - ffpass1_loss), eps);

	// Calculate gradient.
	mic::types::MatrixPtr<double> dy = MAKE_MATRIX_PTR(double, 2, 1);
	(*dy) = (*nn.loss.calculateGradient(target_y, nn.getPredictions()));

	// Check gradient.
	ASSERT_LE( abs( (*dy)[0] - (*ffpass1_dy)[0]), eps);
	ASSERT_LE( abs( (*dy)[1] - (*ffpass1_dy)[1]), eps);

	// Backpropagate the gradients from last layer to the first.
	nn.backward(dy);

	// Check weight gradients.
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[0] - (*bwpass1_lin2_dW)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[1] - (*bwpass1_lin2_dW)[1]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[2] - (*bwpass1_lin2_dW)[2]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[3] - (*bwpass1_lin2_dW)[3]), eps);

	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[0] - (*bwpass1_lin1_dW)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[1] - (*bwpass1_lin1_dW)[1]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[2] - (*bwpass1_lin1_dW)[2]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[3] - (*bwpass1_lin1_dW)[3]), eps);

	// Apply changes.
	nn.update(0.5 , 0.0);

	// Check weight gradients after the update.
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[0] - (*bwpass1_lin2_dW_updated)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[1] - (*bwpass1_lin2_dW_updated)[1]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[2] - (*bwpass1_lin2_dW_updated)[2]), eps);
	ASSERT_LE( abs( (*nn.layers[2]->g["W"])[3] - (*bwpass1_lin2_dW_updated)[3]), eps);

	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[0] - (*bwpass1_lin1_dW_updated)[0]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[1] - (*bwpass1_lin1_dW_updated)[1]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[2] - (*bwpass1_lin1_dW_updated)[2]), eps);
	ASSERT_LE( abs( (*nn.layers[0]->g["W"])[3] - (*bwpass1_lin1_dW_updated)[3]), eps);

}






int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
