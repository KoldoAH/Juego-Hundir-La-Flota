/* TITULO:  cliente2.c
 * AUTORES: Koldo Aranegui y Mikel Arrastia
 * DESCRIPCION: cliente del objetivo 2 de la Practica final.
 * Se conecta con el servidor para poder jugar al juego hundir la flota.
 * Si ya hay dos clientes conectados, los siguientes podrán observar
 * la partida.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc,char* argv[]) {
	int socketfd, num_puerto,valor;
	struct sockaddr_in servidor_dir;
	char buffer_fd [200], id[200], val[200], linea[200];
	FILE *fichero;
	
	sscanf(argv[2], "%d", &num_puerto); //conversion del string puerto a entero
	
	
	//EstableciendoSocket
	if((socketfd=socket(PF_INET,SOCK_STREAM,0)) == -1) {
		perror("ErrordeSocket");
		exit(EXIT_FAILURE); 
	}
	
	//DefiniendoServidor->Ip,Puerto
	servidor_dir.sin_family=AF_INET;
	servidor_dir.sin_port=htons(num_puerto);
	servidor_dir.sin_addr.s_addr=inet_addr(argv[1]);
	memset(&(servidor_dir.sin_zero),'\0',8);
	
	//EstableciendoConexion
	if(connect(socketfd, (struct sockaddr*) &servidor_dir,sizeof(struct sockaddr_in) ) == -1) {
		perror("ErrordeConnect()");
		exit(EXIT_FAILURE); 
	}
	
	send(socketfd, "\n", sizeof("\n"), 0); //envia un salto de linea
	recv(socketfd, buffer_fd, sizeof(buffer_fd), 0); //recive el S HOLA
	printf("%s", buffer_fd);
	
	int conectado = 0;
	int comienzo = 0;
	
	//extraer la id y enviarla por el socket
	if (fichero = fopen("cliente.txt", "r+")){
		char mensaje[200] = "P LOGIN ";
		fgets(linea, 200, (FILE*)fichero);
		sscanf(linea, "%s %s", id, val);
		strcat(mensaje, id);
		strcat(mensaje, " ");
		strcat(mensaje, val);
		send(socketfd, mensaje, sizeof(mensaje), 0); //enviar el P LOGIN id c
		recv(socketfd, buffer_fd, sizeof(buffer_fd), 0); //recibe el S LOGIN OK
		if(strcmp(buffer_fd, "S LOGIN OK\n") == 0){
			printf("Te has conectado correctamente \n");
			conectado = 1;
		}
	}
	else{
		printf("NO EXISTE EL FICHERO \n");
		send(socketfd, "P REGISTRAR\n", sizeof("P REGISTRAR\n"), 0);
		memset(buffer_fd, 0, 200);
		recv(socketfd, buffer_fd, sizeof(buffer_fd), 0); //recibe S RESUELVE a b
		char s[10], aux1[100], aux2[50], aux3[50], aux_suma[50];
		sscanf(buffer_fd, "%s %s %s %s", s, aux1, aux2, aux3);
		printf("%s %s \n", aux2,aux3);
		int a,b, suma;
		a = atoi(aux2);
		b = atoi(aux3);
		suma = a+b;
		printf("El resultado de la suma es %d\n", suma);
		sprintf(aux_suma, "%d", suma);
		char mensaje[200] = "P RESPUESTA ";
		strcat(mensaje, aux_suma);
		send(socketfd, mensaje, sizeof(mensaje), 0);
		memset(buffer_fd, 0, 200);
		recv(socketfd, buffer_fd, sizeof(buffer_fd), 0); //recibe S REGISTRADO OK id o S REGISTRADO ERROR
		sscanf(buffer_fd, "%s %s %s %s", s, aux1, aux2, aux3);
		printf("El id es %s\n", aux3);
		
		if(strcmp(aux2, "OK") == 0){ //se ha registrado correctamente
			//escribir los datos en cliente.txt
			FILE *fichero2;
			fichero2 = fopen("cliente.txt","w");
			printf("id es %s\n", aux3);
			fprintf(fichero2, "%s %s", aux3, aux_suma);
			//fputs(aux3, fichero2);
			//fputs(" ", fichero2);
			//printf("El numerico es %s", aux_suma);
			//fputs(aux_suma, fichero2);
			conectado = 1;
			printf("Te has conectado correctamente \n");
			fclose(fichero2);
			//no esta escribiendo correctamente los datos en cliente.txt
		}	
		else{
			printf("ERROR AL REGISTRARSE. VUELVA A INTENTARLO \n");
			//cerrar conexion
		}
		
		
		
	}
	if(conectado == 1){
		char nombre[50], aux1[50], aux2[50], aux3[50], s[2];
		printf("Introduce tu nombre: ");
		scanf("%s", nombre);
		char auxNombre[200] = "P NOMBRE ";
		strcat(auxNombre, nombre);
		send(socketfd, auxNombre, sizeof(auxNombre), 0); //enviar P NOMBRE nombre
		memset(buffer_fd, 0, 200);
		recv(socketfd, buffer_fd, sizeof(buffer_fd), 0);
		sscanf(buffer_fd, "P NOMBRE %s", aux1); //recibe S NOMBRE OK o S NOMBRE ERROR
		if(strcmp(aux1, "OK")){ 
			int tambarcos[7] = {5, 4, 4, 3, 3, 2, 2};
			int barcosvalidos = 0;
			recv(socketfd, buffer_fd, sizeof(buffer_fd), 0);
			//empezamos a posicionar los barcos en el tablero
			while(barcosvalidos < 7){
				char col[20], fila[20];
				char dir[20];
				memset(buffer_fd, 0, 200);
				//recv(socketfd, buffer_fd, sizeof(buffer_fd), 0);
				sscanf(buffer_fd, "S BARCO %s", aux2);
				printf("Coloca un barco de %d casillas.\n", tambarcos[barcosvalidos]);
				scanf("%s %s %s", col, fila, dir);
				char barco[50] = "P BARCO ";
				strcat(barco, col);
				strcat(barco, " ");
				strcat(barco, fila);
				strcat(barco, " ");
				strcat(barco, dir);
				send(socketfd, barco, sizeof(barco), 0); //mandar nuestro barco
				memset(buffer_fd, 0, 200);
				recv(socketfd, buffer_fd, sizeof(buffer_fd), 0);
				while(strcmp(buffer_fd, "S BARCO ERROR\n") == 0){
					printf("Error, vuelve a colocar el barco.\n");
					scanf("%s %s %s", col, fila, dir);
					char barco[50] = "P BARCO ";
					strcat(barco, col);
					strcat(barco, " ");
					strcat(barco, fila);
					strcat(barco, " ");
					strcat(barco, dir);
					send(socketfd, barco, sizeof(barco), 0); //mandar nuestro barco
					memset(buffer_fd, 0, 200);
					recv(socketfd, buffer_fd, sizeof(buffer_fd), 0);
				}
				barcosvalidos++;
			}
			char inicio[200];
			//memset(buffer_fd, 0, 200);
			//recv(socketfd, buffer_fd, sizeof(buffer_fd), 0); //podemos recibir S ESPERA ó S INICIO nombre1 nombre2
			sscanf(buffer_fd, "%s %s %s %s", s, aux1, aux2, aux3);
			if(strcmp(aux1, "ESPERA") == 0){
				printf("Esperando a que comienze la partida.......\n");
				recv(socketfd, inicio, sizeof(inicio), 0); //recibe S INICIO nombre1 nombre2
				comienzo = 1;
				
			}
			else if(strcmp(aux1, "INICIO") == 0){
				comienzo = 1;
			}
			
			if(comienzo == 1){ //entra aqui
				char resultado[200];
				char col[20], fila[20];
				int col_disparo, fila_disparo;
				printf("COMIENZA LA PARTIDA\n");
				recv(socketfd, resultado, sizeof(resultado), 0);
				while(strcmp(resultado, "S PREMIO\n") != 0 && strcmp(resultado, "S SIGUE JUGANDO\n") != 0){ 
					if(strcmp(resultado, "S TUTURNO\n") == 0){
						memset(resultado, 0, 200);
						printf("Es tu turno\n");
						char disparo[200] = "P DISPARA ";
						printf("Tu disparo: ");
						scanf("%d %d", &col_disparo, &fila_disparo);
						sprintf(col, "%d", col_disparo);
						sprintf(fila, "%d", fila_disparo);
						strcat(disparo, col);
						strcat(disparo, " ");
						strcat(disparo, fila);
						strcat(disparo, "\n");
						send(socketfd, disparo, sizeof(disparo), 0); //enviar P DISPARA 
						recv(socketfd, resultado, sizeof(resultado), 0);
					}
					else if(strcmp(resultado, "S TOCADO\n") == 0){
						memset(resultado, 0, 200);
						printf("Has tocado el barco enemigo!\n");
						char disparo[200] = "P DISPARA ";
						printf("Tu disparo: ");
						scanf("%d %d", &col_disparo, &fila_disparo);
						sprintf(col, "%d", col_disparo);
						sprintf(fila, "%d", fila_disparo);
						strcat(disparo, col);
						strcat(disparo, " ");
						strcat(disparo, fila);
						strcat(disparo, "\n");
						send(socketfd, disparo, sizeof(disparo), 0); //enviar P DISPARA 
						recv(socketfd, resultado, sizeof(resultado), 0);
					}
					else if(strcmp(resultado, "S HUNDIDO\n") == 0){
						memset(resultado, 0, 200);
						printf("Has hundido un barco enemigo!!\n");
						char disparo[200] = "P DISPARA ";
						printf("Tu disparo: ");
						scanf("%d %d", &col_disparo, &fila_disparo);
						sprintf(col, "%d", col_disparo);
						sprintf(fila, "%d", fila_disparo);
						strcat(disparo, col);
						strcat(disparo, " ");
						strcat(disparo, fila);
						strcat(disparo, "\n");
						send(socketfd, disparo, sizeof(disparo), 0); //enviar P DISPARA 
						recv(socketfd, resultado, sizeof(resultado), 0);
					}
					else if(strcmp(resultado, "S NOVALE\n") == 0){
						memset(resultado, 0, 200);
						printf("Disparo no valido, prueba otra vez\n");
						char disparo[200] = "P DISPARA ";
						printf("Tu disparo: ");
						scanf("%d %d", &col_disparo, &fila_disparo);
						sprintf(col, "%d", col_disparo);
						sprintf(fila, "%d", fila_disparo);
						strcat(disparo, col);
						strcat(disparo, " ");
						strcat(disparo, fila);
						strcat(disparo, "\n");
						send(socketfd, disparo, sizeof(disparo), 0); //enviar P DISPARA 
						recv(socketfd, resultado, sizeof(resultado), 0);
						printf("He recibido %s\n", resultado);
					}
					else if(strcmp(resultado, "S AGUA\n") == 0){
						memset(resultado, 0, 200);
						printf("Agua!!\n");
						recv(socketfd, resultado, sizeof(resultado), 0);
					}
					else{
						printf("%s", resultado);
						memset(resultado, 0, 200);
						recv(socketfd, resultado, sizeof(resultado), 0);
					}
					
			    }
			    
			    if(strcmp(resultado, "S PREMIO\n") == 0){
					printf("HAS GANADO!!!!\n");
				}
				else if(strcmp(resultado, "S SIGUE JUGANDO\n") == 0){
					printf("HAS PERDIDO...\n");
				}
			}
		}
	}
	
	//fclose(fichero);
	//cerrar fichero
	/*fclose(fichero);
	
	fclose(socket_stream);

	close(socketfd);
	return EXIT_SUCCESS;*/
	
}