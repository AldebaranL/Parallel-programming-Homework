#include "global.h"

int NUM_THREADS = 4;

//int NUM_EACH_LAYER[10] = {4,1024*2, 1024*2,1024*2, 4};
int NUM_EACH_LAYER[10] = { 1,1,1,1,1 };
int NUM_LAYERS = 1;

const int trainClass = 4; //�����
const int numPerClass = 16;  //ÿ��������������

const int NUM_SAMPLE = trainClass * numPerClass;     //�ܵ�������=ÿ��ѵ��������*�����
float** TRAIN_MAT;
float** LABEL_MAT;

