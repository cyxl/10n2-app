#ifndef MOT_IMU_TF_H
#define TMOT_IMU_TF_H

#define REST_LABEL 0
#define FORWARD_LABEL 1
#define BACKWARD_LABEL 2
#define LEFT_LABEL 3
#define RIGHT_LABEL 4
#define UP_LABEL 5
#define DOWN_LABEL 6
#define LEFTSIDE_LABEL 7
#define RIGHTSIDE_LABEL 8
#define UNCERTAIN_LABEL 9

#include <cwchar>

#define NUM_CLASSES 10
    void init_mot_imu(void);
    void mot_imu_task(void *pvParameters);
    int get_max_from_confs(int last_n, float conf);
    int get_latest_inf(int n_last,float conf);

#endif