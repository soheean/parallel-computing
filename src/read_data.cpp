//
// Created by victi on 16-2-27.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "forcingMaker.h"
#include "matrix.hpp"
#include "Time.hpp"

using namespace std;

/**
 * 读取站点数据文件
 *
 */
void read_stn_data(Matrix<double>* stn_data, int skip_days, string* data_names, string input_path){
    int data_num = stn_data->get_z();
    int stn_num = stn_data->get_y();
    int itp_days = stn_data->get_x();

    ifstream fin;
    string get;

    for(int dn = 1; dn <= data_num; dn++){

        string file_name = data_names[dn - 1];
        string file_path = input_path + file_name;
        fin.open(file_path.c_str());
        cout<<"  Read file "<<file_path<<endl;
        if(!fin.is_open()){
            cout<<"  Error: Data file "<<file_path<<" not found.\n";
        }

        for(int sk = 1; sk <= skip_days; sk++){     // 跳过不读的行
            getline(fin,get);
        }

        for(int d = 1; d <= itp_days; d++){

//            fin >> get; // 新版本应不要这句

            for(int s = 1; s <= stn_num; s++){
                fin >> get;
                double value;
                if(get == "NA") {
                    value = -131702.0;
                }
                else {
                    value = atof(get.c_str());
                }
                stn_data->set(d, s, dn, value);
            }   // 读取每日各站点数据
            if(fin.eof() && d < itp_days){
                cout<<"  Error: Data is not enough.\n";
                exit(1);
            }

        }   // 读取历时每日的数据
        fin.close();

    }// 读取每个类别的数据
}

/**
 * 读取站点坐标
 *
 */
int read_stn(vector<double>* stn_x, vector<double>* stn_y, string path){
    ifstream fin;
    fin.open(path.c_str());
    if(!fin.is_open()){
        cout<<"  Error: Station coordinate file "<< path <<" not found.\n";
        exit(1);
    }

    double x,y;
    int n = 0;

    while(true){
        fin >> x;
        if(fin.eof())break;
        fin >> y;

        n++;
        stn_x->push_back(x);
        stn_y->push_back(y);
    }
    fin.close();
    return n;
}

/**
 * 读取网格坐标
 *
 */
int read_grid(vector<double>* grid_x,vector<double>* grid_y, string path){
    ifstream fin;
    fin.open(path.c_str());
    if(!fin.is_open()){
        cout<<"  Error: Grids coordinate file " << path <<" not found.\n";
        exit(1);
    }

    double x,y;
    int n = 0;

    while(true){
        fin >> x;
        if(fin.eof())break;
        fin >> y;

        n++;
        grid_x->push_back(x);
        grid_y->push_back(y);
    }
    fin.close();
    return n;
}