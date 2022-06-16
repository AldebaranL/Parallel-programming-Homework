#include "ANN_MPI.h"

ANN_MPI::ANN_MPI (int* _num_each_layer, int _num_epoch, int _batch_size, int _num_layers, float _study_rate)
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

ANN_MPI::~ANN_MPI()
{
    //printf ("begin free ANN_MPI");
    delete[]num_each_layer;
    for (int i = 0; i < layers.size(); i++) delete layers[i];
    // printf ("free ANN_MPI\n");
}
void ANN_MPI::shuffle (const int num_sample, float** _trainMat, float** _labelMat)
{
    //init
    int* shuffle_index = new int[num_sample];
    float** trainMat_old = new float* [num_sample];
    float** labelMat_old = new float* [num_sample];
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
        delete[]trainMat_old[i];
        delete[]labelMat_old[i];
    }
    delete[]trainMat_old;
    delete[]labelMat_old;
    delete[] shuffle_index;

    printf ("finish shuffle\n");
}

void ANN_MPI::train (const int num_sample, float** _trainMat, float** _labelMat)
{
    //�����㷨
    printf ("begin training\n");
    float thre = 1e-2;
    float* avr_X = new float[num_each_layer[0]];
    float* avr_Y = new float[num_each_layer[num_layers + 1]];

    long long time_ori = 0;
    long long head, tail, head_all, tail_all, freq;// timers
    QueryPerformanceFrequency ( (LARGE_INTEGER*) &freq);
    QueryPerformanceCounter ( (LARGE_INTEGER*) &head_all); // start time
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
                for (int i = 0; i < num_each_layer[1]; i++)
                {
                    layers[0]->output_nodes[i] = 0.0;
                    for (int j = 0; j < num_each_layer[0]; j++)
                    {
                        layers[0]->output_nodes[i] += layers[0]->weights[i][j] * _trainMat[index][j];
                    }
                    layers[0]->output_nodes[i] += layers[0]->bias[i];
                    layers[0]->output_nodes[i] = layers[0]->activation_function (layers[0]->output_nodes[i]);
                }

                QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
                for (int i_layer = 1; i_layer <= num_layers; i_layer++)
                {
                    for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
                    {
                        layers[i_layer]->output_nodes[i] = 0.0;
                        for (int j = 0; j < num_each_layer[i_layer]; j++)
                        {
                            layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
                        }
                        layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
                        layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);

                    }

                }
                QueryPerformanceCounter ( (LARGE_INTEGER*) &tail); // start time
                time_ori += tail - head;
                //static int tt = 0;
                //float loss = 0.0;
                //for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
                //{
                //    loss += (layers[num_layers]->output_nodes[t] - _labelMat[index][t]) * (layers[num_layers]->output_nodes[t] - _labelMat[index][t]);
                // }
                //printf ("��%d��ѵ����%0.12f\n", tt++, loss);


                //for (int i = 0; i < min (5, num_each_layer[num_layers + 1]); i++)
                //printf ("%f,", layers[num_layers]->output_nodes[i]);
                //printf ("\n");
                //printf ("finish predict\n");
//
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
            if (index >= num_sample)
            {
                static int tt = 0;
                float loss = 0.0;
                for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
                {
                    loss += (layers[num_layers]->output_nodes[t] - avr_Y[t]) * (layers[num_layers]->output_nodes[t] - avr_Y[t]);
                }
                // printf ("��%d��ѵ����%0.12f\n", tt++, loss);
            }

            //���򴫲����²���

            //����ÿ���delta,�з����Ż�
            for (int i = num_layers - 1; i >= 0; i--)
            {
                float* error = new float[num_each_layer[i + 1]];
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
                    layers[0]->weights[k][j] -= study_rate * avr_X[j] * layers[0]->delta[k];
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
            //printf ("finish bp with index:%d\n",index);
        }

        // display();
    }
    //printf("finish training\n");
    int myid, numprocs;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    QueryPerformanceCounter ( (LARGE_INTEGER*) &tail_all); // start time
    if (myid == 0)
    {
        std::cout << myid << "ori_all:" << (tail_all - head_all) * 1.0 / freq << "s" << endl;
        std::cout << myid << "ori:" << time_ori * 1.0 / freq << "s" << endl;
    }

    delete[]avr_X;
    delete[]avr_Y;
}

void ANN_MPI::predict_serial()
{
    //predict���а汾
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
        {
            layers[i_layer]->output_nodes[i] = 0.0;
            for (int j = 0; j < num_each_layer[i_layer]; j++)
            {
                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
            }
            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);

        }
    }
}

void ANN_MPI::train_MPI_all_static (const int num_sample, float** _trainMat, float** _labelMat)
{
    //�Դ˺�����ȫ���ؼ�ѭ����������MPI�Ż������þ�̬���䷽ʽ
    printf ("begin training\n");  // cout<<' ';
    float thre = 1e-2;
    float* avr_X = new float[num_each_layer[0]];
    float* avr_Y = new float[num_each_layer[num_layers + 1]];


    long long time_mpi = 0;
    long long head, tail, freq;// timers
    QueryPerformanceFrequency ( (LARGE_INTEGER*) &freq);

    QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time

    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
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
                //1.ǰ�򴫲�����i���з���

                int i_size = (num_each_layer[1] + numprocs - 2) / (numprocs - 1);
                if (myid == 0)
                {
                    for (int procs_i = 1; procs_i < numprocs; procs_i++)
                    {
                        for (int i = (procs_i - 1) * i_size; i < min (num_each_layer[0 + 1], procs_i * i_size); i++)
                        {
                            MPI_Send (layers[0]->weights[i], num_each_layer[0], MPI_FLOAT, procs_i, 99 + i, MPI_COMM_WORLD);
                        }
                    }
                    for (int procs_i = 1; procs_i < numprocs; procs_i++)
                    {
                        MPI_Recv (layers[0]->output_nodes + (procs_i - 1) * i_size, i_size, MPI_FLOAT, procs_i, 97, MPI_COMM_WORLD, &status);
                    }
                }
                else
                {
                    for (int i = (myid - 1) * i_size; i < min (num_each_layer[0 + 1], myid * i_size); i++)
                    {
                        MPI_Recv (layers[0]->weights[i], num_each_layer[0], MPI_FLOAT, 0, 99 + i, MPI_COMM_WORLD, &status);
                        layers[0]->output_nodes[i] = 0.0;
                        for (int j = 0; j < num_each_layer[0]; j++)
                        {
                            layers[0]->output_nodes[i] += layers[0]->weights[i][j] * _trainMat[index][j];
                        }
                        layers[0]->output_nodes[i] += layers[0]->bias[i];
                        layers[0]->output_nodes[i] = layers[0]->activation_function (layers[0]->output_nodes[i]);
                    }
                    MPI_Send (layers[0]->output_nodes + (myid - 1) * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
                }

                for (int i_layer = 1; i_layer <= num_layers; i_layer++)
                {
                    int i_size = (num_each_layer[i_layer + 1] + numprocs - 2) / (numprocs - 1);
                    if (myid == 0)
                    {
                        for (int procs_i = 1; procs_i < numprocs; procs_i++)
                        {
                            MPI_Send (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, procs_i, 98, MPI_COMM_WORLD);
                            for (int i = (procs_i - 1) * i_size; i < min (num_each_layer[i_layer + 1], procs_i * i_size); i++)
                            {
                                MPI_Send (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, procs_i, 1000 + i, MPI_COMM_WORLD);
                            }
                        }
                        for (int procs_i = 1; procs_i < numprocs; procs_i++)
                        {
                            MPI_Recv (layers[i_layer]->output_nodes + (procs_i - 1) * i_size, i_size, MPI_FLOAT, procs_i, 97, MPI_COMM_WORLD, &status);
                        }
                    }
                    else
                    {
                        MPI_Recv (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, 98, MPI_COMM_WORLD, &status);
                        for (int i = (myid - 1) * i_size; i < min (num_each_layer[i_layer + 1], myid * i_size); i++)
                        {
                            MPI_Recv (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, 1000 + i, MPI_COMM_WORLD, &status);
                            layers[i_layer]->output_nodes[i] = 0.0;
                            for (int j = 0; j < num_each_layer[i_layer]; j++)
                            {
                                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
                            }
                            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
                            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
                        }
                        MPI_Send (layers[i_layer]->output_nodes + (myid - 1) * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
                    }

                }

                if (myid != 0)
                    continue;

                // cout<<"finish pridect"<<endl;

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
            if (index >= num_sample)
            {
                static int tt = 0;
                float loss = 0.0;
                for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
                {
                    loss += (layers[num_layers]->output_nodes[t] - avr_Y[t]) * (layers[num_layers]->output_nodes[t] - avr_Y[t]);
                }
                // printf ("��%d��ѵ����%0.12f\n", tt++, loss);
            }

            //���򴫲����²���

            //2.����ÿ���delta,�з����Ż�����j�������ݷ���
            for (int i = num_layers - 1; i >= 0; i--)
            {
                //��0�Ž��̹㲥
                for (int k = 0; k < num_each_layer[i + 2]; k++)
                {
                    MPI_Bcast (layers[i + 1]->weights[k], num_each_layer[i + 1], MPI_FLOAT, 0, MPI_COMM_WORLD);
                }
                MPI_Bcast (layers[i + 1]->delta, num_each_layer[i + 2], MPI_FLOAT, 0, MPI_COMM_WORLD);

                float* error = new float[num_each_layer[i + 1]];
                //0�Ž���Ҳ�������
                int i_size = (num_each_layer[i + 1] + numprocs - 1) / (numprocs);
                for (int j = myid * i_size; j < min (num_each_layer[i + 1], (myid + 1) * i_size); j++)
                {
                    for (int k = 0; k < num_each_layer[i + 2]; k++)
                    {
                        error[j] = 0.0;
                        error[j] += layers[i + 1]->weights[k][j] * layers[i + 1]->delta[k];
                        layers[i]->delta[j] = error[j] * layers[num_layers]->derivative_activation_function (layers[i]->output_nodes[j]);
                    }
                }
                //�����ݻ��ܵ�0�Ž���
                if (myid == 0)
                {
                    for (int procs_i = 1; procs_i < numprocs; procs_i++)
                    {
                        MPI_Recv (layers[i]->delta + procs_i * i_size, i_size, MPI_FLOAT, procs_i, 96, MPI_COMM_WORLD, &status);
                    }
                }
                else
                {
                    MPI_Send (layers[i]->delta + myid * i_size, i_size, MPI_FLOAT, 0, 96, MPI_COMM_WORLD);
                }

                delete[]error;
            }
            //cout<<"finish cal delta"<<endl;

            //3.���򴫲���weights��bias���£���k�������ݷ���

            MPI_Bcast (layers[0]->delta, num_each_layer[1], MPI_FLOAT, 0, MPI_COMM_WORLD); //��0�Ž��̹㲥
            int i_size = (num_each_layer[1] + numprocs - 1) / (numprocs);//0�Ž���Ҳ�������
            for (int k = myid * i_size; k < min (num_each_layer[1], (myid + 1) * i_size); k++)
            {
                for (int j = 0; j < num_each_layer[0]; j++)
                {
                    layers[0]->weights[k][j] -= study_rate * avr_X[j] * layers[0]->delta[k];
                }
                layers[0]->bias[k] -= study_rate * layers[0]->delta[k];
            }
            //�����ݻ��ܵ�0�Ž���
            if (myid == 0)
            {
                for (int procs_i = 1; procs_i < numprocs; procs_i++)
                {
                    MPI_Recv (layers[0]->bias + procs_i * i_size, i_size, MPI_FLOAT, procs_i, 94, MPI_COMM_WORLD, &status);
                    for (int k = procs_i * i_size; k < min (num_each_layer[1], (procs_i + 1) * i_size); k++)
                    {
                        MPI_Recv (layers[0]->weights[k], num_each_layer[0], MPI_FLOAT, procs_i, 3000 + k, MPI_COMM_WORLD, &status);
                    }
                }
            }
            else
            {
                MPI_Send (layers[0]->bias + myid * i_size, i_size, MPI_FLOAT, 0, 94, MPI_COMM_WORLD);
                for (int k = myid * i_size; k < min (num_each_layer[1], (myid + 1) * i_size); k++)
                {
                    MPI_Send (layers[0]->weights[k], num_each_layer[0], MPI_FLOAT, 0, 3000 + k, MPI_COMM_WORLD);
                }
            }
            //cout<<"finish first bp"<<endl;
            //ͬ��
            for (int i = 1; i <= num_layers; i++)
            {
                MPI_Bcast (layers[i]->delta, num_each_layer[i + 1], MPI_FLOAT, 0, MPI_COMM_WORLD);
                MPI_Bcast (layers[i - 1]->output_nodes, num_each_layer[i], MPI_FLOAT, 0, MPI_COMM_WORLD);
                int i_size = (num_each_layer[i + 1] + numprocs - 1) / (numprocs);
                // cout << "i_size" << i_size<<' '<< num_each_layer[i + 1]<<' '<< numprocs<<endl;

                for (int k = myid * i_size; k < min (num_each_layer[i + 1], (myid + 1) * i_size); k++)
                {
                    for (int j = 0; j < num_each_layer[i]; j++)
                    {
                        layers[i]->weights[k][j] -= study_rate * layers[i - 1]->output_nodes[j] * layers[i]->delta[k];
                    }
                    layers[i]->bias[k] -= study_rate * layers[i]->delta[k];
                }
                if (myid == 0)
                {
                    for (int procs_i = 1; procs_i < numprocs; procs_i++)
                    {
                        MPI_Recv (layers[i]->bias + procs_i * i_size, i_size, MPI_FLOAT, procs_i, 95, MPI_COMM_WORLD, &status);
                        for (int k = procs_i * i_size; k < min (num_each_layer[i + 1], (procs_i + 1) * i_size); k++)
                        {
                            MPI_Recv (layers[i]->weights[k], num_each_layer[i], MPI_FLOAT, procs_i, 2000 + k, MPI_COMM_WORLD, &status);
                        }
                    }
                }
                else
                {
                    MPI_Send (layers[i]->bias + myid * i_size, i_size, MPI_FLOAT, 0, 95, MPI_COMM_WORLD);
                    for (int k = myid * i_size; k < min (num_each_layer[i + 1], (myid + 1) * i_size); k++)
                    {
                        MPI_Send (layers[i]->weights[k], num_each_layer[i], MPI_FLOAT, 0, 2000 + k, MPI_COMM_WORLD);
                    }
                }

            }
            //printf ("finish bp with index:%d\n",index);
        }
        // display();
    }
    QueryPerformanceCounter ( (LARGE_INTEGER*) &tail); // start time
    time_mpi += tail - head;
    //MPI_Finalize();
    printf ("finish training\n");
    std::cout << myid << "mpi_all:" << time_mpi * 1.0 / freq << "s" << endl;
    delete[]avr_X;
    delete[]avr_Y;
}

void ANN_MPI::predict_MPI_static1()
{
    //��̬�黮�ֵ����ط���
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        int i_size = (num_each_layer[i_layer + 1] + numprocs - 2) / (numprocs - 1);//0�Ž��̲��������

        if (myid == 0)
        {
            //���ݷ���
            for (int procs_i = 1; procs_i < numprocs; procs_i++)
            {
                MPI_Send (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, procs_i, 98, MPI_COMM_WORLD);
                for (int i = (procs_i - 1) * i_size; i < min (num_each_layer[i_layer + 1], procs_i * i_size); i++)
                {
                    MPI_Send (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, procs_i, 99 + i, MPI_COMM_WORLD);
                }

                //  cout << "0 end send" << endl;
            }
            //�������
            for (int procs_i = 1; procs_i < numprocs; procs_i++)
            {
                MPI_Recv (layers[i_layer]->output_nodes + (procs_i - 1) * i_size, i_size, MPI_FLOAT, procs_i, 97, MPI_COMM_WORLD, &status);
            }
            // cout<<myid<<"finish recv"<<endl;

        }
        else
        {
            //���ݽ���
            MPI_Recv (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, 98, MPI_COMM_WORLD, &status);

            //�ӽڵ����
            for (int i = (myid - 1) * i_size; i < min (num_each_layer[i_layer + 1], myid * i_size); i++)
            {
                MPI_Recv (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, 99 + i, MPI_COMM_WORLD, &status);
                layers[i_layer]->output_nodes[i] = 0.0;
                for (int j = 0; j < num_each_layer[i_layer]; j++)
                {
                    layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
                }
                layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
                layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
            }

            //�ӽڵ㽫������ͻ����ڵ㣨0�ţ�
            MPI_Send (layers[i_layer]->output_nodes + (myid - 1) * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
            // cout<<myid<<"finish send"<<endl;
        }

    }
}

void ANN_MPI::predict_MPI_gather()
{
    //����̹�Լ
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        //���ݷ���
        int position = 0;
        int buff_size = num_each_layer[i_layer + 1] * num_each_layer[i_layer] * sizeof (float);
        float* buffer_for_packed = new float[buff_size];
        float* recv_buffer = new float[buff_size];

        //�ɽ���0���wieghts
        if (myid == 0)
        {
            for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            {
                MPI_Pack (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, buffer_for_packed, buff_size, &position, MPI_COMM_WORLD);
            }
        }

        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������
        int my_i_size = i_size;
        if (myid == numprocs - 1)
        {
            my_i_size = num_each_layer[i_layer + 1] - myid * i_size;
        }
        //Scatter����õ�weights
        MPI_Scatter (buffer_for_packed, num_each_layer[i_layer] * i_size, MPI_PACKED, recv_buffer, num_each_layer[i_layer]*i_size, MPI_PACKED, 0, MPI_COMM_WORLD);

        //�㲥layers[i_layer - 1]->output_nodes
        MPI_Bcast (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);

        //�ӽ�����Ҫ��weights��������к�������
        if (myid != 0)
        {
            position = 0;
            for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
            {
                MPI_Unpack ( recv_buffer, buff_size, &position, layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, MPI_COMM_WORLD);
            }
        }
        delete[]buffer_for_packed;
        delete[]recv_buffer;

        //����
        for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
        {
            layers[i_layer]->output_nodes[i] = 0.0;
            for (int j = 0; j < num_each_layer[i_layer]; j++)
            {
                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
            }
            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
        }

        //�����ռ���gather��0�Ž���
        float* buff_output = new float[num_each_layer[i_layer + 1]];
        MPI_Gather (layers[i_layer]->output_nodes + myid * i_size, my_i_size, MPI_FLOAT, buff_output, my_i_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
        if (myid == 0)
            memcpy (layers[i_layer]->output_nodes, buff_output, num_each_layer[i_layer + 1]);
        delete[] buff_output;
    }
}

void ANN_MPI::predict_MPI_alltoall()
{
    //all-to-all
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

    //��һ����Ҫ���й㲥
    MPI_Bcast (layers[0]->output_nodes, num_each_layer[0], MPI_FLOAT, 0, MPI_COMM_WORLD);

    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        //���ݴ�������͡����
        int position = 0;
        int buff_size = num_each_layer[i_layer + 1] * num_each_layer[i_layer] * sizeof (float);
        float* buffer_for_packed = new float[buff_size];
        float* recv_buffer = new float[buff_size];

        if (myid == 0)
        {
            for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            {
                MPI_Pack (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, buffer_for_packed, buff_size, &position, MPI_COMM_WORLD);
            }
        }

        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������
        MPI_Scatter (buffer_for_packed, num_each_layer[i_layer] * i_size, MPI_PACKED, recv_buffer, num_each_layer[i_layer]*i_size, MPI_PACKED, 0, MPI_COMM_WORLD);

        if (myid != 0)
        {
            position = 0;
            for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
            {
                MPI_Unpack ( recv_buffer, buff_size, &position, layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, MPI_COMM_WORLD);
            }
        }
        delete[]buffer_for_packed;
        delete[]recv_buffer;

        //����
        for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
        {
            layers[i_layer]->output_nodes[i] = 0.0;
            for (int j = 0; j < num_each_layer[i_layer]; j++)
            {
                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
            }
            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
            //�����ռ������͸����н���
            MPI_Allgather (&layers[i_layer]->output_nodes[i], 1, MPI_FLOAT, &layers[i_layer]->output_nodes[i], 1, MPI_FLOAT, MPI_COMM_WORLD);
        }

    }
}

void ANN_MPI::predict_MPI_static2()
{
    //��̬�黮�֣�0�Ž��̽�������ʱ����MPI_ANY_SOURCE������˳�����
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������
        //���ݷ���
        MPI_Bcast (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            MPI_Bcast (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        //����
        for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
        {
            layers[i_layer]->output_nodes[i] = 0.0;
            for (int j = 0; j < num_each_layer[i_layer]; j++)
            {
                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
            }
            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
        }
        //������ͺͽ���
        if (myid == 0)
        {
            float* temp_nodes = new float[num_each_layer[i_layer + 1]];
            for (int temp_procs_i = 1; temp_procs_i < numprocs; temp_procs_i++)
            {
                //0�Ž��̽�������ʱ����MPI_ANY_SOURCE������˳�����
                MPI_Recv (temp_nodes, i_size, MPI_FLOAT, MPI_ANY_SOURCE, 97, MPI_COMM_WORLD, &status);
                memcpy (layers[i_layer]->output_nodes + status.MPI_SOURCE * i_size, temp_nodes, sizeof (float) * i_size);
            }
            delete[] temp_nodes;
            // cout<<myid<<"finish recv"<<endl;
        }
        else
        {
            //�ӽڵ㽫���ݷ��ͻ����ڵ㣨0�ţ�
            MPI_Send (layers[i_layer]->output_nodes + myid * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
            // cout<<myid<<"finish send"<<endl;
        }
    }
}

void ANN_MPI::predict_MPI_rma()
{
    //rma����ͨ�ţ���ÿ���output_nodes����
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    //�������ھ��
    MPI_Win win_previous_layer;
    MPI_Win win_current_layer;
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������

        //����һ��͵�ǰ���output_nodes����
        MPI_Win_create(layers[i_layer - 1]->output_nodes, num_each_layer[i_layer] * sizeof(float), sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &win_previous_layer);
        MPI_Win_create(layers[i_layer]->output_nodes, num_each_layer[i_layer + 1] * sizeof(float), sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &win_current_layer);


        //����weights����
        for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            MPI_Bcast(layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);

        //����
        for (int i = myid * i_size; i < min(num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
        {
            layers[i_layer]->output_nodes[i] = 0.0;
            for (int j = 0; j < num_each_layer[i_layer]; j++)
            {
                layers[i_layer]->output_nodes[i] += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
            }
            layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
            layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function(layers[i_layer]->output_nodes[i]);
        }
        //������Ҫ������ͺͽ���,��Ϊ��֤��һ��Ľڵ��Ѿ�ȫ��������ɣ���Ҫ����ͬ����
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Win_free(&win_previous_layer);
    MPI_Win_free(&win_current_layer);
}

void ANN_MPI::predict_MPI_dynamic()
{
    //��̬��������,����ʽ,0�Ž��̲��������
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        //���ݷ���
        //int g = 1;���ȱ���Ϊ1�У���Ϊ���еĽ��ղ��ܱ�֤Ϊԭ�Ӳ���
        //�㲥layers[i_layer - 1]->output_nodes,
        MPI_Bcast (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        if (myid == 0)
        {
            int i;
            for (i = 0; i < numprocs - 1; i++)
            {
                //for (int g_i = 0; g_i < g; g_i++)
                MPI_Send (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, i + 1, i, MPI_COMM_WORLD); //����һ�У�tagΪ�к�
            }
            float temp_node;
            int finish = 0;
            while (finish < numprocs - 1)
            {
                //cout << "finish"<<finish << endl;
                //��i-1�������
                MPI_Recv (&temp_node, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                //memcpy(layers[i_layer]->output_nodes + status.MPI_TAG * g, temp_nodes, sizeof(float) * g);
                if (status.MPI_TAG < num_each_layer[i_layer + 1])
                    layers[i_layer]->output_nodes[status.MPI_TAG] = temp_node;

                if (i < num_each_layer[i_layer + 1])  //����tag�ж��Ƿ����ȫ������
                {
                    //��δ��ɣ������µ�һ�У�i
                    MPI_Send (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, status.MPI_SOURCE, i, MPI_COMM_WORLD);
                    i++;
                }
                else
                {
                    //�Ѿ���ɣ������������ݣ�ʹ�ӽ����˳�ѭ��
                    //cout << "finish" << finish << endl;
                    MPI_Send (layers[i_layer]->weights[0], num_each_layer[i_layer], MPI_FLOAT, status.MPI_SOURCE, num_each_layer[i_layer + 1] + 1, MPI_COMM_WORLD);
                    finish++;
                }
            }
        }
        else
        {
            float* temp_nodes = new float[num_each_layer[i_layer]];
            int i = myid;
            while (i < num_each_layer[i_layer + 1])
            {
                //����һ�У�tagΪ�к�
                MPI_Recv (temp_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                i = status.MPI_TAG;
                if (i >= num_each_layer[i_layer + 1]) break;

                //����
                float output_nodes = 0.0;
                for (int j = 0; j < num_each_layer[i_layer]; j++)
                {
                    output_nodes += temp_nodes[j] * layers[i_layer - 1]->output_nodes[j];
                }
                output_nodes += layers[i_layer]->bias[i];
                output_nodes = layers[i_layer]->activation_function (output_nodes);

                //�ӽڵ㽫���ݷ��ͻ����ڵ㣨0�ţ�
                MPI_Send (&output_nodes, 1, MPI_FLOAT, 0, i, MPI_COMM_WORLD);
            }
            delete[] temp_nodes;
        }
        //cout<<myid<<"finish pridect"<< i_layer<<endl;
    }
}

/*void ANN_MPI::predict_MPI_dynamic_g() {
	//��̬��������,����ʽ,0�Ž��̲��������,���񻮷ֵ�����Ϊg
	int myid, numprocs;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	for (int i_layer = 1; i_layer <= num_layers; i_layer++)
	{
		//���ݷ���
		int g = 1;
		//�㲥layers[i_layer - 1]->output_nodes,
		MPI_Bcast(layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
		float* temp_nodes=new float[num_each_layer[i_layer + 1]];
		if (myid == 0)
		{
			int i;
			for (i = 0; i < numprocs - 1; i++) {
				for (int g_i = 0; g_i < g; g_i++)
					MPI_Send(layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, i + 1, i*g+g_i, MPI_COMM_WORLD);//����һ�У�tagΪ�к�
			}

			int finish = 0;
			float temp_node;
			while (finish < numprocs - 1) {
				//cout << "finish"<<finish << endl;
				//��i-1�������
				MPI_Recv(&temp_node, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				//memcpy(layers[i_layer]->output_nodes + status.MPI_TAG * g, temp_nodes, sizeof(float) * g);
				if (status.MPI_TAG < num_each_layer[i_layer + 1])
					layers[i_layer]->output_nodes[status.MPI_TAG] = temp_node;

				if (i < num_each_layer[i_layer + 1]) {//����tag�ж��Ƿ����ȫ������
					//�����µ�g�У�i
					for (int g_i = 0; g_i < g; g_i++)
						MPI_Send(layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, status.MPI_SOURCE, i, MPI_COMM_WORLD);
					i++;
				}
				else {
					//�����������ݣ�ʹ�ӽ����˳�ѭ��
					//cout << "finish" << finish << endl;
					MPI_Send(layers[i_layer]->weights[0], num_each_layer[i_layer], MPI_FLOAT, status.MPI_SOURCE, num_each_layer[i_layer + 1] + 1, MPI_COMM_WORLD);
					finish++;
				}
			}

		}
		else
		{
			float* temp_weights = new float[num_each_layer[i_layer]];
			int i = myid;
			while (i < num_each_layer[i_layer + 1]) {
				MPI_Recv(temp_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				i = status.MPI_TAG;
				if (i >= num_each_layer[i_layer + 1]) break;
				//����
				float output_nodes = 0.0;
				for (int j = 0; j < num_each_layer[i_layer]; j++)
				{
					output_nodes += temp_weights[j] * layers[i_layer - 1]->output_nodes[j];
				}
				output_nodes += layers[i_layer]->bias[i];
				output_nodes = layers[i_layer]->activation_function(output_nodes);

				//�ӽڵ㽫���ݷ��ͻ����ڵ㣨0�ţ�
				MPI_Send(&output_nodes, 1, MPI_FLOAT, 0, i, MPI_COMM_WORLD);
			}
			delete[] temp_weights;
		}
		delete[] temp_nodes;
		//cout<<myid<<"finish pridect"<< i_layer<<endl;
	}
}*/

void ANN_MPI::predict_MPI_threads()
{
    //��predict_MPI_static2��ͬ�������˶��̣߳�MPI+openMP
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������
        //���ݷ���
        MPI_Bcast (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            MPI_Bcast (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        //����
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            //#pragma omp parallel for
            for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
            {
                float sum = 0.0;
                #pragma omp parallel for reduction(+:sum)
                for (int j = 0; j < num_each_layer[i_layer]; j++)
                {
                    sum += layers[i_layer]->weights[i][j] * layers[i_layer - 1]->output_nodes[j];
                }
                sum += layers[i_layer]->bias[i];
                layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (sum);
            }
        }
        if (myid == 0)
        {
            float* temp_nodes = new float[num_each_layer[i_layer + 1]];
            for (int temp_procs_i = 1; temp_procs_i < numprocs; temp_procs_i++)
            {
                MPI_Recv (temp_nodes, i_size, MPI_FLOAT, MPI_ANY_SOURCE, 97, MPI_COMM_WORLD, &status);
                memcpy (layers[i_layer]->output_nodes + status.MPI_SOURCE * i_size, temp_nodes, sizeof (float) * i_size);
            }
            delete[] temp_nodes;
            // cout<<myid<<"finish recv"<<endl;
        }
        else
        {
            //�ӽڵ㽫���ݷ��ͻ����ڵ㣨0�ţ�
            MPI_Send (layers[i_layer]->output_nodes + myid * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
            // cout<<myid<<"finish send"<<endl;
        }
    }
}

void ANN_MPI::predict_MPI_threads_SIMD()
{
    //��̬���Ż�����,����ȫ������,MPI+openMP
    int myid, numprocs;
    MPI_Status status;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
    for (int i_layer = 1; i_layer <= num_layers; i_layer++)
    {
        int i_size = (num_each_layer[i_layer + 1] + numprocs - 1) / (numprocs);//0�Ž��̲������
        //���ݷ���
        MPI_Bcast (layers[i_layer - 1]->output_nodes, num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < num_each_layer[i_layer + 1]; i++)
            MPI_Bcast (layers[i_layer]->weights[i], num_each_layer[i_layer], MPI_FLOAT, 0, MPI_COMM_WORLD);
        //����
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            #pragma omp for
            for (int i = myid * i_size; i < min (num_each_layer[i_layer + 1], (myid + 1) * i_size); i++)
            {
                layers[i_layer]->output_nodes[i] = 0.0;
                int max_j = num_each_layer[i_layer];
                __m128 ans = _mm_setzero_ps();
                for (int j = 0; j < max_j; j += 4)
                    // for (int j = t_id; j < class_p->num_each_layer[0]; j += NUM_THREADS)
                {
                    __m128 t1, t2;
                    //���ڲ�ѭ����Ϊ4λ����������
                    t1 = _mm_loadu_ps (layers[i_layer]->weights[i] + j);
                    t2 = _mm_loadu_ps (layers[i_layer - 1]->output_nodes + j);
                    t1 = _mm_mul_ps (t1, t2);

                    ans = _mm_add_ps (ans, t1);
                }
                // 4���ֲ������
                ans = _mm_hadd_ps (ans, ans);
                ans = _mm_hadd_ps (ans, ans);
                //�����洢
                float z;
                _mm_store_ss (&z, ans);
                layers[i_layer]->output_nodes[i] += z;
                layers[i_layer]->output_nodes[i] += layers[i_layer]->bias[i];
                layers[i_layer]->output_nodes[i] = layers[i_layer]->activation_function (layers[i_layer]->output_nodes[i]);
            }
        }
        if (myid == 0)
        {
            float* temp_nodes = new float[num_each_layer[i_layer + 1]];
            for (int temp_procs_i = 1; temp_procs_i < numprocs; temp_procs_i++)
            {
                MPI_Recv (temp_nodes, i_size, MPI_FLOAT, MPI_ANY_SOURCE, 97, MPI_COMM_WORLD, &status);
                memcpy (layers[i_layer]->output_nodes + status.MPI_SOURCE * i_size, temp_nodes, sizeof (float) * i_size);
            }
            delete[] temp_nodes;
            // cout<<myid<<"finish recv"<<endl;
        }
        else
        {
            //�ӽڵ㽫���ݷ��ͻ����ڵ㣨0�ţ�
            MPI_Send (layers[i_layer]->output_nodes + myid * i_size, i_size, MPI_FLOAT, 0, 97, MPI_COMM_WORLD);
            // cout<<myid<<"finish send"<<endl;
        }
    }
}

void ANN_MPI::train_MPI_predict (const int num_sample, float** _trainMat, float** _labelMat)
{
    //�������򴫲�ѭ��������MPI�Ż�


    int myid, numprocs;
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);

    printf ("%dbegin training\n", myid);

    float thre = 1e-2;
    float* avr_X = new float[num_each_layer[0]];
    float* avr_Y = new float[num_each_layer[num_layers + 1]];


    long long time_mpi1 = 0, time_mpi2 = 0, time_mpi3 = 0, time_mpi4 = 0;
    long long head, tail, freq;// timers
    QueryPerformanceFrequency ( (LARGE_INTEGER*) &freq);

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

                for (int i = 0; i < num_each_layer[1]; i++)
                {
                    layers[0]->output_nodes[i] = 0.0;
                    for (int j = 0; j < num_each_layer[0]; j++)
                    {
                        layers[0]->output_nodes[i] += layers[0]->weights[i][j] * _trainMat[index][j];
                    }
                    layers[0]->output_nodes[i] += layers[0]->bias[i];
                    layers[0]->output_nodes[i] = layers[0]->activation_function (layers[0]->output_nodes[i]);
                }//cout <<myid<< "here52" << endl;
                MPI_Barrier (MPI_COMM_WORLD);
                //cout <<myid<< "here55" << endl;
                QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
                this->predict_MPI_rma();
                QueryPerformanceCounter ( (LARGE_INTEGER*) &tail);
                time_mpi1 += tail - head;//cout <<myid<< "rma" << endl;

                MPI_Barrier (MPI_COMM_WORLD);
                QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
                //this->predict_MPI_alltoall();
                predict_serial();
                QueryPerformanceCounter ( (LARGE_INTEGER*) &tail);
                time_mpi2 += tail - head;//cout << myid<<"all" << endl;

                MPI_Barrier (MPI_COMM_WORLD);
                QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
                //this->predict_MPI_dynamic();
                predict_MPI_static2();
                QueryPerformanceCounter ( (LARGE_INTEGER*) &tail);
                time_mpi3 += tail - head; //cout << myid<<"simd" << endl;
                MPI_Barrier (MPI_COMM_WORLD);
                QueryPerformanceCounter ( (LARGE_INTEGER*) &head); // start time
                //this->predict_MPI_threads();
                //predict_MPI_gather();
                predict_MPI_dynamic();
                QueryPerformanceCounter ( (LARGE_INTEGER*) &tail);
                time_mpi4 += tail - head;// cout << myid<<"gather" << endl;
                MPI_Barrier (MPI_COMM_WORLD);

                if (myid != 0)
                    continue;

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
            if (index >= num_sample)
            {
                static int tt = 0;
                float loss = 0.0;
                for (int t = 0; t < num_each_layer[num_layers + 1]; ++t)
                {
                    loss += (layers[num_layers]->output_nodes[t] - avr_Y[t]) * (layers[num_layers]->output_nodes[t] - avr_Y[t]);
                }
                // printf ("��%d��ѵ����%0.12f\n", tt++, loss);
            }

            //���򴫲����²���

            //����ÿ���delta,�з����Ż�
            for (int i = num_layers - 1; i >= 0; i--)
            {
                float* error = new float[num_each_layer[i + 1]];
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
                    layers[0]->weights[k][j] -= study_rate * avr_X[j] * layers[0]->delta[k];
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
            //printf ("finish bp with index:%d\n",index);
        }
        // display();
    }

    //MPI_Finalize();
    //if(myid==0){
    cout << myid << "mpi1:" << time_mpi1 * 1.0 / freq << "s" << endl;
    cout << myid << "mpi2:" << time_mpi2 * 1.0 / freq << "s" << endl;
    cout << myid << "mpi3:" << time_mpi3 * 1.0 / freq << "s" << endl;
    cout << myid << "mpi4:" << time_mpi4 * 1.0 / freq << "s" << endl;
    //}
    printf ("%d finish training\n", myid);

    delete[]avr_X;
    delete[]avr_Y;
}


bool ANN_MPI::isNotConver_ (const int _sampleNum, float** _trainMat, float** _labelMat, float _thresh)
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

void ANN_MPI::predict (float* in)
{
    layers[0]->_forward (in);
    for (int i = 1; i <= num_layers; i++)
    {
        layers[i]->_forward (layers[i - 1]->output_nodes);
    }
}
void ANN_MPI::display()
{
    for (int i = 0; i <= num_layers; i++)
    {
        layers[i]->display();
    }
}


void ANN_MPI::get_predictions (float* X)
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
