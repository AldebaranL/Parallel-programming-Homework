#ifndef ANN_MPI_H
#define ANN_MPI_H

#include"Layer.h"
#include"global.h"
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <ctime>
#include <math.h>
#include<vector>
#include<algorithm>
#include <omp.h>
#include<semaphore.h>
#include <mpi.h>

using namespace std;
class ANN_MPI
{
    //ANN�Ľ��汾2���ع��˿�ܣ�����ָ���ݶ��½���batch_size
public:
    ANN_MPI(int* _num_each_layer, int _num_epoch = 10, int _batch_size = NUM_SAMPLE, int _num_layers = 1, float _study_rate = 0.1);
    ~ANN_MPI();

    void shuffle(const int num_sample, float** _trainMat, float** _labelMat);
    void train(int _sampleNum, float** _trainMat, float** _labelMat);
    void train_MPI_predict(const int num_sample, float** _trainMat, float** _labelMat);
    void train_MPI_predict_nonblocking(const int num_sample, float** _trainMat, float** _labelMat);
    void train_MPI_all_static(const int num_sample, float** _trainMat, float** _labelMat);
    void get_predictions(float* X);
    void display();

private:
    int num_layers;           //�������ز�����Ĭ��Ϊ1
    int* num_each_layer;      //����ά������[0]Ϊ����㣬[numLayers]Ϊ����㣬[1]��[numLayers-1]Ϊ���ز�

    int batch_size;
    int num_epoch;
    vector<Layer*> layers;
    float study_rate;               //ѧϰ����
    friend class Layer;
    void predict(float* in);
    bool isNotConver_(const int _sampleNum, float** _trainMat, float** _labelMat, float _thresh);

    void predict_serial();
    void predict_MPI_static1();
    void predict_MPI_static2();
    void predict_MPI_dynamic();
    //void predict_MPI_dynamic_g();
    //void predict_MPI_reduce();
    void predict_MPI_gather();
    void predict_MPI_alltoall();
    void predict_MPI_threads();
    void predict_MPI_threads_SIMD();
    void predict_MPI_rma();
};

#endif // ANN_MPI_H
