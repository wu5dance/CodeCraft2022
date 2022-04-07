#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <random>
//#include <time.h>

using namespace std;

#define MAX_M 135
#define MAX_N 35
#define MAX_T 8928

//�ͻ��ڵ�
typedef struct clientNode
{
    //string id;
    //string t;
    int bd_demand_t;//tʱ�̵Ĵ�������
    int bd_demand_t_left;//tʱ�̵Ĵ�������ʣ��
    //int e_counter;//�ܹ����з���ı�Ե�ڵ���Ŀ
    unordered_map<string, int> qos;
    //vector<pair<string, int>> qos;
    unordered_map<string, int> demand_t;

}CN;

//��Ե�ڵ�
typedef struct edgeNode
{
    //string id;
    int bd_max;//��������
    int bd_sum_t;//tʱ�̽��յ��Ĵ��������
    int bd_left;//��ǰʣ��Ĵ���
    int counter;//��¼��Ե�ڵ�ʱ�������в�Ϊ0�ĸ���
    vector<int> timeSequence;//��¼��Ե�ڵ��ʱ������
    int c_counter;//��¼�ܽ����Ŀͻ��ڵ����
    //unordered_map<string, int> qos;
    //unordered_map<string, int> demand_t;//��¼��tʱ�̷�����ͻ��ڵ�Ĵ���
}EN;

//��������
typedef struct Qos
{
    //string CN_id;
    //string EN_id;
    CN* client;
    EN* edge;
    int timeDelay;//ʱ�ӣ���ʱ��С��ʱ������Qʱ���ܹ������������䣩
    int demand_t;//��¼��tʱ�����пͻ��ڵ����Ե�ڵ��Ӧ����Ĵ���
}Qos;

unordered_map<string, CN> c;
//vector<pair<string,vector<pair<string, int>>>> c_qos_vector;
vector<pair<string, CN>> c_vector;
unordered_map<string, EN> e;
vector<pair<string, EN>> e_vector;
vector<vector<Qos>> q;
string t;
int t_counter = 0;//��¼��ǰ�ǵڼ���ʱ��
int t_sum = 0;//��¼�ܹ��ж��ٸ�ʱ��
int qos_max = 0;//qos����
int M = 0, N = 0;


//��ʼ���ͻ��ڵ㣬�����ؿͻ��ڵ���Ŀ
int initClient(string fileDemand)
{
    ifstream file(fileDemand);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int cols = 0;
    string lineStr;//��¼��ȡ��ÿ���ַ���
    string str;
    getline(file, lineStr);//��ȡһ���ַ��������ͻ��ڵ�id��һ��
    lineStr.erase(lineStr.find_last_not_of('\r') + 1, string::npos);
    stringstream ss(lineStr);
    vector<string> rowStr;
    pair<string, CN> c_vector_new;
    while (getline(ss, str, ','))
    {
        rowStr.push_back(str);
    }
    cols = rowStr.size();
    for (int i = 0; i < cols - 1; i++)
    {
        CN c_new;
        c_new.bd_demand_t = 0;
        c_new.bd_demand_t_left = 0;
        c_new.qos.clear();
        c_new.demand_t.clear();
        c[rowStr[i + 1]] = c_new;
        c_vector_new.first = rowStr[i + 1];
        c_vector_new.second = c_new;
        c_vector.push_back(c_vector_new);
    }
    file.close();
    return c.size();
}

bool lessSort_bd_max(pair<string, EN> a, pair<string, EN> b) { return (a.second.bd_max > b.second.bd_max); }
bool GreaterSort_c_counter(pair<string, EN> a, pair<string, EN> b) { return (a.second.c_counter > b.second.c_counter); }

//��ʼ����Ե�ڵ㣬���ر�Ե�ڵ���Ŀ
int ReadSite(string fileSite)
{
    ifstream file(fileSite);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    string lineStr, str;

    int rows = 0;
    while (getline(file, lineStr) && file.good())
    {
        rows++;
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        if (rows > 1)
        {
            EN e_new;
            e_new.bd_max = atoi(rowStr[1].c_str());
            e_new.bd_left = e_new.bd_max;
            e_new.bd_sum_t = 0;
            e_new.counter = 0;
            e_new.timeSequence.clear();
            e_new.c_counter = 0;
            e_vector.push_back(make_pair(rowStr[0], e_new));
            e[rowStr[0]] = e_new;
        }
    }
    N = e_vector.size();
    //sort(e_vector.begin(), e_vector.end(), lessSort_bd_max);

    file.close();
    return e.size();
}

bool GreaterSort(pair<string, int> a, pair<string, int> b) { return (a.second < b.second); }

vector<string> cid;

//��ȡqos.csv,��֮�洢����Ե�ڵ���
int ReadQos(string fileQos)
{
    ifstream file(fileQos);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;
    pair<string, int> qos_new;

    while (getline(file, lineStr) && file.good())
    {
        rows++;
        cols = 0;

        lineStr.erase(lineStr.find_last_not_of('\r') + 1, string::npos);

        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            if (rows == 1)
            {
                cid.push_back(str);
            }
            else
                rowStr.push_back(str);
        }
        cols = rowStr.size();
        unordered_map<string, CN>::iterator cget;
        //pair<string, vector<pair<string, int>>> c_qos_vector_new;
        //vector<pair<string, int>> qos_new_vector;
        if (rows > 1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                qos_new.first = rowStr[0];
                qos_new.second = atoi(rowStr[i + 1].c_str());
                cget = c.find(cid[i + 1]);
                //cget->second.qos. = atoi(rowStr[i + 1].c_str());
                cget->second.qos.insert(qos_new);
                //c_qos_vector_new.first = cid[i + 1];
                //qos_new_vector.push_back(qos_new);
                //c_qos_vector_new.second = qos_new_vector;
                //c_qos_vector.push_back(c_qos_vector_new);
                cget->second.demand_t[rowStr[0]] = 0;
            }
        }
    }
    file.close();

    //vector<pair<string, int>> tmp;
    //pair<string, int> tmp_new;
    //unordered_map<string, int> tmp_map;
    //for (int i = 0; i < c_qos_vector.size();i++)//��ÿ���ͻ��ڵ��Ӧ�ı�Ե�ڵ��qos��������
    //{
    //    sort(c_qos_vector[i].second.begin(), c_qos_vector[i].second.end() , GreaterSort);
    //}
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        for (auto evit = e_vector.begin(); evit != e_vector.end(); evit++)
        {
            if (cit->second.qos[evit->first] < qos_max)
            {
                auto eget = e.find(evit->first);
                eget->second.c_counter++;
                evit->second.c_counter++;
            }
        }
    }
    sort(e_vector.begin(), e_vector.end(), GreaterSort_c_counter);
    return 1;
}

//��ȡConfig.ini��������qos����qos_max
int ReadConfig(string fileConfig)
{
    ifstream file(fileConfig);

    string lineStr;
    int rows = 0;
    while (getline(file, lineStr))
    {
        rows++;
        //cout << "���ǵ�" << rows << "��" << endl;
        //cout << lineStr << endl;
        if (rows == 2)
        {
            //cout << lineStr << endl;
            int loc = lineStr.find("=");
            int end = lineStr.size();
            lineStr = lineStr.substr(loc + 1, end - 1);
            //cout << lineStr << endl;
            qos_max = atoi(lineStr.c_str());
        }
    }
    file.close();
    return qos_max;
}

int getSolution_free()
{
    int demand_t = 0;
    int percent5 = t_sum - ceil(t_sum * 0.95);
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        if (cit->second.bd_demand_t_left > 0)
        {
            for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                auto eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }
        }
    }
    return 1;
}
int getSolution_free2()
{
    int demand_t = 0;
    int percent5 = t_sum - ceil(t_sum * 0.95);
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        if (cit->second.bd_demand_t_left > 0)
        {
            for (auto evit = e_vector.begin(); evit != e_vector.end(); evit++)
            {
                auto eget = e.find(evit->first);
                if (cit->second.qos[eget->first] < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }
            /*for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                auto eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }*/
        }
    }
    return 1;
}

int getSolution_free3()
{
    int demand_t = 0;
    int percent5 = t_sum - ceil(t_sum * 0.95);
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        if (cit->second.bd_demand_t_left > 0)
        {
            srand(time(nullptr));
            int i = rand() % N;
            //cout << "������ӣ�" << i << endl;
            for (; i != (N + i) % N; i++)
            {
                auto evit = e_vector[i];
                auto eget = e.find(evit.first);
                if (cit->second.qos[eget->first] < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }
            /*for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                auto eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }*/
        }
    }
    return 1;
}

int seedj = 0, seedi = 0;
int getSolution_free4()
{
    int demand_t = 0;
    int percent5 = t_sum - ceil(t_sum * 0.95);
    srand(time(nullptr));
    seedj = rand() % M;
    seedi = rand() % N;
    //cout << "j��������ӣ�" << j << endl;
    for (int j = seedj; j != (M + seedj - 1) % M; j = (j + 1) % M)
    {
        auto cit = c.find(c_vector[j].first);
        if (cit->second.bd_demand_t_left > 0)
        {


            //cout << "i��������ӣ�" << i << endl;
            for (int i = seedi; i != (N + seedi - 1) % N; i = (i + 1) % N)
            {
                auto evit = e_vector[i];
                auto eget = e.find(evit.first);
                if (cit->second.qos[eget->first] < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }
            /*for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                auto eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }*/
        }
    }
    return 1;
}

int getSolution_free4(int seedj_r, int seedi_r)
{
    int demand_t = 0;
    int percent5 = t_sum - ceil(t_sum * 0.95);
    srand(time(nullptr));
    //int seedj = rand() % M;
    //cout << "j��������ӣ�" << j << endl;
    for (int j = seedj_r; j != (M + seedj_r - 1) % M; j = (j + 1) % M)
    {
        auto cit = c.find(c_vector[j].first);
        if (cit->second.bd_demand_t_left > 0)
        {

            //int seedi = rand() % N;
            //cout << "i��������ӣ�" << i << endl;
            for (int i = seedi_r; i != (N + seedi_r - 1) % N; i = (i + 1) % N)
            {
                auto evit = e_vector[i];
                auto eget = e.find(evit.first);
                if (cit->second.qos[eget->first] < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }
            /*for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                auto eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0 && eget->second.counter < percent5)
                {
                    if (eget->second.bd_left >= cit->second.bd_demand_t_left)
                    {
                        demand_t = cit->second.bd_demand_t_left;
                    }
                    else
                    {
                        demand_t = eget->second.bd_left;
                    }
                    cit->second.bd_demand_t_left -= demand_t;
                    eget->second.bd_left -= demand_t;
                    eget->second.bd_sum_t += demand_t;
                    cit->second.demand_t[eget->first] += demand_t;
                }
            }*/
        }
    }
    return 1;
}

void getSolution_scale2()
{
    int demand_t = 0;
    int bd_sum_t = 0;
    double scale = 0.0;
    unordered_map<string, EN>::iterator eget;
    int e_counter = 0;
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        if (cit->second.bd_demand_t_left > 0)
        {
            bd_sum_t = 0;
            e_counter = 0;
            for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                //cout <<cit->first<<"��"<<qit->first<<"��qosΪ:"<< qit->second << endl;
                eget = e.find(qit->first);
                if (qit->second < qos_max && eget->second.bd_left>0)
                {
                    e_counter++;
                    //cout << eget->first << "��ʣ�����Ϊ:" << eget->second.bd_left << endl;
                    bd_sum_t += eget->second.bd_left;
                }
            }
            //cout << cit->first <<"ʣ�������"<<cit->second.bd_demand_t_left<< "�ܷ���ı�Ե�ڵ������" << e_counter <<"�ܵõ��Ĵ����ܺͣ�"<<bd_sum_t<< endl;
            int c_demand_left = cit->second.bd_demand_t_left;
            int counter = 0;
            for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
            {
                if (qit->second < qos_max)
                {
                    eget = e.find(qit->first);
                    if (eget->second.bd_left > 0 && cit->second.bd_demand_t_left > 0)
                    {
                        scale = (double)eget->second.bd_left / (double)bd_sum_t;
                        //cout << eget->first << "�ķ������Ϊ��" << scale << endl;
                        //auto qnext = qit;
                        //qnext++;
                        counter++;
                        if (/*qnext->second<qos_max*/counter < e_counter)
                        {
                            demand_t = floor(c_demand_left * scale);
                        }
                        else
                        {
                            demand_t = cit->second.bd_demand_t_left;
                        }
                        if (demand_t > 0)
                        {
                            cit->second.bd_demand_t_left -= demand_t;
                            eget->second.bd_left -= demand_t;
                            eget->second.bd_sum_t += demand_t;
                            cit->second.demand_t[eget->first] += demand_t;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
        }
    }
}

//void getSolution_scale()
//{
//    int demand_t = 0;
//    int bd_sum_t = 0;
//    double scale = 0.0;
//    unordered_map<string, EN>::iterator eget;
//    vector<pair<string, int>>::iterator qnext;
//    int e_counter = 0;
//    for (auto cit = c.begin(); cit != c.end(); cit++)
//    {
//        if (cit->second.bd_demand_t_left > 0)
//        {
//            bd_sum_t = 0;
//            e_counter = 0;
//            for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
//            {
//                //cout <<cit->first<<"��"<<qit->first<<"��qosΪ:"<< qit->second << endl;
//                eget = e.find(qit->first);
//                if (qit->second < qos_max&&eget->second.bd_left>0)
//                {
//                    e_counter++;
//                    //cout << eget->first << "��ʣ�����Ϊ:" << eget->second.bd_left << endl;
//                    bd_sum_t += eget->second.bd_left;
//                }
//                else
//                {
//                    //cout << qit->second << endl;
//                    break;
//                }
//            }
//            //cout << cit->first <<"ʣ�������"<<cit->second.bd_demand_t_left<< "�ܷ���ı�Ե�ڵ������" << e_counter <<"�ܵõ��Ĵ����ܺͣ�"<<bd_sum_t<< endl;
//            int c_demand_left = cit->second.bd_demand_t_left;
//            int counter = 0;
//            for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
//            {
//                if (qit->second < qos_max)
//                {
//                    eget = e.find(qit->first);
//                    if (eget->second.bd_left > 0&&cit->second.bd_demand_t_left>0)
//                    {
//                        scale = (double)eget->second.bd_left / (double)bd_sum_t;
//                        //cout << eget->first << "�ķ������Ϊ��" << scale << endl;
//                        qnext = qit;
//                        qnext++;
//                        counter++;
//                        if (/*qnext->second<qos_max*/counter<e_counter)
//                        {
//                            demand_t = floor(c_demand_left * scale);
//                        }
//                        else
//                        {
//                            demand_t = cit->second.bd_demand_t_left;
//                        }
//                        if (demand_t > 0)
//                        {
//                            cit->second.bd_demand_t_left -= demand_t;
//                            eget->second.bd_left -= demand_t;
//                            eget->second.bd_sum_t += demand_t;
//                            cit->second.demand_t[eget->first] += demand_t;
//                        }
//                        else
//                        {
//                            continue;
//                        }
//                    }
//                }
//                else
//                {
//                    break;
//                }
//            }
//        }
//    }
//}

void writeSolution(string fileSolution)
{
    ////д��֮ǰ���
    //ofstream file_writer(fileSolution, ios_base::out);
    //file_writer.close();

    ofstream outfile;
    outfile.open(fileSolution, ios::app);

    int no0_counter = 0, no0_sum = 0;
    int demand = 0;
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        demand = 0;
        no0_sum = 0;
        for (auto dit = cit->second.demand_t.begin(); dit != cit->second.demand_t.end(); dit++)
        {
            if (dit->second > 0)
            {
                no0_sum++;
                demand += dit->second;
            }
        }
        //if (demand != cit->second.bd_demand_t)
        //{
        //    cout << cit->first << "�õ����ܴ���Ϊ��" << demand << ",��ʱ�̵�������Ϊ��" << cit->second.bd_demand_t <<"���䲻����"<< endl;
        //}


        if (cit->second.bd_demand_t == 0)
        {
            outfile << cit->first << ":" << endl;
            //cout << cit->first << ":" << endl;
        }
        else
        {
            outfile << cit->first << ":";
            //cout << cit->first << ":";
            no0_counter = 0;
            for (auto dit = cit->second.demand_t.begin(); dit != cit->second.demand_t.end(); dit++)
            {
                if (dit->second > 0)
                {
                    no0_counter++;
                    if (no0_counter < no0_sum)
                    {
                        outfile << "<" << dit->first << "," << dit->second << ">,";
                        //cout << "<" << eit->first << "," << eit->second.demand_t[cit->first] << ">,";
                        //cout << "<" << dit->first << "," << dit->second << ">,";
                    }
                    else
                    {
                        if (t_counter == t_sum && no0_counter == no0_sum && cit == c.end())
                        {
                            outfile << "<" << dit->first << "," << dit->second << ">";
                            //cout << "<" << dit->first << "," << dit->second << ">";
                        }
                        else
                        {
                            outfile << "<" << dit->first << "," << dit->second << ">" << endl;
                            //cout << "<" << dit->first << "," << dit->second << ">" << endl;
                        }
                    }
                }
            }
        }
    }
    outfile.close();
}


int ReadDemand(string fileDemand, string fileSolution)
{
    ifstream file(fileDemand);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;

    ifstream file_t(fileDemand);
    t_sum = 0;
    while (getline(file_t, lineStr) && file_t.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        t_sum++;
    }
    file_t.close();
    t_sum -= 1;
    //cout << "һ����" << t_sum << "��ʱ��" << endl;
    t_counter = 0;
    while (getline(file, lineStr) && file.good())
    {
        rows++;

        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            if (rows == 1)
            {
                cid.push_back(str);
            }
            else
                rowStr.push_back(str);
        }

        cols = rowStr.size();
        unordered_map<string, CN>::iterator cGet;
        if (rows > 1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                t = rowStr[0];
                cGet = c.find(cid[i + 1]);
                cGet->second.bd_demand_t = atoi(rowStr[i + 1].c_str());
                cGet->second.bd_demand_t_left = cGet->second.bd_demand_t;
            }
            t_counter++;
            //cout << t_counter << endl;
            //getSolution_free3();
            getSolution_free4();
            getSolution_scale2();
            writeSolution(fileSolution);
            for (auto eit = e.begin(); eit != e.end(); eit++)
            {
                eit->second.timeSequence.push_back(eit->second.bd_sum_t);
                if (eit->second.bd_sum_t > 0)
                    eit->second.counter++;

                eit->second.bd_left = eit->second.bd_max;
                eit->second.bd_sum_t = 0;
            }
            for (auto cit = c.begin(); cit != c.end(); cit++)
            {
                for (auto dit = cit->second.demand_t.begin(); dit != cit->second.demand_t.end(); dit++)
                {
                    dit->second = 0;
                }
            }
        }
    }
    file.close();
    return t_sum;
}

int ReadDemand(string fileDemand, string fileSolution, int seedj_r, int seedi_r)
{
    ifstream file(fileDemand);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;

    ifstream file_t(fileDemand);
    t_sum = 0;
    while (getline(file_t, lineStr) && file_t.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        t_sum++;
    }
    file_t.close();
    t_sum -= 1;
    //cout << "һ����" << t_sum << "��ʱ��" << endl;

    t_counter = 0;
    while (getline(file, lineStr) && file.good())
    {
        rows++;

        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            if (rows == 1)
            {
                cid.push_back(str);
            }
            else
                rowStr.push_back(str);
        }

        cols = rowStr.size();
        unordered_map<string, CN>::iterator cGet;
        if (rows > 1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                t = rowStr[0];
                cGet = c.find(cid[i + 1]);
                cGet->second.bd_demand_t = atoi(rowStr[i + 1].c_str());
                cGet->second.bd_demand_t_left = cGet->second.bd_demand_t;
            }
            t_counter++;
            //cout << t_counter << endl;
            //getSolution_free3();
            getSolution_free4(seedj_r, seedi_r);
            getSolution_scale2();
            writeSolution(fileSolution);
            for (auto eit = e.begin(); eit != e.end(); eit++)
            {
                eit->second.timeSequence.push_back(eit->second.bd_sum_t);
                if (eit->second.bd_sum_t > 0)
                    eit->second.counter++;

                eit->second.bd_left = eit->second.bd_max;
                eit->second.bd_sum_t = 0;
            }
            for (auto cit = c.begin(); cit != c.end(); cit++)
            {
                for (auto dit = cit->second.demand_t.begin(); dit != cit->second.demand_t.end(); dit++)
                {
                    dit->second = 0;
                }
            }
        }
    }
    file.close();
    return t_sum;
}


int getCost()
{
    int percent95 = ceil(t_sum * 0.95);
    int cost = 0;
    //int counter = 0;
    for (auto eit = e.begin(); eit != e.end(); eit++)
    {
        sort(eit->second.timeSequence.begin(), eit->second.timeSequence.end());
        //for (int i=0;i<t_sum;i++)
        //{
        //    if (i > percent95)
        //    {
        //        if (eit->second.timeSequence[i] == eit->second.bd_max)
        //        {
        //            cout << "��Ե�ڵ�" << eit->first << "��" << i << "����������" << endl;
        //            counter++;
        //        }
        //        else
        //        {
        //            cout << "��Ե�ڵ�" << eit->first << "��" << i << "������û������" << endl;
        //        }
        //    }
        //    //cout << eit->second.timeSequence[i] << ",";
        //}
        //if (counter == t_sum - percent95)
        //{
        //    cout << "��Ե�ڵ�" << eit->first << "��5%����������" << endl;
        //}
        //cout << endl;
        cost += eit->second.timeSequence[percent95];
    }
    return cost;
}

int main()
{
    clock_t startTime, endTime;
    startTime = clock();

    string fileDemand = "/data/demand.csv";
    string fileSiteBD = "/data/site_bandwidth.csv";
    string fileQos = "/data/qos.csv";
    string fileConfig = "/data/config.ini";
    string fileSolution = "/output/solution.txt";

    qos_max = ReadConfig(fileConfig);

    M = initClient(fileDemand);
    //cout << "�ͻ��ڵ���Ŀ��" << M << endl;
    //for (auto cit = c.begin(); cit != c.end(); cit++)
    //{
    //    cout << "�ͻ��ڵ�id��" << cit->first << ",��������" << cit->second.bd_demand_t << ",ʣ������" << cit->second.demand_t_left << endl;
    //}

    N = ReadSite(fileSiteBD);
    //cout << "��Ե�ڵ���Ŀ:" << N << endl;
    //for (auto eit = e.begin(); eit != e.end(); eit++)
    //{
    //    cout << "��Ե�ڵ�id:" << eit->first << ",��������:" << eit->second.bd_max << ",����ʣ�ࣺ" << eit->second.bd_left << ",���ô���ͣ�" << eit->second.bd_sum_t << endl;
    //}

    ReadQos(fileQos);
    //for (auto cit = c.begin(); cit != c.end(); cit++)
    //{
    //    cout << cit->first << " "<<endl;
    //    for (auto qit = cit->second.qos.begin(); qit != cit->second.qos.end(); qit++)
    //    {
    //        cout << qit->first << "," << qit->second <<endl;
    //    }
    //    cout << endl;
    //}

    //д��֮ǰ���
    ofstream file_writer(fileSolution, ios_base::out);
    file_writer.close();

    int best_cost = 0;
    int cost = 0;
    ReadDemand(fileDemand, fileSolution);
    best_cost = getCost();
    endTime = clock();
    int seedj_r = 0, seedi_r = 0;
    vector<int> cost_vector;
    cost_vector.push_back(best_cost);
    while ((double)(endTime - startTime) / CLOCKS_PER_SEC < 250)
    {
        //д��֮ǰ���
        ofstream file_writer1(fileSolution, ios_base::out);
        file_writer1.close();
        ReadDemand(fileDemand, fileSolution);
        cost = getCost();
        cost_vector.push_back(cost);
        if (best_cost > cost)
        {
            best_cost = cost;
            seedj_r = seedj;
            seedi_r = seedi;
        }
        endTime = clock();
    }

    //д��֮ǰ���
    ofstream file_writer2(fileSolution, ios_base::out);
    file_writer2.close();
    ReadDemand(fileDemand, fileSolution, seedj_r, seedi_r);

    //cout << "ÿ�ε����ĳɱ�:";
    //for (int i = 0; i < cost_vector.size(); i++)
    //{
    //    cout << cost_vector[i] << ",";
    //}
    //cout << endl;
    //cout << "��ͳɱ���" << best_cost << endl;

    //ReadDemand(fileDemand, fileSolution);

    //int cost = getCost();
    //cout << "������䷽���ĳɱ�Ϊ��" << cost << endl;

    //endTime = clock();
    //cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
}