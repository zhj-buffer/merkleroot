#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>


#define TRUE 1


//初始化串口


void setTermios(struct termios * pNewtio, int uBaudRate)
{
	bzero(pNewtio, sizeof(struct termios));
	//8N1
	pNewtio->c_cflag = uBaudRate | CS8 | CREAD | CLOCAL;
	pNewtio->c_iflag = IGNPAR;
	pNewtio->c_oflag = 0;
	pNewtio->c_lflag = 0; //non ICANON


	pNewtio->c_cc[VINTR] = 0;
	pNewtio->c_cc[VQUIT] = 0;
	pNewtio->c_cc[VERASE] = 0;
	pNewtio->c_cc[VKILL] = 0;
	pNewtio->c_cc[VEOF] = 4;
	pNewtio->c_cc[VTIME] = 5;
	pNewtio->c_cc[VMIN] = 0;
	pNewtio->c_cc[VSWTC] = 0;
	pNewtio->c_cc[VSTART] = 0;
	pNewtio->c_cc[VSTOP] = 0;
	pNewtio->c_cc[VSUSP] = 0;
	pNewtio->c_cc[VEOL] = 0;
	pNewtio->c_cc[VREPRINT] = 0;
	pNewtio->c_cc[VDISCARD] = 0;
	pNewtio->c_cc[VWERASE] = 0;
	pNewtio->c_cc[VLNEXT] = 0;
	pNewtio->c_cc[VEOL2] = 0;
}






#define BUFSIZE 512


int main(int argc, char **argv)
{
	int fd;
	int nread;
	int Rlength = BUFSIZE;
	int Length = 0;
	char buff[BUFSIZE] = {0};
	char cMsg[BUFSIZE] = {0};
	char *pch = buff;
	int i = 0;
	struct termios oldtio, newtio;
	struct timeval tv;
	char *dev =(char *)"/dev/ttyO1";
	//char *dev =(char *)"/dev/ttyS1";
	fd_set rfds;
	if ((fd = open(dev, O_RDWR | O_NOCTTY))<0)
	{
		printf("err: can't open serial port!\n");
		return -1;
	}
	tcgetattr(fd, &oldtio);
	//setTermios(&newtio, B9600);
	setTermios(&newtio, B115200);
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);


	tv.tv_sec=3;
	tv.tv_usec=0;
	while (TRUE)
	{
		//printf("fd is <%d> wait...\n", fd);
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		if (select(1+fd, &rfds, NULL, NULL, &tv)>0)
		{
			//printf("wait.......\n");
			if (FD_ISSET(fd, &rfds))
			{
				//printf("FD_ISSET...\n");
				//nread=read(fd, buff, BUFSIZE);
				nread=read(fd, pch, BUFSIZE-Length);
				//buff[nread]='\0';

				for(i=Length;i<nread+Length;i++)
				{
					//printf("buff[%d] = <0x%x><%c>.\n", i,buff[i],buff[i]);

					printf("%c\n", buff[i]);
				}
				//printf("\n");
				//printf("nread = %d,Length = %d, %s\n",nread,Length, buff);
				Length += nread;
				if(Length >= BUFSIZE)
				{
					printf("######Length<%d> is too long.\n", Length);
					return -1;
				}   
				//printf("nread is %d...\n", nread);      
				pch += nread;  
			}
		}
		else
		{
			//memcpy(cMsg,buff,Length);
			printf("Uart msg is :\n");
			printf("%s\n", buff);
			//printf("\n");
			memset(buff, 0, BUFSIZE);
			pch = buff;
			Length = 0;

			tv.tv_sec=3;
			tv.tv_usec=0;

			//sleep(1);
		}
	}
	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}


WriteUart.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>


//serial port set function
void setTermios(struct termios *pNewtio, unsigned short uBaudRate)
{
	bzero(pNewtio,sizeof(struct termios));
	pNewtio->c_cflag = uBaudRate|CS8|CREAD|CLOCAL;
	pNewtio->c_iflag = IGNPAR;
	pNewtio->c_oflag = 0;
	pNewtio->c_lflag = 0;
	pNewtio->c_cc[VINTR] = 0;
	pNewtio->c_cc[VQUIT] = 0;
	pNewtio->c_cc[VERASE] = 0;
	pNewtio->c_cc[VKILL] = 0;
	pNewtio->c_cc[VEOF] = 4;
	pNewtio->c_cc[VTIME] = 5;
	pNewtio->c_cc[VMIN] = 0;
	pNewtio->c_cc[VSWTC] = 0;
	pNewtio->c_cc[VSTART] = 0;
	pNewtio->c_cc[VSTOP] = 0;
	pNewtio->c_cc[VSUSP] = 0;
	pNewtio->c_cc[VEOL] = 0;
	pNewtio->c_cc[VREPRINT] = 0;
	pNewtio->c_cc[VDISCARD] = 0;
	pNewtio->c_cc[VWERASE] = 0;
	pNewtio->c_cc[VLNEXT] = 0;
	pNewtio->c_cc[VEOL2] = 0;
}
int main(int argc,char **argv)
{
	int fd;
	int nCount = 0, ulen = 0, nTotal = 0;
	int i,j,m;
	int ReadByte = 0;


	char Buffer[512];
	struct termios oldtio,newtio;
	char *dev = (char*)"/dev/ttyO1";


	if((argc!=3)||(sscanf(argv[1],"%d",&nTotal)!=1))
	{
		printf("Usage:COMSend count datat! ");
		return -1;
	}


	if((fd=open(dev,O_RDWR|O_NOCTTY|O_NDELAY))<0) //open serial COM2
	{
		printf("Can't open serial port! ");
		return -1;
	}
	tcgetattr(fd,&oldtio);
	setTermios(&newtio,B115200);  // B9600
	//setTermios(&newtio,B57600);  
	tcflush(fd,TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);


	//Send data
	for(i=0;i<nTotal;i++)
	{
		nCount = write(fd,argv[2],strlen(argv[2]));
		ulen = write(fd, "\r\n",2);
		printf("send data OK!count=%d, nTotal=%d, ulen=%d.\n",nCount, nTotal, ulen);
		//tcflush(fd,TCIFLUSH);
		//tcsetattr(fd,TCSANOW,&newtio);
		usleep(500000);
	}

	/*
	//receive data
	for(j=0;j<20;j++)
	{
	ReadByte = read(fd,Buffer,512);
	if(ReadByte>0)
	{
	printf("readlength=%d ",ReadByte);
	Buffer[ReadByte]='\0';
	printf("%s ",Buffer);
	sleep(3);
	}
	else printf("Read data failure times=%d ",j);
	}     
	printf("Receive data finished! ");
	tcsetattr(fd,TCSANOW,&oldtio);
	*/
	close(fd);
	return 0;
}

