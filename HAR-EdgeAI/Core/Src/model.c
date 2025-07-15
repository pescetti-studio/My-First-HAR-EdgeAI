// Include
#include "model.h"
#include "stm32f4xx_hal.h"
#include <arm_math.h>
#include "activations.h"
#include <string.h>
#include <math.h>

// Global output variables
uint8_t predicted_class;
uint8_t confidence;

// Neural network model structure
// LSTM: (6 features, 100 samples)
// BatchNormalization0: Input 100
// Dense0: Input 100, Output 32 (activation: ReLU)
// BatchNormalization1: Input 32
// Dense1: Input 32, Output 5 (activation: Softmax)

// Declaration of weight arrays (in model_weights.c)
// const float weights_0[6][400];        // lstm_0_w (input_features * 4*units)
// const float weights_1[100][400];    // lstm_0_r (units * 4*units)
// const float weights_2[400];            // lstm_0_b (4*units)
// const float weights_3[100];            // batch_normalization_0_gamma
// const float weights_4[100];            // batch_normalization_0_beta
// const float weights_5[100];            // batch_normalization_0_mean
// const float weights_6[100];            // batch_normalization_0_variance
// const float weights_7[100][32];        // dense_0_w (input_units * output_units)
// const float weights_8[32];            // dense_0_b (output_units)
// const float weights_9[32];            // batch_normalization_1_gamma
// const float weights_10[32];            // batch_normalization_1_beta
// const float weights_11[32];            // batch_normalization_1_mean
// const float weights_12[32];            // batch_normalization_1_variance
// const float weights_13[32][5];        // dense_1_w
// const float weights_14[5];            // dense_1_b

// Definition of the weight structure
struct net_model {
	const float (*lstm_0_w)[400];
	const float (*lstm_0_r)[400];
	const float* lstm_0_b;

	const float* batch_normalization_0_gamma;
	const float* batch_normalization_0_beta;
	const float* batch_normalization_0_mean;
	const float* batch_normalization_0_variance;

	const float (*dense_0_w)[32];
	const float* dense_0_b;

	const float* batch_normalization_1_gamma;
	const float* batch_normalization_1_beta;
	const float* batch_normalization_1_mean;
	const float* batch_normalization_1_variance;

	const float (*dense_1_w)[5];
	const float* dense_1_b;
};

// Bind structure pointers to weight arrays
extern const float weights_0[6][400];
extern const float weights_1[100][400];
extern const float weights_2[400];
extern const float weights_3[100];
extern const float weights_4[100];
extern const float weights_5[100];
extern const float weights_6[100];
extern const float weights_7[100][32];
extern const float weights_8[32];
extern const float weights_9[32];
extern const float weights_10[32];
extern const float weights_11[32];
extern const float weights_12[32];
extern const float weights_13[32][5];
extern const float weights_14[5];


const struct net_model net_weights = { // Renamed from 'weights' to 'net_weights' to avoid conflicts
	.lstm_0_w = weights_0,
	.lstm_0_r = weights_1,
	.lstm_0_b = weights_2,

	.batch_normalization_0_gamma = weights_3,
	.batch_normalization_0_beta = weights_4,
	.batch_normalization_0_mean = weights_5,
	.batch_normalization_0_variance = weights_6,

	.dense_0_w = weights_7,
	.dense_0_b = weights_8,

	.batch_normalization_1_gamma = weights_9,
	.batch_normalization_1_beta = weights_10,
	.batch_normalization_1_mean = weights_11,
	.batch_normalization_1_variance = weights_12,

	.dense_1_w = weights_13,
	.dense_1_b = weights_14
};

// Structure for LSTM cell state
struct LSTM_State {
	float x[6];
	float h[100];
	float one;
	float c[100];
} static lstm_state_values;

static float lstm_activation_buffer[400];
// Buffer for Dense0 output before ReLU/BN1
static float dense_0_output_buffer[32];

// Batch Normalization parameters
static float bn0_scale[100];
static float bn0_bias[100];

static float bn1_scale[32];
static float bn1_bias[32];

// Sigmoid activation function
void act_sigmoid(float* x, unsigned len)
{
	for (unsigned i = 0; i < len; ++i) {
		x[i] = 1.0f / (1.0f + expf(-x[i]));
	}
}

// Tanh1 activation function
void act_tanh1(float* x, unsigned len)
{
	for (unsigned i = 0; i < len; ++i) {
		x[i] = tanhf(x[i]);
	}
}

// Tanh2 activation function
void act_tanh2(float const* restrict x, float* restrict y, unsigned len)
{
	for (unsigned i = 0; i < len; ++i) {
		y[i] = tanhf(x[i]);
	}
}

// Argmax function to find the index of the class with maximum probability
static unsigned argmax(float const* data, unsigned len)
{
	unsigned idx = 0;
	unsigned i = idx;
	while (++i < len)
		if (data[i] > data[idx])
			idx = i;
	return idx;
}

// Softmax function to convert outputs into a probability distribution
static void softmax(float* data, unsigned len)
{
	float sum_exp = 0;
	for (unsigned i = 0; i < len; ++i) {
		data[i] = exp(data[i]);
		sum_exp += data[i];
	}
	if (sum_exp != 0) {
		float inv_sum_exp = 1.0f / sum_exp;
		for (unsigned i = 0; i < len; ++i) {
			data[i] *= inv_sum_exp;
		}
	}
	else {
		for (unsigned i = 0; i < len; ++i) {
			data[i] = 0.0f;
		}
	}
}


// INFERENCE FUNCTION
// x[100][6] is the normalized buffer passed from main.c (2 seconds of data at 50 Hz)
void lstm_inference(const float x[100][6]) {
	const float epsilon = 0.001f;

	// Calculate BatchNormalization parameters
	for (int i = 0; i < 100; ++i) { // BN0 (after LSTM)
		bn0_scale[i] = net_weights.batch_normalization_0_gamma[i] / sqrtf(net_weights.batch_normalization_0_variance[i] + epsilon);
		bn0_bias[i] = net_weights.batch_normalization_0_beta[i] - net_weights.batch_normalization_0_mean[i] * bn0_scale[i];
	}
	for (int i = 0; i < 32; ++i) { // BN1 (after Dense0)
		bn1_scale[i] = net_weights.batch_normalization_1_gamma[i] / sqrtf(net_weights.batch

