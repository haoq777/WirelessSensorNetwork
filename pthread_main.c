#include "data_global.h"
#include "uart_cache.h"

extern unsigned char dev_led_mask;
extern unsigned char dev_camera_mask;
extern unsigned char dev_buzzer_mask;
extern unsigned char dev_uart_mask;

extern pthread_cond_t cond_camera;
extern pthread_cond_t cond_refresh;
extern pthread_cond_t cond_uart_cmd;
extern pthread_cond_t cond_sqlite;

extern pthread_mutex_t mutex_global;
extern pthread_mutex_t mutex_uart_cmd;
extern pthread_mutex_t mutex_camera;
extern pthread_mutex_t mutex_slinklist;

extern int msgid;
extern struct env_info_array all_info_RT; 

extern uart_cache_list zigbee_cache_head, zigbee_cache_tail;

void *pthread_main (void *arg)
{
	key_t key;
	ssize_t msgsize;
	struct msg msgbuf;
	
	//创建消息队列
	if ((key = ftok ("/app", 'g')) < 0)
	{
		perror ("ftok msgqueue");
		exit (-1);
	}
	if ((msgid = msgget (key, IPC_CREAT | IPC_EXCL | 0666)) < 0)
	{
		if(errno == EEXIST)
		{
			msgid = msgget (key,0666);
			return 0;
		} 
		else
		{
			perror ("msgget msgid");
			exit (-1);
		}
	}

	//zigbee链表初始化
	zigbee_cache_head = CreateEmptyCacheList ();
	zigbee_cache_tail = zigbee_cache_head;
	unsigned char *zigbee_temp;

	printf ("pthread_main is ok\n");
	while (1)
	{
		bzero (&msgbuf, sizeof(msgbuf));//清空其他进程通信的消息队列
		printf ("\nwait for the msg\n");
		msgsize = msgrcv (msgid, &msgbuf, sizeof (msgbuf) - sizeof (long), 1L, 0);//1L优化为宏定义

		//msgtype用来选择是发给camera还是zigbee
		printf ("  Get %ldL msg [%ld]\n", msgbuf.msgtype,msgsize);
		printf ("  text[0] = %#x\n", msgbuf.text[0]);

		switch (msgbuf.msgtype)
		{
			case MSG_CAMERA:
			{//发送给camera控制线程
				pthread_mutex_lock (&mutex_camera);
				dev_camera_mask = msgbuf.text[0]; //拍照张数
				pthread_cond_signal (&cond_camera);
				pthread_mutex_unlock (&mutex_camera);
				break;
			}
			case MSG_ZIGBEE:
			{//zigbee发送命令
				//usleep (200000);
				pthread_mutex_lock (&mutex_uart_cmd);	
				//避免多个消息覆盖
				zigbee_temp = (unsigned char *)malloc (sizeof (unsigned char));
				*zigbee_temp = msgbuf.text[0];
				printf("  msgbuf.text = %x\n",msgbuf.text[0]);
				InsertCacheNode (&zigbee_cache_tail, zigbee_temp);//加入到链表
				
				break;//dev_uart_mask = msgbuf.text[0];
				pthread_mutex_unlock (&mutex_uart_cmd);
				pthread_cond_signal (&cond_uart_cmd);
			}
			default : 
				break;
		}
	}

}
