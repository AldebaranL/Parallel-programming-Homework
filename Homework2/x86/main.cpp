#include <iostream>
#include<ANN.h>
#include<ANN_SIMD.h>
#include<windows.h>
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
	const int hidnodes = 1024; //�������ز�Ľ����
	const int inNodes = 128;   //���������
	const int outNodes = 128;  //���������

	const int trainClass = 8; //�����
	const int numPerClass = 32;  //ÿ��������������

	int sampleN = trainClass * numPerClass;     //�ܵ�������=ÿ��ѵ��������*�����
	float** trainMat = new float* [sampleN];                         //����ѵ������
	for (int k = 0; k < trainClass; ++k) {
		for (int i = k * numPerClass; i < (k + 1) * numPerClass; ++i) {
			trainMat[i] = new float[inNodes];
			for (int j = 0; j < inNodes; ++j) {
				trainMat[i][j] = rand() % 1000 / 10000.0 + 0.1 * (2 * k + 1);

			}
		}
	}

	float** labelMat = new float* [sampleN]; //���ɱ�ǩ����
	for (int k = 0; k < trainClass; ++k) {
		for (int i = k * numPerClass; i < (k + 1) * numPerClass; ++i) {
			labelMat[i] = new float[outNodes];
			for (int j = 0; j < trainClass; ++j) {
				if (j == k)
					labelMat[i][j] = 1;
				else
					labelMat[i][j] = 0;
			}

		}
	}
    long long head, tail, freq;// timers
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

   // /*
    ANN_SIMD ann_simd_classify3(sampleN, inNodes, outNodes, hidnodes, 0.12);

    QueryPerformanceCounter((LARGE_INTEGER*)&head);// start time
	ann_simd_classify3.train_avx(sampleN, trainMat, labelMat);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout <<"avx:"<< (tail - head) * 1000.0 / freq << "ms" << endl;
    //*/
	ANN_SIMD ann_simd_classify1(sampleN, inNodes, outNodes, hidnodes, 0.12);  //�����ΪinNodes����㣬�����outNodes����㣬�������ز�,studyRateΪ0.12

    QueryPerformanceCounter((LARGE_INTEGER*)&head);// start time
	ann_simd_classify1.train_sse(sampleN, trainMat, labelMat);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout <<"sse:"<< (tail - head) * 1000.0 / freq << "ms" << endl;

    ANN_SIMD ann_simd_classify2(sampleN, inNodes, outNodes, hidnodes, 0.12);

    QueryPerformanceCounter((LARGE_INTEGER*)&head);// start time
	ann_simd_classify2.train_cache(sampleN, trainMat, labelMat);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout <<"cache:"<< (tail - head) * 1000.0 / freq << "ms" << endl;



    ANN ann_classify(sampleN, inNodes, outNodes, hidnodes, 0.12);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);// start time
	ann_classify.train(sampleN, trainMat, labelMat);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout <<"original:"<< (tail - head) * 1000.0 / freq << "ms" << endl;

	for (int i = 0; i < 30; ++i) {
		ann_classify.predict(trainMat[i + 120], NULL);
		//std::cout << std::endl;
	}


	//�ͷ��ڴ�
	for (int i = 0; i < sampleN; ++i)
		delete[] trainMat[i];
	delete[] trainMat;

	for (int i = 0; i < sampleN; ++i)
		delete[] labelMat[i];
	delete[] labelMat;

	return 0;
}
