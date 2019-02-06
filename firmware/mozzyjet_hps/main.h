#ifndef MOZZYJET_H_
#define MOZZYJET_H_

#define STATE_PIN         BIT3

#define STATE_RUN_5       1
#define STATE_RUN_10      2
#define STATE_RUN_20      3

#define TASK_NONE         0
#define TASK_SPRAY        1
#define TASK_OPEN         2
#define TASK_CLOSE        3

// 35735.0 * cweight:
#define ALPHA_5           178675.0
// 62162.0 * cweight:
#define ALPHA_10          621620.0
#define ALPHA_20          1243240.0

#define COMM_COMMAND      0
#define COMM_TIME_L       1
#define COMM_TIME_R       2
#define COMM_VOL0_L       3
#define COMM_VOL0_R       4
#define COMM_SPV_L        5
#define COMM_SPV_R        6
#define COMM_LAM_L        7
#define COMM_LAM_R        8
#define COMM_PWM0_L       9
#define COMM_PWM0_R       10
#define COMM_PWM1_L       11
#define COMM_PWM1_R       12

#endif
