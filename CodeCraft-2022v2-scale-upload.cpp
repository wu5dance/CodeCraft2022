#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <algorithm>
using namespace std;

#define MAX_N 135
#define MAX_M 35

typedef struct clientNode//�ͻ��ڵ�
{
    string id;
    //string t;
    int bd_demand_t;//tʱ�̵Ĵ�������
    int e_counter;//�ܹ����з���ı�Ե�ڵ���Ŀ

}CN;
typedef struct edgeNode//��Ե�ڵ�
{
    string id;
    int bd_max;//��������
    int bd_sum_t;//tʱ�̽��յ��Ĵ��������
    int bd_left;//��ǰʣ��Ĵ���
    int counter;
}EN;
typedef struct Qos//��������
{
    //string CN_id;
    //string EN_id;
    CN* client;
    EN* edge;
    int timeDelay;//ʱ�ӣ���ʱ��С��ʱ������Qʱ���ܹ������������䣩
}Qos;

void getSolutionAtMinute_t_scale(vector<CN>& c, vector<EN>& e, vector<vector<Qos>>& q, string filename, int qos_max, int t_counter, int t_sum);
void getSolutionAtMinute_t(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> qos, string filename, int qos_max, int t_sum, int t_counter);

int init_Client(vector<CN>& c, string filename)//��ʼ���ͻ��ڵ�(ֻ��ȡdemand.csv�ļ��еĿͻ��ڵ�id���������нڵ��demand_t��Ϊ0
{
    ifstream file(filename);
    //file.open(fileDemand.c_str(),ios::in);

    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int cols = 0;
    string lineStr, str;
    getline(file, lineStr);
    lineStr.erase(lineStr.find_last_not_of('\r') + 1, string::npos);
    stringstream ss(lineStr);
    vector<string> rowStr;
    while (getline(ss, str, ','))
    {
        rowStr.push_back(str);
    }
    cols = rowStr.size();
    //cout <<"������"<< cols << endl;
    for (int i = 0; i < cols - 1; i++)
    {
        CN c_new;
        c_new.id = rowStr[i + 1];
        c_new.bd_demand_t = 0;
        c_new.e_counter = 0;
        c.push_back(c_new);
    }
    file.close();
    //for (int i = 0; i < 10; i++)
    //{
    //    cout << c[i].id << "," << c[i].bd_demand_t << endl;
    //}
    return c.size();
}

int init_Edege(vector<EN>& e, string filename)//��ʼ����Ե�ڵ�
{
    ifstream file(filename);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}
    int rows = 0;
    string lineStr, str;
    while (getline(file, lineStr) && file.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        rows++;
        //cout <<"������"<< rows_demand << endl;
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        //cout <<"������"<< cols_demand << endl;
        if (rows > 1)
        {
            EN e_new;
            e_new.id = rowStr[0];
            e_new.bd_max = atoi(rowStr[1].c_str());
            e_new.bd_left = e_new.bd_max;
            e_new.bd_sum_t = 0;
            e_new.counter = 0;
            e.push_back(e_new);
        }
    }
    file.close();
    return e.size();
}

int init_Q(vector<vector<Qos>>& q, vector<CN>& c, vector<EN>& e)
{
    for (long unsigned int i = 0; i < c.size(); i++)
    {
        Qos q_new;
        vector<Qos> q_line_new;
        for (long unsigned int j = 0; j < e.size(); j++)
        {
            q_new.client = &c[0];
            q_new.edge = &e[0];
            q_new.timeDelay = 0;
            q_line_new.push_back(q_new);
        }
        q.push_back(q_line_new);
    }
    //cout << "q������" << q.size() << ",q����:" << q[0].size() << endl;
    return 1;
}

int Read_Qos(vector<vector<Qos>>& q, string filename, vector<CN>& c, vector<EN>& e)
{
    ifstream file(filename);
    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;

    while (getline(file, lineStr) && file.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        rows++;
        cols = 0;
        //cout <<"�����ַ�����"<< lineStr << endl;
        lineStr.erase(lineStr.find_last_not_of('\r') + 1, string::npos);
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        cols = rowStr.size();
        if (rows == 1)
        {
            for (long unsigned int i = 0; i < rowStr.size() - 1; i++)
            {
                if (rowStr[i + 1] != c[i].id)
                {
                    //cout << "qos�����demand���ͻ��ڵ�˳��һ��" << endl;
                    //cout << c[0].id << endl;
                    //cout << i << " " << rowStr[i + 1] << "," << c[i].id << endl;
                    return 0;
                }
            }
        }
        else
        {
            if (rowStr[0] != e[rows - 2].id)
            {
                //cout << "qos�����site_bandwidth����Ե�ڵ�˳��һ��" << endl;
                //cout << rows << " " << rowStr[0] << "," << e[rows - 2].id << endl;
                return 0;
            }
            for (int i = 0; i < cols - 1; i++)
            {
                q[i][rows - 2].client = &c[i];
                q[i][rows - 2].edge = &e[rows - 2];
                q[i][rows - 2].timeDelay = atoi(rowStr[i + 1].c_str());
                //cout << q[i][rows - 2].timeDelay << endl;
            }
        }
    }
    file.close();
    return 1;
}

void c_e_count(vector<CN>& c, vector<EN>& e, vector<vector<Qos>>& q, int qos_max)//����ÿ���ͻ��ڵ�i�ܹ������ٸ���Ե�ڵ���з���
{
    int M = c.size(), N = e.size();
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (q[i][j].timeDelay < qos_max)
            {
                c[i].e_counter++;
            }
        }
    }
}

int Read_Config(string filename)
{
    ifstream file(filename);
    string lineStr;
    int rows = 0, qos_max = 0;
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

int Read_Client(vector<CN>& c, string filename, vector<EN>& e, vector<vector<Qos>>& q, string fileSolution, int qos_max)//��ȡdemand.csv����Ϊÿʱ�̽��з���
{
    ifstream file(filename);
    //file.open(fileDemand.c_str(),ios::in);

    //if (file.fail())
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //else
    //{
    //    cout << "�ļ��򿪳ɹ���" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str, t;
    int t_sum = 0, t_counter = 0;

    ifstream file_t(filename);
    while (getline(file_t, lineStr) && file.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        t_sum++;
    }
    file_t.close();
    t_sum -= 1;
    //cout << "һ����" << t_sum << "��ʱ��" << endl;

    while (getline(file, lineStr) && file.good())//good()���ļ�û�з����κδ���ʱ����true
    {
        rows++;
        //cout <<"������"<< rows_demand << endl;
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        cols = rowStr.size();
        //cout <<"������"<< cols_demand << endl;
        if (rows > 1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                t = rowStr[0];
                c[i].bd_demand_t = atoi(rowStr[i + 1].c_str());
            }
            t_counter++;
            //cout << t << endl;
            for (long unsigned int i = 0; i < e.size(); i++)
            {
                e[i].bd_left = e[i].bd_max;
                e[i].bd_sum_t = 0;
            }
            getSolutionAtMinute_t_scale(c, e, q, fileSolution, qos_max, t_counter, t_sum);
            //getSolutionAtMinute_t(c, e, q, fileSolution,qos_max,t_counter,t_sum);
            cout << endl;
            for (long unsigned int i = 0; i < e.size(); i++)
            {
                if (e[i].bd_sum_t != 0)
                {
                    e[i].counter++;
                }
            }
        }
        /*for (int i = 0; i < cols_demand - 1; i++)
        {
            cout << c[i].id << " " << c[i].bd_demand_t << endl;
        }*/
    }
    file.close();
    return 1;
}

bool GreaterSort(Qos a, Qos b) { return (a.timeDelay < b.timeDelay); }


void getSolutionAtMinute_t_scale(vector<CN>& c, vector<EN>& e, vector<vector<Qos>>& q, string filename, int qos_max, int t_counter, int t_sum)
{
    int M = c.size();
    int N = e.size();
    ofstream outFile;
    outFile.open(filename, ios::app);
    //if (outFile.fail())
    //{
    //    cout << "�ļ���ʧ�ܣ�" << endl;
    //}

    for (int i = 0; i < M; i++)
    {
        if (c[i].bd_demand_t > 0)
        {
            //cout << c[i].id << ":";
            outFile << c[i].id << ":";
            int bandwidth_sum = 0, c_demand_t = c[i].bd_demand_t, demand_t = 0;
            double scale = 0;
            for (int j = 0; j < N; j++)//�����������ÿͻ��ڵ�i���з���ı�Ե�ڵ��ʣ������
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    bandwidth_sum += q[i][j].edge->bd_left;
                }
            }
            //cout << "�ͻ��ڵ�" << i << "����������֮����ı�Ե�ڵ��ʣ������Ϊ��" << bandwidth_sum << endl;
            for (int j = 0; j < N; j++)//���ͻ��ڵ�i�������������з��������ı�Ե�ڵ�
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    scale = (double)q[i][j].edge->bd_left / (double)bandwidth_sum;//��Ե�ڵ�j��ʣ�����/���д����=�������
                    if (q[i][j + 1].timeDelay < qos_max)
                    {
                        demand_t = int(c_demand_t * scale);//��Ե�ڵ�j������ͻ��ڵ�i�Ĵ���=i������*����
                    }
                    else
                    {
                        demand_t = c[i].bd_demand_t;
                    }
                    //cout << "��Ե�ڵ�" << j << "�ķ������Ϊ:" << scale << ",������ͻ��ڵ�" << i << "�Ĵ���Ϊ��" << demand_t << endl;
                    if (demand_t > 0)
                    {
                        q[i][j].edge->bd_left -= demand_t;
                        q[i][j].edge->bd_sum_t += demand_t;
                        c[i].bd_demand_t -= demand_t;
                        //cout << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        if (c[i].bd_demand_t > 0)
                        {
                            //cout << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                            outFile << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        }
                        else
                        {
                            if (t_counter == t_sum && i == M - 1)
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">";
                                //cout << "<" << q[i][j].edge->id << "," << demand_t << ">";
                            }
                            else
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                                //cout << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                            }
                        }
                    }
                    else
                        continue;
                }
            }
            //cout << "��" << i << "���ͻ��ڵ�Ŀǰʣ������" << c[i].bd_demand_t << endl;
        }
        else
        {
            //cout << c[i].id << ":" << endl;
            outFile << c[i].id << ":" << endl;
        }

    }
    outFile.close();
}


int main()
{
    //clock_t startTime, endTime;
    //startTime = clock();//��ʱ��ʼ

    string fileDemand = "/data/demand.csv";
    string fileSiteBD = "/data/site_bandwidth.csv";
    string fileQos = "/data/qos.csv";
    string fileConfig = "/data/config.ini";
    string fileSolution = "/output/solution.txt";

    vector<CN> c;
    vector<EN> e;
    vector<vector<Qos>> q;

    int qos_max = Read_Config(fileConfig);//��ȡini�ļ�����ȡqos����
    //cout << qos_max << endl;

    init_Client(c, fileDemand);//��ʼ���ͻ��ڵ�
    //cout << "�ͻ��ڵ���Ŀ��" << M << endl;
    //for (int i = 0; i < M; i++)
    //{
    //    cout << i << "," << c[i].id << " ";
    //}
    //cout << endl;

    init_Edege(e, fileSiteBD);//��ʼ����Ե�ڵ�
    //cout << "��Ե�ڵ���Ŀ��" << N << endl;
    //for (int i = 0; i < e.size(); i++)
    //{
    //    cout << i << " " << e[i].id << "," << e[i].bd_max << e[i].bd_left << e[i].bd_sum_t << endl;
    //}

     init_Q(q, c, e);//��qos��ʼ��Ϊһ��M��N�еĶ�ά���飬�����������ļ�

    Read_Qos(q, fileQos, c, e);//��ȡqos�ļ�
    //if (error == 1)
    //    cout << "qos����ȡ�ɹ���" << endl;
    //else cout << "qos����ȡʧ�ܣ�" << endl;
    //cout << "q��������" << q.size() << endl;
    //cout << "q��������" << q[0].size() << endl;
    //for (int i = 0; i < q.size(); i++)
    //{
    //    for (int j = 0; j < q[0].size(); j++)
    //    {
    //        cout << q[i][j].timeDelay << " ";
    //    }
    //    cout << endl;
    //}
    for (long unsigned int i = 0; i < q.size(); i++)//��ÿ���ͻ��ڵ��Ӧ�������б�Ե�ڵ��qos������������
    {
        sort(q[i].begin(), q[i].end(), GreaterSort);
    }
    //for (int i = 0; i < q.size(); i++)
    //{
    //    for (int j = 0; j < q[0].size(); j++)
    //    {
    //        cout <<q[i][j].edge->id<<","<< q[i][j].timeDelay << "   ";
    //    }
    //    cout << endl;
    //}
    c_e_count(c, e, q, qos_max);

    //��д��֮ǰ���
    ofstream file_writer(fileSolution, ios_base::out);
    file_writer.close();

    Read_Client(c, fileDemand, e, q, fileSolution, qos_max);//��ȡdemand.csv����������

    // for(int i=0;i<e.size();i++)
    // {
    //     //cout<<"��"<<i<<"����Ե�ڵ�"<<e[i].id<<"��ʣ�����Ϊ��"<<e[i].bd_left<<endl;
    //     cout<<"��"<<i<<"����Ե�ڵ�"<<e[i].id<<"���õ�ʱ����ĿΪ��"<<e[i].counter<<endl;
    // }

    //endTime = clock();//��ʱ����
    //cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
}
