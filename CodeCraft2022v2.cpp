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

typedef struct clientNode//客户节点
{
    string id;
    //string t;
    int bd_demand_t;//t时刻的带宽需求
    int e_counter;//能够进行分配的边缘节点数目

}CN;
typedef struct edgeNode//边缘节点
{
    string id;
    int bd_max;//带宽上限
    int bd_sum_t;//t时刻接收到的带宽需求和
    int bd_left;//当前剩余的带宽
    int counter;
}EN;
typedef struct Qos//服务质量
{
    //string CN_id;
    //string EN_id;
    CN *client;
    EN *edge;
    int timeDelay;//时延（当时延小于时延上限Q时才能够进行流量分配）
}Qos;

void getSolutionAtMinute_t_scale(vector<CN>& c, vector<EN>& e, vector<vector<Qos>>& q, string filename,int qos_max,int t_counter,int t_sum);
void getSolutionAtMinute_t(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> qos, string filename, int qos_max,int t_sum,int t_counter);
void getSolutionAtMinute_t_average(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> &q,string filename,int qos_max,int t_counter,int t_sum);

int init_Client(vector<CN> &c, string filename)//初始化客户节点(只读取demand.csv文件中的客户节点id，并将所有节点的demand_t设为0
{
    ifstream file(filename);
    //file.open(fileDemand.c_str(),ios::in);

    if (file.fail())
        cout << "文件打开失败！" << endl;
    else
    {
        cout << "文件打开成功！" << endl;
    }

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
    //cout <<"列数："<< cols << endl;
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

int init_Edege(vector<EN> &e, string filename)//初始化边缘节点
{
    ifstream file(filename);
    if (file.fail())
        cout << "文件打开失败！" << endl;
    else
    {
        cout << "文件打开成功！" << endl;
    }
    int rows = 0;
    string lineStr, str;
    while (getline(file, lineStr) && file.good())//good()：文件没有发生任何错误时返回true
    {
        rows++;
        //cout <<"行数："<< rows_demand << endl;
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        //cout <<"列数："<< cols_demand << endl;
        if (rows > 1)
        {
            EN e_new;
            e_new.id = rowStr[0];
            e_new.bd_max = atoi(rowStr[1].c_str());
            e_new.bd_left = e_new.bd_max;
            e_new.bd_sum_t = 0;
            e_new.counter=0;
            e.push_back(e_new);
        }
    }
    file.close();
    return e.size();
}

int init_Q(vector<vector<Qos>>& q, vector<CN>& c, vector<EN>& e)
{
    for (int i=0; i < c.size(); i++)
    {
        Qos q_new;
        vector<Qos> q_line_new;
        for (int j = 0; j < e.size(); j++)
        {
            q_new.client = &c[0];
            q_new.edge = &e[0];
            q_new.timeDelay = 0;
            q_line_new.push_back(q_new);
        }
        q.push_back(q_line_new);
    }
    cout << "q行数：" << q.size() << ",q列数:" << q[0].size() << endl;
    return 1;
}

int Read_Qos(vector<vector<Qos>> &q, string filename,vector<CN> &c,vector<EN> &e)
{
    ifstream file(filename);
    if (file.fail())
        cout << "文件打开失败！" << endl;
    else
    {
        cout << "文件打开成功！" << endl;
    }

    int rows = 0, cols = 0;
    string lineStr, str;

    while (getline(file, lineStr) && file.good())//good()：文件没有发生任何错误时返回true
    {
        rows++;
        cols = 0;
        //cout <<"该行字符串："<< lineStr << endl;
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
            for (int i = 0; i < rowStr.size() - 1; i++)
            {
                if (rowStr[i + 1] != c[i].id)
                {
                    cout << "qos表格与demand表格客户节点顺序不一致" << endl;
                    cout << c[0].id << endl;
                    cout << i << " " << rowStr[i + 1] << "," << c[i].id << endl;
                    return 0;
                }
            }
        }
        else
        {
            if (rowStr[0] != e[rows - 2].id)
            {
                cout << "qos表格与site_bandwidth表格边缘节点顺序不一致" << endl;
                cout << rows << " " << rowStr[0] << "," << e[rows - 2].id << endl;
                return 0;
            }
            for (int i = 0; i < cols - 1; i++)
            {
                q[i][rows-2].client = &c[i];
                q[i][rows-2].edge = &e[rows-2];
                q[i][rows - 2].timeDelay = atoi(rowStr[i+1].c_str());
                //cout << q[i][rows - 2].timeDelay << endl;
            }
        }
    }
    file.close();
    return 1;
}

void c_e_count(vector<CN>& c, vector<EN>& e, vector<vector<Qos>>& q,int qos_max)//计算每个客户节点i能够跟多少个边缘节点进行分配
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
    int rows = 0,qos_max=0;
    while (getline(file, lineStr))
    {
        rows++;
        //cout << "这是第" << rows << "行" << endl;
        //cout << lineStr << endl;
        if (rows == 2)
        {
            //cout << lineStr << endl;
            int loc=lineStr.find("=");
            int end = lineStr.size();
            lineStr = lineStr.substr(loc + 1, end - 1);
            //cout << lineStr << endl;
            qos_max=atoi(lineStr.c_str());
        }
    }
    file.close();
    return qos_max;
}

int Read_Client(vector<CN>& c, string filename,vector<EN> &e,vector<vector<Qos>> &q,string fileSolution,int qos_max)//读取demand.csv，并为每时刻进行分配
{
    ifstream file(filename);
    //file.open(fileDemand.c_str(),ios::in);

    if (file.fail())
        cout << "文件打开失败！" << endl;
    else
    {
        cout << "文件打开成功！" << endl;
    }

    int rows = 0, cols = 0;
    string lineStr, str,t;
    int t_sum = 0, cols_solution = 0,t_counter=0;

    ifstream file_t(filename);
    while (getline(file_t, lineStr) && file_t.good())//good()：文件没有发生任何错误时返回true
    {
        t_sum++;
    }
    file_t.close();
    t_sum -= 1;
    cout << "一共有" << t_sum << "个时刻" << endl;

    while (getline(file, lineStr) && file.good())//good()：文件没有发生任何错误时返回true
    {
        rows++;
        //cout <<"行数："<< rows_demand << endl;
        stringstream ss(lineStr);
        vector<string> rowStr;
        while (getline(ss, str, ','))
        {
            rowStr.push_back(str);
        }
        cols = rowStr.size();
        //cout <<"列数："<< cols_demand << endl;
        if(rows>1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                t = rowStr[0];
                c[i].bd_demand_t = atoi(rowStr[i + 1].c_str());
            }
            t_counter++;
            cout << t << endl;
            for (int i = 0; i < e.size(); i++)
            {
                e[i].bd_left = e[i].bd_max;
                e[i].bd_sum_t = 0;
            }
            getSolutionAtMinute_t_scale(c, e, q, fileSolution,qos_max,t_counter,t_sum);
            //getSolutionAtMinute_t_average(c,e,q,fileSolution,qos_max,t_counter,t_sum);
            //getSolutionAtMinute_t(c, e, q, fileSolution,qos_max,t_counter,t_sum);
            cout << endl;
            for(int i=0;i<e.size();i++)
            {
                if(e[i].bd_sum_t!=0)
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

void getSolutionAtMinute_t(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> q, string filename,int qos_max,int t_counter,int t_sum)
{
    int M = c.size();
    int N = e.size();
    int i_get = 0, j_get = 0;
    ofstream outFile;
    outFile.open(filename, ios::app);
    if (outFile.fail())
    {
        cout << "文件打开失败！" << endl;
    }
    int demand_t = 0;
    for (int i = 0; i < M; i++)
    {
        if (c[i].bd_demand_t > 0)
        {
            //cout << c[i].id << ":";
            outFile << c[i].id << ":";
            for (int j = 0; j < N; j++)
            {
                if (c[i].bd_demand_t>0 && q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left > 0)
                {
                    if (q[i][j].edge->bd_left >= c[i].bd_demand_t)
                    {
                        demand_t = c[i].bd_demand_t;
                        c[i].bd_demand_t -= demand_t;
                        q[i][j].edge->bd_left -= demand_t;
                        q[i][j].edge->bd_sum_t += demand_t;
                        //cout << "<" << e[j].id << "," << demand_t << ">" << endl;
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
                    else
                    {
                        demand_t = q[i][j].edge->bd_left;
                        c[i].bd_demand_t -= demand_t;
                        q[i][j].edge->bd_left -= demand_t;
                        q[i][j].edge->bd_sum_t+=demand_t;
                        //cout<< "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        outFile << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                    }
                }
                if (c[i].bd_demand_t == 0)
                {
                    break;
                }
            }
        }
        else
        {
            //cout << c[i].id << ":" << endl;
            outFile << c[i].id << ":" << endl;
        }
    }
    outFile.close();
}

bool GreaterSort(Qos a, Qos b) { return (a.timeDelay < b.timeDelay); }


void getSolutionAtMinute_t_scale(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> &q,string filename,int qos_max,int t_counter,int t_sum)
{
    int M = c.size();
    int N = e.size();
    ofstream outFile;
    outFile.open(filename, ios::app);
    if (outFile.fail())
    {
        cout << "文件打开失败！" << endl;
    }

    int demand = 0;
    for (int i = 0; i < M; i++)
    {
        if (c[i].bd_demand_t > 0)
        {
            cout << c[i].id << ":";
            outFile << c[i].id << ":";
            int bandwidth_sum = 0, c_demand_t = c[i].bd_demand_t, demand_t = 0;
            double scale = 0;
            for (int j = 0; j < N; j++)//算出所有能与该客户节点i进行分配的边缘节点的剩余带宽和
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    bandwidth_sum += q[i][j].edge->bd_left;
                }
            }
            //cout << "客户节点" << i << "的所有能与之分配的边缘节点的剩余带宽和为：" << bandwidth_sum << endl;
            for (int j = 0; j < N; j++)//将客户节点i的需求分配给所有符合条件的边缘节点
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    scale = (double)q[i][j].edge->bd_left / (double)bandwidth_sum;//边缘节点j的剩余带宽/所有带宽和=分配比例
                    if(q[i][j+1].timeDelay<qos_max)
                    {
                        demand_t = int(c_demand_t * scale);//边缘节点j分配给客户节点i的带宽=i的需求*比例
                    }
                    else
                    {
                        demand_t=c[i].bd_demand_t;
                    }
                    //cout << "边缘节点" << j << "的分配比例为:" << scale << ",分配给客户节点" << i << "的带宽为：" << demand_t << endl;
                    if (demand_t > 0)
                    {
                        q[i][j].edge->bd_left -= demand_t;
                        q[i][j].edge->bd_sum_t += demand_t;
                        c[i].bd_demand_t -= demand_t;
                        //cout << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        if(c[i].bd_demand_t>0)
                        {
                            cout<< "<" << q[i][j].edge->id << "," << demand_t << ">,";
                            outFile << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        }
                        else 
                        {
                            if (t_counter == t_sum && i == M - 1)
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">";
                                cout << "<" << q[i][j].edge->id << "," << demand_t << ">";
                            }
                            else
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                                cout << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                            }
                        }
                    }
                    else
                        continue;
                }
            }
            cout <<"第"<<i<<"个客户节点目前剩余需求："<< c[i].bd_demand_t << endl;
        }
        else
        {
            cout << c[i].id << ":" << endl;
            outFile << c[i].id << ":" << endl;
        }
        
    }
    outFile.close();
}

void getSolutionAtMinute_t_average(vector<CN>& c, vector<EN>& e, vector<vector<Qos>> &q,string filename,int qos_max,int t_counter,int t_sum)
{
    int M = c.size();
    int N = e.size();
    ofstream outFile;
    outFile.open(filename, ios::app);
    if (outFile.fail())
    {
        cout << "文件打开失败！" << endl;
    }

    int demand = 0;
    for (int i = 0; i < M; i++)
    {
        if (c[i].bd_demand_t > 0)
        {
            cout << c[i].id << ":";
            outFile << c[i].id << ":";
            int e_able_counter = 0, c_demand_t = c[i].bd_demand_t, demand_t = 0;
            double scale = 0;
            for (int j = 0; j < N; j++)//算出所有能与该客户节点i进行分配的边缘节点的个数
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    e_able_counter ++;
                }
            }
            //cout << "客户节点" << i << "的所有能与之分配的边缘节点的个数为：" << e_able_counter << endl;
            for (int j = 0; j < N; j++)//将客户节点i的需求分配给所有符合条件的边缘节点
            {
                if (q[i][j].timeDelay < qos_max && q[i][j].edge->bd_left>0)
                {
                    scale = (double)1 / (double)e_able_counter;//1/所有能与之分配的边缘节点个数=分配比例
                    if(q[i][j+1].timeDelay<qos_max)
                    {
                        demand_t = int(c_demand_t * scale);//边缘节点j分配给客户节点i的带宽=i的需求*比例
                    }
                    else
                    {
                        demand_t=c[i].bd_demand_t;
                    }
                    //cout << "边缘节点" << j << "的分配比例为:" << scale << ",分配给客户节点" << i << "的带宽为：" << demand_t << endl;
                    if (demand_t > 0)
                    {
                        q[i][j].edge->bd_left -= demand_t;
                        q[i][j].edge->bd_sum_t += demand_t;
                        c[i].bd_demand_t -= demand_t;
                        //cout << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        if(c[i].bd_demand_t>0)
                        {
                            cout<< "<" << q[i][j].edge->id << "," << demand_t << ">,";
                            outFile << "<" << q[i][j].edge->id << "," << demand_t << ">,";
                        }
                        else 
                        {
                            if (t_counter == t_sum && i == M - 1)
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">";
                                cout << "<" << q[i][j].edge->id << "," << demand_t << ">";
                            }
                            else
                            {
                                outFile << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                                cout << "<" << q[i][j].edge->id << "," << demand_t << ">" << endl;
                            }
                        }
                    }
                    else
                        continue;
                }
            }
            cout <<"第"<<i<<"个客户节点目前剩余需求："<< c[i].bd_demand_t << endl;
        }
        else
        {
            cout << c[i].id << ":" << endl;
            outFile << c[i].id << ":" << endl;
        }
        
    }
    outFile.close();
}

int main()
{
    clock_t startTime, endTime;
    startTime = clock();//计时开始

    string fileDemand = "./data/demand.csv";
    string fileSiteBD = "./data/site_bandwidth.csv";
    string fileQos = "./data/qos.csv";
    string fileConfig = "./data/config.ini";
    string fileSolution = "./output/solution.txt";

    vector<CN> c;
    vector<EN> e;
    vector<vector<Qos>> q;

    int qos_max=Read_Config(fileConfig);//读取ini文件，获取qos上限
    //cout << qos_max << endl;

    int M=init_Client(c, fileDemand);//初始化客户节点
    cout << "客户节点数目：" << M << endl;
    for (int i = 0; i < M; i++)
    {
        cout << i << "," << c[i].id << " ";
    }
    cout << endl;

    int N = init_Edege(e, fileSiteBD);//初始化边缘节点
    cout << "边缘节点数目：" << N << endl;
    //for (int i = 0; i < e.size(); i++)
    //{
    //    cout << i << " " << e[i].id << "," << e[i].bd_max << e[i].bd_left << e[i].bd_sum_t << endl;
    //}

    int error=init_Q(q, c, e);//将qos初始化为一个M行N列的二维数组，但并不读入文件
    if (error == 1)
        cout << "qos初始化成功！" << endl;
    else
        cout << "qos初始化失败！" << endl;

    error=Read_Qos(q, fileQos, c, e);//读取qos文件
    if (error == 1)
        cout << "qos表格读取成功！" << endl;
    else cout << "qos表格读取失败！" << endl;
    //cout << "q的行数：" << q.size() << endl;
    //cout << "q的列数：" << q[0].size() << endl;
    //for (int i = 0; i < q.size(); i++)
    //{
    //    for (int j = 0; j < q[0].size(); j++)
    //    {
    //        cout << q[i][j].timeDelay << " ";
    //    }
    //    cout << endl;
    //}
    for (int i = 0; i < q.size(); i++)//对每个客户节点对应的与所有边缘节点的qos进行升序排序
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

    //在写入之前清空
    ofstream file_writer(fileSolution, ios_base::out);
    file_writer.close();

    Read_Client(c, fileDemand, e, q, fileSolution,qos_max);//读取demand.csv并分配需求

    // for(int i=0;i<e.size();i++)
    // {
    //     //cout<<"第"<<i<<"个边缘节点"<<e[i].id<<"的剩余带宽为："<<e[i].bd_left<<endl;
    //     cout<<"第"<<i<<"个边缘节点"<<e[i].id<<"所用的时刻数目为："<<e[i].counter<<endl;
    // }

    endTime = clock();//计时结束
    cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
}
