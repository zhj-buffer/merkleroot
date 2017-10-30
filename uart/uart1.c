#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include <semaphore.h>
#include <string.h>

#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART



#define FAN_SPEED (8000)



//串口初始化函数
int  Uart_Init(void)
{

	//以阻塞的方式打开串口设备
	int fd;
	fd = open("/dev/ttyTHS2",O_RDWR|O_NOCTTY);
	if (fd == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;//波特率115200bps，1个起始位，8 bit 数据位，一个停止位，无奇偶校验位，无流控
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	options.c_cc[VTIME]=0;
	options.c_cc[VMIN]=5;//VTIME=0，VMIN=5表示，串口读数据的时候，只有读到5个字节才返回，读不到五个字节，一直阻塞
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
	return fd;
}



//串口发送函数
void UART_Send(int fd,unsigned char *buff,unsigned int len )
{
	unsigned int count=0;
	count = write(fd, buff,len);	
	if (count !=len)
	{
		printf("UART TX error\n");
	}	
}

//串口接收函数
void UART_Rec(int fd ,unsigned char *buff,unsigned int len)
{
	unsigned int count=0;
	count = read(fd,buff,len);
	if(count!=len)
	{
		printf("UART Read error\n");
	}
}


//检查STM32下位机返回的指令
void check_return_value(unsigned char *return_buff)
{
	unsigned short int temp=0;
	
	temp=return_buff[2]*256+return_buff[3];
	
	if((return_buff[0]==0xa5)&&(return_buff[1]==0x5a))
	{
		if(return_buff[4]==(unsigned char )(return_buff[3]+return_buff[2]+return_buff[1]+return_buff[0]))
		{
			
			switch(temp)
			{
				//代表发送指令发送成功。
				case 0xaaaa:
				printf("Send Frame successed!\n");
				break;
				//代表发送指令的帧头错误
				case 0x5555:
				printf("Send Frame header error!\n");
				break;
				//代表发送指令的校验和错误
				case 0x6666:
				printf("Send Frame Checksum error!\n");
				break;
				//代表发送指令的参数范围错误
				case 0x7777:
				printf("Send Frame parameter error!\n");
				break;
				default:
				break;
			}
		}
		else
		{
			//下位机返回的指令校验和错误
			printf("Return frame Checksum error!\n");
		}
	}
	else
	{
		//下位机返回的指令帧头错误
		printf("Return frame header error!\n");
	}
}

//设置风扇1和风扇2的转速，两个风扇转速相同，范围为6000~12000转/min。
void setfanspeed(int fd,unsigned short int fanspeed)
{
	unsigned char Set_fan_speed_command[6];
	unsigned char return_buff[5];
	Set_fan_speed_command[0]=0xa5;
	Set_fan_speed_command[1]=0x5a;
	Set_fan_speed_command[2]=0x11;
	Set_fan_speed_command[3]=fanspeed/256;
	Set_fan_speed_command[4]=fanspeed%256;
	Set_fan_speed_command[5]=Set_fan_speed_command[0]+
							 Set_fan_speed_command[1]+
							 Set_fan_speed_command[2]+
							 Set_fan_speed_command[3]+
							 Set_fan_speed_command[4];
	
	UART_Send(fd,Set_fan_speed_command,6);
	UART_Rec(fd,return_buff,5);
	check_return_value(return_buff);
}

//获取风扇的转速，函数的第二个参数为1，代表风扇1,2代表风扇2。
unsigned short int  getfanspeed(int fd,unsigned char fanindex)
{
	unsigned char Get_fan1_speed_command[6];
	unsigned char Get_fan2_speed_command[6];
	unsigned char return_buff[5];
	
	if(1==fanindex)
	{
		Get_fan1_speed_command[0]=0xa5;
		Get_fan1_speed_command[1]=0x5a;
		Get_fan1_speed_command[2]=0x22;
		Get_fan1_speed_command[3]=0;
		Get_fan1_speed_command[4]=0;
		Get_fan1_speed_command[5]=Get_fan1_speed_command[0]+Get_fan1_speed_command[1]+Get_fan1_speed_command[2]+Get_fan1_speed_command[3]+Get_fan1_speed_command[4];
		UART_Send(fd,Get_fan1_speed_command,6);
		UART_Rec(fd,return_buff,5);
		if((return_buff[0]==0xa5)&&(return_buff[1]==0x5a))
		{
			if(return_buff[4]==(unsigned char )(return_buff[3]+return_buff[2]+return_buff[1]+return_buff[0]))
			{
				return(return_buff[2]*256+return_buff[3]);
			}
			else
			{
				printf("Return frame Checksum error!\n");
			}
		}
		else
		{
			printf("Return frame header error!\n");
		}
	}
	if(2==fanindex)
	{
		Get_fan2_speed_command[0]=0xa5;
		Get_fan2_speed_command[1]=0x5a;
		Get_fan2_speed_command[2]=0x33;
		Get_fan2_speed_command[3]=0;
		Get_fan2_speed_command[4]=0;
		Get_fan2_speed_command[5]=Get_fan2_speed_command[0]+Get_fan2_speed_command[1]+Get_fan2_speed_command[2]+Get_fan2_speed_command[3]+Get_fan2_speed_command[4];
		UART_Send(fd,Get_fan2_speed_command,6);  
		UART_Rec(fd,return_buff,5);
		if((return_buff[0]==0xa5)&&(return_buff[1]==0x5a))
		{
			if(return_buff[4]==(unsigned char )(return_buff[3]+return_buff[2]+return_buff[1]+return_buff[0]))
			{
				return(return_buff[2]*256+return_buff[3]);
			}
			else
			{
				printf("Return frame Checksum error!\n");
			}
		}
		else
		{
			printf("Return frame header error!\n");
		}
	}
	
}
int main(int argc,char *argv[])
{
	//UART
	int uart0_filestream = -1;
	
	//串口初始化。
	uart0_filestream=Uart_Init();
	
	unsigned short int fan1speed=0;
	unsigned short int fan2speed=0;
	
	while(1)
	{
		//设置风扇转速为宏定义中的FAN_SPEED。
		setfanspeed(uart0_filestream,FAN_SPEED);
		sleep(5);
		//读取风扇1转速
		fan1speed=getfanspeed(uart0_filestream,1);
		//读取风扇2转速
		fan2speed=getfanspeed(uart0_filestream,2);
		printf("fan1speed=%d,fan2speed=%d\n",fan1speed,fan2speed);
	}
}

