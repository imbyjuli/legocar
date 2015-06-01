#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h> //uart
#include <termios.h> //uart 
#include <opencv2/highgui/highgui.hpp>
#include "cv.h"
#include <pthread.h>
#include <semaphore.h>



using namespace cv;
using namespace std;

Mat src, src_gray;
Mat dst, detected_edges;
bool change=0; 



void uart_send_command (int uart0_filestream,char identifier, char message)		// Use this command for communicating with the board
{
	if(change)
	{
	uart_send_byte(uart0_filestream,identifier); 
	uart_send_byte(uart0_filestream,message); 
	change = 0; 
	}
	
}


int uart_setup()
{
	int  uart0_filestream = -1;
	uart0_filestream = open("/dev/ttyAMA0",O_RDWR | O_NOCTTY | O_NDELAY);
	if(uart0_filestream == -1)
	{	
		printf("[Error] Unable to open UART. Ensure it is not used by another application/n");
	}
	
	struct termios options;
	tcgetattr(uart0_filestream, &options);
		options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
		options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
	
	return uart0_filestream;
	
}


int uart_send_byte(int  uart0_filestream, char data)
{

	if (uart0_filestream != -1)
	{
		int out = write(uart0_filestream, &data, 1);
		if (out < 0)
		{
			printf("[ERROR] UART TX\n");
		}
		else
		{
			printf("[STATUS: TX 1 Byte] %c\n" , data);
		} 
	} //if uart0

}

int uart_send_string(int  uart0_filestream, char str[])
{


	if (uart0_filestream != -1)
	{
		int out = write(uart0_filestream, str, strlen(str));
		if (out < 0)
		{
			printf("[ERROR] UART TX\n");
		}
		else
		{
			printf("[STATUS: TX %d Byte] %s\n", strlen(str), str);
		} 
	} //if uart0

}


int uart_receive(int  uart0_filestream)
{
	//receive Bytes
	if (uart0_filestream != -1)
	{
		unsigned char BUF_RX[50];
		int rx_length = read(uart0_filestream, (void*)BUF_RX, 50);

		if(rx_length < 0) 
		{
			printf("[ERROR] UART RX\n");
		}
		else if(rx_length == 0)
		{
			printf("[ERROR] UART RX - no data \n");
		}
		else
		{
			BUF_RX[rx_length] = '\0';
			printf("[STATUS: RX %i Bytes] %s\n", rx_length , BUF_RX);
		}	 //rx_length check
	}//if uart0

}

int cam_read(Mat& frame)
{
	VideoCapture cap;
	bool bSuccess;
	double dWidth;
	double dHeight;
	
	cap.open(0);	//Cam wird automatisch deinitialisiert, wenn das Programm beendet wird.
	if(!cap.isOpened())
	{
		printf("Cannot open the vido cam\n");
		return -1;
	}

	cap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

	dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	printf("Frame size: %.0f x %.0f \n", dWidth, dHeight);
	
	bSuccess = cap.read(frame);
	if(!bSuccess)
	{
		printf("Cannot read a frame from video stream\n");
	}
	
	return 1;
}


int main(void)
{
	int speed 			= 0; //-100 100
	int servo 			= 0 ; // -7,7 
	char keyboardInput 	= 0;
	char  identifier;	// 201/0xC9 = LENKEN 202/0xCA = GAS 0xCB = RESET 
	char  msg_out;	
	Mat newFrame;
  	printf("Beginn\n"); 
	
	namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);
	
	
	
	while (1)
	{
			printf("\nEingabe (5:faster; 2:slower; SPACE:stop; 1:left; 3:right; ESC:Ende): ");
		keyboardInput = getchar();
		getchar(); //Enter (2.Zeichen) aus Eingabepuffer löschen
		
		


		switch(keyboardInput){
			case 0x1b:	
						printf("Esc was pressed by user\n");
						stop = 1;
						identifier =0xCB; 
						break;//uart_send_byte(uart0_filestream, keyboardInput); 
			case 0x35: 	
						printf("5 (for up) was pressed by user\n");	
						speed= speed==100 ? speed : speed+10; 
						msg_out*=(char)(speed+100);	
						identifier=0xCA;
						break;  
			case 0x32:  
						printf("2 (for down) was pressed by user\n");
						speed = speed==100 ? speed :speed-10;			
						msg_out=(char)(speed+100);
						identifier =0xCA; 
						change = 1; 
						break; 
			case 0x31:  
						printf("1 (for left) was pressed by user\n");
						servo = servo == -7 ? servo : servo -1: 
						msg_out (char) (servo+7); 
						identifier = 0xC9; 
						change = 1; 
						break;
			case 0x33: 
						printf("3 (for right) was pressed by user\n"); 
						servo = servo == 7 ? servo : servo+1; 
						identifier = 0xC9; 
						change = 1;
						break; 
			case 0x20:
						printf("SPACE (for reset) was pressed by user\n"); 
						identifier = 0xCB; 
						change = 	1; 
						break; 
			default:	printf("wrong input\n"); 
						break; 			

		}
			uart_send_command(uart0_filestream,identifier,msg_out); 



	}

	
	close(uart0_filestream);


	printf("Ende\n\n");
 	return 0;
}
