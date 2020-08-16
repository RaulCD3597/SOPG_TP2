#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "SerialManager.h"

#define BUFFER_LEN 			100
#define SERIAL_PORT 		1
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
pthread_t 			thread_TCP;
struct sigaction 	sign_action_1, sign_action_2;
bool				socket_open = false;
sig_atomic_t		running = 1;
pthread_mutex_t 	mutexSocket = PTHREAD_MUTEX_INITIALIZER;

static void SerialProcessPacket(void);
static void TcpProcessPacket(void);
static void *TCP_Task(void *params);
static void BlockSignals(void);
static void UnblockSignals(void);

static void SigInt_handler()
{
	write(1, "\nCtrl+c pressed!!\n",18);
	running = 0;
}

static void SigTerm_handler()
{
	write(1, "Sigterm received!\n", 18);
	running = 0;
}

static void end_process(void)
{
	if ( 0 != pthread_cancel(thread_TCP) )
	{
		perror("Error");
	}

	if ( 0 != pthread_join(thread_TCP, NULL) )
	{
		perror("Error");
	}
	exit(0);
}

int main(void)
{
	sign_action_1.sa_handler = SigInt_handler;
	sign_action_1.sa_flags = 0; // SA_RESTART; //
	sigemptyset(&sign_action_1.sa_mask);

	sign_action_2.sa_handler = SigTerm_handler;
	sign_action_2.sa_flags = 0; // SA_RESTART; //
	sigemptyset(&sign_action_2.sa_mask);

	if (sigaction(SIGINT, &sign_action_1, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	if (sigaction(SIGTERM, &sign_action_2, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

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

	BlockSignals();

	// Creamos thread para tarea TCP
	int ret = pthread_create(&thread_TCP, NULL, TCP_Task, NULL);
	if (0 != ret)
	{
		perror("Thread creation error");
		exit(ret);
	}

	UnblockSignals();

	printf("Inicio Serial Service\r\n");

	// Abrimos el puerto serial
	if (serial_open(SERIAL_PORT, BAUDRATE))
	{
		exit(EXIT_SUCCESS);
	}

	// El hilo principal del proceso se encarga de la tarea de puerto serie
	while (running)
	{
		int bytes_received;
		while(!(bytes_received = serial_receive(RX_buffer, BUFFER_LEN)))
		{
			if ( !running )
			{
				end_process();
			}
			usleep(5000);
		}

		// Ya racibi un paquete de puerto serie
		SerialProcessPacket();
	}

	end_process();
	return 0;
}

static void *TCP_Task(void *params)
{
	while (running)
	{
		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( -1 == (newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addr_len)) )
      	{
		    perror("error en accept");
		    exit(1);
	    }

		pthread_mutex_lock(&mutexSocket);
		socket_open = true;
		pthread_mutex_unlock(&mutexSocket);
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
		pthread_mutex_lock(&mutexSocket);
		socket_open = false;
		pthread_mutex_unlock(&mutexSocket);
    	close(newfd);
	}
	return NULL;
}

static void SerialProcessPacket(void)
{
	if ( !memcmp(RX_buffer, SERIAL_EV_STRING, strlen(SERIAL_EV_STRING)) )
	{
		pthread_mutex_lock(&mutexSocket);
		if ( socket_open )
		{
			int led;
			sscanf(RX_buffer, SERIAL_EV_FORMAT, &led);
			sprintf(RX_buffer, TCP_EV_FORMAT, led);
			if ( -1 == write(newfd, RX_buffer, strlen(RX_buffer)) )
			{
				perror("Error escribiendo mensaje en socket");
				exit(1);
			}
		}
		pthread_mutex_unlock(&mutexSocket);
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

static void BlockSignals(void)
{
	sigset_t set;
    sigemptyset(&set);
    sigfillset(&set);
    int err = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if ( 0 !=err )
	{
		perror("Error blocking signals");
		exit(1);
	}
}

static void UnblockSignals(void)
{
	sigset_t set;
    sigemptyset(&set);
    sigfillset(&set);
    int err = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	if ( 0 !=err )
	{
		perror("Error unblocking signals");
		exit(1);
	}
}