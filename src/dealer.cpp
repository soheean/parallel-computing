//
// Created by victi on 16-2-27.
//
/**
 * 2016-03-07 0.11
 * 支持二进制输出，支持指定日期或只指定开始行号以及计算行数目，
 * 支持IDW插值的一些参数输入。
 *
 * 2016-03-16 0.12
 * 修复了点bug，支持泰森多边形插值，
 * 支持不同类型数据设置不同插值参数
 * 增加了多线程插值功能
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>

#include <vector>

#include "forcingMaker.h"
#include "matrix.hpp"
#include "Time.hpp"

using namespace std;

int main(int argc, char* argv[]){

    if(argc == 1){
        cout << "Usage: fm <input file path>\n";
        exit(0);
    }

    ifstream fin;
    fin.open(argv[1]);
    if(!fin.is_open()){
        cout << "Input file not found.\n";
        exit(1);
    }

    // debug用

//    ifstream fin;
//    fin.open("/home/victi/PMS/input.fm");

    stringstream sst;

    int thread_num = 1;
    if(argc > 2){
        sst << argv[2];
        sst >> thread_num;
        if(thread_num < 1)thread_num = 1;
    }

    time_t t_start, t_stop;
    time(&t_start);

    string line;

    string in_path;     // 输入路径
    string stn_file;    // 站点坐标文件
    string grids_file;  // 栅格坐标文件
    string out_path;    // 输出路径
    int stn_num;        // 站点个数
    int grid_num;       // 网格个数
    int data_type_num;       // 数据种数
    int prec;           // 小数位数
    int out_format;     // 输出文件格式

    int itp_type;    // 插值类型


    Time data_begin_date,begin_date, end_date;  // 数据起始时间，插值起止日期
    int skip_days, itp_days;

    vector<double>* grid_x = new vector<double>(0);
    vector<double>* grid_y = new vector<double>(0);     // 网格坐标

    vector<double>* stn_x  = new vector<double>(0);
    vector<double>* stn_y  = new vector<double>(0);    // 站点坐标

    Matrix<double>* stn_data = NULL;                           // 站点数据

    Matrix<int>*out_param_list = NULL;
    Matrix<double>*itp_param_list = NULL;            // 参数列表

    /** 读取输入文件参数 **/

    getline(fin,line);
    getline(fin,in_path);       // 输入文件路径

    getline(fin,line);
    getline(fin,stn_file);      // 站点坐标文件

    getline(fin,line);
    getline(fin,grids_file);    // 网格坐标文件

    getline(fin,line);
    getline(fin,line);
    sst.clear();
    sst.str("");
    sst << line;
    sst >> data_type_num;            // 数据种数

    string data_names[data_type_num];

    getline(fin,line);
    sst.clear();
    sst.str("");
    sst << line;
    for(int i = 0; i < data_type_num; i++)
        sst >> data_names[i];

    getline(fin,line);
    getline(fin,out_path);  // 输出路径

    getline(fin,line);
    getline(fin,line);
    sst.clear();
    sst.str("");
    sst << line;
    sst >> prec;            // 小数位数

    getline(fin,line);
    getline(fin,line);
    if(line == "BINARY")
        out_format = BIN;
    else
        out_format = ASC;   // 输出文件格式

    if(out_format == BIN){  // 如果要输出二进制还要这些内容: 有无符号短整型以及乘上倍数后输出
        out_param_list = new Matrix<int>(2, data_type_num);
        for(int ty = 1; ty <= data_type_num; ty++){
            getline(fin,line);
            sst.clear();
            sst.str("");
            sst << line;
            int v1,v2;
            sst >> v1;
            sst >> v2;
            out_param_list->set(1, ty, v1);
            out_param_list->set(2, ty, v2);
        }
    }
    else{
        out_param_list = new Matrix<int>(1, 1);
    }

    /** 输出时间范围 **/
    getline(fin,line);
    getline(fin,line);
    if(data_begin_date.set_time(line) == 3){
        getline(fin,line);
        begin_date.set_time(line);
        getline(fin,line);
        end_date.set_time(line);    // 数据输出的起止时间
        cout<< "  Period of interpolation "<<begin_date<<" , "<<end_date<<endl;
        skip_days = begin_date - data_begin_date;
        itp_days = end_date - begin_date + 1;   // 计算天数
    }
    else{
        sst.clear();
        sst.str(line);
        sst >> skip_days;
        skip_days --;

        getline(fin,line);
        sst.clear();
        sst.str(line);
        sst >> itp_days;
    }                           // 读取数据的起始编号

    /** 插值类型及其相关参数 **/
    do {
        getline(fin, line);
    }while(line[0] == '#');
    itp_param_list = new Matrix<double>(data_type_num, 10);
    itp_param_list->set_all(-1);
    for(int ty = 1; ty <= data_type_num; ty++){
        sst.clear();
        sst.str("");
        sst << line;
        sst << -1;
        string tmp;
        sst >> tmp;

        if(tmp == "IDW")itp_param_list->set(ty, 1, _IDW);
        else if(tmp == "AIDW")itp_param_list->set(ty, 1, _AIDW);
        else if(tmp == "KRIGE")itp_param_list->set(ty, 1, _KRIGE);
        else itp_param_list->set(ty, 1, _VOR);
        double v;
        for(int i = 1; i <= 9; i++){
            sst >> v;
            itp_param_list->set(ty, i + 1, v);
        }

        getline(fin,line);
    }

    fin.close();

    stn_num = read_stn(stn_x,stn_y,in_path + stn_file);         // 读取站点坐标
    grid_num = read_grid(grid_x,grid_y,in_path + grids_file);   // 读取网格坐标

    stn_data = new Matrix<double>(itp_days, stn_num, data_type_num);    // 建立站点数据矩阵
    read_stn_data(stn_data, skip_days, data_names, in_path);      // 读入站点数据

    /** 插值 **/
    if(thread_num > 1)
        itp_multi_thread(*stn_x, *stn_y, *grid_x, *grid_y, *stn_data, out_path, prec, *itp_param_list, *out_param_list, thread_num);
    else
        itp_single_thread(*stn_x, *stn_y, *grid_x, *grid_y, *stn_data, out_path, prec, *itp_param_list, *out_param_list);

    time(&t_stop);

    cout<<"done.  ("<<t_stop-t_start<<"s costs.)\n";

    delete grid_x;
    delete grid_y;
    delete stn_x;
    delete stn_y;

    delete stn_data;
    if(out_param_list != NULL)
        delete out_param_list;

    return 0;
}
