#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define getpch(type) (type*)malloc(sizeof(type))
#define MAX_CHANNELS 10
#define MAX_JOBS 100

int n;                // 作业数量
int m = 2;            // 通道数量，默认2个
int times = 0;        // 系统的当前时间
int max_needtime = 0; // 最大需要运行时间，用于优先级计算

// 作业控制块结构体
struct jcb
{
    char name[10];         // 作业名称
    int reachtime;         // 作业到达时间
    int starttime;         // 作业开始时间
    int needtime;          // 作业需要运行的时间
    int finishtime;        // 作业完成时间
    float cycletime;       // 作业周转时间
    float cltime;          // 带权周转时间
    int priority;          // 作业优先级（数值越大优先级越高）
    char state;            // 作业状态 ('W'等待, 'R'运行, 'F'完成)
    struct jcb *next;      // 下一个作业控制块
} *ready = NULL, *p, *q;

typedef struct jcb JCB;

// 通道结构体
typedef struct {
    JCB *current_job;      // 当前正在处理的作业
    int finish_time;       // 当前作业的完成时间
} Channel;

// 全局数组用于存储通道信息
Channel channels[MAX_CHANNELS];      // 最多支持MAX_CHANNELS个通道

// 存储完成的作业，用于最终统计
JCB *completed_jobs_array[MAX_JOBS];
int completed_count = 0;

// 初始化作业队列
void inital()
{
    int i;
    printf("\n输入作业数: ");
    scanf("%d", &n);
    if(n > MAX_JOBS) {
        printf("作业数量超过最大限制 (%d)。\n", MAX_JOBS);
        exit(1);
    }
    for (i = 0; i < n; i++)
    {
        p = getpch(JCB);
        printf("\n输入作业名: ");
        scanf("%s", p->name);
        printf("输入作业到达时间: ");
        scanf("%d", &p->reachtime);  // 每个作业的到达时间
        printf("输入作业需要运行的时间: ");
        scanf("%d", &p->needtime);
        p->state = 'W';  // 初始状态为等待
        p->next = NULL;

        // 更新最大需要运行时间
        if(p->needtime > max_needtime)
            max_needtime = p->needtime;
        
        // 插入到就绪队列的尾部
        if (ready == NULL)
            ready = q = p;
        else
        {
            q->next = p;
            q = p;
        }
    }
}

// 计算作业优先级的方法
void calculate_priority()
{
    JCB *temp = ready;
    while(temp != NULL)
    {
        // 优先级计算方法：优先级 = 10 - (需要运行时间 / 最大需要运行时间) * 10
        // 结果范围大约在1到10之间，运行时间越短优先级越高
        temp->priority = 10 - ((float)(temp->needtime) / max_needtime * 10);
        if(temp->priority < 1)
            temp->priority = 1; // 确保最低优先级为1
        temp = temp->next;
    }
}

// 按优先级排序就绪队列（优先级高的在前）
void sort_ready_queue()
{
    if(ready == NULL || ready->next == NULL)
        return; // 无需排序
    
    JCB *sorted = NULL;
    JCB *current = ready;
    while(current != NULL)
    {
        JCB *next = current->next;
        // 插入到sorted链表中
        if(sorted == NULL || current->priority > sorted->priority)
        {
            current->next = sorted;
            sorted = current;
        }
        else
        {
            JCB *temp = sorted;
            while(temp->next != NULL && temp->next->priority >= current->priority)
                temp = temp->next;
            current->next = temp->next;
            temp->next = current;
        }
        current = next;
    }
    ready = sorted;
}

// 显示作业运行后的结果
void disp(JCB *q)
{
    printf("\n作业%s运行完成，运行情况：\n", q->name);
    printf("开始运行时刻：%d\n", q->starttime);
    printf("完成时刻：%d\n", q->finishtime);
    printf("周转时间：%.2f\n", q->cycletime);
    printf("带权周转时间：%.2f\n", q->cltime);
    printf("优先级：%d\n", q->priority);
}

// 运行作业
void running(JCB *p, int channel_id)
{
    p->state = 'R';  // 更新状态为运行
    p->starttime = times;  // 记录开始时间
    p->finishtime = p->starttime + p->needtime;  // 完成时间 = 开始时间 + 需要的时间
    p->cycletime = (float)(p->finishtime - p->reachtime);  // 周转时间
    p->cltime = (float)(p->cycletime / p->needtime);  // 带权周转时间
    times = p->finishtime;  // 系统时间推进到作业完成时间
    disp(p);  // 显示运行状态
    p->state = 'F';  // 作业完成
    printf("\n%s 已在通道 %d 完成!\n", p->name, channel_id + 1);
    completed_jobs_array[completed_count++] = p;  // 存储完成的作业
}

// 打印作业调度的最终结果
void final_result()
{
    float totalCycleTime = 0, totalWeightCycleTime = 0;
    for(int i = 0; i < completed_count; i++) {
        totalCycleTime += completed_jobs_array[i]->cycletime;
        totalWeightCycleTime += completed_jobs_array[i]->cltime;
    }

    printf("\n\n所有作业已完成!\n");
    printf("\n%d个作业的平均周转时间是：%.2f", n, totalCycleTime / n);
    printf("\n%d个作业的平均带权周转时间是：%.2f\n", n, totalWeightCycleTime / n);
}

// FCFS多通道调度算法
void priority_multi_channel()
{
    inital();  // 初始化作业队列
    calculate_priority();  // 计算每个作业的优先级
    sort_ready_queue();  // 按优先级排序就绪队列

    // 初始化通道
    for(int i = 0; i < m; i++) {
        channels[i].current_job = NULL;
        channels[i].finish_time = 0;
    }

    while (ready != NULL || 1) {
        // 分配作业到空闲通道
        for(int i = 0; i < m; i++) {
            if(channels[i].current_job == NULL) { // 通道空闲
                // 查找就绪队列中优先级最高且到达时间 <= 当前时间的作业
                JCB *prev = NULL, *current = ready;
                while(current != NULL && current->reachtime > times)
                    current = current->next;
                
                if(current != NULL) {
                    // 分配作业到通道i
                    channels[i].current_job = current;
                    channels[i].finish_time = times + current->needtime;
                    current->state = 'R';
                    current->starttime = times;
                    printf("\n时间 %d: 作业 %s 分配到通道 %d\n", times, current->name, i+1);
                    
                    // 从就绪队列中移除
                    if(prev == NULL) {
                        ready = current->next;
                    }
                    else {
                        prev->next = current->next;
                    }
                    current->next = NULL;
                }
            }
        }

        // 查找下一个完成的作业
        int next_event_time = -1;
        int next_channel = -1;
        for(int i = 0; i < m; i++) {
            if (channels[i].current_job != NULL) {
                if (next_event_time == -1 || channels[i].finish_time < next_event_time) {
                    next_event_time = channels[i].finish_time;
                    next_channel = i;
                }
            }
        }

        if (next_event_time == -1) {
            // 所有通道为空，找下一个到达的作业
            if (ready != NULL) {
                times = ready->reachtime;
            } else {
                break;  // 所有作业已完成
            }
        } else {
            // 系统时间推进到下一个事件时间
            times = next_event_time;
            // 完成作业
            JCB *finished_job = channels[next_channel].current_job;
            channels[next_channel].current_job = NULL;
            channels[next_channel].finish_time = 0;
            finished_job->finishtime = times;
            finished_job->cycletime = (float)(finished_job->finishtime - finished_job->reachtime);
            finished_job->cltime = (float)(finished_job->cycletime / finished_job->needtime);
            finished_job->state = 'F';
            printf("\n时间 %d: 作业 %s 在通道 %d 完成\n", times, finished_job->name, next_channel + 1);
            disp(finished_job);
            completed_jobs_array[completed_count++] = finished_job;

            // 重新排序就绪队列，以考虑新到达的作业
            sort_ready_queue();
        }

        // 检查是否所有作业已完成
        if (ready == NULL) {
            int all_channels_free = 1;
            for(int i = 0; i < m; i++) {
                if (channels[i].current_job != NULL) {
                    all_channels_free = 0;
                    break;
                }
            }
            if (all_channels_free) {
                break;  // 所有作业已完成
            }
        }
    }

    // 输出最终结果
    final_result();
}

int main()
{
    printf("\n\n\t\t*********************************************\t\t\n");
    printf("\t\t\t\t作业调度演示 - 优先级多通道调度\n");
    printf("\t\t*********************************************\t\t\n");

    printf("请输入通道数量（最大%d，默认2）: ", MAX_CHANNELS);
    scanf("%d", &m);
    if(m <= 0 || m > MAX_CHANNELS) {
        printf("通道数量必须在1到%d之间。使用默认值2。\n", MAX_CHANNELS);
        m = 2;
    }

    priority_multi_channel();  // 执行优先级多通道调度算法
    return 0;
}
