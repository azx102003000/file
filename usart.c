/************************************
*
*		树莓派串口编程
*		如果串口缓存中没有可用的数据，则会等待10秒，如果10后还有没，返回-1
*		所以，在读取前，做好通过 serialDataAvail 判断下
*		利用超时接口,线程接口,超时信号
*
**************************************/

#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

char usart_recv[200]={0};
int delete_character(char *src, char delete_c);


void time_out(int value)
{
	
	if(strlen(usart_recv))
	{
		delete_character(usart_recv, '\n');
		printf("usart_recv | %s\n", usart_recv);
		memset(usart_recv, 0, sizeof(usart_recv));
	}
	alarm(1);//重新装置
}



void* thread_func(void *usart_fd)
{
	int fd = *(int*)usart_fd;
	//char buff[200] = {0};	
	time_t loca_time;
	struct tm *ttime;
	int min = 0;
	FILE *flie_fd = NULL;
	char PATH[50] = {0};
	while(1)
	{
		
		//read(0, buff, sizeof(buff);
		//buff[strlen(buff)-1] = '\0';
		//strcat(buff, "\r\n");
		time(&loca_time);
		ttime = localtime(&loca_time);
		if(min != ttime->tm_min)
		{
			write(fd, "all_env\r\n", 12);
			usleep(500);
			sprintf(PATH, "./%d%d%d_env.log",ttime->tm_year + 1900, ttime->tm_mon + 1, ttime->tm_mday);
			
			flie_fd = fopen(PATH, "a+");
			if(flie_fd == NULL)
			{
				perror("open cheak.log fali");
				return NULL;
			}
			fprintf(flie_fd, "%s\n", usart_recv);
			
			fclose(flie_fd);
			flie_fd = NULL;
			memset(usart_recv, 0, sizeof(usart_recv));
			min = ttime->tm_min;
		}
		
		
		
		//memset(buff, 0, sizeof(buff));
	}
	
}

int main(int argc, char **argv)
{
	int usart_fd;
	int temp,i;
	char buff[50] = {};
	
	if(wiringPiSetup() < 0)
	{
		perror("wiringPi error");
		return -1;
	}
	usart_fd = serialOpen("/dev/ttyAMA0",115200);
	if(usart_fd < 0)
	{
		perror("usart error");
		exit(0);
	}
	//signal(SIGALRM,time_out);//注册信号
	//alarm(1);//装置1s运行
	
	pthread_t tid;
	pthread_create(&tid, NULL, thread_func, (void*)&usart_fd);
	while(1)
	{
		temp = serialDataAvail(usart_fd);
		if(temp)
		{	
			for(i = 0; i < temp; i++)
			{
				buff[i] = serialGetchar(usart_fd);
			}
			strcat(usart_recv, buff);
			
			//printf("temp %d | %s\n", temp, buff);
			memset(buff, 0, sizeof(buff));
			//printf("usart_recv | %s\n", usart_recv);
		}

	}
	serialFlush(usart_fd);        //清空串口缓冲区
    serialClose(usart_fd);        //关闭串口设备

	return 0;
}
//删除指定的字符
int delete_character(char *src, char delete_c)
{
	int i = 0, j = 0;
	
	if(src)
	{
		for(i = 0; src[i]; i++);
		char temp[i];
		for(i = 0; src[i]; i++)
		{
			if(src[i] != delete_c)
			{
				temp[j++] = src[i];
			}
		}
		for(i = 0; src[i]; i++)
		{
			if(i >= j)
			{
				src[i] = '\0';
			}
			else
			{
				src[i] = temp[i];
			}
		}
			
	}
	else
		return -1;
	return 0;
	
}
