
#include "global.h"

const int NUM_THREADS = 2;

//int NUM_EACH_LAYER[10] = {4,1024*2, 1024*2,1024*2, 4};
//int NUM_EACH_LAYER[100] = { 9,64,64,2 };
int NUM_EACH_LAYER[100] = { 4,64,64,3 };
int NUM_LAYERS = 2;

const int trainClass = 4; //�����
const int numPerClass = 16;  //ÿ��������������

int NUM_SAMPLE = 120;     //�ܵ�������=ÿ��ѵ��������*�����
float** TRAIN_MAT;
float** LABEL_MAT;

int MAX_NUM_LAYER = 1024;