#include "ANN_2.h"

ANN_2::ANN_2 ( int* _num_each_layer, int _num_epoch, int _batch_size, int _num_layers, float _study_rate)
{
    num_layers = _num_layers;
    study_rate = _study_rate;
    num_each_layer = new int[_num_layers + 2];
    num_each_layer[0] = _num_each_layer[0];
    for (int i = 1; i <= num_layers + 1; i++)
    {
        num_each_layer[i] = _num_each_layer[i];
        layers.push_back (new Layer (num_each_layer[i - 1], (num_each_layer[i]) ) );
    }
    //display();
    num_epoch = _num_epoch;
    batch_size = _batch_size;
}

ANN_2::~ANN_2()
{
    //printf ("begin free ANN_2");
    delete[]num_each_layer;
    for (int i = 0; i < layers.size(); i++) delete layers[i];
    // printf ("free ANN_2\n");
}
void ANN_2::shuffle (const int num_sample, float** _trainMat, float** _labelMat)
{
    //init
    int *shuffle_index = new int[num_sample];
    float **trainMat_old = new float*[num_sample];
    float **labelMat_old = new float*[num_sample];
    for (int i = 0; i < num_sample; i++)
    {
        trainMat_old[i] = new float[num_each_layer[0]];
        labelMat_old[i] = new float[num_each_layer[num_layers + 1]];
    }
    for (int i = 0; i < num_sample; i++)
    {
        for (int j = 0; j < num_each_layer[0]; j++)
            trainMat_old[i][j] = _trainMat[i][j];
        for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
            labelMat_old[i][j] = _labelMat[i][j];
    }

    //shuffle
    for (int i = 0; i < num_sample; i++) shuffle_index[i] = i;
    random_shuffle (shuffle_index, shuffle_index + num_sample);
    for (int i = 0; i < num_sample; i++)
    {
        for (int j = 0; j < num_each_layer[0]; j++) _trainMat[i][j] = trainMat_old[shuffle_index[i]][j];
        for (int j = 0; j < num_each_layer[num_layers + 1]; j++) _labelMat[i][j] = labelMat_old[shuffle_index[i]][j];
    }

    //delete
    for (int i = 0; i < num_sample; i++)
    {
        delete []trainMat_old[i];
        delete []labelMat_old[i];
    }
    delete []trainMat_old;
    delete []labelMat_old;
    delete[] shuffle_index;

    printf ("finish shuffle\n");
}

void ANN_2::train_SIMD  (const int num_sample, float** _trainMat, float** _labelMat)
{
    printf ("begin training\n");
    float thre = 1e-2;
    float *avr_X = new float[num_each_layer[0]];

    for (int epoch = 0; epoch < num_epoch; epoch++)
    {
       // if (epoch % 5 == 0) printf ("round%d:\n", epoch);
        int index = 0;

        ANN_2 *class_p = this;
        while (index < num_sample)
        {
            for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] = 0.0;
            for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
            {
                layers[num_layers]->delta[j] = 0.0;
            }

            for (int i = 0; i < batch_size && index < num_sample; i++, index++) //Ĭ��batch_size=1������������ݶ��½�����ÿ��ʹ��ȫ������ѵ�������²���
            {
                //ǰ�򴫲�

                for (int i = 0; i < num_each_layer[1]; i++)
                {
                    layers[0]->output_nodes[i] = 0.0;
                    //printf("%d,%d ",i,class_p->num_each_layer[1]);
                    int max_j = class_p->num_each_layer[0];
                    __m128 ans = _mm_setzero_ps();
                    for (int j = 0; j < max_j; j += 4)
                        // for (int j = t_id; j < class_p->num_each_layer[0]; j += NUM_THREADS)
                    {
                        __m128 t1, t2;
                        //���ڲ�ѭ����Ϊ4λ����������
                        t1 = _mm_loadu_ps (layers[0]->weights[i] + j);
                        t2 = _mm_loadu_ps (_trainMat[index] + j);
                        t1 = _mm_mul_ps (t1, t2);

                        ans = _mm_add_ps (ans, t1);

                        // printf("%d,%d ",j,class_p->num_each_layer[0]);
                        //  class_p->layers[0]->output_nodes[i] += class_p->layers[0]->weights[i][j] * p->sampleMat[sample_index][j];
                        // printf("%d,%d ",i,sample_index);
                    }
                    // 4���ֲ������
                    ans = _mm_hadd_ps (ans, ans);
                    ans = _mm_hadd_ps (ans, ans);
                    //�����洢
                    float z;
                    _mm_store_ss (&z, ans);
                    layers[0]->output_nodes[i] += z;
                    layers[0]->output_nodes[i] += layers[0]->bias[i];
                    layers[0]->output_nodes[i] = layers[0]->activation_function (layers[0]->output_nodes[i]);
                }
                // printf("@");
                for (int layers_i = 1; layers_i <= num_layers; layers_i++)
                {

                    for (int i = 0; i < num_each_layer[layers_i + 1]; i++)
                    {
                        class_p->layers[layers_i]->output_nodes[i] = 0.0;
                        int max_j = class_p->num_each_layer[layers_i];
                        __m128 ans = _mm_setzero_ps();
                        for (int j = 0; j < max_j; j += 4)
                            // for (int j = t_id; j < class_p->num_each_layer[0]; j += NUM_THREADS)
                        {
                            __m128 t1, t2;
                            //���ڲ�ѭ����Ϊ4λ����������
                            t1 = _mm_loadu_ps (class_p->layers[layers_i]->weights[i] + j);
                            t2 = _mm_loadu_ps (_trainMat[index] + j);
                            t1 = _mm_mul_ps (t1, t2);

                            ans = _mm_add_ps (ans, t1);
                        }
                        // 4���ֲ������
                        ans = _mm_hadd_ps (ans, ans);
                        ans = _mm_hadd_ps (ans, ans);
                        //�����洢
                        float z;
                        _mm_store_ss (&z, ans);
                        class_p->layers[layers_i]->output_nodes[i] += z;
                        class_p->layers[layers_i]->output_nodes[i] += class_p->layers[layers_i]->bias[i];
                        class_p->layers[layers_i]->output_nodes[i] = class_p->layers[layers_i]->activation_function (class_p->layers[layers_i]->output_nodes[i]);
                    }
                }


                //����loss�������һ���delta,����minibatch������loss��ƽ��ֵ
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    //���������ʧ����
                    layers[num_layers]->delta[j] += (layers[num_layers]->output_nodes[j] - _labelMat[index][j]) * layers[num_layers]->derivative_activation_function (layers[num_layers]->output_nodes[j]);
                    //��������ʧ����
                    //layers[num_layers]->delta[j] += (layers[num_layers]->output_nodes[j] - _labelMat[index][j]);
                }
                //printf ("finish cal error\n");
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] += _trainMat[index][X_i];
            }
            if (index % batch_size == 0)
            {
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    if (batch_size == 0) printf ("wrong!\n");
                    layers[num_layers]->delta[j] /= batch_size;
                    //for(int i=0;i<5;i++)printf("delta=%f\n",layers[num_layers]->delta[i]);
                }
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] /= batch_size;

            }
            else
            {
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    if (index % batch_size == 0) printf ("wrong!\n");
                    layers[num_layers]->delta[j] /= (index % batch_size);
                }
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] /= (index % batch_size);
            }

            //���򴫲����²���
            for (int k = 0; k < class_p->num_each_layer[1]; k++)
            {
                int max_j = class_p->num_each_layer[0];
                __m128 sr = _mm_set1_ps (class_p->study_rate);
                __m128 t2 = _mm_set1_ps (class_p->layers[0]->delta[k]);
                for (int j = 0; j < max_j; j += 4)

                    //for (int j = t_id; j < class_p->num_each_layer[0]; j += NUM_THREADS)
                {
                    __m128 t1, t3, product;
                    t1 = _mm_loadu_ps (class_p->layers[0]->weights[k] + j);
                    t3 = _mm_loadu_ps (avr_X + j);
                    //product=sr*t2*t3��������λ���
                    product = _mm_mul_ps (t3, t2);
                    product = _mm_mul_ps (product, sr);
                    t1 = _mm_sub_ps (t1, product);
                    _mm_storeu_ps (class_p->layers[0]->weights[k] + j, t1);

                    //class_p->layers[0]->weights[k][j] -= class_p->study_rate * p->sampleMat[sample_index - 1][j] * class_p->layers[0]->delta[k];
                }
                class_p->layers[0]->bias[k] -=  class_p->study_rate *  class_p->layers[0]->delta[k];
            }
            for (int i = 1; i <= class_p->num_layers; i++)
            {
                for (int k = 0; k < class_p->num_each_layer[i + 1]; k++)
                {
                    int max_j = class_p->num_each_layer[i];
                    __m128 sr = _mm_set1_ps (class_p->study_rate);
                    __m128 t2 = _mm_set1_ps (class_p->layers[i]->delta[k]);//printf("1");
                    for (int j = 0; j < max_j; j += 4)
                        //for (int j = t_id; j < class_p->num_each_layer[i]; j += NUM_THREADS)
                    {
                        __m128 t1, t3, product;
                        t1 = _mm_loadu_ps (class_p->layers[i]->weights[k] + j);
                        t3 = _mm_loadu_ps (class_p->layers[i - 1]->output_nodes + j);
                        //product=sr*t2*t3��������λ���
                        product = _mm_mul_ps (t3, t2);
                        product = _mm_mul_ps (product, sr);
                        t1 = _mm_sub_ps (t1, product);

                        _mm_storeu_ps (class_p->layers[i]->weights[k] + j, t1);

                        //class_p->layers[i]->weights[k][j] -= class_p->study_rate * class_p->layers[i - 1]->output_nodes[j] * class_p->layers[i]->delta[k];
                    }//printf("2");
                    class_p->layers[i]->bias[k] -=  class_p->study_rate * class_p->layers[i]->delta[k];
                }
            }

            //printf ("finish bp with index:%d\n",index);
        }



        // display();
    }
    printf ("finish training\n");
    delete[]avr_X;
}
void ANN_2::train (const int num_sample, float** _trainMat, float** _labelMat)
{
    printf ("begin training\n");
    float thre = 1e-2;
    float *avr_X = new float[num_each_layer[0]];
    float *avr_Y = new float[num_each_layer[num_layers + 1]];

    for (int epoch = 0; epoch < num_epoch; epoch++)
    {
        if (epoch % 50 == 0)
        {
          //  printf ("round%d:\n", epoch);
        }
        int index = 0;

        while (index < num_sample)
        {
            for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] = 0.0;
            for (int Y_i = 0; Y_i < num_each_layer[num_layers + 1]; Y_i++) avr_Y[Y_i] = 0.0;

            for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
            {
                layers[num_layers]->delta[j] = 0.0;
            }

            for (int batch_i = 0; batch_i < batch_size && index < num_sample; batch_i++, index++) //Ĭ��batch_size=1������������ݶ��½�����ÿ��ʹ��ȫ������ѵ�������²���
            {
                //ǰ�򴫲�
                predict (_trainMat[index]);

//                static int tt = 0;
//                float loss = 0.0;
//                for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
//                {
//                    loss += (layers[num_layers]->output_nodes[t] - _labelMat[index][t]) * (layers[num_layers]->output_nodes[t] - _labelMat[index][t]);
//                }
//                printf ("��%d��ѵ����%0.12f\n", tt++,loss);


                //for (int i = 0; i < min (5, num_each_layer[num_layers + 1]); i++)
                //printf ("%f,", layers[num_layers]->output_nodes[i]);
                //printf ("\n");
                // printf ("finish predict\n");

                //����loss�������һ���delta,����minibatch������loss��ƽ��ֵ
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    //���������ʧ����,batch��ȡƽ��
                    layers[num_layers]->delta[j] += (layers[num_layers]->output_nodes[j] - _labelMat[index][j]) * layers[num_layers]->derivative_activation_function (layers[num_layers]->output_nodes[j]);
                    //��������ʧ����
                    //layers[num_layers]->delta[j] += (layers[num_layers]->output_nodes[j] - _labelMat[index][j]);
                }
                // printf ("finish cal error\n");
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] += _trainMat[index][X_i];
                for (int Y_i = 0; Y_i < num_each_layer[num_layers + 1]; Y_i++) avr_Y[Y_i] += _labelMat[index][Y_i];
            }

            //delta��batch��ȡƽ����avr_X��avr_Y�ֱ�Ϊ����batch�����롢���������ƽ��ֵ
            if (index % batch_size == 0)
            {
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    if (batch_size == 0) printf ("wrong!\n");
                    layers[num_layers]->delta[j] /= batch_size;
                    //for(int i=0;i<5;i++)printf("delta=%f\n",layers[num_layers]->delta[i]);
                    avr_Y[j] /= batch_size;
                }
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] /= batch_size;

            }
            else
            {
                for (int j = 0; j < num_each_layer[num_layers + 1]; j++)
                {
                    if (index % batch_size == 0) printf ("wrong!\n");
                    layers[num_layers]->delta[j] /= (index % batch_size);
                    avr_Y[j] /= (index % batch_size);
                }
                for (int X_i = 0; X_i < num_each_layer[0]; X_i++) avr_X[X_i] /= (index % batch_size);
            }
           // printf("index:%d\n",index);
           //����loss func�����������
            if ( index >= num_sample)
            {
                static int tt = 0;
                float loss = 0.0;
                for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
                {
                    loss += (layers[num_layers]->output_nodes[t] - avr_Y[t]) * (layers[num_layers]->output_nodes[t] - avr_Y[t]);
                }
              //  printf ("��%d��ѵ����%0.12f\n", tt++, loss);
            }

            //���򴫲����²���
            back_propagation (avr_X, avr_Y);
            //printf ("finish bp with index:%d\n",index);
        }

        // display();
    }
    printf ("finish training\n");
    delete[]avr_X;
    delete[]avr_Y;
}

bool ANN_2::isNotConver_ (const int _sampleNum, float** _trainMat, float** _labelMat, float _thresh)
{
    float lossFunc = 0.0;
    for (int k = 0; k < _sampleNum; ++k)
    {
        predict (_trainMat[k]);
        float loss = 0.0;
        for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
        {
            loss += (layers[num_layers]->output_nodes[t] - _labelMat[k][t]) * (layers[num_layers]->output_nodes[t] - _labelMat[k][t]);
        }
        lossFunc += (1.0 / 2) * loss;
    }

    lossFunc = lossFunc / _sampleNum;

    static int tt = 0;
    tt++;
   // printf ("��%d��ѵ����%0.12f\n", tt, lossFunc);

    if (lossFunc > _thresh) return true;
    return false;
}

void ANN_2::predict (float* in)
{
    layers[0]->_forward (in);
    for (int i = 1; i <= num_layers; i++)
    {
        layers[i]->_forward (layers[i - 1]->output_nodes);
    }
}
void ANN_2::display()
{
    for (int i = 0; i <= num_layers; i++)
    {
        layers[i]->display();
    }
}

void ANN_2::back_propagation (float* X, float * Y)
{
    //����ÿ���delta,�з����Ż�
    for (int i = num_layers - 1; i >= 0; i--)
    {
        float *error = new float[num_each_layer[i + 1]];
        for (int j = 0; j < num_each_layer[i + 1]; j++) error[j] = 0.0;
        for (int k = 0; k < num_each_layer[i + 2]; k++)
        {
            for (int j = 0; j < num_each_layer[i + 1]; j++)
            {
                error[j] += layers[i + 1]->weights[k][j] * layers[i + 1]->delta[k];
            }
        }
        for (int j = 0; j < num_each_layer[i + 1]; j++)
        {
            layers[i]->delta[j] = error[j] * layers[num_layers]->derivative_activation_function (layers[i]->output_nodes[j]);
        }
        delete[]error;
    }

    //���򴫲���weights��bias����
    for (int k = 0; k < num_each_layer[1]; k++)
    {
        for (int j = 0; j < num_each_layer[0]; j++)
        {
            layers[0]->weights[k][j] -= study_rate * X[j] * layers[0]->delta[k];
        }
        layers[0]->bias[k] -= study_rate * layers[0]->delta[k];
    }
    for (int i = 1; i <= num_layers; i++)
    {
        for (int k = 0; k < num_each_layer[i + 1]; k++)
        {
            for (int j = 0; j < num_each_layer[i]; j++)
            {
                layers[i]->weights[k][j] -= study_rate * layers[i - 1]->output_nodes[j] * layers[i]->delta[k];
            }
            layers[i]->bias[k] -= study_rate * layers[i]->delta[k];
        }
    }

}
void ANN_2::get_predictions (float* X)
{
    predict (X);
    static int t = 0;
    printf ("in%d:", t);
    for (int i = 0; i < min (5, num_each_layer[0]); i++) printf ("%f,", X[i]);
    printf ("  out%d:", t);
    for (int i = 0; i < min (5, num_each_layer[num_layers + 1]); i++)
        printf ("%f,", layers[num_layers]->output_nodes[i]);
    t++;
    printf ("\n");
}
