#ifndef LAYER_H
#define LAYER_H

#include<stdlib.h>
#include<math.h>
#include<stdio.h>
#include"global.h"

class Layer
{

public:
    Layer(int, int, activationType);
    virtual ~Layer();
    void _forward(float*);
    void display();

protected:
    float** weights,** weights_delta;//Ϊ�Ż����ʴ���weightsΪ[�����ά��][�����ά��]
    float* bias;//[�����ά��]
    int num_input, num_output;
    float* output_nodes;//[�����ά��]
    float* bias_delta;//[�����ά��]
    float activation_function(float);
    float derivative_activation_function(float);
    friend class ANN_4;
    activationType activation_type;//��ָ�������
};

#endif // LAYER_H
