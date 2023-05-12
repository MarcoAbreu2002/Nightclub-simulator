#include "util.h"
#include "unix.h"

int sockfd, newsockfd, clilen,servlen,childpid;
struct sockaddr_un cli_addr, serv_addr;

int  opcao = 0;
int simulacao_finalizada = 0;
int auxestado = 0;
int id , zona ,gendercli = 0,minutos ,vips = 0,pessoas_normais_total,pessoas_vip_total, pessoas_normais = 0, pessoas_vip = 0, pessoasTotal = 0, desistencias = 0,tempMedio = 0,pessoas_normais_fila = 0, pessoas_vip_fila = 0,dinheiro = 0;

void trata_cliente_stream(int sockfd){
        int Msg_Rec = 0;
        char buffer[MAXLINE + 1];

    	while(simulacao_finalizada == 0){
		Msg_Rec = read(sockfd, buffer, MAXLINE+1);
		if(Msg_Rec==0){
			break;
		}
		else if(Msg_Rec < 0){
                	perror("Erro na socket recebida\n");
        	}
		else{
			trata_mensagem(buffer);
		}
	}
}

void trata_mensagem(char buffer[]){
	char auxbuff[90];
	strcpy(auxbuff, buffer);
	int gender = 0;
	int estado = 0;
	int ids = 0;
	int zonas = 0;
	int minuto = 0;
	int vip = 0;
	int tmpmedio = 0;
	sscanf(auxbuff,"%d %d %d %d %d %d %d", &estado, &ids, &zonas, &minuto, &vip, &tmpmedio, &gender);
	switch(estado)
	{
		case 1://chegada de cliente
			auxestado = estado;
			id = ids;
			zona = zonas;
			minutos = minuto;
			vips = vip;
			tempMedio = tmpmedio;
			gendercli = gender;
			switch(vip){
				case 0:
					pessoas_normais_fila++;
					break;
				case 1:
					pessoas_vip_fila++;
					break;
			}
			break;
		case 2://entrada na disco
			auxestado = estado;
			id = ids;
			zona = zonas;
			minutos = minuto;
			vips = vip;
			gendercli = gender;
			switch(vip){
				case 0:
					pessoas_normais++;
					pessoas_normais_total++;
					pessoas_normais_fila--;
					pessoasTotal++;
					break;
				case 1:
					pessoas_vip++;
					pessoas_vip_total++;
					pessoas_vip_fila--;
					pessoasTotal++;
					break;
			}
			break;
		case 3://desistencia
                        auxestado = estado;
                        id = ids;
                        zona = zonas;
                        minutos = minuto;
                        vips = vip;
			gendercli = gender;
                        switch(vip){
                                case 0:
				        pessoas_normais_fila--;
					desistencias++;
                                        break;
                                case 1:
				        pessoas_vip_fila--;
					desistencias++;
                                        break;
                        }
                        break;
		case 4://saida disco
                        auxestado = estado;
                        id = ids;
                        zona = zonas;
                        minutos = minuto;
                        vips = vip;
			gendercli = gender;
                        switch(vip){
                                case 0:
                                        pessoas_normais--;
                                        break;
                                case 1:
                                        pessoas_vip--;
                                        break;
                        }
                        break;

		case 5://espera na fila
                        auxestado = estado;
                        id = ids;
                        zona = zonas;
                        minutos = minuto;
                        vips = vip;
			gendercli = gender;
                        break;

		case 6://mudar de zona
                        auxestado = estado;
                        id = ids;
                        zona = zonas;
                        minutos = minuto;
                        vips = vip;
			gendercli = gender;
                        break;

		case 7://fecha disco
			auxestado = estado;
			minutos = minuto;
			break;
	}

	criarReport();
}



void  escrever(){
        FILE *filePointer;
        filePointer = fopen("log.txt" , "a");
        if(filePointer == NULL){
                printf("Nao foi possivel abrir o ficheiro!\n");
        }
        else{
		fprintf(filePointer,"********************************************\n");
                fprintf(filePointer,"Estado atual => Simulacao a decorrer!\n");
		fprintf(filePointer,"Total de pessoas normais: %d\n",pessoas_normais_total);
		fprintf(filePointer,"Total de pessoas vip: %d\n",pessoas_vip_total);
		fprintf(filePointer,"Total de pessoas: %d\n",pessoasTotal);
		fprintf(filePointer,"Numero total de desistencias: %d\n",desistencias);
                fprintf(filePointer,"Estado atual => Simulacao terminada!\n");
                fprintf(filePointer,"********************************************\n\n");

                if(fclose(filePointer) == EOF){
                        printf("ERRO ao fechar o ficheiro\n");
               }
        }
}


void criarReport(){
	if(simulacao_finalizada == 0){
	//	printf("Estado atual => Simulacao a decorrer!\n");
	}
	int a = pessoas_normais + pessoas_vip;
/*	printf("\n\n*********Informacao Geral da Simualacao**********\n\n");
	printf("Numero de pessoas totais: %d\n", pessoasTotal);
	printf("Numero de pessoas totais dentro da disco %d\n", a);
	printf("Numero de pessoas normais na fila normal: %d\n", pessoas_normais_fila);
	printf("Tempo medio de espera: %d\n", tempMedio);
	printf("Numero total de desistencias: %d\n\n", desistencias);
	printf("**********Informacoes detalhadas**********\n\n");*/
	if(auxestado==1){
		switch(vips){
			case 0:
				//pessoas_normais_total = pessoas_normais_total + 1;
				printf("Chegou um utilizador normal de genero %d com  numero %d na fila\n",gendercli, id);
				printf("--------------------------------------------------------\n");
				break;
			case 1:
				//pessoas_vip_total = pessoas_vip_total + 1;
				printf("Chegou um utilizador vip com numero %d na fila\n",id);
                                printf("--------------------------------------------------------\n");
				break;
		}
	}
	if(auxestado==2){
		switch(vips){
                        case 0:
								switch(zona){
									case 1: 
											printf("O utilizador numero %d de gender %d entrou na FILA DA PISTA NORMAL\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 2:
											printf("O utilizador numero %d de gender %d entrou na PISTA NORMAL\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 3:
											printf("O utilizador numero %d de gender %d entrou na FILA DO BAR\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 4:
											printf("O utilizador numero %d de gender %d entrou no BAR\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 5:
											printf("O utilizador numero %d de gender %d entrou na FILA DO WC\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 6:
											printf("O utilizador numero %d de gender %d entrou no WC\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 7: 
											printf("O utilizador numero %d de gender %d entrou na FILA DA ZONA VIP\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
									case 8:
											printf("O utilizador numero %d de gender %d entrou na ZONA VIP\n", id,gendercli);
											printf("--------------------------------------------------------\n");
											break;
								}
								break;
								
                        case 1:
                                switch(zona){
									case 1: 
											printf("O utilizador VIP numero %d entrou na FILA DA PISTA NORMAL\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 2:
											printf("O utilizador VIP numero %d entrou na PISTA NORMAL\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 3:
											printf("O utilizador VIP numero %d entrou na FILA DO BAR\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 4:
											printf("O utilizador VIP numero %d entrou no BAR\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 5:
											printf("O utilizador VIP numero %d entrou na FILA DO WC\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 6:
											printf("O utilizador VIP numero %d entrou no WC\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 7: 
											printf("O utilizador VIP numero %d entrou na FILA DA ZONA VIP\n", id);
											printf("--------------------------------------------------------\n");
											break;
									case 8:
											printf("O utilizador VIP numero %d entrou na ZONA VIP\n", id);
											printf("--------------------------------------------------------\n");
											break;
								}
								break;
                }

	}
	if(auxestado==3){
		switch(vips){
					case 0:
								switch(zona){
									case 1: 
											printf("O utilizador numero %d de gender %d desistiu da FILA  apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 3:
											printf("O utilizador numero %d de gender %d desistiu da FILA DO BAR apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 5:
											printf("O utilizador numero %d de gender %d desistiu da FILA DO WC apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 7: 
											printf("O utilizador numero %d de gender %d desistiu da FILA DA ZONA VIP apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
								}
								break;
								
                        case 1:
                                switch(zona){
									case 1: 
											printf("O utilizador VIP numero %d de gender %d desistiu da FILA  apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 3:
											printf("O utilizador VIP numero %d de gender %d desistiu da FILA DO BAR apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 5:
											printf("O utilizador VIP numero %d de gender %d desistiu da FILA DO WC apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
									case 7: 
											printf("O utilizador VIP numero %d de gender %d desistiu da FILA DA ZONA VIP apos %d minutos\n", id,gendercli, minutos);
											printf("--------------------------------------------------------\n");
											break;
								}
								break;
				
                }
	}

	if(auxestado == 4){
		switch(vips){
                        case 0:
                                printf("O utilizador numero %d de gender %d saiu da discoteca apos %d minutos\n", id, gendercli,minutos);
                                printf("--------------------------------------------------------\n");
                                break;
                        case 1:
                                printf("O utilizador vip com numero %d de gender %d saiu da discoteca apos %d minutos\n", id,gendercli,minutos);
                                printf("--------------------------------------------------------\n");
                                break;
                }
	}
	if(auxestado == 5){
                switch(vips){
                        case 0:
                                printf("O utilizador numero %d esta na fila h'a %d minutos\n", id, minutos);
                                printf("--------------------------------------------------------\n");
                                break;
                        case 1:
                                printf("O utilizador vip com numero %d esta na fila h'a %d minutos\n", id,minutos);
                                printf("--------------------------------------------------------\n");
                                break;
                }
        }
	if(auxestado == 6){
		switch(zona){
			case 1: 
					printf("O utilizador numero %d de gender %d PASSOU para a zona FILA DA PISTA NORMAL\n", id,gendercli);
					printf("--------------------------------------------------------\n");
					break;
			case 3:	
					printf("O utilizador numero %d de gender %d PASSOU para a zona FILA DO BAR\n", id,gendercli);
					printf("--------------------------------------------------------\n");
					break;
			case 5:
					printf("O utilizador numero %d de gender %d PASSOU para a zona FILA DO WC\n", id,gendercli);
					printf("--------------------------------------------------------\n");
					break;
			case 7: 
					printf("O utilizador numero %d de gender %d PASSOU para a zona FILA DA ZONA VIP\n", id,gendercli);
					printf("--------------------------------------------------------\n");
					break;
								}		
	}

	if(auxestado == 7){
		printf("A discoteca fechou apos %d minutos\n", minutos);
		printf("--Fim da simulacao--\n\n");
                printf("Estado atual => Simulacao finalizada!\n");
                printf("--------------------------------------------------------\n");
		simulacao_finalizada = 1;
	}

}


void servidor(){

	if((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0){
                perror("Monitor: can't open stream socket\n");
	}
        unlink(UNIXSTR_PATH);
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strcpy(serv_addr.sun_path, UNIXSTR_PATH);
        servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

        if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0){
                perror("server, can't bind local address\n");
	}
        if(listen(sockfd, 1) < 0){
                perror("Can't listen monitor\n");
	}
        listen(sockfd, 1);

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0){
                perror("Server not accepted\n");
        }
	if((childpid = fork()) < 0){
                perror("Fork ERROR\n");
	}
        else if(childpid == 0){
                close(sockfd);
		trata_cliente_stream(newsockfd);
        }
        	close(newsockfd);
	}




int main(){
	system("clear");
	printf("             ******************************************************************\n");
	printf("             **                 Simulacao de uma Discoteca                   **\n");
	printf("             ******************************************************************\n");
	printf("                                                                               \n");
	printf("             ******************************************************************\n");
	printf("             **                          Simulacao                           **\n");
	printf("             ******************************************************************\n");
	printf("                                                                               \n");
	printf("             ******************************************************************\n");
	printf("             ** 1 - Simulacao                                                **\n");
	printf("             ** 2 - Sair                                                     **\n");
	printf("             **                                                              **\n");
	printf("             **                                                              **\n");
	printf("             **                                                              **\n");
	printf("             ******************************************************************\n");
	printf("                                                                               \n");

	int choice = 0;
	int finish = 0;
	while(simulacao_finalizada == 0){
		while(choice !=1 ){
			printf("Escolha uma opcao: \n");
			scanf("%d", &choice);
			if(choice !=1){
				printf("Erro na simulacao!\n");
				opcao = 0;
			}
		}
		servidor();
	}
	escrever();
	close(newsockfd);
	exit(0);
        return 0;
}
