#ifndef ANN_SIMD_ALIGNED_H
#define ANN_SIMD_ALIGNED_H

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <ctime>
#include <math.h>
#include <immintrin.h>

class ANN_SIMD_aligned
{
public:
	explicit ANN_SIMD_aligned(int _SampleN, int nNIL, int nNOL, const int nNHL, float _sR = 0.2);
	~ANN_SIMD_aligned();

	void train(int _sampleNum, float** _trainMat, float** _labelMat);
	void train_cache(int _sampleNum, float** _trainMat, float** _labelMat);
	void train_sse(int _sampleNum, float** _trainMat, float** _labelMat);
	void train_avx(int _sampleNum, float** _trainMat, float** _labelMat);

	void predict(float* in, float* proba);

private:
    int MAXTT;
	int numNodesInputLayer;
	int numNodesOutputLayer;
	int numNodesHiddenLayer;
	int SampleCount;               //总的训练样本数
	float*** weights;            //网络权值
	float** bias;                 //网络偏置
	float studyRate;               //学习速率

	float* hidenLayerOutput;     //隐藏层各结点的输出值
	float* outputLayerOutput;     //输出层各结点的输出值

	float*** allDeltaBias;        //所有样本的偏置更新量
	float**** allDeltaWeights;    //所有样本的权值更新量
	float** outputMat;            //所有样本的输出层输出

	void train_vec(const float* _trainVec, const float* _labelVec, int index);
	void train_vec_cache(const float* _trainVec, const float* _labelVec, int index);
	void train_vec_sse(const float* _trainVec, const float* _labelVec, int index);
    void train_vec_avx(const float* _trainVec, const float* _labelVec, int index);

	float sigmoid(float x) { return 1 / (1 + exp(-1 * x)); }
	bool isNotConver_(const int _sampleNum, float** _labelMat, float _thresh);

};


#endif // ANN_SIMD_ALIGNED_H

