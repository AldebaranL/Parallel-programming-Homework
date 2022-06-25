#include <iostream>
#include"ANN_4.h"
#include<windows.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include"global.h"
using namespace std;


void creat_samples()
{
    TRAIN_MAT = new float* [NUM_SAMPLE];                         //����ѵ������
    for (int i = 0; i < NUM_SAMPLE; ++i)
    {
        TRAIN_MAT[i] = new float[NUM_EACH_LAYER[0]];
    }
    LABEL_MAT = new float* [NUM_SAMPLE]; //���ɱ�ǩ����
    for (int i = 0; i < NUM_SAMPLE; ++i)
    {
        LABEL_MAT[i] = new float[NUM_EACH_LAYER[NUM_LAYERS + 1]];
        for (int j = 0; j < NUM_EACH_LAYER[NUM_LAYERS + 1]; ++j)
        {
            LABEL_MAT[i][j] = 0;
        }
    }
    MAX_NUM_LAYER = NUM_EACH_LAYER[0];
    for (int i = 1; i <= NUM_LAYERS + 1; i++) {
        MAX_NUM_LAYER = max(NUM_EACH_LAYER[i], MAX_NUM_LAYER);
    }
    printf("finished creating samples\n");

}
void print_samples() {
    for (int i = 0; i < min(5, NUM_SAMPLE); i++)
    {
        for (int j = 0; j < min(5, NUM_EACH_LAYER[0]); j++) printf("%f ", TRAIN_MAT[i][j]); printf("\n");
        for (int j = 0; j < min(5, NUM_EACH_LAYER[NUM_LAYERS+1]); j++) printf("%f ", LABEL_MAT[i][j]);
        printf("\n");
    }
}
void delet_samples()
{
    printf("begin delete samples ");
    //�ͷ��ڴ�
    for (int i = 0; i < NUM_SAMPLE; ++i)
        delete[] TRAIN_MAT[i];
    delete[] TRAIN_MAT;
    for (int i = 0; i < NUM_SAMPLE; ++i)
        delete[] LABEL_MAT[i];
    delete[] LABEL_MAT;
    printf("finish delete\n");
}

void read_samples(const char* filename) {
    ifstream inFile(filename, ios::in);
    if (!inFile)
    {
        cout << "���ļ�ʧ�ܣ�" << endl;
        exit(1);
    }
    NUM_SAMPLE = 0;
    string line;
    getline(inFile, line);
    while (getline(inFile, line))//getline(inFile, line)��ʾ���ж�ȡCSV�ļ��е�����
    {
        string field;
        istringstream sin(line); //�������ַ���line���뵽�ַ�����sin��
        for (int j = 0; j < NUM_EACH_LAYER[0]; j++) {
            getline(sin, field, ',');
            TRAIN_MAT[NUM_SAMPLE][j] = atof(field.c_str());
        }
        getline(sin, field);
        LABEL_MAT[NUM_SAMPLE][atoi(field.c_str())] = 1;
        NUM_SAMPLE++;
    }
    inFile.close();
}
int main(const int argc, char* argv[])
{
    creat_samples();
    read_samples("./iris_training.csv");
    print_samples();
    long long head, tail, freq;// timers
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    ANN_4 ann((int*)NUM_EACH_LAYER, 100, 1, NUM_LAYERS, 0.1);
    ann.shuffle(NUM_SAMPLE, TRAIN_MAT, LABEL_MAT);
    //QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
    ann.train(100, TRAIN_MAT, LABEL_MAT);
    ann.get_results(20, &TRAIN_MAT[100], &LABEL_MAT[100]);
    //QueryPerformanceCounter ( (LARGE_INTEGER*) &tail);	// end time
    //cout << "ori:" << (tail - head)*1.0 / freq << "s" << endl;

    delet_samples();
    return 0;
}

