/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 
 ***create** zcl
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <linux/types.h>
#include <linux/spi/spidev.h>


#include <stdint.h>


#include <getopt.h>




#define BCM53115M_SPI_STATUS_REG 0xFE/*RO*/
#define BCM53115M_SPI_STATUS_SPIF
0x80
#define BCM53115M_SPI_STATUS_RACK
0x20
#define BCM53115M_SPI_STATUS_MDIO
0x04


#define BCM53115M_SPI_PAGE_REG 0xFF/*R/W*/


#define BCM53115M_SPI_DATA_REG 0xF0/*R/W*/


#ifndef ERROR
#define ERROR    (-1)
#endif


#ifndef OK
#define OK       (0)
#endif


static void pabort(const char *s)
{
	perror(s);
	abort();
}


static const char *device = "/dev/spidev32766.1";
static const char *spiDevice0 = "/dev/spidev32766.0";
static const char *spiDevice1 = "/dev/spidev32766.1";
static const char *spiDevice2 = "/dev/spidev32766.2";
static uint8_t mode ;
static uint8_t bits ;
static uint32_t speed ;




uint8_t SPI_ReadByte_LSB(int fd, uint8_t readAddress)
{
	int ret;
	uint8_t readCmd[1];
	uint8_t spiStateReg[1];
	uint8_t receiveData[1];
	struct spi_ioc_transfer xfer[2];

	readCmd[0] = 0x03;
	spiStateReg[0] = readAddress;

	memset(xfer, 0, 2*sizeof(struct spi_ioc_transfer));

	xfer[0].tx_buf = (unsigned long)readCmd;
	xfer[0].len = 1;


	xfer[1].tx_buf = (unsigned long)spiStateReg;
	xfer[1].rx_buf = (unsigned long)receiveData;
	xfer[1].len = 1;

	ret = ioctl(fd, SPI_IOC_MESSAGE(2), &xfer);

	if (ret < 1)
		pabort("can't xfer spi message1");

	// printf("receiveData:%x ;\r\n",receiveData[0]);

	return receiveData[0];
}


int SPI_WriteByte_LSB(int fd, uint8_t writeAddress, uint8_t writedata)
{ 
	int ret;
	uint8_t writeCmd[1];
	uint8_t spiStateReg[1];
	uint8_t rev[1];
	struct spi_ioc_transfer xfer[3];

	writeCmd[0] = 0x03;
	spiStateReg[0] = writeAddress;
	rev[0] = writedata;

	memset(xfer, 0, 3*sizeof(struct spi_ioc_transfer));

	xfer[0].tx_buf = (unsigned long)writeCmd;
	xfer[0].len = 1;

	xfer[1].tx_buf = (unsigned long)spiStateReg;
	xfer[1].len = 1;

	xfer[2].tx_buf = (unsigned long)rev;
	xfer[2].len = 1;


	ret = ioctl(fd, SPI_IOC_MESSAGE(3), &xfer);

	if (ret < 1)
		pabort("can't xfer spi message1");
	return OK;  
}
int SPI_WriteBytes_LSB(int fd, uint8_t reg, uint8_t *data, uint8_t length)
{ 
	int ret;
	uint8_t writeCmd[1];
	uint8_t spiStateReg[1];
	uint8_t receiveData[1];
	struct spi_ioc_transfer xfer[3];

	writeCmd[0] = 0x02;
	spiStateReg[0] = reg;

	memset(xfer, 0, 3*sizeof(struct spi_ioc_transfer));

	xfer[0].tx_buf = (unsigned long)writeCmd;
	xfer[0].len = 1;

	xfer[1].tx_buf = (unsigned long)spiStateReg;
	xfer[1].len = 1;

	xfer[2].tx_buf = (unsigned long)data;
	xfer[2].len = length;

	ret = ioctl(fd, SPI_IOC_MESSAGE(3), &xfer);

	if (ret < 1)
		pabort("can't xfer spi message1");
	return OK;  
}
int SPI_ReadBytes_LSB(int fd, uint8_t reg, uint8_t *data, uint8_t length)
{ 
	int ret,i;
	uint8_t readCmd[1];
	uint8_t spiStateReg[1];
	uint8_t receiveData[8];
	struct spi_ioc_transfer xfer[3];

	readCmd[0] = 0x03;
	spiStateReg[0] = reg;

	memset(xfer, 0, 3*sizeof(struct spi_ioc_transfer));

	xfer[0].tx_buf = (unsigned long)readCmd;
	xfer[0].len = 1;

	xfer[1].tx_buf = (unsigned long)spiStateReg;
	xfer[1].len = 1;

	xfer[2].rx_buf = (unsigned long)receiveData;
	xfer[2].len = length;

	ret = ioctl(fd, SPI_IOC_MESSAGE(3), &xfer);

	if (ret < 1)
		pabort("can't xfer spi message1");



	for(i = length; i>0; i--)
	{
		printf(" rxbuf[%d] = 0x %02x \r\n", (i-1), receiveData[i-1]);

	}

	return OK;  
}






static void print_usage(const char *prog)
{
	printf("Usage: %s [-D]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	    );
	exit(1);
}


static void parse_opts(int argc, char *argv[])
{
	while (1) 
	{
		static const struct option lopts[] = 
		{
			{ "device",  1, 0, 'D' },

			{ NULL, 0, 0, 0 },
		};
		int c;


		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR24", lopts, NULL);


		if (c == -1)
			break;


		switch (c) 
		{
			case 'D':
				device = optarg;
				break;

			default:
				print_usage(argv[0]);
				break;
		}
	}

}
static int spidev_usage(void)
{
	printf ("Usage : ./spi cs get  <reg> <size>\n"
			"        ./spi cs set  <reg> <data>\n");
	return 0;
}
unsigned int axtoi(char *str)
{
	unsigned int result = 0;


	char *p =  NULL;


	if(str == NULL)
	{
		return 0;
	}
	else
		if(str[0] == 'x' || str[0] == 'X')
		{
			p = str+1;
		}
		else
			if(str[0] == '0')
			{
				if(str[1] == 'x' || str[1] == 'X')
				{
					p = str+2;
				}
				else
				{
					p = str;
				}
			}
			else
			{
				p = str;
			}


	while(*p != '\0')
	{
		if('0' <= *p && *p <= '9')
			result = result * 16 + (*p - 0x30);
		else
			if('a' <= *p && *p <= 'f')
				result = result * 16 + (*p - 0x61 + 10);
			else
				if('A' <= *p && *p <= 'F')
					result = result * 16 + (*p - 0x41 + 10);
				else
					return result;

		p++;
	}


	return result;
}


int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	uint8_t buf[4]={};
	uint8_t *bp;

	int read_flag = 0;
	int write_flag = 0;
	int i;
	unsigned char temp;
	unsigned char page = 0;
	unsigned char reg = 0;
	unsigned char size = 0;
	unsigned char txdata[8];
	unsigned char rxdata[8];

	//parse_opts(argc, argv);
	if(argc > 4 && !strcmp("cs0",argv[1]) )
	{
		fd = open(spiDevice0, O_RDWR);
		if (fd < 0)
		{
			pabort("can't open device");
			goto out_close;
		}
	}
	else if(argc > 4 && !strcmp("cs1",argv[1]) )
	{
		fd = open(spiDevice1, O_RDWR);
		if (fd < 0)
		{
			pabort("can't open device");
			goto out_close;
		}
	} else 	if(argc > 4 && !strcmp("cs2",argv[1]) )
	{
		fd = open(spiDevice2, O_RDWR);
		if (fd < 0)
		{
			pabort("can't open device");
			goto out_close;
		}
	} else 	{ 
		return   spidev_usage();
		goto out_close;
	}

	if (argc > 4 && !strcmp("get",argv[2]))
	{
		read_flag = 1;
		reg = strtoul(argv[3],NULL,0);
		size = atoi (argv[4]);

		if((size<1)||(size>8))
		{
			printf("\r\n the parameter len must be 1 ~8 !\r\n");
			return OK;
		}
		printf("reg: %x;size:%d; \r\n",reg,size);


	}
	else if (argc > 4 && !strcmp("set",argv[2]))
	{
		write_flag = 1;
		reg = strtoul(argv[3],NULL,0);
		size = argc - 4;

		printf("reg: %x;size:%d; \r\n",reg,size);
		for(i = size; i>0; i--)
		{ 
			txdata[i-1] = (unsigned char)axtoi(argv[size-i+4]);
			printf(" data[%d] = 0x %02x \r\n", i-1, txdata[i-1]);
		}

	}
	else return spidev_usage();

	if (read_flag)
	{
		SPI_ReadBytes_LSB(fd,reg, rxdata, size);

	}
	else if (write_flag)
	{ 
		SPI_WriteBytes_LSB(fd, reg, txdata, size);
	}
	else
		spidev_usage();





	// bcmreadBytes(fd, 0x02, 0x30, buf,4);

out_close :
	close(fd);


	return ret;
}

