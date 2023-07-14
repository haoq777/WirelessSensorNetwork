#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "data_global.h"

//信号量
extern pthread_cond_t cond_zigbee_rcv;
extern pthread_cond_t cond_uart_cmd;
extern pthread_cond_t cond_main_thread;
extern pthread_cond_t cond_camera;
extern pthread_cond_t cond_refresh;
extern pthread_cond_t cond_refresh_updata;

//互斥锁
extern pthread_mutex_t mutex_slinklist;
extern pthread_mutex_t mutex_zigbee_rcv;
extern pthread_mutex_t mutex_uart_cmd;
extern pthread_mutex_t mutex_main_thread;
extern pthread_mutex_t mutex_camera;
extern pthread_mutex_t mutex_refresh;
extern pthread_mutex_t mutex_refresh_updata;
extern pthread_mutex_t mutex_global;
extern pthread_mutex_t mutex_linklist;

extern int dev_camera_fd;
extern int dev_uart_fd; //"ttyUSB0"

extern int msgid; //消息队列
extern int shmid; //共享内存
extern int semid; //信号量

//用于对应共享内存中数据
extern struct env_info_array all_info_RT; 

pthread_t 	id_zigbee_rcv,
			id_uart_cmd,
			id_main_thread,
			id_camera,
			id_refresh;

void ReleaseResource (int signo)
{

	pthread_mutex_destroy (&mutex_linklist);
	pthread_mutex_destroy (&mutex_global);
	pthread_mutex_destroy (&mutex_refresh_updata);
	pthread_mutex_destroy (&mutex_refresh);
	pthread_mutex_destroy (&mutex_camera);
	pthread_mutex_destroy (&mutex_main_thread);
	pthread_mutex_destroy (&mutex_uart_cmd);
	pthread_mutex_destroy (&mutex_zigbee_rcv);
	pthread_mutex_destroy (&mutex_slinklist);

	pthread_cond_destroy (&cond_refresh_updata);
	pthread_cond_destroy (&cond_refresh);
	pthread_cond_destroy (&cond_camera);
	pthread_cond_destroy (&cond_main_thread);
	pthread_cond_destroy (&cond_uart_cmd);

	msgctl (msgid, IPC_RMID, NULL);
	shmctl (shmid, IPC_RMID, NULL);
	semctl (semid, 1, IPC_RMID, NULL);

	pthread_cancel (id_refresh);
	pthread_cancel (id_camera);
	pthread_cancel (id_main_thread);
	pthread_cancel (id_uart_cmd);
	pthread_cancel (id_zigbee_rcv);

	close (dev_camera_fd);
	close (dev_uart_fd);

	printf ("All quit\n");

	exit(0);
}
int main(int argc, char **argv)
{
	pthread_mutex_init (&mutex_slinklist, NULL);
	pthread_mutex_init (&mutex_uart_cmd, NULL);
	pthread_mutex_init (&mutex_main_thread, NULL);
	pthread_mutex_init (&mutex_camera, NULL);
	pthread_mutex_init (&mutex_refresh, NULL);
	pthread_mutex_init (&mutex_refresh_updata, NULL);
	pthread_mutex_init (&mutex_global, NULL);
	pthread_mutex_init (&mutex_linklist, NULL);

	pthread_cond_init (&cond_zigbee_rcv, NULL);
	pthread_cond_init (&cond_uart_cmd, NULL);
	pthread_cond_init (&cond_main_thread, NULL);
	pthread_cond_init (&cond_camera, NULL);
	pthread_cond_init (&cond_refresh, NULL);
	pthread_cond_init (&cond_refresh_updata, NULL);

	signal (SIGINT, ReleaseResource);

	pthread_create (&id_zigbee_rcv, 0, pthread_zigbee_rcv, NULL);
	sleep (1);
	pthread_create (&id_uart_cmd, 0, pthread_uart_send, NULL);
	pthread_create (&id_main_thread, 0, pthread_main, NULL);
	pthread_create (&id_camera, 0, pthread_camera, NULL);
	pthread_create (&id_refresh, 0, pthread_refresh, NULL);

	pthread_join (id_zigbee_rcv, NULL);
	printf ("g1\n");
	pthread_join (id_uart_cmd, NULL);
	printf ("g2\n");
	pthread_join (id_main_thread, NULL);
	printf ("g3\n");
	pthread_join (id_camera, NULL);
	printf ("g4\n");
	pthread_join (id_refresh, NULL);
	printf ("g5\n");

	return 0;
}
