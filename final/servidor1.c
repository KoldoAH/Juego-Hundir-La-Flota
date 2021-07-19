/* TITULO:  servidor1.c
 * AUTORES: Koldo Aranegui y Mikel Arrastia
 * DESCRIPCION: servidor del objetivo 1 de la Practica final.
 * gestiona la comunicacion entre dos clientes para que puedan 
 * jugar al juego hundir la flota.
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio_ext.h>
#include <time.h>


void sobreescribirLinea(int posicionLinea,char lineaNueva[200]);
int CasillasLibres(int col,int fila,char dir[],int tablero[10][10], int tamanioBarco);
int comprobarDisparo(int tablero[10][10],int col,int fila );
int finaldepartida(int tablero[10][10]);

int main(int argc,char* argv[]) {
	
	fd_set master;
	fd_set read_fds;
	struct sockaddr_in servidor_dir, cliente_dir;
	int fdmax, listener, num_puerto, max_conexiones;
	int newfd;
	char buffer [200];
	int nbytes;
	int yes = 1;
	unsigned int dirlen;
	int i;
	FD_ZERO(&master);
	FD_ZERO(&read_fds); 
	
	FILE *fichero;
	
	int tablero [2][10][10];
	int jugadoresListos=0, conexiones;
	int idJugadores[2];
	int turno;

	sscanf(argv[1], "%d", &num_puerto);
	sscanf(argv[2], "%d", &max_conexiones);
	printf("maximo de conexiones %d \n", max_conexiones);
	int numeroJugadoresAhora=0;
	char tablaNombresJugadores[2][200];
	memset (tablaNombresJugadores,0,200);
	printf("num jugadores: %d\n", numeroJugadoresAhora);
	
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("ErrordeSocket");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {            
		perror("setsockopt");
		exit(1);
	}
	
	servidor_dir.sin_family=AF_INET;
	servidor_dir.sin_port=htons(num_puerto);
	servidor_dir.sin_addr.s_addr=INADDR_ANY;
	memset(&(servidor_dir.sin_zero),'\0',8);
	
	if( bind(listener, (struct sockaddr *)&servidor_dir, sizeof(servidor_dir)) < 0)
    {
        //print the error message
        perror("Error en el bind");
        return 1;
    }
	
	if (listen(listener, max_conexiones) == -1){  //cambiar 10 por el maximo de usuarios permitidos
		perror("listen");
		exit(1);
	}
	
	FD_SET(listener, &master);
	fdmax = listener;
	
	int login_ok[max_conexiones+1];  //mirar si el +1 está bien!!!!!!!!!!!!
	for(int j = 0; j < max_conexiones+1; j++){
		login_ok[j] = 0;
	}
	
	conexiones = 0;
	
	while(1){
		read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++){
			char id[200];
			
			if (FD_ISSET(i, &read_fds)){
				if (login_ok[i-3] == 0){ 
					if (i == listener){
						dirlen = sizeof(cliente_dir);
						if ((newfd = accept(listener, (struct sockaddr*)&cliente_dir,&dirlen)) == -1){
							perror("accept");
						} else{
							FD_SET(newfd, &master);
							if (newfd > fdmax){
								fdmax = newfd;
							}
							printf("selectserver: new connection from %s on socket %d\n", inet_ntoa(cliente_dir.sin_addr),newfd);
							conexiones++;
						}
					} else{
						// gestionar datos de un cliente 
						
						if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0){  //hacemos la llamada a send
							// error o conexión cerrada por el client
							if (nbytes == 0){
								// conexión cerrada
								printf("selectserver: socket %d hung up\n", i);
							} else{
								perror("recv");
							}
							close(i);
							FD_CLR(i, &master);
						}
						else{  //AQUI GESTIONAMOS LOS DATOS DE LOS CLIENTES QUE NO SON CONEXIONES AL SERVIDOR
							send(i, "S HOLA\n", sizeof("S HOLA\n"), 0);
							//printf("he mandado el S HOLA \n");
							recv(i, buffer, sizeof(buffer), 0);
							char mensaje[200],c[200];						
							sscanf(buffer, "P %s %s %s", mensaje, id, c);
							printf("mensaje: %s ,id: %s, c: %s\n", mensaje, id, c);
							
							if(strcmp(mensaje, "REGISTRAR") == 0){
								//creamos el mensaje de registro
								int entero1, entero2, suma;
								char clave1[5], clave2[5], valsuma[5], respuesta[200];
								entero1 = rand() % 10;
								entero2 = rand() % 10;
								suma = entero1 + entero2;
								sprintf(clave1, "%d", entero1);
								sprintf(clave2, "%d", entero2);
								sprintf(valsuma, "%d", suma);
								char registro[200] = "S RESUELVE ";
								strcat(registro, clave1);
								strcat(registro, " ");
								strcat(registro, clave2);
								strcat(registro, "\n");
								
								send(i, registro, sizeof(registro), 0);
								memset(buffer,0,200);
								recv(i, buffer, sizeof(buffer), 0);
								char valor[200];
								sscanf(buffer, "P %s %s", respuesta,valor);
								
								
								//creamos el mensaje que deberiamos recibir para compararlo
								memset(registro, 0, 200);
								strcat(registro, "P RESPUESTA ");
								strcat(registro, valsuma);
								strcat(registro, "\n");

								
								if(strcmp(valsuma, valor) == 0){
									
									//fclose(fichero);
									fichero= fopen("servidor.txt","a+");
									time_t t;
									srand((unsigned) time(&t));
									char cadena[100] = "abcdefghijklmnopqrstuvwxyz0123456789";
									//char id[12];
									int tamanio = (rand()%7) + 6;
									//añadir el id aleatorio
									memset(id, 0, 200);
									for(int iteracion = 0; iteracion < tamanio; iteracion++){
										char caracter = cadena[rand()%36];
										fputc(caracter, fichero);
										id[iteracion]=caracter;
									}
									
									char registrado[200] = "S REGISTRADO OK ";
									strcat(registrado, id);
									strcat(registrado, "\n");
									send(i, registrado, sizeof(registrado), 0);
									
									fputs(" ", fichero);
									fputs(valor, fichero);
									fputs("\n", fichero);
									//fclose(fichero);
									login_ok[i-3] = 1;
									
								}
								else{
									send(i, "S REGISTRADO ERROR\n", sizeof("S REGISTRADO ERROR\n"), 0);
								}
							}
							else if(strcmp(mensaje,"LOGIN")==0){
								printf("LLEGA A LOGIN\n");
								char nombre_fichero[200], id_fichero[200], linea[200],valor_fichero[200];
								fichero = fopen("servidor.txt", "r+");
								printf("abre fichero servidor\n");
								
								fgets(linea,200,fichero);
								sscanf(linea,"%s %s %s", id_fichero, valor_fichero, nombre_fichero);	
								printf("id_fichero: %s, valor_fichero: %s\n", id_fichero,valor_fichero);
								printf("id: %s\n",id);
								while (feof(fichero) == 0 && (strcmp(id, id_fichero) != 0)){ //encontrar un id que sea igual, para parar la busqueda
										fgets(linea,200,fichero);
										sscanf(linea,"%s %s %s", id_fichero, valor_fichero, nombre_fichero);	
								}
								if (strcmp(id, id_fichero) != 0){
									send(i, "S L0GIN ERROR\n", sizeof("S LOGIN ERROR\n"), 0);
										
								}
								else {
									send(i, "S LOGIN OK\n", sizeof("S LOGIN OK\n"), 0);
									login_ok[i-3] = 1;
								}
								
							}
						}
					}
				}
				else if (login_ok[i-3] == 1){ 
					char p[10], aux1[200], nombre_elegido[200], id_fichero[200], valor[200], linea[200], nombre[200];
					//send(i, "S NOMBRE OK\n", sizeof("S NOMBRE OK\n"), 0);
					memset(buffer, 0, 200);
					recv(i, buffer, sizeof(buffer), 0);
					sscanf(buffer, "%s %s %s", p, aux1, nombre_elegido);
					fclose(fichero);
					fichero=fopen("servidor.txt","r+");
					int posicionLinea=0;
					memset(linea,0,200);
					fgets(linea,200,fichero);
					printf("linea: %s\n", linea);
					printf("id: %s\n", id);
					printf("Nombre_elegido: %s\n", nombre_elegido);
					sscanf(linea,"%s %s %s\n", id_fichero, valor, nombre);
					while(strcmp(id_fichero, id)!=0){
						posicionLinea++;
						fgets(linea,200,fichero);
						sscanf(linea,"%s %s %s", id_fichero, valor, nombre);
					}
					
					char lineaNueva[200];
					memset(lineaNueva,0,200);
					strcat(lineaNueva,id);
					strcat(lineaNueva, " ");
					strcat(lineaNueva, valor);
					strcat(lineaNueva, " ");
					strcat(lineaNueva, nombre_elegido);
					strcat(lineaNueva, "\n");
					fclose(fichero);
					
					sobreescribirLinea(posicionLinea+1,lineaNueva); 
					posicionLinea=0;
					
					memset(lineaNueva,0,200);
					if (numeroJugadoresAhora<2){
						strcpy(tablaNombresJugadores[numeroJugadoresAhora],nombre_elegido);
						numeroJugadoresAhora= numeroJugadoresAhora+1;
						send(i,"S NOMBRE OK\n",sizeof("S NOMBRE OK\n"),0);
						
						int tamaniobarcos[7],col, fila;
						char stringElementoListaBarco[16], auxcol[16], auxfila[16], dir[16];
						
						tamaniobarcos[0]= 5; 
						tamaniobarcos[1]= 4;
						tamaniobarcos[2]=4;
						tamaniobarcos[3]=3; 
						tamaniobarcos[4]=3;
						tamaniobarcos[5]=2;
						tamaniobarcos[6]=2;
						char arraySendBarco[200];
						for (int fila=0; fila<10; fila ++){
							for (int columna =0; columna < 10; columna ++){
								tablero[numeroJugadoresAhora-1][fila][columna]= 0;
							}
						}
						
						int elementoListaBarco=0;
						while (elementoListaBarco<7){
							int barcovalido = 0;
							memset(buffer, 0, 200);
							memset(auxfila,0,16);
							memset(dir, 0 , 16);
							memset(auxcol, 0 ,16);
							memset(arraySendBarco,0,200);
							strcat(arraySendBarco, "S BARCO ");
							sprintf(stringElementoListaBarco,"%d",tamaniobarcos[elementoListaBarco]);
							strcat(arraySendBarco, stringElementoListaBarco);
							strcat(arraySendBarco, "\n");
							printf("arraySendBarco: %s\n", arraySendBarco);
							send(i,arraySendBarco, sizeof(arraySendBarco), 0);
							while (barcovalido == 0){
								recv(i, buffer, sizeof(buffer), 0);
								printf("buffer: %s\n", buffer);
								sscanf(buffer, "P BARCO %s %s %s",auxcol, auxfila, dir);
								col= atoi(auxcol);
								fila= atoi(auxfila);

								if (strcmp(dir, "h")==0){
									if (fila<10 && (col+tamaniobarcos[elementoListaBarco] <= 10) && (CasillasLibres(col,fila,dir,tablero[numeroJugadoresAhora-1],tamaniobarcos[elementoListaBarco])==1)){ 
										for (int casillaBarco= 0; casillaBarco<tamaniobarcos[elementoListaBarco]; casillaBarco++){
											tablero[numeroJugadoresAhora-1][fila][col+casillaBarco]=1;
										}	
										for(int filaprox = fila-1; filaprox <= fila + 1; filaprox++){
											for(int colprox = col -1; colprox < col + tamaniobarcos[elementoListaBarco] + 1; colprox++){
												if(filaprox >= 0 && filaprox <= 9 && colprox >= 0 && colprox <= 9){ //comprobar que las casillas de alrededor no se salgan del tablero
													if (tablero[numeroJugadoresAhora-1][filaprox][colprox] == 0){
														tablero[numeroJugadoresAhora-1][filaprox][colprox] = 2; //sustituir por -1
													}
												}
											}
										}
										
										elementoListaBarco= elementoListaBarco+1;
										barcovalido = 1;
									}
									else{
										send(i,"S BARCO ERROR\n", sizeof("S BARCO ERROR\n"), 0);
									}
								}
								else if(strcmp(dir,"v")==0){
									if((fila+tamaniobarcos[elementoListaBarco] <= 10) && (col <10)&&(CasillasLibres(col, fila,dir, tablero[numeroJugadoresAhora-1], tamaniobarcos[elementoListaBarco])==1)){
										for (int casillaBarco= 0; casillaBarco<tamaniobarcos[elementoListaBarco]; casillaBarco++){
											tablero[numeroJugadoresAhora-1][fila + casillaBarco][col]=1;
										}
										//rellenar las casillas de al lado con -1
										for(int filaprox = fila-1; filaprox < fila + tamaniobarcos[elementoListaBarco] + 1; filaprox++){
											for(int colprox = col -1; colprox <= col + 1; colprox++){
												if(filaprox >= 0 && filaprox <= 9 && colprox >= 0 && colprox <= 9){ //comprobar que las casillas de alrededor no se salgan del tablero
													if (tablero[numeroJugadoresAhora-1][filaprox][colprox] == 0){
														tablero[numeroJugadoresAhora-1][filaprox][colprox] = 2; 
													}
												}
											}
										}

										elementoListaBarco= elementoListaBarco+1;
										barcovalido = 1;
									}
									else{
										send(i,"S BARCO ERROR\n", sizeof("S BARCO ERROR\n"), 0);
									}
								}
								else{
									send(i,"S BARCO ERROR\n", sizeof("S BARCO ERROR\n"), 0);
								}
							}
							//MOSTRAR EL BARCO EN EL TABLERO
							for (int f=0; f<10; f ++){
								for (int columna =0; columna < 10; columna ++){
									printf("%d ",tablero[numeroJugadoresAhora-1][f][columna]);
								}
								printf("\n");
							}	
						}
						idJugadores[jugadoresListos] = i;
						jugadoresListos= jugadoresListos+1;
						
						//EMPEZAR JUEGO
						if (jugadoresListos==2){
							char inicio[200] = "S INICIO ";
							strcat(inicio, tablaNombresJugadores[0]);
							strcat(inicio, " ");
							strcat(inicio, tablaNombresJugadores[1]);
							strcat(inicio, "\n");
							
							for (int tableroJugador=0;tableroJugador<2;tableroJugador ++){
								printf("tablero del jugador %s\n", tablaNombresJugadores[tableroJugador]);
								for(int fila=0; fila<10;fila++){
									for (int col=0; col<10; col++){
										printf("%d ",tablero[tableroJugador][fila][col]);
									}
									printf("\n");
								}
								printf("\n\n");
								send(idJugadores[tableroJugador], inicio, sizeof(inicio),0);
							}

							turno = 0;
							
							//enviar inicio a los dos jugadores					
							char auxcolumna[5], auxfila[5];
							
							send(idJugadores[turno], "S TUTURNO\n", sizeof("S TUTURNO\n"),0);
							for (int jug = 0; jug < 2; jug++){
								while(finaldepartida(tablero[(turno+1)%2]) != 0){
									
									recv(idJugadores[turno], buffer, sizeof(buffer), 0); //va a recibir P DISPARA col fila
									sscanf(buffer, "P DISPARA %s %s", auxcolumna, auxfila);
									col = atoi(auxcolumna);
									fila = atoi(auxfila);
									//comprobar disparo 
									
									printf("fila columna %d %d\n", fila, col);
									for (int tableroJugador=0;tableroJugador<2;tableroJugador ++){
										printf("tablero del jugador %s\n", tablaNombresJugadores[tableroJugador]);
										for(int fila=0; fila<10;fila++){
											for (int col=0; col<10; col++){
												printf("%d ",tablero[tableroJugador][fila][col]);
											}
											printf("\n");
										}
										printf("\n\n");
									}
									
									int disparo = comprobarDisparo(tablero[(turno+1)%2],col,fila ); //hacer la funcion
									if (disparo == 1){ //tocado
										char info[200] = "S INFO ";
										send(idJugadores[turno], "S TOCADO\n", sizeof("S TOCADO\n"), 0);
										strcat(info, "TOCADO ");
										strcat(info, tablaNombresJugadores[(turno+1)%2]);
										strcat(info, " ");
										strcat(info, auxcolumna);
										strcat(info, " ");
										strcat(info, auxfila);
										strcat(info, "\n"); //crear S INFO TOCADO nombre col fila\n
										//send(idJugadores[(turno+1)%2], info, sizeof(info),0);
										tablero[(turno+1)%2][fila][col]=3;
									}
									else if(disparo == 2){ //hundido
										char info[200] = "S INFO ";
										tablero[(turno+1)%2][fila][col]=3;
										if(finaldepartida(tablero[(turno+1)%2]) != 0){
											send(idJugadores[turno], "S HUNDIDO\n", sizeof("S HUNDIDO\n"), 0);
										}
										strcat(info, "HUNDIDO ");
										strcat(info, tablaNombresJugadores[(turno+1)%2]);
										strcat(info, " ");
										strcat(info, auxcolumna);
										strcat(info, " ");
										strcat(info, auxfila);
										strcat(info, "\n"); //crear S INFO HUNDIDO nombre col fila\n
										//send(idJugadores[(turno+1)%2], info, sizeof(info),0);
									}
									else if(disparo==3){ //agua
										char info[200] = "S INFO ";
										send(idJugadores[turno], "S AGUA\n", sizeof("S AGUA\n"), 0);
										strcat(info, "AGUA ");
										strcat(info, tablaNombresJugadores[(turno+1)%2]);
										strcat(info, " ");
										strcat(info, auxcolumna);
										strcat(info, " ");
										strcat(info, auxfila);
										strcat(info, "\n"); //crear S INFO AGUA nombre col fila\n
										//send(idJugadores[(turno+1)%2], info, sizeof(info),0);
										//cambiar turno
										tablero[(turno+1)%2][fila][col]=3;
										turno = (turno+1)%2;
										send(idJugadores[turno], "S TUTURNO\n", sizeof("S TUTURNO"),0);
									}
									else if(disparo == 4){
										//REPETIR DISPARO
										send(idJugadores[turno],"S NOVALE\n", sizeof("S NOVALE\n"),0);
									}
								}
								
							}
							printf("mando el s premio\n");
							send(idJugadores[turno], "S PREMIO\n", sizeof("S PREMIO\n"), 0);
							printf("mando el s sigue jugando\n");
							send(idJugadores[(turno+1)%2], "S SIGUE JUGANDO\n", sizeof("S SIGUE JUGANDO\n"), 0);
								
							
						}
						else if(jugadoresListos == 1){
							send(i, "S ESPERA\n", sizeof("S ESPERA\n"), 0);
						}
					}
				}

			}
			
		}
	}
	
}


void sobreescribirLinea(int posicionLinea,char lineaNueva[200]){
	FILE * fPtr;
    FILE * fTemp;
   
    
    char buffer[200];
    int  count;

    /* Remove extra new line character from stdin */
    //fflush(stdin);


    /*  Open all required files */
    fPtr  = fopen("servidor.txt", "r");
    fTemp = fopen("replace.tmp", "w"); 

    /* fopen() return NULL if unable to open file in given mode. */
    if (fPtr == NULL || fTemp == NULL)
    {
        /* Unable to open file hence exit */
        printf("\nUnable to open file.\n");
        printf("Please check whether file exists and you have read/write privilege.\n");
        exit(EXIT_SUCCESS);
    }

    /*
     * Read line from source file and write to destination 
     * file after replacing given line.
     */
    count = 0;
    while ((fgets(buffer, 200, fPtr)) != NULL)
    {
        count++;

        /* If current line is line to replace */
        if (count == posicionLinea)
            fputs(lineaNueva, fTemp);
        else
            fputs(buffer, fTemp);
    }


    /* Close all files to release resource */
    fclose(fPtr);
    fclose(fTemp);


    /* Delete original source file */
    remove("servidor.txt");

    /* Rename temporary file as original file */
    rename("replace.tmp", "servidor.txt");
    
}


int CasillasLibres(int col,int fila,char dir[],int tablero[10][10], int tamanioBarco){
	int resultado;
	int casillaBarco=0;
	if (strcmp(dir,"h")==0){
		while(casillaBarco<tamanioBarco && (tablero[fila][col+casillaBarco]==0)){
			casillaBarco= casillaBarco+1;
		}
	}
	else if (strcmp(dir,"v")==0){
		while(casillaBarco<tamanioBarco && (tablero[fila+casillaBarco][col]==0)){
			casillaBarco= casillaBarco+1;
			
		}
	}	
	if (casillaBarco == tamanioBarco){
		resultado = 1;
	}
	else{
		resultado = 0;
	}
	
	return resultado;
}


int comprobarDisparo(int tablero[10][10],int col,int fila ){
	int disparo;
	if(col > 9 || col < 0 || fila > 9 || fila < 0){ //fuera de rango
		disparo = 4;
	}
	else{
		if(tablero[fila][col] == 0 || tablero[fila][col] == 2){ //agua
			disparo = 3;
		}
		else if(tablero[fila][col] == 1){
			disparo = 2;
			if((fila-1) >= 0){
				if(tablero[fila-1][col] == 1){
					disparo = 1;
				}
			}
			if((fila+1) <= 9){
				if(tablero[fila+1][col] == 1){
					disparo = 1;
				}
			}
			if((col-1) >= 0){
				if(tablero[fila][col-1] == 1){
					disparo = 1;
				}
			}
			if((col+1) <= 9){
				if(tablero[fila][col+1] == 1){
					disparo = 1;
				}
			}
		}
		else{ //posicion ya elegida
			disparo = 4;
		}
	}
	return disparo;
}

int finaldepartida(int tablero[10][10]){
	int hayUno= 0;
	int fila =0;
	int columna;
	
	while(fila<10 && hayUno!=1){
		columna = 0;
		while(columna <10 && hayUno!=1){
			if (tablero[fila][columna]==1){
				hayUno=1;
			}
			columna++;
		}
		fila++;
	}
	return hayUno;
}
