#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <cstdlib>
#define getpch(type) (type*)malloc(sizeof(type))
//#define NULL 0

struct pcb { /* 定义进程控制块PCB */
    char name[10];     /* 进程名称 */
    char state;        /* 进程状态 */
    int super;         /* 优先级 */
    int ntime;         /* 运行时间 */
    int rtime;         /* 已运行时间 */
    struct pcb* link;  /* 链接指针 */
} *ready = NULL, *p;

typedef struct pcb PCB;

/* 对进程进行优先级排序 */
void sort() {
    PCB *first, *second;
    int insert = 0;
    if ((ready == NULL) || ((p->super) > (ready->super))) /* 如果就绪队列为空或当前进程优先级高于队首 */
    {
        p->link = ready;
        ready = p;
    } else /* 比较优先级，将进程插入到合适位置 */
    {
        first = ready;
        second = first->link;
        while (second != NULL) {
            if ((p->super) > (second->super)) /* 若当前进程优先级高，插入到前面 */
            {
                p->link = second;
                first->link = p;
                second = NULL;
                insert = 1;
            } else /* 若当前进程优先级低，则继续后移 */
            {
                first = second;
                second = second->link;
            }
        }
        if (insert == 0) first->link = p; /* 如果优先级最低，插入到队尾 */
    }
}

/* 建立进程控制块 */
void input() {
    int i, num;
    system("cls");
    //clrscr(); /* 清屏 */
    printf("\n 请输入进程个数: ");
    scanf("%d", &num);
    for (i = 0; i < num; i++) {
        printf("\n 进程号 No.%d:\n", i + 1);
        p = getpch(PCB);
        printf(" 输入进程名: ");
        scanf("%s", p->name);
        printf(" 输入进程优先数: ");
        scanf("%d", &p->super);
        printf(" 输入进程运行时间: ");
        scanf("%d", &p->ntime);
        printf("\n");
        p->rtime = 0;
        p->state = 'w';
        p->link = NULL;
        sort(); /* 调用sort函数，将进程插入就绪队列 */
    }
}

/* 获取就绪队列的长度 */
int space() {
    int l = 0;
    PCB* pr = ready;
    while (pr != NULL) {
        l++;
        pr = pr->link;
    }
    return l;
}

/* 显示当前进程信息 */
void disp(PCB* pr) {
    printf("\n | name \t | state \t | super \t | ntime \t | rtime |\n");
    printf(" | %s\t", pr->name);
    printf(" | %c\t", pr->state);
    printf(" | %d\t", pr->super);
    printf(" | %d\t", pr->ntime);
    printf(" | %d\t", pr->rtime);
    printf("\n");
}

/* 查看当前运行状态 */
void check() {
    PCB* pr;
    printf("\n **** 当前正在运行的进程是: %s", p->name);
    disp(p); /* 显示当前运行进程 */
    pr = ready;
    printf("\n **** 当前就绪队列状态为:\n");
    while (pr != NULL) {
        disp(pr); /* 遍历显示就绪队列 */
        pr = pr->link;
    }
}

/* 撤销进程 (进程运行结束) */
void destroy() {
    printf("\n 进程 [%s] 已完成.\n", p->name);
    free(p);
}

/* 就绪进程继续运行 */
void running() {
    (p->rtime)++;
    if (p->rtime == p->ntime)
        destroy(); /* 如果进程完成，调用destroy函数 */
    else {
        (p->super)--; /* 运行一次后优先级降低 */
        p->state = 'w'; /* 设置为就绪状态 */
        sort(); /* 重新排序 */
    }
}

int main() {
    int len, h = 0;
    char ch;
    input(); /* 创建进程 */
    len = space(); /* 获取进程数 */
    while ((len != 0) && (ready != NULL)) {
        h++;
        printf("\n 第 %d 次执行: \n", h);
        p = ready; /* 取出就绪队列首进程 */
        ready = p->link; /* 移动队列指针 */
        p->link = NULL;
        p->state = 'R'; /* 设置为运行状态 */
        check(); /* 显示当前状态 */
        running(); /* 执行进程 */
        printf("\n 按任一键继续...");
        ch = getchar();
    }
    printf("\n\n 所有进程已完成.\n");
    ch = getchar();
    return 0;
}
