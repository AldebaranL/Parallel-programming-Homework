#ifndef ANN_new_H
#define ANN_new_H

#include<Layer.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <ctime>
#include <math.h>
#include<vector>
#include<algorithm>
#include <semaphore.h>
#include"global.h"
//../../Sources/include/
using namespace std;

class ANN_new
{
public:
    ANN_new ( int* _num_each_layer, int _num_epoch = 10, int _batch_size = 1, int _num_layers = 1, float _study_rate = 0.1);
    ~ANN_new();

    void shuffle (const int num_sample, float** _trainMat, float** _labelMat);
    void train (int _sampleNum, float** _trainMat, float** _labelMat);
    void get_predictions (float* X);
    void display();

    void* threadFunc_sem (void *param);
    void* threadFunc_sem_SIMD (void *param);
private:
    int num_layers;           //�������ز�����Ĭ��Ϊ1
    int* num_each_layer;            //����ά����[0]Ϊ����㣬[numLayers]Ϊ����㣬[1]��[numLayers-1]Ϊ���ز�

    int batch_size;
    int num_epoch;
    vector<Layer*> layers;
    float study_rate;               //ѧϰ����
    void back_propagation (float* X, float * Y);
    void predict (float* in);

    //float sigmoid(float x) { return 1 / (1 + exp(-1 * x)); }
    //float Dsigmoid(float x) { return sigmoid(x)*(1-sigmoid(x)); }
    bool isNotConver_ (const int _sampleNum, float** _trainMat, float** _labelMat, float _thresh);
    friend class Layer;

    //int sample_index;
    //int num_sample;


    //extern sem_t *sem_before_bp;// ÿ���߳����Լ�ר�����ź���
    //extern sem_t *sem_before_fw;
    //extern sem_t sem_main_after_bp;
    //extern sem_t sem_main_after_fw;



};

#endif // ANN_new_H
