#include "data_global.h"

extern char dev_camera_mask;
extern int dev_camera_fd;

extern pthread_mutex_t mutex_camera;

extern pthread_cond_t cond_camera;

void *pthread_camera (void *arg)
{
	unsigned char picture = 0;
#if 1
	//打开设备
	if ((dev_camera_fd = open (DEV_CAMERA, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		printf ("Cann't open file %s\n",DEV_CAMERA);
	//	exit (-1);
	}
	printf ("pthread_camera is ok\n");
#endif
	while (1)	
	{
		pthread_mutex_lock (&mutex_camera);
		pthread_cond_wait (&cond_camera, &mutex_camera);//主线程释放
		picture = dev_camera_mask;
		pthread_mutex_unlock (&mutex_camera);
#if 1
		for (; picture > 0; picture--)
		{
			//write (dev_camera_fd, "one", 3); //无效
			system("echo one > /tmp/webcom");  //ok system是后面的字符串作为命令执行
			printf("picture = %d\n", picture);
			sleep(4);
		}
#endif
	}
}
