#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#define SIZE 10

sem_t producer_sem;
sem_t consumer_sem;
static int pro=SIZE;//一开始生产者可以生产
static int con=0;//一开始没有产品可供消费
static int p=0;//p，c，d变量均是为了打印定义的
static int c=0;
static int d=0;
static int table[SIZE];//用数组表示一个环形队列

static unsigned counter=0;
static int serv_sock=0;
static unsigned char data[2000] = {0};  

void* producer_fun(void* val)
{
    int i=1;
    int index=0;
    while(i)
    {
        sem_wait(&producer_sem);//申请一个可生产的空间，即申请成功表示邻界资源可写，否则表示邻界资源已满不可写入。

        table[p]=i;
        pro--;
        con++;
        printf("-----------------------------------------\n");
        printf("which producer:%u\n",(unsigned int)pthread_self());//哪个生产者生产的
        printf("producer#  producer success,total:%d\n",i++);//一共生产了多少个
        printf("profuct#  you have %d blank source!\n",pro);//生产者可用的生产空间
        printf("producer#  you have %d data source!\n",con);//消费可供消费的产品的数量
        for(index=0;index<SIZE;index++)//输出当前生产的所有产品
        {
            if(table[index]!=0)
                printf("array[%d]:%d ",index,table[index]);
        }
        printf("\n");
        printf("-----------------------------------------\n");
        printf("\n");

        sem_post(&consumer_sem);//给消费者可供消费的产品数量加一，即将邻界资源中的可使用的数据加一
        p=(p+1)%SIZE;//下标调整
        sleep(1);
    }
    return NULL;
}

void* consumer_fun(void* val)
{
    int i=1;
    int temp=0;
    while(i)
    {
        sem_wait(&consumer_sem);// 申请一个消费者可消费的产品，即此时邻界资源中有可使用的数据。

        if(c!=0)
            d=c-1;
        temp=table[c];
        table[c]=0;
        pro++;
        con--;
        printf("##########################################\n");
        printf("which consumer:%u\n",(unsigned int)pthread_self());//哪个消费者消费
        printf("consume: %d\n",temp);//消费的数据为temp
        printf("consumer#  you have %d data source!\n",con);//可供消费的数据量
        printf("consumer#  you have %d blank source!\n",pro);//可供生产的空间
        printf("##########################################\n");
        printf("\n");

        sem_post(&producer_sem);//给邻界资源可写的资源加一，表示消费一个数据，拿走数据后，空出一个位置可写
        c=(c+1)%SIZE;
    }
    return NULL;
}

void destroy()
{
    sem_destroy(&producer_sem);
    sem_destroy(&consumer_sem);
    exit(0);
}

void initsem()
{
    signal(2,destroy);//信号捕捉函数，自定义2号信号量为销毁两个信号量
    int i=0;
    for(i=0;i<SIZE;++i)
        table[i]=0;

    sem_init(&producer_sem,0,SIZE);
    sem_init(&consumer_sem,0,0);
}

int init_socket(char* server_ip, int udp_port)
{
    int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    //创建sockaddr_in结构体变量
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);  //具体的IP地址
    serv_addr.sin_port = htons(udp_port);  //端口
    //将套接字和IP、端口绑定
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return serv_sock;
}

unsigned get_sock_buffer_len(int servSock)
{
    unsigned optVal;
    socklen_t optLen = sizeof(socklen_t);
    getsockopt(servSock, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, &optLen);
    printf("Buffer length: %d\n", optVal);
    return optVal;
}

void * udp_receiver(void* val)
{
    int n =0;
    while(1){
        n = read(serv_sock, data, 2000);
        if( n > 0){
            counter++;
        }
    }
}

int main()
{
    unsigned socket_buffer_len = 0;
/* 
    initsem();
    pthread_t producer1,producer2,consumer1,consumer2;
    pthread_create(&producer1,NULL,producer_fun,NULL);
    pthread_create(&producer2,NULL,producer_fun,NULL);
    pthread_create(&consumer1,NULL,consumer_fun,NULL);
    pthread_create(&consumer2,NULL,consumer_fun,NULL);
    pthread_join(producer1,NULL);
    pthread_join(producer2,NULL);
    pthread_join(consumer1,NULL);
    pthread_join(consumer2,NULL);
*/
    pthread_t p_udp_receiver;

    int ret = 0;
    int n = 0;
    serv_sock = init_socket("100.0.0.2", 1000);
    socket_buffer_len = get_sock_buffer_len(serv_sock);
    printf("The socket buffer length is %u \n", socket_buffer_len);

    pthread_create(&p_udp_receiver,NULL,udp_receiver,NULL);


/*
    struct sockaddr_in gg;
    socklen_t len=0;  
    ret = recvfrom(serv_sock, data, 1500, 0, (struct sockaddr *)&gg, &len);
    if(ret < 0) {
        perror("recvfrom");
        return 1;        
    }
    printf("send said: %s\n", data);
*/
    while(1)
    {
        printf("Counter is %d\n", counter);
        sleep(5);
    }


    close(serv_sock);
    return 0;
}
