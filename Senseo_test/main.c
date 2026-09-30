#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

// 模拟传感器数据结构
typedef struct {
    double value;
    uint64_t timestamp_ms;
} SensorData;

// 线程共享上下文
typedef struct {
    SensorData data;
    bool has_new_data;           // 标志位：是否有新数据生成
    pthread_mutex_t mutex;       // 互斥锁：保护共享数据区
    pthread_cond_t cond;         // 条件变量：用于唤醒消费者
} SharedContext;

// 全局运行状态标志（volatile 保证线程/信号可见性）
static volatile sig_atomic_t g_running = 1;
static SharedContext g_ctx;

// 信号处理函数：捕获 Ctrl+C
static void sig_handler(int sig)
{
    (void)sig;
    g_running = 0;
    // 退出时广播唤醒可能阻塞在条件变量上的线程，避免死锁挂起
    pthread_cond_broadcast(&g_ctx.cond);
}

// 获取毫秒级单调时间戳（不受系统时间被网络对时修改的影响）
static uint64_t get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// ==================== 生产者：采集线程 ====================
static void *producer_thread(void *arg)
{
    (void)arg;
    struct timespec next_period;
    clock_gettime(CLOCK_MONOTONIC, &next_period);

    const long PERIOD_NS = 20 * 1000 * 1000; // 50Hz (周期 20ms)

    printf("[Producer] 采集线程已启动 (50Hz 定时采样)...\n");

    while (g_running) {
        // 1. 严格周期计算：在基准时间上累加周期，消除执行时间带来的时钟累积漂移
        next_period.tv_nsec += PERIOD_NS;
        if (next_period.tv_nsec >= 1000000000L) {
            next_period.tv_sec += 1;
            next_period.tv_nsec -= 1000000000L;
        }

        // 2. 绝对时间休眠：释放 CPU 调度权给操作系统，直到指定时间片到达
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_period, NULL);

        if (!g_running) break;

        // 3. 模拟采集硬件数据（如读取 IMU 加速度计）
        SensorData local_data;
        local_data.value = (rand() % 1000) / 10.0; // 模拟浮点读数
        local_data.timestamp_ms = get_time_ms();

        //加锁更新临界区数据，并发送通知
        pthread_mutex_lock(&g_ctx.mutex);
        g_ctx.data = local_data;
        g_ctx.has_new_data = true;


        pthread_cond_signal(&g_ctx.cond); // 唤醒消费者线程
        pthread_mutex_unlock(&g_ctx.mutex);
    }

        printf("[Producer] 采集线程退出...\n");
    return NULL;
}

/*消费者线程*/
static void *consumer_thread(void *arg)
{
    (void)arg;
    printf("[Consumer] 消费者线程已启动...\n");

    while (g_running) {
        pthread_mutex_lock(&g_ctx.mutex);
        // 等待新数据到来
        while (!g_ctx.has_new_data && g_running) {
            pthread_cond_wait(&g_ctx.cond, &g_ctx.mutex);
        }

        if (!g_running) {
            pthread_mutex_unlock(&g_ctx.mutex);
            break;
        }

        // 处理新数据
        SensorData current = g_ctx.data;
        g_ctx.has_new_data = false; // 重置标志位
        pthread_mutex_unlock(&g_ctx.mutex);

        // 模拟数据处理（如打印或存储）
        printf("[Process] T: %lu ms | 收到传感器数值: %6.2f\n",
               current.timestamp_ms, current.value);
    }

    printf("[Consumer] 消费者线程退出...\n");
    return NULL;
}

int main(void)
{
    struct sigaction sa = {.sa_handler = sig_handler};
    sigaction(SIGINT, &sa, NULL); // 捕获 Ctrl+C 信号
 
    /*初始化互斥锁和条件变量*/
    pthread_mutex_init(&g_ctx.mutex, NULL);
    pthread_cond_init(&g_ctx.cond, NULL);
    g_ctx.has_new_data = false;

    pthread_t producer_tid, consumer_tid;
    if(pthread_create(&producer_tid, NULL, producer_thread, NULL) != 0 || pthread_create(&consumer_tid, NULL, consumer_thread, NULL) != 0) {
        perror("创建生产者线程失败");
        exit(EXIT_FAILURE);
    }

    printf("[Main] 按 Ctrl+C 退出程序...\n");

    /*等待子线程结束*/
    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    /*销毁同步资源*/
    pthread_mutex_destroy(&g_ctx.mutex);
    pthread_cond_destroy(&g_ctx.cond);
    printf("[Main] 程序已退出.\n");
    return 0;
}