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
    //��ԭʼ�汾ANN�Ͻ�����SIMD�Ż����ڴ���������
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
	int SampleCount;               //�ܵ�ѵ��������
	float*** weights;            //����Ȩֵ
	float** bias;                 //����ƫ��
	float studyRate;               //ѧϰ����

	float* hidenLayerOutput;     //���ز���������ֵ
	float* outputLayerOutput;     //�������������ֵ

	float*** allDeltaBias;        //����������ƫ�ø�����
	float**** allDeltaWeights;    //����������Ȩֵ������
	float** outputMat;            //������������������

	void train_vec(const float* _trainVec, const float* _labelVec, int index);
	void train_vec_cache(const float* _trainVec, const float* _labelVec, int index);
	void train_vec_sse(const float* _trainVec, const float* _labelVec, int index);
    void train_vec_avx(const float* _trainVec, const float* _labelVec, int index);

	float sigmoid(float x) { return 1 / (1 + exp(-1 * x)); }
	bool isNotConver_(const int _sampleNum, float** _labelMat, float _thresh);

};


#endif // ANN_SIMD_ALIGNED_H

