#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "SerialManager.h"

#define BUFFER_LEN 			100
#define SERIAL_PORT 		2
#define BAUDRATE 			115200
#define SERIAL_EV_STRING 	">TOGGLE STATE:"
#define SERIAL_EV_FORMAT 	">TOGGLE STATE:%d"
#define SERIAL_SET_FORMAT 	">OUTS:%d,%d,%d,%d\r\n"
#define TCP_EV_FORMAT 		":LINE%dTG\n"
#define TCP_SET_STRING 		":STATES"
#define TCP_SET_FORMAT 		":STATES%d%d%d%d"

char 				RX_buffer[BUFFER_LEN];
char 				TCP_buffer[BUFFER_LEN];
socklen_t 			addr_len;
struct sockaddr_in 	clientaddr;
struct sockaddr_in 	serveraddr;
int 				sockfd, newfd;

static void SerialProcessPacket(void);
static void TcpProcessPacket(void);
static void *TCP_Task(void *params);

int main(void)
{
	pthread_t thread_TCP;

	// Creamos socket
	sockfd = socket( AF_INET,SOCK_STREAM, 0 );
	if (-1 == sockfd)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
	{
		printf("Socket successfully created..\n");
	}
	
	// Cargamos datos de IP:PORT del server
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(10000);
	if( 0 >= inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr)) )
	{
		fprintf(stderr, "ERROR invalid server IP\n");
		exit(1);
	}

	// Abrimos puerto con bind()
	if ( -1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) )
	{
		close(sockfd);
		printf("socket bind failed...\n"); 
        exit(1);
	}
    else
	{
        printf("Socket successfully binded..\n");
	}

	// Seteamos socket en modo Listening
	if ( -1 == (listen(sockfd, 5)) ) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
	{
        printf("Server listening..\n");
	}

	// Creamos thread para tarea TCP
	int ret = pthread_create(&thread_TCP, NULL, TCP_Task, NULL);
	if (0 != ret)
	{
		perror("Thread creation error");
		exit(ret);
	}

	printf("Inicio Serial Service\r\n");

	// Abrimos el puerto serial
	if (serial_open(SERIAL_PORT, BAUDRATE))
	{
		exit(EXIT_FAILURE);
	}

	// El hilo principal del proceso se encarga de la tarea de puerto serie
	for (;;)
	{
		int bytes_received;
		while((bytes_received = serial_receive(RX_buffer, BUFFER_LEN)))
		{
			usleep(5000);
		}

		// Ya racibi un paquete de puerto serie
		SerialProcessPacket();
	}

	pthread_join(thread_TCP, NULL);

	exit(EXIT_SUCCESS);
	return 0;
}

static void *TCP_Task(void *params)
{
	for (;;)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( -1 == (newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addr_len)) )
      	{
		    perror("error en accept");
		    exit(1);
	    }

		char ipClient[32];
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
		printf  ("server:  conexion desde:  %s\n",ipClient);

		int n;

		while ( 0 != (n = recv(newfd, TCP_buffer, BUFFER_LEN, 0)) )
		{
			if ( -1 == n)
			{
				perror("Error leyendo mensaje en socket");
			}
			else
			{
				TCP_buffer[n] = 0x00;
				TcpProcessPacket();
			}
		}
		// Cerramos conexion con cliente
    	close(newfd);
	}
	return NULL;
}

static void SerialProcessPacket(void)
{
	if ( !memcmp(RX_buffer, SERIAL_EV_STRING, strlen(SERIAL_EV_STRING)) )
	{
		int led;
		sscanf(RX_buffer, SERIAL_EV_FORMAT, &led);
		sprintf(RX_buffer, TCP_EV_FORMAT, led);
		if ( -1 == write(newfd, RX_buffer, strlen(RX_buffer)) )
		{
			perror("Error escribiendo mensaje en socket");
		}
	}
}

static void TcpProcessPacket(void)
{
	if( !memcmp( TCP_buffer, TCP_SET_STRING, strlen(TCP_SET_STRING)) )
	{
		int out1, out2, out3, out4;
		out1 = TCP_buffer[7] - '0';
		out2 = TCP_buffer[8] - '0';
		out3 = TCP_buffer[9] - '0';
		out4 = TCP_buffer[10] - '0';
		sprintf( TCP_buffer, SERIAL_SET_FORMAT, out1, out2, out3, out4 );
		serial_send( TCP_buffer, strlen(TCP_buffer) );
	}
}