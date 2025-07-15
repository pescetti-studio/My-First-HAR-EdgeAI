#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	// The input is a 100x6 float matrix
	void lstm_inference(const float input[100][6]);

#ifdef __cplusplus
}
#endif

#endif

