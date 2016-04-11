//
// Created by victi on 16-3-16.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <pthread.h>

#include "forcingMaker.h"
#include "matrix.hpp"
#include "Time.hpp"

using namespace std;

/** 用于给线程回调函数传递参数的参数包 **/
struct arg_package {
    const vector<double> *stn_x;
    const vector<double> *stn_y;
    const vector<double> *grid_x;
    const vector<double> *grid_y;
    const Matrix<double> *stn_data;
    string out_path;
    int prec;
    const Matrix<double> *param_list;
    const Matrix<int> *out_param;

    pthread_mutex_t *mutex_t;
    int *thread_sn;
    int grids_num;
    int *grid;
};

/*
 * 用单线程进行插值并输出到文件
 *
 */
void itp_single_thread(const vector<double> &stn_x, const vector<double> &stn_y,
                       const vector<double> &grid_x, const vector<double> &grid_y,
                       const Matrix<double> &stn_data, string out_path, int prec,  const Matrix<double> &param_list,
                       const Matrix<int> &out_param){

    int grids_num = grid_x.size();
    int type_num = stn_data.get_z();
    int days = stn_data.get_x();
    Matrix<double>* results = new Matrix<double>(days,type_num);

    for(int g = 0; g < grids_num; g++){
        double gx = grid_x[g];
        double gy = grid_y[g];

        string file_name = make_filename(out_path, gx, gy, prec);

        itp_point(stn_x,stn_y,stn_data,gx,gy, param_list, results);

        write_file(*results, file_name, out_param);

        cout<<"  "<< g+1 << " of "<< grids_num << " complete.\n";
    }

    delete results;
}

/**
 * 使用多线程进行插值
 *
 */
void itp_multi_thread(const vector<double> &stn_x, const vector<double> &stn_y,
                      const vector<double> &grid_x, const vector<double> &grid_y,
                      const Matrix<double> &stn_data, string out_path, int prec,  const Matrix<double> &param_list,
                      const Matrix<int> &out_param, int thread_num){

    int grids_num = grid_x.size();
    pthread_mutex_t mutex_t;
    int thread_sn = 0;
    int grid = 0;

    /** 将参数装入参数包 **/
    arg_package *argp = new arg_package;
    argp->grid_x = &grid_x;
    argp->grid_y = &grid_y;
    argp->stn_x = &stn_x;
    argp->stn_y = &stn_y;
    argp->stn_data = &stn_data;

    argp->out_path = out_path;
    argp->prec = prec;

    argp->param_list = &param_list;
    argp->out_param = &out_param;

    argp->mutex_t = &mutex_t;
    argp->thread_sn = &thread_sn;
    argp->grid = &grid;
    argp->grids_num = grids_num;

    /** 创建线程并启动 **/
    pthread_t threads[thread_num];
    pthread_mutex_init(&mutex_t, 0);

    for(int nt = 0; nt < thread_num; nt++)
        pthread_create(&threads[nt], 0, itp_each_thread, argp);

    for(int nt = 0; nt < thread_num; nt++)
        pthread_join(threads[nt], 0);

    pthread_mutex_destroy(&mutex_t);

    delete argp;
}

/** 多线程运算时的线程回调函数 **/
void* itp_each_thread(void* arg){
    arg_package *argp = (arg_package *)arg;

    /** 从参数包中取出参数 **/
    const vector<double> *stn_x = argp->stn_x;
    const vector<double> *stn_y = argp->stn_y;
    const vector<double> *grid_x = argp->grid_x;
    const vector<double> *grid_y = argp->grid_y;
    const Matrix<double> *stn_data = argp->stn_data;
    string out_path = argp->out_path;
    int prec = argp->prec;
    const Matrix<double> *param_list = argp->param_list;
    const Matrix<int> *out_param = argp->out_param;

    pthread_mutex_t *mutex_t = argp->mutex_t;
    int *thread_sn = argp->thread_sn;
    int grids_num = argp->grids_num;
    int *grid = argp->grid;

    /** 互斥的方式获取线程编号 **/
    pthread_mutex_lock(mutex_t);
    int current_sn = *thread_sn;
    *thread_sn = current_sn + 1;
    pthread_mutex_unlock(mutex_t);

    /** 插值和输出 **/
    int type_num = stn_data->get_z();
    int days = stn_data->get_x();
    Matrix<double>* results = new Matrix<double>(days,type_num);

    int current_grid = 0;

    while(true){
        // 互斥的方式领取任务（获取下一个要计算的网格的编号）
        pthread_mutex_lock(mutex_t);
        current_grid = *grid;
        *grid = current_grid + 1;
        pthread_mutex_unlock(mutex_t);

        if(current_grid >= grids_num)   // 任务被领完了就收工
            break;

        double gx = (*grid_x)[current_grid];
        double gy = (*grid_y)[current_grid];

        // 对该点进行插值
        itp_point(*stn_x, *stn_y, *stn_data,gx,gy, *param_list, results);

        // 获取输出文件名
        string file_name = make_filename(out_path, gx, gy, prec);

        // 输出该点数据
        write_file(*results, file_name, *out_param);

        // 控制台输出
        pthread_mutex_lock(mutex_t);
        cout<<"  Thread "<< current_sn << ": "<< current_grid + 1 << " of "<< grids_num << " complete.\n";
        pthread_mutex_unlock(mutex_t);
    }

    delete results;
}

/*
 * 对某个点进行插值
 *
 */
void itp_point(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
               double grid_x, double grid_y, const Matrix<double> &param_list,
               Matrix<double>* results){

    int type_num = stn_data.get_z();
    int days = stn_data.get_x();

    for(int ty = 1; ty <= type_num; ty++){
        int itp_type = param_list.get(ty,1);
        if(itp_type == _IDW)
            _idw(stn_x,stn_y,stn_data,grid_x,grid_y,ty,param_list,results);
        else
            _vor(stn_x,stn_y,stn_data,grid_x,grid_y,ty,param_list,results);

    }
}

