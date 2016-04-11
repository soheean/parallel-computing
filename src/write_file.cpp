//
// Created by victi on 16-2-27.
//

#include <iostream>
#include <fstream>
#include <string>

#include "forcingMaker.h"
#include "matrix.hpp"
#include "Time.hpp"

string make_filename(string out_path, double x, double y, int prec){
    char prefix_format[32];
    sprintf(prefix_format,"%%.%df_%%.%df",prec,prec);

    char prefix[32];
    sprintf(prefix,prefix_format,y,x);
    string file_name = out_path + prefix;   // 生成带坐标的文件名
    return  file_name;
}

void write_file(const Matrix<double>& result, string file_name, const Matrix<int> &out_param){
    if(out_param.get_x() == 1)
        _write_file_asc(result, file_name);
    else
        _write_file_bin(result, file_name, out_param);
}

void _write_file_asc(const Matrix<double> &result, string file_name){
    ofstream fout;
    fout.open(file_name.c_str());
    if(!fout.is_open()){
        cout<<"  Error: File "<<file_name<<" can't open to write.\n";
        exit(1);
    }
    fout.setf(ios::fixed);
    fout.precision(2);

    int days = result.get_x();
    int types = result.get_y();

    for(int d = 1; d <= days; d++){
        for(int ty = 1; ty <= types; ty++){
            fout<<result.get(d,ty)<<" ";
        }
        fout<<"\n";
    }
    fout.close();
}

void _write_file_bin(const Matrix<double> &result, string file_name, const Matrix<int> &bin_param){
    ofstream fout;
    fout.open(file_name.c_str(),ios::binary);
    if(!fout.is_open()){
        cout<<"  Error: File "<<file_name<<" can't open to write.\n";
        exit(1);
    }

    int days = result.get_x();
    int types = result.get_y();

    for(int d = 1; d <= days; d++){
        for(int ty = 1; ty <= types; ty++){
            int is_signed = bin_param.get(1, ty);
            int factor = bin_param.get(2,ty);
            if(is_signed){
                double trans_value = result.get(d,ty) * factor +0.5;
                short out_value = trans_value;
                fout.write((char*)&out_value,sizeof(short));
            }else{
                double trans_value = result.get(d,ty) * factor + 0.5; // 为了转换时四舍五入而非直接取整数。
                unsigned short out_value = trans_value;
                fout.write((char*)&out_value,sizeof(unsigned short));
            }
        }
    }
    fout.close();

}