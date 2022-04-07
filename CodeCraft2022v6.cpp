#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

#define MAX_M 135
#define MAX_N 35
#define MAX_T 8928

//客户节点
typedef struct clientNode
{
    //string id;
    //string t;
    int bd_demand_t;//t时刻的带宽需求
    int bd_demand_t_left;//t时刻的带宽需求剩余
    //int e_counter;//能够进行分配的边缘节点数目
    //unordered_map<string, int> qos;

}CN;

//边缘节点
typedef struct edgeNode
{
    //string id;
    int bd_max;//带宽上限
    int bd_sum_t;//t时刻接收到的带宽需求和
    int bd_left;//当前剩余的带宽
    int counter;//记录边缘节点时间序列中不为0的个数
    vector<int> timeSequence;//记录边缘节点的时间序列
    unordered_map<string, int> qos;
    unordered_map<string, int> demand_t;//记录在t时刻分配给客户节点的带宽
}EN;

//服务质量
typedef struct Qos
{
    //string CN_id;
    //string EN_id;
    CN* client;
    EN* edge;
    int timeDelay;//时延（当时延小于时延上限Q时才能够进行流量分配）
    int demand_t;//记录在t时刻所有客户节点与边缘节点对应分配的带宽
}Qos;

unordered_map<string, CN> c;
unordered_map<string, EN> e;
vector<vector<Qos>> q;
string t;
int t_counter=0;//记录当前是第几个时刻
int t_sum=0;//记录总共有多少个时刻
int qos_max=0;//qos上限
int M = 0, N = 0;

int getSolution();
void writeSolution(string fileSolution);

//初始化客户节点，并返回客户节点数目
int initClient(string fileDemand)
{
    ifstream file(fileDemand);
    //if (file.fail())
    //    cout << "文件打开失败！" << endl;
    //else
    //{
    //    cout << "文件打开成功！" << endl;
    //}

    int cols = 0;
    string lineStr;//记录读取的每行字符串
    string str;
    getline(file, lineStr);//读取一行字符串，即客户节点id那一行
    lineStr.erase(lineStr.find_last_not_of('\r') + 1, string::npos);
    stringstream ss(lineStr);
    vector<string> rowStr;
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
        //c_new.qos.clear();
        c[rowStr[i + 1]] = c_new;
    }
    file.close();
    return c.size();
}

//初始化边缘节点，返回边缘节点数目
int ReadSite(string fileSite)
{
    ifstream file(fileSite);
    //if (file.fail())
    //    cout << "文件打开失败！" << endl;
    //else
    //{
    //    cout << "文件打开成功！" << endl;
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
            e_new.demand_t.clear();
            e[rowStr[0]] = e_new;
        }
    }
    file.close();
    return e.size();
}

vector<string> cid;

//读取qos.csv,将之存储到边缘节点中
int ReadQos(string fileQos)
{
    ifstream file(fileQos);
    //if (file.fail())
    //    cout << "文件打开失败！" << endl;
    //else
    //{
    //    cout << "文件打开成功！" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;

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
        unordered_map<string, EN>::iterator get;
        if (rows > 1)
        {
            for (int i = 0; i < cols - 1; i++)
            {
                get = e.find(rowStr[0]);
                get->second.qos[cid[i + 1]] = atoi(rowStr[i + 1].c_str());
            }
        }
    }
    file.close();
    return 1;
}

//读取Config.ini，并返回qos上限qos_max
int ReadConfig(string fileConfig)
{
    ifstream file(fileConfig);

    string lineStr;
    int rows = 0;
    while (getline(file, lineStr))
    {
        rows++;
        //cout << "这是第" << rows << "行" << endl;
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

int ReadDemand(string fileDemand,string fileSolution)
{
    ifstream file(fileDemand);
    //if (file.fail())
    //    cout << "文件打开失败！" << endl;
    //else
    //{
    //    cout << "文件打开成功！" << endl;
    //}

    int rows = 0, cols = 0;
    string lineStr, str;
    
    ifstream file_t(fileDemand);
    while (getline(file_t, lineStr) && file_t.good())//good()：文件没有发生任何错误时返回true
    {
        t_sum++;
    }
    file_t.close();
    t_sum -= 1;
    //cout << "一共有" << t_sum << "个时刻" << endl;

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
            getSolution();
            writeSolution(fileSolution);
            for (auto eit = e.begin(); eit != e.end(); eit++)
            {
                eit->second.timeSequence.push_back(eit->second.bd_sum_t);
                if (eit->second.bd_sum_t > 0)
                    eit->second.counter++;

                eit->second.bd_left = eit->second.bd_max;
                eit->second.bd_sum_t = 0;
                eit->second.demand_t.clear();
            }
        }
    }
    file.close();
    return t_sum;
}



int getSolution()
{
    int demand_t = 0;
    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        if (cit->second.bd_demand_t_left > 0)
        {
            for (auto eit = e.begin(); eit != e.end(); eit++)
            {
                if (cit->second.bd_demand_t_left > 0)
                {
                    //cout << eit->second.qos[cit->first] << " " << eit->second.bd_left << endl;
                    if (eit->second.qos[cit->first] < qos_max && eit->second.bd_left > 0)
                    {
                        if (eit->second.bd_left >= cit->second.bd_demand_t_left)
                        {
                            demand_t = cit->second.bd_demand_t_left;
                        }
                        else
                        {
                            demand_t = eit->second.bd_left;
                        }
                        cit->second.bd_demand_t_left -= demand_t;
                        eit->second.bd_left -= demand_t;
                        eit->second.bd_sum_t += demand_t;
                        eit->second.demand_t[cit->first] += demand_t;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    return 1;
}

void writeSolution(string fileSolution)
{
    ofstream outfile;
    outfile.open(fileSolution,ios::app);

    int no0_counter = 0, no0_sum = 0;

    for (auto cit = c.begin(); cit != c.end(); cit++)
    {
        for (auto eit = e.begin(); eit != e.end(); eit++)
        {
            if (eit->second.demand_t[cit->first] > 0)
            {
                no0_sum++;
            }
        }

        if (cit->second.bd_demand_t == 0)
        {
            outfile << cit->first << ":" << endl;
        }
        else
        {
            outfile << cit->first << ":";
            for (auto eit = e.begin(); eit != e.end(); eit++)
            {
                if (eit->second.demand_t[cit->first] > 0)
                {
                    no0_counter++;
                    if (no0_counter < no0_sum)
                    {
                        outfile << "<" << eit->first << "," << eit->second.demand_t[cit->first] << ">,";
                        //cout << "<" << eit->first << "," << eit->second.demand_t[cit->first] << ">,";
                    }
                    else
                    {
                        if (t_counter == t_sum && no0_counter == no0_sum && cit == c.end())
                        {
                            outfile << "<" << eit->first << "," << eit->second.demand_t[cit->first] << ">";
                        }
                        else
                        {
                            outfile << "<" << eit->first << "," << eit->second.demand_t[cit->first] << ">" << endl;

                        }
                    }
                }
            }
        }
    }
    outfile.close();
}

int main()
{
    clock_t startTime, endTime;
    startTime = clock();

    string fileDemand = "./data/demand.csv";
    string fileSiteBD = "./data/site_bandwidth.csv";
    string fileQos = "./data/qos.csv";
    string fileConfig = "./data/config.ini";
    string fileSolution = "./output/solution.txt";

    qos_max = ReadConfig(fileConfig);

    M = initClient(fileDemand);
    //cout << "客户节点数目：" << M << endl;
    //for (auto cit = c.begin(); cit != c.end(); cit++)
    //{
    //    cout << "客户节点id：" << cit->first << ",带宽需求：" << cit->second.bd_demand_t << ",剩余需求：" << cit->second.demand_t_left << endl;
    //}

    N = ReadSite(fileSiteBD);
    //cout << "边缘节点数目:" << N << endl;
    //for (auto eit = e.begin(); eit != e.end(); eit++)
    //{
    //    cout << "边缘节点id:" << eit->first << ",带宽上限:" << eit->second.bd_max << ",带宽剩余：" << eit->second.bd_left << ",已用带宽和：" << eit->second.bd_sum_t << endl;
    //}

    ReadQos(fileQos);
    //for (auto cit = c.begin(); cit != c.end(); cit++)
    //{
    //    cout << cit->first << " ";
    //    for (auto eit = e.begin(); eit != e.end(); eit++)
    //    {
    //        cout << eit->first << "," << eit->second.qos[cit->first] <<endl;
    //    }
    //    cout << endl;
    //}

    //写入之前清空
    ofstream file_writer(fileSolution, ios_base::out);
    file_writer.close();

    ReadDemand(fileDemand,fileSolution);

    //endTime = clock();
    //cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
}