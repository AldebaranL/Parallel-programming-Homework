#ifndef ANN_H
#define ANN_H

#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <ctime>
#include <math.h>

class ANN
{
public:
	explicit ANN(int _SampleN, int nNIL, int nNOL, const int nNHL, float _sR = 0.2);
	~ANN();

	void train(int _sampleNum, float** _trainMat, float** _labelMat);
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
	float sigmoid(float x) { return 1 / (1 + exp(-1 * x)); }
	bool isNotConver_(const int _sampleNum, float** _labelMat, float _thresh);

};


#endif // ANN_H
