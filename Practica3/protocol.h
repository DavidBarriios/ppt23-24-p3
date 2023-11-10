#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define HL "HELO"  // Saludo al servidor SMTP
#define MF "MAIL FROM:"  // Introducimos el correo del remitente
#define RT "RCPT TO:"  // Introducimos el destinatario
#define DT "DATA"   // Introducimos los datos
#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion
#define ECHO "ECHO" // Definicion del comando "ECHO" para el servicio de eco


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_INIT 0
#define S_HELO 1
#define S_MAIL 2
#define S_RCPT 3
#define S_DATA 4
#define S_CAB 9
#define S_MSG 5
#define S_QUIT 6
#define S_RSET 7
#define S_EXIT 8


//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"
#define PASSWORD	"123456"