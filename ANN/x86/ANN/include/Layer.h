#ifndef LAYER_H
#define LAYER_H

#include<stdlib.h>
#include<math.h>
#include<stdio.h>
class Layer
{

public:
    Layer (int, int);
    virtual ~Layer();
    void _forward (float *);
    void display();

protected:
    float **weights;//Ϊ�Ż����ʴ���weightsΪ[�����ά��][�����ά��]
    float *bias;//[�����ά��]
    int num_input, num_output;
    float *output_nodes;
    float *error;//��ANN_2���Ժ���ʹ��
    float *delta;//[�����ά��]
    float activation_function (float);
    float derivative_activation_function (float);
    friend class ANN_pthread;
    friend class ANN_new;
    friend class ANN_1;
    friend class ANN_2;
    friend class ANN_3;
    friend class ANN_openMP;
    enum {sigmoid,tanh} activation_type;//��ָ�������
};

#endif // LAYER_H
