/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingenier�a Telem�tica
 * Dpto. Ingen�er�a de Telecomunicaci�n
 * Univerisdad de Ja�n
 *
 *******************************************************
 * Pr�ctica 1.
 * Fichero: cliente.c
 * Versi�n: 3.1
 * Curso: 2023/2024
 * Descripci�n:
 * 	Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Mart�nez
 *
 ******************************************************
 * Alumno 1:
 * Alumno 2:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca est�ndar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida b�sica
#include <locale.h>		// Para establecer el idioma de la codificaci�n de texto, n�meros, etc.
#include "protocol.h"	//Declarar constantes y funciones de la pr�ctica

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculaci�n (linking) la biblioteca Ws2_32.lib


int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int recibidos = 0, enviados = 0;
	int estado;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";
	char subj[64], to[64], from[64];
	int recibir = 1, cabeceras = 0;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicializaci�n de idioma
	setlocale(LC_ALL, "es-ES");


	//Inicializaci�n Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicializaci�n Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> �Qu� versi�n de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se emplear� IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versi�n IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Direcci�n por defecto seg�n la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			//Cada nueva conexi�n establece el estado incial en
			estado = S_INIT;

			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la m�quina de estados
				do {
					switch (estado) {																//Empezamos con el switch de estados
					case S_INIT:
						// Se recibe el mensaje de bienvenida
						break;

					case S_HELO:																	//En el caso HELO indicamos que introduzca el nombre del host
						// establece la conexion de aplicacion										//si no introducimos ningun valor nos salimos							
						printf("SMTP> Introduzca el host (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HL, input, CRLF);
						}
						break;

					case S_MAIL:     // En el estado MAIL, pedimos el correo del remitente y si no introduce ningun car�cter nos vamos al estado QUIT
						printf("SMTP> Introduzca el correo del remitente (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MF, input, CRLF);
						break;

					case S_RCPT:      // En el estado RCPT, pedimos el usuario de destino y al igual que en el estado MAIL, si no introducimos nada nos vamos al estado QUIT
						printf("SMTP> Introduzca el correo del destinatario (enter para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RT, input, CRLF);

						break;

					case S_DATA:
						/*
						printf("SMTP> Introduzca datos (enter o QUIT para salir): ");
						gets_s(input, sizeof(input));
						if (strlen(input) == 0) {
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else {

							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", ECHO, input, CRLF);
						}
						*/
						cabeceras = 0;
						sprintf_s(buffer_out, sizeof(buffer_out), " %s%s", DT, CRLF);
						break;

					case S_MSG:									//En el estado MSG montamos el mensaje que vamos a mandar
						recibir = 0;								//Inicializamos la variable recibir a 0, para repetir el proceso de escribir un mensaje hasta que haya un "."
						if (cabeceras == 0) {                       //Definimos una variable que sera la que usaremos para no repetir el bucle en caso de no 
							printf("SMTP> Subject: \r\n");		    //Introducir un "." en el mensaje, pedios el subject, el to y el from
							gets_s(input, sizeof(input));
							strcpy_s(subj, sizeof(subj), input);
							printf("SMTP> To: \r\n");
							gets_s(input, sizeof(input));
							strcpy_s(to, sizeof(to), input);
							printf("SMTP> From: \r\n");
							gets_s(input, sizeof(input));
							strcpy_s(from, sizeof(from), input);
							cabeceras = 1;

							break;

						}

						if (estado != S_INIT) {
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);    //send es una funci�n que escribe hasta un numero de bytes	de datos del socket especificado
							if (enviados == SOCKET_ERROR) {										//sockfd es un descriptor de socker creado anterior mente
								printf("ERROR> Se ha perdido la conexi�n con el servidor\r\n");
								estado = S_QUIT;
								continue;// La sentencia continue hace que la ejecuci�n dentro de un
										 // bucle salte hasta la comprobaci�n del mismo.
							}
						}

						recibidos = recv(sockfd, buffer_in, 512, 0);
						if (recibidos <= 0) {																	//Si recibimos 0 car�cteres, nos saltara el �ltimo error que hayamos cometido
							DWORD error = GetLastError();														//y nos dira que ha habido "x" error en la recepci�n de datos
							if (recibidos < 0) {
								printf("CLIENTE> Error %d en la recepci�n de datos\r\n", error);
								estado = S_QUIT;
							}
							else {
								printf("CLIENTE> Conexi�n con el servidor cerrada\r\n");
								estado = S_QUIT;
							}
						}
						else {
							buffer_in[recibidos] = 0x00;
							printf(buffer_in);
							switch (estado) {
							case S_INIT:
								if (strncmp(buffer_in, "220", 3) == 0) {		//Si recibimos un 220 "codigo usado para dar la bienvenida"	
									estado = S_HELO;							//pasamos al siguiente estado, si no nos vamos al estado QUIT
								}
								else {
									estado = S_QUIT;
								}
								break;

							case S_HELO:
								if (strncmp(buffer_in, "250", 3) == 0) {		//Si recibimos un 250 "codigo usado para decir que todo esta bien OK"	
									estado = S_MAIL;							//pasamos al siguiente estado, si no nos vamos al estado QUIT

								}
								else {
									estado = S_QUIT;
								}
								break;

							case S_MAIL:
								if (strncmp(buffer_in, "250", 3) == 0) {		//Si recibimos un 250 "codigo usado para decir que todo esta bien OK"
									estado = S_RCPT;							//pasamos al siguiente estado, si no nos vamos al estado QUIT
								}
								else {
									estado = S_QUIT;
								}
								break;

							case S_RCPT:
								if (strncmp(buffer_in, "250", 3) == 0) {		//Si recibimos un 250 "codigo usado para decir que todo esta bien OK"
									estado = S_DATA;							//pasamos al siguiente estado, si no nos vamos al estado QUIT
								}
								else {
									estado = S_QUIT;
								}
								break;

							case S_DATA:

								estado = S_MSG;					//En el estado DATA pasamos directamente al estado MSG
								break;


							case S_MSG:

								break;

							case S_RSET:

								break;

							}
						}// fin switch estados
					}

				} while (estado != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}