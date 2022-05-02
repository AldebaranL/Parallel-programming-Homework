#ifndef GLOBAL_H
#define GLOBAL_H

#include<semaphore.h>
#include<pthread.h>
#include"ANN_pthread.h"
//#include <iostream>
//using namespace std;
class ANN_pthread;
typedef struct
{
    int t_id; //�߳� id
    float **sampleMat;
    ANN_pthread* class_pointer;
} threadParam_t;
extern const int NUM_THREADS;
extern sem_t *sem_before_bp;// ÿ���߳����Լ�ר�����ź���
extern sem_t *sem_before_fw;
extern sem_t sem_main_after_bp;
extern sem_t sem_main_after_fw;

extern    pthread_t *handles;
extern   threadParam_t *params;

extern const int trainClass ; //�����
extern const int numPerClass;  //ÿ��������������

extern int NUM_EACH_LAYER[10];
extern int NUM_LAYERS;

extern const int NUM_SAMPLE;     //�ܵ�������=ÿ��ѵ��������*�����
extern float** TRAIN_MAT;

extern float** LABEL_MAT;
//extern  sem_t	sem_parent;
//extern  sem_t	sem_children;
void creat_params();
void delet_params();

#endif
