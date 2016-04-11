//
// Created by victi on 16-2-27.
//

#ifndef FORCINGMAKER_FORCINGMAKER_H
#define FORCINGMAKER_FORCINGMAKER_H

#include <string>
#include <iostream>
#include <vector>

#include "matrix.hpp"
#include "Time.hpp"

#define SK 2

#define ASC 0
#define BIN 1

#define _VOR 0
#define _IDW 1
#define _AIDW 2
#define _KRIGE 3

int read_grid(vector<double>* grid_x,vector<double>* grid_y, string path);
int read_stn(vector<double>* stn_x, vector<double>* stn_y, string path);

void read_stn_data(Matrix<double>* stn_data, int skip_days, string* data_names, string input_path);

void _vor(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
          double grid_x, double grid_y, int col, const Matrix<double> &param_list,
          Matrix<double>* results);

void _idw(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
          double grid_x, double grid_y, int col, const Matrix<double> &param_list,
          Matrix<double>* results);

void itp_multi_thread(const vector<double> &stn_x, const vector<double> &stn_y,
                      const vector<double> &grid_x, const vector<double> &grid_y,
                      const Matrix<double> &stn_data, string out_path, int prec,  const Matrix<double> &param_list,
                      const Matrix<int> &out_param, int thread_num);

void* itp_each_thread(void* arg);

void itp_single_thread(const vector<double> &stn_x, const vector<double> &stn_y,
                       const vector<double> &grid_x, const vector<double> &grid_y,
                       const Matrix<double> &stn_data, string out_path, int prec,  const Matrix<double> &param_list,
                       const Matrix<int> &out_param);

void itp_point(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
               double grid_x, double grid_y, const Matrix<double> &param_list,
               Matrix<double>* results);

string make_filename(string out_path, double x, double y, int prec);

void write_file(const Matrix<double>& result, string file_name, const Matrix<int> &out_param);
void _write_file_asc(const Matrix<double> &result, string file_name);
void _write_file_bin(const Matrix<double> &result, string file_name, const Matrix<int> &bin_param);

#endif //FORCINGMAKER_FORCINGMAKER_H
