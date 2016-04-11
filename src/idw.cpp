//
// Created by victi on 16-2-27.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#include "forcingMaker.h"
#include "matrix.hpp"
#include "Time.hpp"

using namespace std;

/**
 * IDW插值
 *
 */
void _idw(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
          double grid_x, double grid_y, int col, const Matrix<double> &param_list,
          Matrix<double>* results) {

    /** 获取参数 **/
    double idp = param_list.get(col, 2);
    double max_distan = param_list.get(col,3);
    int max_station_num = (int)param_list.get(col,4);

    /** 获取数据边界 **/
    int stn_num = stn_data.get_y();
    int days = stn_data.get_x();

    /** 计算各站点离该点的距离平方的倒数 **/
    double ori_power[stn_num];
    double max_dis_sq = max_distan * max_distan;

    for (int i = 0; i < stn_num; i++) {
        double dx = grid_x - stn_x[i];
        double dy = grid_y - stn_y[i];
        double dis_sq = dx * dx + dy * dy;
        if(max_distan < 0 || dis_sq < max_dis_sq)
            ori_power[i] = 1.0 / dis_sq;
        else ori_power[i] = 0.0;
    }

    /** 若idp不为2则计算得该idp的值 **/
    if (idp > 0 && idp != 2.0) {
        double ind = idp / 2.0;
        for (int i = 0; i < stn_num; i++) {
            double tmp = ori_power[i];
            ori_power[i] = pow(tmp,ind);
        }
    }

    /** 按初步权重进行插入排序 **/
    int sort_sn[stn_num];
    for(int i = 0; i < stn_num; i++)sort_sn[i] = i;

    for (int j = 1; j < stn_num; j++)
    {
        int key = sort_sn[j];
        int i = j-1;
        while (i >= 0 && ori_power[sort_sn[i]] < ori_power[key]){
            sort_sn[i+1] = sort_sn[i];
            i--;
        }
        sort_sn[i+1] = key;
    }

    if(max_station_num <= 0 || max_station_num > stn_num) max_station_num = stn_num;

    /** 插值 **/
    for(int d = 1; d <= days; d++) {
        double value = 0.0;
        double base = 0.0;
        int sk;
        if(DEB)sk = SKRAN;else sk = 1;
        int mn = max_station_num;

        for(int stn = 0; stn < mn; stn++){
            double ori_p = ori_power[sort_sn[stn]];
            if(ori_p <= 0.0) break;
            int sn = sort_sn[stn] + sk;
            if(sn > stn_num)sn = stn_num;

            double v = stn_data.get(d, sn, col);
            if(v > -65536.0){
                value += v * ori_p;     // 按反距离权计算各值
                base += ori_p;          // 反距离权要除的基数
            }
            else if(mn < stn_num) mn ++;// 遇到缺失值站点则放弃此站点，再多算一个站点
        }

        double itpv;
        if(base > 0)
            itpv = value/base;
        else itpv = 0.0;
        results->set(d, col, itpv);
    }
}

/**
 * 泰森多边形插值
 *
 */
void _vor(const vector<double> &stn_x, const vector<double> &stn_y,const Matrix<double>& stn_data,
          double grid_x, double grid_y, int col, const Matrix<double> &param_list,
          Matrix<double>* results){

    int stn_num = stn_data.get_y();
    int days = stn_data.get_x();

    double dis_sq[stn_num];
    for (int i = 0; i < stn_num; i++) {
        double dx = grid_x - stn_x[i];
        double dy = grid_y - stn_y[i];
        dis_sq[i] = dx * dx + dy * dy;
    }

    /** 找出离插值点最近的站点 **/
    int nearest[stn_num];
    for(int i = 0; i < stn_num; i++)nearest[i] = i;
    for(int j=1; j < stn_num; j++){
        int key = nearest[j];
        int i = j-1;
        while (i>=0 && dis_sq[nearest[i]] > dis_sq[key]){
            nearest[i + 1] = nearest[i];
            i--;
        }
        nearest[i + 1] = key;
    }

    for(int d = 1; d <= days; d++) {
        double nearest_v = stn_data.get(d, nearest[0]+1, col);

        /** 若最近的站点无数据则找第二近的站点的数据，以此类推 **/
        for(int i = 1; nearest_v < -65536.0 && i < stn_num; i++){
            nearest_v = stn_data.get(d, nearest[i]+1, col);
        }
        results->set(d, col, nearest_v);
    }
}