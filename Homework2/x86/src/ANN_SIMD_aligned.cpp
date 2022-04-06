#include "ANN_SIMD_aligned.h"


ANN_SIMD_aligned::ANN_SIMD_aligned(int _SampleN, int nNIL, int nNOL, const int nNHL, float _sR) :
	SampleCount(_SampleN), numNodesInputLayer(nNIL), numNodesOutputLayer(nNOL),
	numNodesHiddenLayer(nNHL), studyRate(_sR)
{
    MAXTT=16;
	//����Ȩֵ�ռ�,����ʼ��
	srand(time(NULL));
	weights = (float***)_aligned_malloc(sizeof(float**)*2, 256);
	weights[0] =(float**)_aligned_malloc(sizeof(float*)*numNodesInputLayer, 256);//new float* [numNodesInputLayer];
	for (int i = 0; i < numNodesInputLayer; ++i) {
		weights[0][i] = (float*)_aligned_malloc(sizeof(float)*numNodesHiddenLayer, 256);//new float[numNodesHiddenLayer];
		for (int j = 0; j < numNodesHiddenLayer; ++j) {
			weights[0][i][j] = (rand() % (2000) / 1000.0 - 1.0); //-1��1֮��
		}
	}
	weights[1] = (float**)_aligned_malloc(sizeof(float*)*numNodesHiddenLayer, 256);//new float* [numNodesHiddenLayer];
	for (int i = 0; i < numNodesHiddenLayer; ++i) {
		weights[1][i] = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];
		for (int j = 0; j < numNodesOutputLayer; ++j) {
			weights[1][i][j] = (rand() % (2000) / 1000.0 - 1.0); //-1��1֮��
		}
	}

	//����ƫ�ÿռ䣬����ʼ��
	bias = (float**)_aligned_malloc(sizeof(float*)*2, 256);//new float* [2];
	bias[0] = (float*)_aligned_malloc(sizeof(float)*numNodesHiddenLayer, 256);//new float[numNodesHiddenLayer];
	for (int i = 0; i < numNodesHiddenLayer; ++i) {
		bias[0][i] = (rand() % (2000) / 1000.0 - 1.0); //-1��1֮��
	}
	bias[1] = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];
	for (int i = 0; i < numNodesOutputLayer; ++i) {
		bias[1][i] = (rand() % (2000) / 1000.0 - 1.0); //-1��1֮��
	}

	//�������ز���������ֵ�ռ�
	hidenLayerOutput = (float*)_aligned_malloc(sizeof(float)*numNodesHiddenLayer, 256);//new float[numNodesHiddenLayer];
	//�����������������ֵ�ռ�
	outputLayerOutput = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];

	//��������������Ȩֵ�������洢�ռ�
	allDeltaWeights = (float****)_aligned_malloc(sizeof(float***)*_SampleN, 256);//new float*** [_SampleN];
	for (int k = 0; k < _SampleN; ++k) {
		allDeltaWeights[k] = (float***)_aligned_malloc(sizeof(float**)*2, 256);//new float** [2];
		allDeltaWeights[k][0] = (float**)_aligned_malloc(sizeof(float*)*numNodesInputLayer, 256);//new float* [numNodesInputLayer];
		for (int i = 0; i < numNodesInputLayer; ++i) {
			allDeltaWeights[k][0][i] = (float*)_aligned_malloc(sizeof(float)*numNodesHiddenLayer, 256);//new float[numNodesHiddenLayer];
		}
		allDeltaWeights[k][1] = (float**)_aligned_malloc(sizeof(float*)*numNodesHiddenLayer, 256);//new float* [numNodesHiddenLayer];
		for (int i = 0; i < numNodesHiddenLayer; ++i) {
			allDeltaWeights[k][1][i] = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];
		}
	}

	//��������������ƫ�ø������洢�ռ�
	allDeltaBias = (float***)_aligned_malloc(sizeof(float**)*_SampleN, 256);//new float** [_SampleN];
	for (int k = 0; k < _SampleN; ++k) {
		allDeltaBias[k] = (float**)_aligned_malloc(sizeof(float*)*2, 256);//new float* [2];
		allDeltaBias[k][0] = (float*)_aligned_malloc(sizeof(float)*numNodesHiddenLayer, 256);//new float[numNodesHiddenLayer];
		allDeltaBias[k][1] = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];
	}

	//�����洢�������������������ռ�
	outputMat = (float**)_aligned_malloc(sizeof(float*)*_SampleN, 256);//new float* [_SampleN];
	for (int k = 0; k < _SampleN; ++k) {
		outputMat[k] = (float*)_aligned_malloc(sizeof(float)*numNodesOutputLayer, 256);//new float[numNodesOutputLayer];
	}


}

ANN_SIMD_aligned::~ANN_SIMD_aligned()
{
	//�ͷ�Ȩֵ�ռ�
	for (int i = 0; i < numNodesInputLayer; ++i)
		_aligned_free(weights[0][i]);
	for (int i = 1; i < numNodesHiddenLayer; ++i)
		_aligned_free(weights[1][i]);
	for (int i = 0; i < 2; ++i)
		_aligned_free(weights[i]);
	_aligned_free(weights);

	//�ͷ�ƫ�ÿռ�
	for (int i = 0; i < 2; ++i)
		_aligned_free(bias[i]);
	_aligned_free(bias);

	//�ͷ�����������Ȩֵ�������洢�ռ�
	for (int k = 0; k < SampleCount; ++k) {
		for (int i = 0; i < numNodesInputLayer; ++i)
			_aligned_free(allDeltaWeights[k][0][i]);
		for (int i = 1; i < numNodesHiddenLayer; ++i)
			_aligned_free(allDeltaWeights[k][1][i]);
		for (int i = 0; i < 2; ++i)
			_aligned_free(allDeltaWeights[k][i]);
		_aligned_free(allDeltaWeights[k]);
	}
	_aligned_free(allDeltaWeights);

	//�ͷ�����������ƫ�ø������洢�ռ�
	for (int k = 0; k < SampleCount; ++k) {
		for (int i = 0; i < 2; ++i)
			_aligned_free(allDeltaBias[k][i]);
		_aligned_free(allDeltaBias[k]);
	}
	_aligned_free(allDeltaBias);

	//�ͷŴ洢�������������������ռ�
	for (int k = 0; k < SampleCount; ++k)
		_aligned_free(outputMat[k]);
	_aligned_free(outputMat);

}

void ANN_SIMD_aligned::train(const int _sampleNum, float** _trainMat, float** _labelMat)
{
	float thre = 1e-2;
	for (int i = 0; i < _sampleNum; ++i) {
		train_vec(_trainMat[i], _labelMat[i], i);
	}
	for(int tt = 0; tt < MAXTT; tt++){
		if(!isNotConver_(_sampleNum, _labelMat, thre)) break;
		//����Ȩֵ
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesInputLayer; ++i) {
				for (int j = 0; j < numNodesHiddenLayer; ++j) {
					weights[0][i][j] -= studyRate * allDeltaWeights[index][0][i][j];
				}
			}
			for (int i = 0; i < numNodesHiddenLayer; ++i) {
				for (int j = 0; j < numNodesOutputLayer; ++j) {
					weights[1][i][j] -= studyRate * allDeltaWeights[index][1][i][j];
				}
			}
		}

		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesHiddenLayer; ++i) {
				bias[0][i] -= studyRate * allDeltaBias[index][0][i];
			}
			for (int i = 0; i < numNodesOutputLayer; ++i) {
				bias[1][i] -= studyRate * allDeltaBias[index][1][i];
			}
		}

		for (int i = 0; i < _sampleNum; ++i) {
			train_vec(_trainMat[i], _labelMat[i], i);
		}
	}

	//printf("ѵ��Ȩֵ��ƫ�óɹ��ˣ�\n");
}

void ANN_SIMD_aligned::train_vec(const float* _trainVec, const float* _labelVec, int index)
{
	//��������ز�������
	for (int i = 0; i < numNodesHiddenLayer; ++i) {
		float z = 0.0;
		for (int j = 0; j < numNodesInputLayer; ++j) {
			z += _trainVec[j] * weights[0][j][i];
		}
		z += bias[0][i];
		hidenLayerOutput[i] = sigmoid(z);

	}

	//���������������ֵ
	for (int i = 0; i < numNodesOutputLayer; ++i) {
		float z = 0.0;
		for (int j = 0; j < numNodesHiddenLayer; ++j) {
			z += hidenLayerOutput[j] * weights[1][j][i];
		}
		z += bias[1][i];
		outputLayerOutput[i] = sigmoid(z);
		outputMat[index][i] = outputLayerOutput[i];
	}

	//����ƫ�ü�Ȩ�ظ���������������

	for (int j = 0; j < numNodesOutputLayer; ++j) {
		allDeltaBias[index][1][j] = (-0.1) * (_labelVec[j] - outputLayerOutput[j]) * outputLayerOutput[j]
			* (1 - outputLayerOutput[j]);
		for (int i = 0; i < numNodesHiddenLayer; ++i) {
			allDeltaWeights[index][1][i][j] = allDeltaBias[index][1][j] * hidenLayerOutput[i];
		}
	}
	for (int j = 0; j < numNodesHiddenLayer; ++j) {
		float z = 0.0;
		for (int k = 0; k < numNodesOutputLayer; ++k) {
			z += weights[1][j][k] * allDeltaBias[index][1][k];
		}
		allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
		for (int i = 0; i < numNodesInputLayer; ++i) {
			allDeltaWeights[index][0][i][j] = allDeltaBias[index][0][j] * _trainVec[i];
		}
	}

}


void ANN_SIMD_aligned::train_cache(const int _sampleNum, float** _trainMat, float** _labelMat)
{
	float thre = 1e-4;
	for (int i = 0; i < _sampleNum; ++i) {
		train_vec_cache(_trainMat[i], _labelMat[i], i);
	}
	for(int tt = 0; tt < MAXTT; tt++){
		if(!isNotConver_(_sampleNum, _labelMat, thre)) break;
		//����Ȩֵ
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesInputLayer; ++i) {
				for (int j = 0; j < numNodesHiddenLayer; j+=4) {
                    //��·ѭ��չ��
                    weights[0][i][j] -= studyRate * allDeltaWeights[index][0][i][j];
                    weights[0][i][j+1] -= studyRate * allDeltaWeights[index][0][i][j+1];
                    weights[0][i][j+2] -= studyRate * allDeltaWeights[index][0][i][j+2];
                    weights[0][i][j+3] -= studyRate * allDeltaWeights[index][0][i][j+3];
				}
			}
			for (int i = 0; i < numNodesHiddenLayer; ++i) {
				for (int j = 0; j < numNodesOutputLayer; j+=4) {
                    //��·ѭ��չ��
					weights[1][i][j] -= studyRate * allDeltaWeights[index][1][i][j];
					weights[1][i][j+1] -= studyRate * allDeltaWeights[index][1][i][j+1];
					weights[1][i][j+2] -= studyRate * allDeltaWeights[index][1][i][j+2];
					weights[1][i][j+3] -= studyRate * allDeltaWeights[index][1][i][j+3];
				}
			}
		}

		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesHiddenLayer; i+=4) {
                //��·ѭ��չ��
				bias[0][i] -= studyRate * allDeltaBias[index][0][i];
				bias[0][i+1] -= studyRate * allDeltaBias[index][0][i+1];
				bias[0][i+2] -= studyRate * allDeltaBias[index][0][i+2];
				bias[0][i+3] -= studyRate * allDeltaBias[index][0][i+3];
			}
			for (int i = 0; i < numNodesOutputLayer; i+=4) {
                //��·ѭ��չ��
				bias[1][i] -= studyRate * allDeltaBias[index][1][i];
				bias[1][i+1] -= studyRate * allDeltaBias[index][1][i+1];
				bias[1][i+2] -= studyRate * allDeltaBias[index][1][i+2];
				bias[1][i+3] -= studyRate * allDeltaBias[index][1][i+3];
			}
		}

		for (int i = 0; i < _sampleNum; ++i) {
			train_vec_cache(_trainMat[i], _labelMat[i], i);
		}
	}

	//printf("ѵ��Ȩֵ��ƫ�óɹ��ˣ�\n");
}
void ANN_SIMD_aligned::train_vec_cache(const float* _trainVec, const float* _labelVec, int index)
{
	//��������ز�������
	for(int i=0;i<numNodesHiddenLayer;i++){
        hidenLayerOutput[i]=0.0;
	}
	//��weights,���Ϊ���з���weights[0]
	for (int j = 0; j < numNodesInputLayer; ++j) {
        for (int i = 0; i < numNodesHiddenLayer; ++i) {
			hidenLayerOutput[i] += _trainVec[j] * weights[0][j][i];
		}
	}
	for(int i=0;i<numNodesHiddenLayer;i++){
        hidenLayerOutput[i]+=bias[0][i];
        hidenLayerOutput[i]=sigmoid(hidenLayerOutput[i]);
	}

	//���������������ֵ
	for(int i=0;i<numNodesOutputLayer;i++){
        outputLayerOutput[i]=0.0;
	}
	//���Ϊ���з���weights[1]
	for (int j = 0; j < numNodesHiddenLayer; ++j) {
        for (int i = 0; i < numNodesOutputLayer; ++i) {
			outputLayerOutput[i] += hidenLayerOutput[j] * weights[1][j][i];
		}
	}
	for(int i=0;i<numNodesOutputLayer;i++){
        outputLayerOutput[i]+=bias[1][i];
        outputLayerOutput[i]=sigmoid(outputLayerOutput[i]);
        outputMat[index][i] = outputLayerOutput[i];
	}

	//����ƫ�ü�Ȩ�ظ���������������

	for (int j = 0; j < numNodesOutputLayer; ++j) {
		allDeltaBias[index][1][j] = (-0.1) * (_labelVec[j] - outputLayerOutput[j]) * outputLayerOutput[j]
			* (1 - outputLayerOutput[j]);
	}
    for (int i = 0; i < numNodesHiddenLayer; ++i) {
        for (int j = 0; j < numNodesOutputLayer; ++j) {
			allDeltaWeights[index][1][i][j] = allDeltaBias[index][1][j] * hidenLayerOutput[i];
		}
	}

	for (int j = 0; j < numNodesHiddenLayer; ++j) {
		float z = 0.0;
		//���з��ʣ�k��
		for (int k = 0; k < numNodesOutputLayer; ++k) {
			z += weights[1][j][k] * allDeltaBias[index][1][k];
		}
		allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
	}
    for (int i = 0; i < numNodesInputLayer; ++i) {
        for (int j = 0; j < numNodesHiddenLayer; ++j) {
			allDeltaWeights[index][0][i][j] = allDeltaBias[index][0][j] * _trainVec[i];
		}
	}
}
void ANN_SIMD_aligned::train_sse(const int _sampleNum, float** _trainMat, float** _labelMat)
{
	float thre = 1e-2;
	for (int i = 0; i < _sampleNum; ++i) {
		train_vec(_trainMat[i], _labelMat[i], i);
	}
	for(int tt = 0; tt < MAXTT; tt++){
		if(!isNotConver_(_sampleNum, _labelMat, thre)) break;
		//����Ȩֵ
		__m128 sr = _mm_set1_ps(studyRate);
        __m128 t1, t2;
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesInputLayer; ++i) {
				for (int j = 0; j <= numNodesHiddenLayer-4; j+=4) {
                    //���ڲ�ѭ����Ϊ4λ����������
				    t1 = _mm_load_ps(allDeltaWeights[index][0][i] + j);
				    t1 = _mm_mul_ps(t1, sr);
				    t2 = _mm_load_ps(weights[0][i] + j);
				    t2 = _mm_sub_ps(t2,t1);
                    //������
                    _mm_store_ps(weights[0][i] + j, t2);
					//weights[0][i][j] -= studyRate * allDeltaWeights[index][0][i][j];
				}
			}
			for (int i = 0; i < numNodesHiddenLayer; ++i) {
				for (int j = 0; j <= numNodesOutputLayer-4; j+=4) {
                    t1 = _mm_load_ps(allDeltaWeights[index][1][i] + j);
				    t1 = _mm_mul_ps(t1, sr);
				    t2 = _mm_load_ps(weights[1][i] + j);
				    t2 = _mm_sub_ps(t2,t1);
                    _mm_store_ps(weights[1][i] + j, t2);
					//weights[1][i][j] -= studyRate * allDeltaWeights[index][1][i][j];
				}
			}
		}
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i <= numNodesHiddenLayer-4; i+=4) {
                t1 = _mm_load_ps(allDeltaBias[index][0]+i);
                t1 = _mm_mul_ps(t1, sr);
                t2 = _mm_load_ps(bias[0] + i);
                t2 = _mm_sub_ps(t2,t1);
                _mm_store_ps(bias[0] + i, t2);
				//bias[0][i] -= studyRate * allDeltaBias[index][0][i];
			}
			for (int i = 0; i <= numNodesOutputLayer-4; i+=4) {
			    t1 = _mm_load_ps(allDeltaBias[index][1]+i);
                t1 = _mm_mul_ps(t1, sr);
                t2 = _mm_load_ps(bias[1] + i);
                t2 = _mm_sub_ps(t2,t1);
                _mm_store_ps(bias[1] + i, t2);
				//bias[1][i] -= studyRate * allDeltaBias[index][1][i];
			}
		}

		for (int i = 0; i < _sampleNum; ++i) {
			train_vec(_trainMat[i], _labelMat[i], i);
		}
	}
}


void ANN_SIMD_aligned::train_vec_sse(const float* _trainVec, const float* _labelVec, int index)
{
	//��������ز�������
    for (int i = 0; i < numNodesHiddenLayer; ++i) {
        float z = 0.0;
        __m128 sums = _mm_setzero_ps();
        // �ڲ�ѭ��ʹ��SSE(AVX)ÿ�δ���4(8)��float����
        for (int j = 0; j <= numNodesInputLayer - 4; j += 4) {
            __m128 vecs = _mm_load_ps(_trainVec + j);
            // weights��������������������ã�ע��ߵ�λ
            __m128 weights128 = _mm_set_ps(weights[0][j + 3][i], weights[0][j + 2][i], weights[0][j + 1][i], weights[0][j][i]);
            // �ںϳ˼�ָ�Ҳ���Էֿ�д
            sums = _mm_fmadd_ps(vecs, weights128, sums);
        }
        // 4���ֲ������
        sums = _mm_hadd_ps(sums, sums);
        sums = _mm_hadd_ps(sums, sums);
        //�����洢
        _mm_store_ss(&z, sums);
        hidenLayerOutput[i] = sigmoid(z);
    }

	//���������������ֵ,ͬ��
	for (int i = 0; i < numNodesOutputLayer; ++i) {
        float z = 0.0;
        __m128 sums = _mm_setzero_ps();
        // �ڲ�ѭ��ʹ��SSE(AVX)ÿ�δ���4(8)��float����
        for (int j = 0; j <= numNodesHiddenLayer - 4; j += 4) {
            __m128 vecs = _mm_load_ps(hidenLayerOutput + j);
            // weights��������������������ã�ע��ߵ�λ
            __m128 weights128 = _mm_set_ps(weights[1][j + 3][i], weights[1][j + 2][i], weights[1][j + 1][i], weights[1][j][i]);
            // �ںϳ˼�ָ����Էֿ�д
            sums = _mm_fmadd_ps(vecs, weights128, sums);
        }
        // 4���ֲ������
        sums = _mm_hadd_ps(sums, sums);
        sums = _mm_hadd_ps(sums, sums);
        _mm_store_ss(&z, sums);
        z += bias[1][i];
        outputLayerOutput[i] = sigmoid(z);
        outputMat[index][i] = outputLayerOutput[i];
    }


	//����ƫ�ø�����allDeltaBias��allDeltaWeights[1]
	/*for (int j = 0; j < numNodesOutputLayer; ++j) {
		allDeltaBias[index][1][j] = (-0.1) * (_labelVec[j] - outputLayerOutput[j]) * outputLayerOutput[j]
			* (1 - outputLayerOutput[j]);
		for (int i = 0; i < numNodesHiddenLayer; ++i) {
			allDeltaWeights[index][1][i][j] = allDeltaBias[index][1][j] * hidenLayerOutput[i];
		}
	}*/
    for (int j = numNodesOutputLayer -4;j>=0;j-= 4) {
        __m128 t1, t2, t3, t4,product;
        //t1, t2, t3, t4�ֱ�Ϊ4���������ѭ����������ÿ�δ���4����
        t1 = _mm_set1_ps(-0.1);
        t2 = _mm_load_ps(_labelVec + j);
        t3 = _mm_load_ps(outputLayerOutput + j);
        t2 = _mm_sub_ps(t2, t3);
        t4 = _mm_set1_ps(1);
        t4 = _mm_sub_ps(t4, t3);

        //product=t1*t2*t3*t4��������λ���
        t1 = _mm_mul_ps(t1, t2);
        t3 = _mm_mul_ps(t3, t4);
        product = _mm_mul_ps(t1, t3);
        //������λ�洢
        _mm_store_ps(allDeltaBias[index][1] + j, product);
    }
    for (int i = numNodesHiddenLayer-1; i>= 0; i --) {
        for (int j = numNodesOutputLayer -4;j>=0;j-= 4) {
            __m128 a1, a2,pro;
            //����load����a1��4��ֵ����ΪhidenLayerOutput[i]
            a1 = _mm_set1_ps (hidenLayerOutput[i]);
            //����load
            a2 = _mm_load_ps(allDeltaBias[index][1]+j);
            //������λ�˷�
            pro =  _mm_mul_ps(a2, a1);
            //�����洢
            _mm_store_ps(allDeltaWeights[index][1][i] + j, pro);
        }
    }//printf("!");

    //����ƫ�ø�����allDeltaBias��allDeltaWeights[0]
    /*for (int j = 0; j < numNodesHiddenLayer; ++j) {
		float z = 0.0;
		for (int i = 0; i < numNodesOutputLayer; ++i) {
			z += weights[1][j][i] * allDeltaBias[index][1][i];
		}
		allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
		for (int i = 0; i < numNodesInputLayer; ++i) {
			allDeltaWeights[index][0][i][j] = allDeltaBias[index][0][j] * _trainVec[i];
		}
	}*/
	//z�������ڴ洢�м���
    float *z=new float[numNodesHiddenLayer];
    if(z==NULL){
        printf("z==NULL");
    }
	for (int j = numNodesHiddenLayer-1; j >= 0; j --) {
        __m128 t1, t2, sum;
        //��ʼ��0
        sum = _mm_setzero_ps();
		for (int i = numNodesOutputLayer - 4; i >= 0; i -= 4){
            t1 = _mm_load_ps(weights[1][j] + i);
            t2 = _mm_load_ps(allDeltaBias[index][1] + i);
            //������
            t1 = _mm_mul_ps(t1, t2);
            //������
            sum = _mm_add_ps(sum, t1);
		}
		//����ӣ�����sum��ÿ32λ��Ϊ�����
		sum = _mm_hadd_ps(sum, sum);
        sum = _mm_hadd_ps(sum, sum);
        //�洢32λ��sum��z[j]
        _mm_store_ss(z+j,sum);
	}
    for (int j = numNodesHiddenLayer-4; j >= 0; j -=4) {
        //allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
        __m128 t1, t2,t3, product;
        t1 = _mm_load_ps(z+j);
        t2 = _mm_load_ps(hidenLayerOutput+j);
        t3 = _mm_set1_ps(1);
        t3 = _mm_sub_ps(t3,t2);
        //product=t1*t2*t3��������λ���
        product = _mm_mul_ps(t1,t2);
        product = _mm_mul_ps(product,t3);
        _mm_store_ps(allDeltaBias[index][0]+j,product);
    }
    delete[]z;
    z=NULL;
    for (int i = numNodesInputLayer-1; i>= 0; i --) {
        for (int j = numNodesHiddenLayer -4;j>=0;j-= 4) {
            __m128 a1, a2,pro;
            //����load
            a1 = _mm_set1_ps (_trainVec[i]);
            //������
            a2 = _mm_load_ps(allDeltaBias[index][0]+j);
            pro =  _mm_mul_ps(a2, a1);
            //�洢
            _mm_store_ps(allDeltaWeights[index][0][i] + j, pro);
        }
    }
}

void ANN_SIMD_aligned::train_avx(const int _sampleNum, float** _trainMat, float** _labelMat)
{
	float thre = 1e-2;
	for (int i = 0; i < _sampleNum; ++i) {
		train_vec_avx(_trainMat[i], _labelMat[i], i);
	}
	for(int tt = 0; tt < MAXTT; tt++){
		if(!isNotConver_(_sampleNum, _labelMat, thre)) break;
		//����Ȩֵ
		__m256 sr = _mm256_set1_ps(studyRate);
        __m256 t1, t2;
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i < numNodesInputLayer; ++i) {
				for (int j = 0; j <= numNodesHiddenLayer-8; j+=8) {
                    //���ڲ�ѭ����Ϊ8λ����������
				    t1 = _mm256_load_ps(allDeltaWeights[index][0][i] + j);
				    t1 = _mm256_mul_ps(t1, sr);
				    t2 = _mm256_load_ps(weights[0][i] + j);
				    t2 = _mm256_sub_ps(t2,t1);
                    //������
                    _mm256_store_ps(weights[0][i] + j, t2);
					//weights[0][i][j] -= studyRate * allDeltaWeights[index][0][i][j];
				}
			}
			for (int i = 0; i < numNodesHiddenLayer; ++i) {
				for (int j = 0; j <= numNodesOutputLayer-8; j+=8) {
                    t1 = _mm256_load_ps(allDeltaWeights[index][1][i] + j);
				    t1 = _mm256_mul_ps(t1, sr);
				    t2 = _mm256_load_ps(weights[1][i] + j);
				    t2 = _mm256_sub_ps(t2,t1);
                    _mm256_store_ps(weights[1][i] + j, t2);
					//weights[1][i][j] -= studyRate * allDeltaWeights[index][1][i][j];
				}
			}
		}
		for (int index = 0; index < _sampleNum; ++index) {
			for (int i = 0; i <= numNodesHiddenLayer-8; i+=8) {
                t1 = _mm256_load_ps(allDeltaBias[index][0]+i);
                t1 = _mm256_mul_ps(t1, sr);
                t2 = _mm256_load_ps(bias[0] + i);
                t2 = _mm256_sub_ps(t2,t1);
                _mm256_store_ps(bias[0] + i, t2);
				//bias[0][i] -= studyRate * allDeltaBias[index][0][i];
			}
			for (int i = 0; i <= numNodesOutputLayer-8; i+=8) {
			    t1 = _mm256_load_ps(allDeltaBias[index][1]+i);
                t1 = _mm256_mul_ps(t1, sr);
                t2 = _mm256_load_ps(bias[1] + i);
                t2 = _mm256_sub_ps(t2,t1);
                _mm256_store_ps(bias[1] + i, t2);
				//bias[1][i] -= studyRate * allDeltaBias[index][1][i];
			}
		}

		for (int i = 0; i < _sampleNum; ++i) {
			train_vec_avx(_trainMat[i], _labelMat[i], i);
		}
	}
}
void ANN_SIMD_aligned::train_vec_avx(const float* _trainVec, const float* _labelVec, int index)
{
   // printf("#");
	//��������ز�������
    for (int i = 0; i < numNodesHiddenLayer; ++i) {
        float z = 0.0;
        __m256 sums = _mm256_setzero_ps();
        // �ڲ�ѭ��ʹ��SSE(AVX)ÿ�δ���4(8)��float����
        for (int j = numNodesInputLayer - 8; j >=0; j -= 8) {
            __m256 vecs = _mm256_load_ps(_trainVec + j);
            // weights��������������������ã�ע��ߵ�λ
            __m256 vweights = _mm256_set_ps(weights[0][j + 7][i], weights[0][j + 6][i], weights[0][j + 5][i], weights[0][j+4][i],
                                           weights[0][j + 3][i], weights[0][j + 2][i], weights[0][j + 1][i], weights[0][j][i]);
            // �ںϳ˼�ָ�Ҳ���Էֿ�д
            sums = _mm256_fmadd_ps(vecs, vweights, sums);
        }
        // 8���ֲ������
        __m128 s1, s2;
        s1 = _mm256_extractf128_ps(sums, 0);  // s1=[a0,a1,a2,a3]
        s2 = _mm256_extractf128_ps(sums, 1);  // s2=[a4,a5,a6,a7]
        s1 = _mm_hadd_ps(s1, s2); // s1=[a0+a1,a2+a3,a4+a5,a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3,a4+a5+a6+a7,a0+a1+a2+a3,a4+a5+a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3+a4+a5+a6+a7,...]
        _mm_store_ss(&z, s1);

        hidenLayerOutput[i] = sigmoid(z);
    }
//printf("$");
	//���������������ֵ,ͬ��
	for (int i = 0; i < numNodesOutputLayer; ++i) {
        float z = 0.0;
       // printf("0");
        __m256 sums = _mm256_setzero_ps();
        // �ڲ�ѭ��ʹ��SSE(AVX)ÿ�δ���4(8)��float����
        for (int j = 0; j <= numNodesHiddenLayer - 8; j += 8) {
            __m256 vecs = _mm256_load_ps(hidenLayerOutput + j);
           // printf("%d@%d",j,i);
            // weights��������������������ã�ע��ߵ�λ
            __m256 vweights = _mm256_set_ps(weights[1][j + 7][i], weights[1][j + 6][i], weights[1][j + 5][i], weights[1][j + 4][i],
                                            weights[1][j + 3][i], weights[1][j + 2][i], weights[1][j + 1][i], weights[1][j][i]);
            // �ںϳ˼�ָ����Էֿ�д
            //printf("2");
            sums = _mm256_fmadd_ps(vecs, vweights, sums);
            //printf("3");
        }
       // printf("1");
        // 8���ֲ������
        __m128 s1, s2;
        s1 = _mm256_extractf128_ps(sums, 0);  // s1=[a0,a1,a2,a3]
        s2 = _mm256_extractf128_ps(sums, 1);  // s2=[a4,a5,a6,a7]
        s1 = _mm_hadd_ps(s1, s2); // s1=[a0+a1,a2+a3,a4+a5,a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3,a4+a5+a6+a7,a0+a1+a2+a3,a4+a5+a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3+a4+a5+a6+a7,...]
        _mm_store_ss(&z, s1);
        z += bias[1][i];
        outputLayerOutput[i] = sigmoid(z);
        outputMat[index][i] = outputLayerOutput[i];
    }


	//����ƫ�ø�����allDeltaBias��allDeltaWeights[1]
	/*for (int j = 0; j < numNodesOutputLayer; ++j) {
		allDeltaBias[index][1][j] = (-0.1) * (_labelVec[j] - outputLayerOutput[j]) * outputLayerOutput[j]
			* (1 - outputLayerOutput[j]);
		for (int i = 0; i < numNodesHiddenLayer; ++i) {
			allDeltaWeights[index][1][i][j] = allDeltaBias[index][1][j] * hidenLayerOutput[i];
		}
	}*///printf("!");
    for (int j = numNodesOutputLayer -8;j>=0;j-= 8) {
        __m256 t1, t2, t3, t4,product;
        //t1, t2, t3, t4�ֱ�Ϊ4���������ѭ����������ÿ�δ���4����
        t1 = _mm256_set1_ps(-0.1);
        t2 = _mm256_load_ps(_labelVec + j);
        t3 = _mm256_load_ps(outputLayerOutput + j);
        t2 = _mm256_sub_ps(t2, t3);
        t4 = _mm256_set1_ps(1);
        t4 = _mm256_sub_ps(t4, t3);
        //product=t1*t2*t3*t4��������λ���
        t1 = _mm256_mul_ps(t1, t2);
        t3 = _mm256_mul_ps(t3, t4);
        product = _mm256_mul_ps(t1, t3);
        //������λ�洢
        //printf("%d",j );
        _mm256_store_ps((float*)(&allDeltaBias[index][1][j]), product);
        //printf("&");
    }//printf("*");
    for (int i = numNodesHiddenLayer-1; i>= 0; i--) {
        for (int j = numNodesOutputLayer - 8;j>=0;j-= 8) {
            __m256 a1, a2,pro;
            //����load����a1��8��ֵ����ΪhidenLayerOutput[i]
            a1 = _mm256_set1_ps (hidenLayerOutput[i]);
            //����load
            // printf("%d",i );
            a2 = _mm256_load_ps(allDeltaBias[index][1]+j);
            // printf("@" );
            //������λ�˷�
            pro =  _mm256_mul_ps(a2, a1);
            //�����洢
           // printf("%d",j );
            _mm256_store_ps((float*)&(allDeltaWeights[index][1][i][j]), pro);
           // printf("!");
        }
    }
    /*
    for (int i = numNodesHiddenLayer-1; i>= 0; i --) {
        for (int j = numNodesOutputLayer -4;j>=0;j-= 4) {
            __m128 a1, a2,pro;
            //����load����a1��4��ֵ����ΪhidenLayerOutput[i]
            a1 = _mm_set1_ps (hidenLayerOutput[i]);
            //����load
            a2 = _mm_load_ps(allDeltaBias[index][1]+j);
            //������λ�˷�
            pro =  _mm_mul_ps(a2, a1);
            //�����洢
            _mm_store_ps(allDeltaWeights[index][1][i] + j, pro);
        }
    }
    *///printf("@");

    //����ƫ�ø�����allDeltaBias��allDeltaWeights[0]
    /*for (int j = 0; j < numNodesHiddenLayer; ++j) {
		float z = 0.0;
		for (int i = 0; i < numNodesOutputLayer; ++i) {
			z += weights[1][j][i] * allDeltaBias[index][1][i];
		}
		allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
		for (int i = 0; i < numNodesInputLayer; ++i) {
			allDeltaWeights[index][0][i][j] = allDeltaBias[index][0][j] * _trainVec[i];
		}
	}*/
	//z�������ڴ洢�м���
    float *z=new float[numNodesHiddenLayer];
	for (int j = numNodesHiddenLayer-1; j >= 0; j --) {
        __m256 t1, t2, sum;
        //��ʼ��0
        sum = _mm256_setzero_ps();
		for (int i = numNodesOutputLayer - 8; i >= 0; i -= 8){
            t1 = _mm256_load_ps(weights[1][j] + i);
            t2 = _mm256_load_ps(allDeltaBias[index][1] + i);
            //������
            t1 = _mm256_mul_ps(t1, t2);
            //������
            sum = _mm256_add_ps(sum, t1);
		}
		//����ӣ�����sum��ÿ32λ��Ϊ�����
        __m128 s1, s2;
        s1 = _mm256_extractf128_ps(sum, 0);  // s1=[a0,a1,a2,a3]
        s2 = _mm256_extractf128_ps(sum, 1);  // s2=[a4,a5,a6,a7]
        s1 = _mm_hadd_ps(s1, s2); // s1=[a0+a1,a2+a3,a4+a5,a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3,a4+a5+a6+a7,a0+a1+a2+a3,a4+a5+a6+a7]
        s1 = _mm_hadd_ps(s1, s1); // s1=[a0+a1+a2+a3+a4+a5+a6+a7,...]
        _mm_store_ss(z+j, s1);

        //_mm256_store_ss(z+j,sum);
	}
    for (int j = numNodesHiddenLayer-8; j >= 0; j -=8) {
        //allDeltaBias[index][0][j] = z * hidenLayerOutput[j] * (1 - hidenLayerOutput[j]);
        __m256 t1, t2,t3, product;
        t1 = _mm256_load_ps(z+j);
        t2 = _mm256_load_ps(hidenLayerOutput+j);
        t3 = _mm256_set1_ps(1);
        t3 = _mm256_sub_ps(t3,t2);
        //product=t1*t2*t3��������λ���
        product = _mm256_mul_ps(t1,t2);
        product = _mm256_mul_ps(product,t3);
        _mm256_store_ps(allDeltaBias[index][0]+j,product);
    }
    delete[]z;
    z=NULL;
    for (int i = numNodesInputLayer-1; i>= 0; i --) {
        for (int j = numNodesHiddenLayer -8;j>=0;j-= 8) {
            __m256 a1, a2,pro;
            //����load
            a1 = _mm256_set1_ps (_trainVec[i]);
            //������
            a2 = _mm256_load_ps(allDeltaBias[index][0]+j);
            pro =  _mm256_mul_ps(a2, a1);
            //�洢
            _mm256_store_ps(allDeltaWeights[index][0][i] + j, pro);
        }
    }
}

bool ANN_SIMD_aligned::isNotConver_(const int _sampleNum,float** _labelMat, float _thresh)
{
	float lossFunc = 0.0;
	for (int k = 0; k < _sampleNum; ++k) {
		float loss = 0.0;
		for (int t = 0; t < numNodesOutputLayer; ++t) {
			loss += (outputMat[k][t] - _labelMat[k][t]) * (outputMat[k][t] - _labelMat[k][t]);
		}
		lossFunc += (1.0 / 2) * loss;
	}

	lossFunc = lossFunc / _sampleNum;

	// /*
	//�ڼ���ʱ����ʧ����ֵ//
	static int tt = 0;
	tt++;
	//if (tt % 1000 == 0) {
		printf("��%d��ѵ����", tt);
		printf("%0.12f\n", lossFunc);
	//
	//*/

	if (lossFunc > _thresh)
		return true;

	return false;
}

void ANN_SIMD_aligned::predict(float* in, float* proba)
{
		//��������ز�������
	for (int i = 0; i < numNodesHiddenLayer; ++i) {
		float z = 0.0;
		for (int j = 0; j < numNodesInputLayer; ++j) {
			z += in[j] * weights[0][j][i];
		}
		z += bias[0][i];
		hidenLayerOutput[i] = sigmoid(z);
	}

	//���������������ֵ
	for (int i = 0; i < numNodesOutputLayer; ++i) {
		float z = 0.0;
		for (int j = 0; j < numNodesHiddenLayer; ++j) {
			z += hidenLayerOutput[j] * weights[1][j][i];
		}
		z += bias[1][i];
		outputLayerOutput[i] = sigmoid(z);
		//if ((i % 100) == 0)
		//	std::cout << outputLayerOutput[i] << " ";
	}

}
