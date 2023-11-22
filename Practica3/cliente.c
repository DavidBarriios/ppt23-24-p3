/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Univerisdad de Jaén
 *
 *******************************************************
 * Práctica 1.
 * Fichero: cliente.c
 * Versión: 3.1
 * Curso: 2023/2024
 * Descripción:
 * 	Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1:
 * Alumno 2:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	//Declarar constantes y funciones de la práctica

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib


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
	char a[1000], subj[64], to[64], from[64];
	int recibir = 1, header = 0;
	int x, z, espacios = 0, letras = 0;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión IPv4
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

			//Dirección por defecto según la familia
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

			//Cada nueva conexión establece el estado incial en
			estado = S_INIT;

			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);

				//Inicio de la máquina de estados
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

					case S_MAIL:     // En el estado MAIL, pedimos el correo del remitente y si no introduce ningun carácter nos vamos al estado QUIT
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
						header = 0;
						sprintf_s(buffer_out, sizeof(buffer_out), " %s%s", DT, CRLF);
						break;

					case S_MSG:									//En el estado MSG montamos el mensaje que vamos a mandar
						recibir = 0;								//Inicializamos la variable recibir a 0, para repetir el proceso de escribir un mensaje hasta que haya un "."
						if (header == 0) {                       //Definimos una variable que sera la que usaremos para no repetir el bucle en caso de no 
							printf("SMTP> Subject: \r\n");		    //Introducir un "." en el mensaje, pedios el subject, el to y el from
							gets_s(input, sizeof(input));
							strcpy_s(subj, sizeof(subj), input);
							printf("SMTP> To: \r\n");
							gets_s(input, sizeof(input));
							strcpy_s(to, sizeof(to), input);
							printf("SMTP> From: \r\n");
							gets_s(input, sizeof(input));
							strcpy_s(from, sizeof(from), input);
							header = 1;

						}  //Aquí montamos las cabeceras, que sera lo que se muestre en el localhost y añadimos el doble CRLF para que no se nos monte unas cabeceras encima de otras
						else {
							printf("SMTP> Escriba un mensaje y pulse '.' para salir \r\n");
							gets_s(input, sizeof(input));
							strcpy_s(a, sizeof(a), input);			//Asignamos la cadena de carácteres a una variable en este caso es "a"
							z = strlen(a);							//El tamaño de "a", es igual a "z" (en números)
							for (x = 0; x <= z; x++) {				//Hacemos un bucle 
								if (a[x] == ' ') {					//Asignamos un contado en este caso espacios y ponemos que cuando se introduzca
									espacios++;						//Un espacio, la varibale espacios suma 1.
								}
							}
							letras = z + espacios;					//Asignamos a letras el número de carácteres introducidos en el mensaje + los espacios en blanco
							if (letras > 998) {						//Si superamos 998 que es el máximo permito, nos saltara un erros y resetaremos, nos volveremos al estado MAIL
								printf("Error has superado los caracterés máximos permitidos.");
								estado = S_RSET;
							}
							else {
								if (strcmp(input, ".") == 0) {    // Si no damos un error y no superamos el número establecido, pasamos a repetir el bucle hasta que introduzcamos un "."
									sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
									recibir = 1;  // Cuando introducimos un "." cambiamos la variable a recibir a 1.
								}
								else
									sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
							}
						}
						break;

					}

					if (estado != S_INIT) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);    //send es una función que escribe hasta un numero de bytes	de datos del socket especificado
						if (enviados == SOCKET_ERROR) {										//sockfd es un descriptor de socker creado anterior mente
							printf("ERROR> Se ha perdido la conexión con el servidor\r\n");
							estado = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
									 // bucle salte hasta la comprobación del mismo.
						}
					}
					if (recibir == 1) {         // Si la variable recibir es igual a 1, para esto tendría que haber un "." en el mensaje, pasamos a recibir los mensajes
						recibidos = recv(sockfd, buffer_in, 512, 0);
						if (recibidos <= 0) {																	//Si recibimos 0 carácteres, nos saltara el último error que hayamos cometido
							DWORD error = GetLastError();														//y nos dira que ha habido "x" error en la recepción de datos
							if (recibidos < 0) {
								printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
								estado = S_QUIT;
							}
							else {
								printf("CLIENTE> Conexión con el servidor cerrada\r\n");
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
								if (strncmp(buffer_in, "250", 3) == 0) {										//Si recibimos un 250 "codigo usado para decir que todo esta bien OK
									printf("¿Desea enviarselo a otro destinatario? (s) (Otra tecla): ");		//Le preguntamos si quiere enviar el mensaje a otro destinatario
									gets_s(input, sizeof(input));												//si introducimos "s" o "S", nos volveremos al estado RCPT
									if (strcmp(input, "s") == 0 || strcmp(input, "S") == 0) {					//para introducir el correo del otro destinatario
										estado = S_RCPT;
									}
									else {

										printf("¿Son correctos los datos? (n) (Otra tecla): ");					//Si no queremos mandarle el correo a otro destinatario
										gets_s(input, sizeof(input));											//le preguntamos si los datos estan bien, por si se equivoco
										if (strcmp(input, "n") == 0 || strcmp(input, "N") == 0) {				//a la hora de introducir el correo del destinatario, si nos dice que no
											printf("SMTP> RESETEO\r\n");										//pasamos al siguiente estado, si pone otro carácter nos vamos al estado RESTET
											estado = S_RSET;													//que el estado RESET nos lleva al estado MAIL
										}
										else
											estado = S_DATA;
									}
								}
								else {
									estado = S_QUIT;
								}

								break;

							case S_DATA:

								estado = S_MSG;					//En el estado DATA pasamos directamente al estado MSG
								break;


							case S_MSG:
								if (strncmp(buffer_in, "250", 3) == 0) {								//Si recibimos un 250 "codigo usado para decir que todo esta bien OK"
									printf("¿Desea enviar otro mensaje? (s) (Otra tecla): ");			//Le preguntamos si quiere enviar el mensaje, si introduce "s" o "S"
									gets_s(input, sizeof(input));										//nos vamos al estado MAIL, para inicializar las variables de nuevo
									if (strcmp(input, "s") == 0 || strcmp(input, "S") == 0) {			//y que nos pida las cabeceras en el estado MSG, si no queremos mandar 
										estado = S_MAIL;												//otro mensaje nos vamos al estado QUIT
									}
									else {
										estado = S_QUIT;
									}
								}
								else {
									estado = S_QUIT;
								}

								break;

								if (strncmp(buffer_in, "250", 3) == 0) {								//Si recibimos un 250 "codigo usado para decir que todo esta bien OK"
									estado = S_MAIL;													//pasamos al estado MAIL, para iniciar de nuevo la petición del correo del remitenten, 
								}																		//si no nos vamos al estado QUIT
								else {
									estado = S_HELO;
								}
								break;

							default:
								if (strncmp(buffer_in, OK, 2) == 0) {									//Ponemos un default, para que por defecto si recibimos un OK, pasar al siguiente estado
									estado++;															//se cual sea el estado
								}


							}// fin switch estados
						}
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
