#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* 设置一个整数的圆形缓冲区 */
#define BUFFER_SIZE 16

struct prodcons {
    int buffer[BUFFER_SIZE];       /* 缓冲区数组 */
    pthread_mutex_t lock;          /* 互斥锁 */
    int readpos, writepos;         /* 读写的位置 */
    pthread_cond_t notempty;       /* 缓冲区非空信号 */
    pthread_cond_t notfull;        /* 缓冲区非满信号 */
};

/*--------------------------------------------------------*/
/* 初始化缓冲区 */
void init(struct prodcons *b) {
    if (pthread_mutex_init(&b->lock, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&b->notempty, NULL) != 0) {
        perror("Condition variable (notempty) initialization failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&b->notfull, NULL) != 0) {
        perror("Condition variable (notfull) initialization failed");
        exit(EXIT_FAILURE);
    }
    b->readpos = 0;
    b->writepos = 0;
}

/*--------------------------------------------------------*/
/* 向缓冲区中写入一个整数 */
void put(struct prodcons *b, int data) {
    pthread_mutex_lock(&b->lock);      /* 获取互斥锁 */

    /* 等待缓冲区非满 */
    while ((b->writepos + 1) % BUFFER_SIZE == b->readpos) {
        printf("缓冲区满，生产者等待...\n");
        pthread_cond_wait(&b->notfull, &b->lock);  /* 等待缓冲区非满 */
    }

    /* 写数据并且指针前移 */
    b->buffer[b->writepos] = data;
    b->writepos = (b->writepos + 1) % BUFFER_SIZE;
    printf("生产者写入: %d\n", data);

    /* 设置缓冲区非空信号 */
    pthread_cond_signal(&b->notempty);

    pthread_mutex_unlock(&b->lock);    /* 释放互斥锁 */
}

/*--------------------------------------------------------*/
/* 从缓冲区中读出一个整数 */
int get_item(struct prodcons *b) {
    int data;

    pthread_mutex_lock(&b->lock);      /* 获取互斥锁 */

    /* 等待缓冲区非空 */
    while (b->writepos == b->readpos) {
        printf("缓冲区空，消费者等待...\n");
        pthread_cond_wait(&b->notempty, &b->lock);  /* 等待缓冲区非空 */
    }

    /* 读数据并且指针前移 */
    data = b->buffer[b->readpos];
    b->readpos = (b->readpos + 1) % BUFFER_SIZE;
    printf("消费者读取: %d\n", data);

    /* 设置缓冲区非满信号 */
    pthread_cond_signal(&b->notfull);

    pthread_mutex_unlock(&b->lock);    /* 释放互斥锁 */

    return data;
}

/*--------------------------------------------------------*/
#define OVER (-1)
struct prodcons buffer;

/*--------------------------------------------------------*/
/* 生产者线程函数 */
void *producer(void *data) {
    int n;
    for (n = 0; n < 1000; n++) {
        put(&buffer, n);
    }
    put(&buffer, OVER);  /* 发送结束信号 */
    printf("生产者已停止！\n");
    return NULL;
}

/*--------------------------------------------------------*/
/* 消费者线程函数 */
void *consumer(void *data) {
    int d;
    while (1) {
        d = get_item(&buffer);
        if (d == OVER) break;  /* 接收到结束信号，退出循环 */
        /* 可以在这里处理读取到的数据 */
    }
    printf("消费者已停止！\n");
    return NULL;
}

/*--------------------------------------------------------*/
/* 主函数 */
int main(void) {
    pthread_t th_a, th_b;
    void *retval;

    /* 初始化缓冲区 */
    init(&buffer);

    /* 创建生产者线程 */
    if (pthread_create(&th_a, NULL, producer, NULL) != 0) {
        perror("创建生产者线程失败");
        exit(EXIT_FAILURE);
    }

    /* 创建消费者线程 */
    if (pthread_create(&th_b, NULL, consumer, NULL) != 0) {
        perror("创建消费者线程失败");
        exit(EXIT_FAILURE);
    }

    /* 等待生产者线程结束 */
    if (pthread_join(th_a, &retval) != 0) {
        perror("等待生产者线程失败");
        exit(EXIT_FAILURE);
    }

    /* 等待消费者线程结束 */
    if (pthread_join(th_b, &retval) != 0) {
        perror("等待消费者线程失败");
        exit(EXIT_FAILURE);
    }

    /* 销毁互斥锁和条件变量 */
    pthread_mutex_destroy(&buffer.lock);
    pthread_cond_destroy(&buffer.notempty);
    pthread_cond_destroy(&buffer.notfull);

    return 0;
}
