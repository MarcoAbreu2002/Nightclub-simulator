#include "util.h"
#include "unix.h"

int total_clientes, tempo_de_espera, tempo_total, capacidade_fila, tmpmedio,tmpmedioVip, capacidade_zona, frequencia,probDesistencia;capacidade_total;
int id , zona ,minutos ,vips = 0,pessoas_normais_total= 0,pessoas_vip_total= 0, pessoas_normais = 0, pessoas_vip = 0, pessoasTotal = 0, desistencias = 0,tempMedio = 0,pessoas_normais_fila = 0, pessoas_vip_fila = 0,dinheiro = 0;
int sockfd, servlen;
struct sockaddr_un serv_addr;
time_t start;
int disco_closed = 0;



void leitura(){
	FILE* filePointer;
	char linha[50];
	char* palavra;

	filePointer = fopen("simulador-conf.txt", "r");
	if(filePointer == NULL){
		printf("Nao foi possivel abrir o ficheiro!\n");
	}else{
		printf("Ficheiro Aberto!\n");
		while(fgets(linha,sizeof(linha), filePointer) != NULL){
			palavra = strtok(linha, ":");    //strtok usado para encontrar padrões neste caso encontrar em linha ":"
			if(strcmp(palavra, "TEMPO_DE_ESPERA") == 0){	//strcmp comparar o valor em palavra com "TEMPO_DE_ESPERA"
				palavra = strtok(NULL, ":");
				tempo_de_espera = atoi(palavra);	// atoi converte o valor de palavra, uma string, para um integer
			}
			if(strcmp(palavra, "TEMPO_TOTAL") == 0){
				palavra = strtok(NULL, ":");
				tempo_total = atoi(palavra);
			}
			if(strcmp(palavra, "CAPACIDADE_TOTAL") == 0){
				palavra = strtok(NULL, ":");
				capacidade_total = atoi(palavra);
			}
			if(strcmp(palavra, "CAPACIDADE_ZONA") == 0){
				palavra = strtok(NULL, ":");
				capacidade_zona = atoi(palavra);
			}
			if(strcmp(palavra, "DESISTIR_PROB") == 0){
				palavra = strtok(NULL, ":");
				tmpmedio = atoi(palavra);
			}
			if(strcmp(palavra,"FREQUENCIA") == 0){
				palavra = strtok(NULL, ":");
				frequencia = atoi(palavra);
			}
	}

	fclose(filePointer);
}

}
typedef struct{
	//carateristicas do cliente
	int id;
	int gender; // female == 1 && male == 0
	int estado;
	int vip;
	int desistiu;
	int zona; //zona onde o cliente se encontra -> 0 = fila entrada; 1 = fila Pista N ; 2 = Pista N ; 3 = Fila bar; 4 = bar ; 5 = fila wc; 6 = wc; 7 = fila zona vip; 8 = zona vip; 9 = sair da discoteca; 10 = desistiu - ja nao existe
	//declaracao de tempos
	int minutos;
	int probDesistencia;
	int tmpmedio;
	int instante_fila_entrada;
	int instante_fila_pistaN;
	int instante_fila_bar;
	int instante_wc;
	int instante_fila_vip;
	int instante_que_entrou_na_zona;
	int MaxTimeFila;
	int MaxTimeZona;
	//declaracao de posicoes na fila
	int posicao_entrada;
	int posicao_pista_N;
	int posicao_bar;
	int posicao_wc;
	int posicao_pista_Vip;
	int posicao_vip_espera;
	int dentro_wc;

}buffer_c;

	int total_clientes_criados = 0;
	int tempo_sim = 0;

	//array clientes
	buffer_c Cliente[9999];

	//trincos
	pthread_mutex_t trinco_comunicacao; //cria um cliente de cada vez
	pthread_mutex_t trinco_wc; //exclusao mutua no wc
	pthread_mutex_t trinco_vip; //exclusao mutua na mesa de autografos
	//semaforos
	sem_t fila_entrada; //lotacao da entrada
	sem_t pode_entrar;   //semaforo pode entrar 2 a 2 na disco clientes normais
	sem_t pode_entrar_bar;	//lotacao bar
	sem_t fila_zona2;    //lotacao pista de dança
	sem_t fila_zona_vip;   //lotacao da zona vip
	//Threads
	pthread_t simulacao;
	pthread_t Clientes_Tarefas[9999];
	//variaveis filas
	int clientes_na_fila_entrada = 0;
	int clientes_na_fila_pista = 0;
	int clientes_na_fila_bar = 0;
	int clientes_na_fila_pistaVip = 0;
	int clientes_vip_na_fila_pistaVip = 0;
	int clientes_na_fila_wc = 0;

	int clientes_na_pista_N = 0;
	int clientes_no_bar = 0;
	int clientes_na_pista_VIP = 0;
	int vips_na_pista_vip = 0;

	int vaiEntrar = 25;
	int id1;
	int id2;

//:::::::::::: Fila de espera ::::::::::::::::::
void estabelece_prioridade_entrada(int id){
	if(clientes_na_fila_entrada == 0){
		Cliente[id].posicao_entrada = 1;
		return;
	}else{
		if(Cliente[id].gender == 1){//female customer arrived
			for(int i = 0;i<total_clientes_criados;i++){
				if(Cliente[i].gender == 0 && Cliente[i].posicao_entrada == 1){//homem encontra-se na entrada
						Cliente[i].posicao_entrada = 1;
						Cliente[id].posicao_entrada = 1;
						for(int x = 0; x<total_clientes_criados;x++){
							Cliente[x].posicao_entrada = Cliente[x].posicao_entrada + 1;
						}
						return;
				}

				if(Cliente[i].gender != 0 && Cliente[i].posicao_entrada == 1){
					for(int l = 0;l<total_clientes_criados;l++){
						if(Cliente[l].gender == 1 && Cliente[l].posicao_entrada == 1){//mulher encontra-se na entrada
							for(int m = l;m<total_clientes_criados;m++){
								while(Cliente[m].gender == 1 ){
									m = m + 1;
									if(Cliente[m].gender != 1){
										int k = m;
										int valor = Cliente[k].posicao_entrada;
										Cliente[id].posicao_entrada = valor;

										for(int k = 0; k<total_clientes_criados;k++){
											if(Cliente[k].posicao_entrada != 1 && Cliente[k].gender != 1){
                                                               					Cliente[k].posicao_entrada = Cliente[k].posicao_entrada + 1;
											}
										}
										return;

									}
								}
							}
						}
					}
				}
}
		}else{//male customer arrived
			Cliente[id].posicao_entrada = total_clientes_criados;
			return;
		}
	}

}

void estabele_prioridade_bar(int id){
	if(clientes_na_fila_bar == 0){
		Cliente[id].posicao_entrada = 1;
		return;
	}else{
		if(Cliente[id].vip == 1){
			for(int i = 0;i<clientes_na_fila_bar;i++){
				if(Cliente[i].vip == 0 && Cliente[i].posicao_bar == 1){
					Cliente[id].posicao_bar = 1;
					Cliente[i].posicao_bar = 1;
					for(int k = 0; k<clientes_na_fila_bar;k++){
						Cliente[k].posicao_bar = Cliente[k].posicao_bar + 1;
					}
				}else if(Cliente[i].vip == 1 && Cliente[i].posicao_bar == 1){
					for(int x = i; x<clientes_na_fila_bar;x++){
						while(Cliente[x].vip == 1){
							x = x + 1;
							if(Cliente[x].vip == 0){
								int a = x;
								int valorvip = Cliente[a].posicao_bar;
								Cliente[id].posicao_bar = valorvip;
								for(int b = 0; b < clientes_na_fila_bar; b++){
									if(Cliente[b].posicao_bar >= valorvip){
										Cliente[b].posicao_bar = Cliente[b].posicao_bar + 1;
									}
								}
							}
						}
					}

				}
			}
		}else if(Cliente[id].vip == 0 && Cliente[id].gender == 1){
			for(int c = 0; c < clientes_na_fila_bar;c++){
				if(Cliente[c].posicao_bar == 1 && Cliente[c].vip == 0 && Cliente[c].gender == 0){
					Cliente[id].posicao_bar = 0;
					Cliente[c].posicao_bar = 1;
					for(int d = 0; d < clientes_na_fila_bar; d++){
						Cliente[d].posicao_bar = Cliente[d].posicao_bar + 1;
					}
				}else if(Cliente[c].posicao_bar == 1 && Cliente[c].vip == 0 && Cliente[c].gender == 1){
					for(int e = 0; e < clientes_na_fila_bar; e++){
						while(Cliente[e].gender == 1){
							e = e + 1;
							if(Cliente[e].gender == 0){
								int f = e;
								int valorFemale = Cliente[f].posicao_bar;
								Cliente[id].posicao_bar = valorFemale;
								for(int g = 0; g < clientes_na_fila_bar;g++){
									if(Cliente[g].posicao_bar >= valorFemale){
										Cliente[g].posicao_bar = Cliente[g].posicao_bar + 1;
									}
								}
							}
						}
					}
				}else if(Cliente[c].posicao_bar == 1 && Cliente[c].vip == 1){
					for(int h = 0; h < clientes_na_fila_bar; h++){
						while(Cliente[h].vip == 1 || Cliente[h].gender == 1){
							h = h + 1;
							if(Cliente[h].vip == 0 && Cliente[h].gender == 0){
								int m = h;
								int valorFemaleAposVip = Cliente[m].posicao_bar;
								Cliente[id].posicao_bar = valorFemaleAposVip;
								for(int n = 0; n < clientes_na_fila_bar; n++){
									if(Cliente[n].posicao_bar >= valorFemaleAposVip){
										Cliente[n].posicao_bar = Cliente[n].posicao_bar + 1;
									}
								}
							}
						}
					}
				}
			}

		}else if(Cliente[id].vip == 0 && Cliente[id].gender == 0){
			Cliente[id].posicao_bar = clientes_na_fila_bar;
		}

	}
}


void avanca_fila_dps_desistir(int id){
	for(int i = 0;i<total_clientes_criados;i++){
		if(Cliente[i].posicao_entrada > 1){
			Cliente[i].posicao_entrada = Cliente[i].posicao_entrada - 1;
		}
	}
	Cliente[id].posicao_entrada = -1;
	Cliente[id].zona = 10;
	total_clientes_criados--;
}

void entra_na_disco_VIP(int id_gerado){
        sleep(1);
        clientes_na_fila_pista = clientes_na_fila_pista + 1;
        Cliente[id_gerado].posicao_pista_N = clientes_na_fila_pista ;
		Cliente[id_gerado].zona = 1;
		Cliente[id_gerado].posicao_entrada = -1;
}

void entra_na_disco(int id_gerado, int vez){
	if(vez == 0){//primeira vez que entra na disco	
				Cliente[id_gerado].instante_fila_pistaN = tempo_sim;
				clientes_na_fila_entrada = clientes_na_fila_entrada - 1;
				clientes_na_fila_pista = clientes_na_fila_pista + 1;
				criaMensagem(2,Cliente[id_gerado].id,1,Cliente[id_gerado].instante_fila_entrada,Cliente[id_gerado].vip,Cliente[id_gerado].gender);
				Cliente[id_gerado].zona = 1;
				Cliente[id_gerado].posicao_entrada = -1;
				Cliente[id_gerado].posicao_pista_N = clientes_na_fila_pista;	
		for(int k = 0; k<=total_clientes_criados;k++){
			if(Cliente[k].posicao_entrada > 2 ){
				Cliente[k].posicao_entrada = Cliente[k].posicao_entrada - 1;
			}
		}
		
	}else if(vez==1){//voltou novamente à pista de dança
		Cliente[id_gerado].instante_fila_pistaN = tempo_sim;
		clientes_na_fila_pista = clientes_na_fila_pista + 1;
		Cliente[id_gerado].zona = 1;
		criaMensagem(2,Cliente[id_gerado].id,1,Cliente[id_gerado].instante_fila_entrada,Cliente[id_gerado].vip,Cliente[id_gerado].gender);
		sleep(1);
		Cliente[id_gerado].posicao_pista_N = clientes_na_fila_pista;

	}
}

int define_gender_random(){
	int gender_obtido = rand() % 2;
	return gender_obtido;
}

int define_vip_random(){
	int vip_obtido = rand() % 10;
	int x;
	if(vip_obtido == 8 || vip_obtido == 2 ){
		x=1;
	}else{
		x=0;
	}
	return x;
}

int define_MaxTimeZona(){
	int time = (rand() % 30) + 1;
	return time;
}

int checkDesistencia(){
	int decisao = rand() % tmpmedio;
	int y;
	if(decisao == 3){
		y = 1;
	}else{
		y = 0;
	}
	return y;
}

void desistiu_fila_pista_N(int id_recebido){
	for(int i = 0;i<clientes_na_fila_pista;i++){
		if(Cliente[i].zona == 1 && Cliente[i].posicao_pista_N > Cliente[id_recebido].posicao_pista_N){//encontra-se na fila da pista de dança Normal
/*			for(int j = i;j<clientes_na_fila_pista;j++){
				if(Cliente[j].posicao_pista_N > Cliente[id_recebido].posicao_pista_N){
                        		Cliente[id_recebido] = Cliente[j];
                		}
			}*/
			Cliente[i].posicao_pista_N = Cliente[i].posicao_pista_N - 1;
		}
	}
	Cliente[id_recebido].posicao_pista_N = 0;
}



void desistiu_fila_bar(int id_recebido){
        for(int i = 0;i<clientes_na_fila_bar;i++){
                if(Cliente[i].zona == 3 && Cliente[i].posicao_bar > Cliente[id_recebido].posicao_bar){//encontra-se na fila do bar
/*                        for(int j = i;j<clientes_na_fila_bar;j++){
                                if(Cliente[j].posicao_bar > Cliente[id_recebido].posicao_bar){
                                        Cliente[id_recebido] = Cliente[j];
                                }
                        }*/
                        Cliente[i].posicao_bar = Cliente[i].posicao_bar - 1;
                }
        }
		Cliente[id_recebido].posicao_bar = 0;
}

int random_zona(){
	int myArray[5] = {1,3,5,7,9};//valores das zonas possiveis que pode ir
	int randomIndex = rand() % 6;
	int randomZona = myArray[randomIndex];
	return randomZona;
}

void entra_na_pista_N(int id_recebido){
	clientes_na_pista_N++;
	clientes_na_fila_pista--;
	//criarMensagem();
	for(int k = 0;k<clientes_na_fila_pista;k++){
		if(Cliente[k].zona == 1 && Cliente[k].posicao_pista_N > 1){//verifica se estao na fila da pista
			Cliente[k].posicao_pista_N = Cliente[k].posicao_pista_N - 1;
		}
	}
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id_recebido].posicao_pista_N = 0;//sai da fila
	Cliente[id_recebido].zona = 2;
}

void entra_fila_bar(int id_recebido){
	clientes_na_fila_bar++;
	Cliente[id_recebido].posicao_bar = clientes_na_fila_bar;
	criaMensagem(2,Cliente[id].id,3,0,Cliente[id].vip,Cliente[id].gender);
	Cliente[id_recebido].zona = 3;
}

void entra_no_bar(int id_recebido){
	clientes_no_bar++;
	clientes_na_fila_bar--;
	for(int k = 0;k<clientes_na_fila_bar;k++){
		if(Cliente[k].zona == 3 && Cliente[k].posicao_bar > 1){//verifica se estao na fila do bar
			Cliente[k].posicao_bar = Cliente[k].posicao_bar - 1;
		}
	}
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id_recebido].posicao_bar = 0;
	Cliente[id_recebido].zona = 4;
}

void entra_fila_wc(int id_recebido){
	clientes_na_fila_wc++;
	Cliente[id_recebido].posicao_wc = clientes_na_fila_wc;
	sleep(1);
	Cliente[id].dentro_wc == 0;
	Cliente[id_recebido].zona = 5;
}

void entra_no_wc(int id_recebido){
	clientes_na_fila_wc--;
	for(int k = 0;k<clientes_na_fila_wc;k++){
		if(Cliente[k].zona == 5 && Cliente[k].posicao_wc > 1){//verifica se estao na fila do bar
			Cliente[k].posicao_wc = Cliente[k].posicao_wc - 1;
		}
	}
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id].dentro_wc == 1;
	Cliente[id_recebido].zona = 6;
}

void sair_da_disco(int id_recebido){
	//printf("Cliente vai sair da disco");
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id_recebido].zona = 9;
}

int getFirstElement(){
	int id;
	for(int x = 0;x<total_clientes_criados;x++){
		if(Cliente[x].posicao_entrada == 1 && Cliente[x].zona == 0){
			id = x;
		}
	}
	return id;
}

int getSecondElement(){
	int id;
        for(int x = 0;x<total_clientes_criados;x++){
            if(Cliente[x].posicao_entrada == 2 && Cliente[x].zona == 0){
                id = x;
            }
        }
        return id;
}

void entra_vip_zonaVip(int id_recebido){
	vips_na_pista_vip = 1;
	clientes_vip_na_fila_pistaVip--;
	for(int k = 0;k<clientes_vip_na_fila_pistaVip;k++){
		if(Cliente[k].zona == 7 && Cliente[k].posicao_vip_espera > 1){//verifica se estao na fila da pista
			Cliente[k].posicao_vip_espera = Cliente[k].posicao_vip_espera - 1;
		}
	}
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id_recebido].posicao_vip_espera = 0;//sai da fila
	Cliente[id_recebido].zona = 8;
}

void entra_fila_zona_vip_vip(int id_recebido){
	clientes_vip_na_fila_pistaVip++;
	Cliente[id_recebido].posicao_vip_espera = clientes_vip_na_fila_pistaVip;
	Cliente[id_recebido].zona = 7;
	
}

void desistiu_fila_vip_vip(int id_recebido){
	for(int i = 0;i<clientes_vip_na_fila_pistaVip;i++){
                if(Cliente[i].zona == 7 && Cliente[i].posicao_vip_espera > Cliente[id].posicao_vip_espera){//encontra-se na fila do bar
                        Cliente[i].posicao_vip_espera = Cliente[i].posicao_vip_espera - 1;
                }
        }
	Cliente[id_recebido].posicao_vip_espera = 0;
}

void desistiu_fila_vip_N(int id_recebido){
	for(int i = 0;i<clientes_na_fila_pistaVip;i++){
                if(Cliente[i].zona == 7 && Cliente[i].posicao_pista_Vip > Cliente[id].posicao_pista_Vip ){//encontra-se na fila do bar
                        Cliente[i].posicao_pista_Vip = Cliente[i].posicao_pista_Vip - 1;
                }
        }
	Cliente[id_recebido].posicao_pista_Vip = 0;
}

void entra_na_zona_vip_N(int id_recebido){
	clientes_na_pista_VIP = clientes_na_pista_VIP + 1;
	clientes_na_fila_pistaVip--;
	for(int k = 0;k<clientes_na_fila_pistaVip;k++){
		if(Cliente[k].zona == 7 && Cliente[k].posicao_pista_Vip > 1){//verifica se estao na fila da pista
			Cliente[k].posicao_pista_Vip = Cliente[k].posicao_pista_Vip - 1;
		}
	}
	Cliente[id_recebido].instante_que_entrou_na_zona = tempo_sim;
	Cliente[id_recebido].posicao_pista_Vip = 0;//sai da fila
	Cliente[id_recebido].zona = 8;
	
}

void entra_fila_vip_N(int id_recebido){
	clientes_na_fila_pistaVip++;
	Cliente[id_recebido].posicao_pista_Vip = clientes_na_fila_pistaVip;
	Cliente[id_recebido].zona = 7;
}



void Event_Manager(int id_gerado){

				//////////////////////////////////////////////////////////////////////////////
 			       //			    define cliente criado			   //
			       //////////////////////////////////////////////////////////////////////////////
				   
pthread_mutex_lock(&trinco_comunicacao);
//sleep(1);
	Cliente[id_gerado].id = id_gerado;
	Cliente[id_gerado].gender = define_gender_random();
	Cliente[id_gerado].vip = define_vip_random();
	Cliente[id_gerado].probDesistencia = tmpmedio ;
	Cliente[id_gerado].MaxTimeFila = tempo_de_espera;
	Cliente[id_gerado].posicao_entrada = 0;
	Cliente[id_gerado].MaxTimeZona = define_MaxTimeZona();

	if(Cliente[id_gerado].vip == 0){
		Cliente[id_gerado].desistiu = checkDesistencia();
		Cliente[id_gerado].zona = 0;
	}else{
		Cliente[id_gerado].desistiu = 0;
		Cliente[id_gerado].zona = 1;
	}
	pthread_mutex_unlock(&trinco_comunicacao);

/*
                        printf("------------------------------------------------------------------\n");
                        printf("ID do cliente gerado: %d\n",Cliente[id_gerado].id);
                        printf("Gender do cliente gerado: %d\n",Cliente[id_gerado].gender);
                        printf("Vip do cliente gerado: %d\n",Cliente[id_gerado].vip);
                        printf("Prob desistir do cliente gerado: %d\n",Cliente[id_gerado].probDesistencia);
                        printf("Tempo maximo espera do cliente gerado: %d\n",Cliente[id_gerado].MaxTimeFila);
			printf("Zona atribuida ao cliente: %d\n",Cliente[id_gerado].zona);
                        printf("-------------------------------------------------------------------\n");

*/


				//////////////////////////////////////////////////////////////////////////////
				//		Clientes sao colocados na fila de entrada		   //
 				/////////////////////////////////////////////////////////////////////////////

		if(Cliente[id_gerado].vip == 0 && Cliente[id_gerado].posicao_entrada == 0/*Ainda nao está na fila*/){
			pthread_mutex_lock(&trinco_comunicacao);
			id1 = getFirstElement();
			id2 = getSecondElement();
			//sleep(1);
			criaMensagem(1,Cliente[id_gerado].id,1,Cliente[id_gerado].instante_fila_entrada, Cliente[id_gerado].vip,Cliente[id_gerado].gender);
			//sleep(1);
			estabelece_prioridade_entrada(id_gerado);
			pthread_mutex_unlock(&trinco_comunicacao);
		}else if(Cliente[id_gerado].vip == 1){

			        //////////////////////////////////////////////////////////////////////////////
                                //              Clientes vip entram diretamente na disco                    //
                                /////////////////////////////////////////////////////////////////////////////

			pthread_mutex_lock(&trinco_comunicacao);
			sleep(1);
			Cliente[id].instante_fila_pistaN = tempo_sim;
			criaMensagem(2,Cliente[id_gerado].id,1,Cliente[id_gerado].instante_fila_entrada,Cliente[id_gerado].vip,Cliente[id_gerado].gender);
			sleep(1);
			entra_na_disco_VIP(id_gerado);
			pthread_mutex_unlock(&trinco_comunicacao);
		}


		for(int id = 0; id<total_clientes_criados;id++){ //atualização dos restantes que já estão em simulação


                                /////////////////////////////////////////////////////////////////////////////
                                //              Verifica Desistencia na fila de entrada                   //
                                ////////////////////////////////////////////////////////////////////////////

			if(Cliente[id].zona == 0 && Cliente[id].desistiu == 1 && tempo_sim > (Cliente[id].MaxTimeFila + Cliente[id].instante_fila_entrada) && Cliente[id].posicao_entrada > 0){//Se o cliente deseja desister se ja passou o seu tempo maximo de espera e se esta na fila de entrada
				pthread_mutex_lock(&trinco_comunicacao);
				sleep(1);
				criaMensagem(3,Cliente[id].id,1,tempo_sim - Cliente[id].instante_fila_entrada,Cliente[id].vip,Cliente[id].gender);
				sleep(1);
				avanca_fila_dps_desistir(id);
				pthread_mutex_unlock(&trinco_comunicacao);
			}

                                //////////////////////////////////////////////////////////////////////////////
                                //              Clientes estao prontos para entrar na disco                 //
                                /////////////////////////////////////////////////////////////////////////////
		if(Cliente[id].zona == 0){
			if(tempo_sim > vaiEntrar){
					int sem_entrar;
	       			sem_wait(&pode_entrar);
					sem_getvalue(&pode_entrar,&sem_entrar);
					printf("VALOR: %d\n",sem_entrar);
					if(sem_entrar == 0){
						vaiEntrar = vaiEntrar + 35;
			   	  		pthread_mutex_lock(&trinco_comunicacao);
						sleep(1);
						entra_na_disco(id1,0);//entra segundo cliente na fila
						sleep(1);
						entra_na_disco(id2,0);
						sem_post(&pode_entrar);
						sem_post(&pode_entrar);
				        pthread_mutex_unlock(&trinco_comunicacao);
					
					}
		}
		}


				//////////////////////////////////////////////////////////////////////////////
                                //              Clientes na fila da pista de dança normal                  //
                                /////////////////////////////////////////////////////////////////////////////

				if(Cliente[id].zona == 1 && Cliente[id].id > 0 && Cliente[id].posicao_entrada == -1){

                                		//////////////////////////////////////////////////////////////////////////////
                                		//              Clientes entram na pista de dança normal                   //
                                		/////////////////////////////////////////////////////////////////////////////

					int sem_value;
					sem_getvalue(&fila_zona2,&sem_value);
					if(sem_value == 0){
						Cliente[id].desistiu = 0;
						sleep(1);
						Cliente[id].desistiu = checkDesistencia();
						if(Cliente[id].desistiu == 0){
							//printf("Espera na fila da pista\n");
						}else if(Cliente[id].desistiu == 1 && Cliente[id].posicao_pista_N != 0){

                      	       	       /////////////////////////////////////////////////////////////////////////////
                                       //             Clientes na fila da pista de dança normal desiste           //
                                       /////////////////////////////////////////////////////////////////////////////

                	                        	pthread_mutex_lock(&trinco_comunicacao);
                        	                	sleep(1);
                                	        	criaMensagem(3,Cliente[id].id,1,tempo_sim - Cliente[id].instante_fila_pistaN,Cliente[id].vip,Cliente[id].gender);
                              	                	sleep(1);
                                       			desistiu_fila_pista_N(id);
                                      			Cliente[id].zona = random_zona();
							pthread_mutex_unlock(&trinco_comunicacao);

							}
						}
							pthread_mutex_lock(&trinco_comunicacao);
							sem_wait(&fila_zona2);
							sleep(1);
							criaMensagem(2,Cliente[id].id,2,tempo_sim - Cliente[id].instante_fila_pistaN,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							entra_na_pista_N(id);
							pthread_mutex_unlock(&trinco_comunicacao);
				}


                                /////////////////////////////////////////////////////////////////////////////
                                //              Verificar se Cliente Deseja mudar de zona                  //
                                ////////////////////////////////////////////////////////////////////////////

			else if(Cliente[id].zona == 2 || Cliente[id].zona == 4 /*|| Cliente[id].zona == 6 */|| Cliente[id].zona == 8){//Se o cliente está no pistaN no Bar no WC ou na pistaVip, respetivamente
				if(tempo_sim > (Cliente[id].MaxTimeZona + Cliente[id].instante_que_entrou_na_zona)){//já ficou na zona o tempo que desejava
				
					switch(Cliente[id].zona){
						case 2:
								sem_post(&fila_zona2);
								break;
						case 4: 
								sem_post(&pode_entrar_bar);
								break;
						case 8:
								sem_post(&fila_zona_vip);
								break;
						default: 
								break;
						
					}
					
					int zonaDestino = random_zona();
					if(zonaDestino == (Cliente[id].zona - 1)){//prevenir ir para a zona que ja estava
						zonaDestino = random_zona();
					}else{
						switch(zonaDestino){
							case 1://pista de dança
							    pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(6,Cliente[id].id,1,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								Cliente[id].zona = 1;
								entra_na_disco(id,1);
								sleep(1);
								pthread_mutex_unlock(&trinco_comunicacao);
								break;
							case 3://Bar
								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(6,Cliente[id].id,3,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								Cliente[id].zona = 3;
								sleep(1);
								pthread_mutex_unlock(&trinco_comunicacao);
								break;
							case 5://WC
								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(6,Cliente[id].id,5,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								Cliente[id].zona = 5;
								sleep(1);
								pthread_mutex_unlock(&trinco_comunicacao);
								break;
							case 7://ZONA VIP	
								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(6,Cliente[id].id,7,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								Cliente[id].zona = 7;
								sleep(1);
								pthread_mutex_unlock(&trinco_comunicacao);
								break;
							case 9://Sair da disco
								sair_da_disco(Cliente[id].id);
								break;
							default:
								sair_da_disco(Cliente[id].id);
								break;
						}
					}
				}
			}

                                //////////////////////////////////////////////////////////////////////////////
                                //             		   Clientes na fila do bar	                   //
                                /////////////////////////////////////////////////////////////////////////////

			else if(Cliente[id].zona == 3 && Cliente[id].id > 0){
					int sem_value_bar;
					sem_getvalue(&pode_entrar_bar,&sem_value_bar);
					if(sem_value_bar == 0){
						Cliente[id].desistiu = 0;
						sleep(1);
						Cliente[id].desistiu = checkDesistencia();
						if(Cliente[id].desistiu == 0){
							
								//////////////////////////////////////////////////////////////////////////////
                                //              	Clientes na fila do bar	 		           //
                                /////////////////////////////////////////////////////////////////////////////
							
							pthread_mutex_lock(&trinco_comunicacao);	
							sleep(1);
							criaMensagem(2,Cliente[id].id,3,0,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							entra_fila_bar(id);
							pthread_mutex_unlock(&trinco_comunicacao);
						}else if(Cliente[id].desistiu == 1){

										//////////////////////////////////////////////////////////////////////////////
                                		//             	      Clientes desistem da fila do bar                     //
                                		/////////////////////////////////////////////////////////////////////////////

							pthread_mutex_lock(&trinco_comunicacao);
							sleep(1);
							criaMensagem(3,Cliente[id].id,3,tempo_sim - Cliente[id].instante_fila_bar,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							desistiu_fila_bar(id);
							Cliente[id].zona = random_zona();
							pthread_mutex_unlock(&trinco_comunicacao);
						}
					}else{

								//////////////////////////////////////////////////////////////////////////////
                                //             		     Clientes entram no bar 	                    //
                                /////////////////////////////////////////////////////////////////////////////
						
					sem_wait(&pode_entrar_bar);
					pthread_mutex_lock(&trinco_comunicacao);
					sleep(1);
					criaMensagem(2,Cliente[id].id,4,0,Cliente[id].vip,Cliente[id].gender);
					sleep(1);
					entra_no_bar(id);
					pthread_mutex_unlock(&trinco_comunicacao);
				}
			}

			else if(Cliente[id].zona == 5 && Cliente[id].id > 0){

	                        //////////////////////////////////////////////////////////////////////////////
                                //             		    Clientes na fila do wc                         //
                                /////////////////////////////////////////////////////////////////////////////

					//int status = pthread_mutex_trylock(&trinco_wc);
					if(Cliente[id].posicao_wc == 1 && pthread_mutex_trylock(&trinco_wc) == 0 && Cliente[id].dentro_wc == 0){
						pthread_mutex_lock(&trinco_comunicacao);
						entra_no_wc(id);
						sleep(1);
						criaMensagem(2,Cliente[id].id,6,0,Cliente[id].vip,Cliente[id].gender);
						sleep(1);
						pthread_mutex_unlock(&trinco_comunicacao);
						sleep(4);
						if(tempo_sim > (3 + Cliente[id].instante_que_entrou_na_zona)){//tempo que fica no wc
							printf("VOU SAIR\n");
							Cliente[id].posicao_wc = 0;
							Cliente[id].zona = random_zona();
							Cliente[id].dentro_wc == 0;
							pthread_mutex_unlock(&trinco_wc);
						}
					}else{
						Cliente[id].desistiu = 0;
						sleep(1);
						Cliente[id].desistiu = checkDesistencia();
						if(Cliente[id].desistiu == 0){
							pthread_mutex_lock(&trinco_comunicacao);
							sleep(1);
							criaMensagem(2,Cliente[id].id,5,0,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							entra_fila_wc(id);
							pthread_mutex_unlock(&trinco_comunicacao);
						}else if(Cliente[id].desistiu == 1){

						//////////////////////////////////////////////////////////////////////////////
                                		//             	      Clientes desistem da fila do wc                     //
                                		/////////////////////////////////////////////////////////////////////////////

							pthread_mutex_lock(&trinco_comunicacao);
							sleep(1);
							criaMensagem(3,Cliente[id].id,5,tempo_sim - Cliente[id].instante_fila_bar,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							desistiu_fila_bar(id);
							Cliente[id].zona = random_zona();
							pthread_mutex_unlock(&trinco_comunicacao);
						}
						
						
					}
			}
			else if(Cliente[id].zona == 7 && Cliente[id].id > 0){ 
			
								//////////////////////////////////////////////////////////////////////////////
                                //             		    Clientes na vip fila zona vip                          //
                                /////////////////////////////////////////////////////////////////////////////
				
				
				if(Cliente[id].vip == 1){//se é um vip 
					//int statusVip = pthread_mutex_trylock(&trinco_vip);
					if(Cliente[id].posicao_vip_espera == 1 && pthread_mutex_trylock(&trinco_vip) == 0){//se o trinco esta aberto
						printf("EntraVIP\n");
						pthread_mutex_unlock(&trinco_comunicacao);
						entra_vip_zonaVip(id);
						//sleep(1);
						criaMensagem(2,Cliente[id].id,8,tempo_sim - Cliente[id].instante_fila_pistaN,Cliente[id].vip,Cliente[id].gender);
						sleep(1);
						pthread_mutex_unlock(&trinco_comunicacao);
						sleep(Cliente[id].MaxTimeZona + 20);
						//if(tempo_sim > (Cliente[id].MaxTimeZona + Cliente[id].instante_que_entrou_na_zona)){
							printf("vai sair vip\n");
							vips_na_pista_vip = 0;
							Cliente[id].posicao_vip_espera = 0;
							pthread_mutex_unlock(&trinco_vip);
						//}
					}else{
						Cliente[id].desistiu = 0;
						sleep(1);
						Cliente[id].desistiu = checkDesistencia();
						if(Cliente[id].desistiu == 0){
							pthread_mutex_lock(&trinco_comunicacao);
							//sleep(1);
							criaMensagem(2,Cliente[id].id,7,0,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							entra_fila_zona_vip_vip(id);
							pthread_mutex_unlock(&trinco_comunicacao);
						}else if(Cliente[id].desistiu == 1){

						//////////////////////////////////////////////////////////////////////////////
                                		//             	      Clientes desistem da fila da zona vip                    //
                                		/////////////////////////////////////////////////////////////////////////////

							pthread_mutex_lock(&trinco_comunicacao);
							//sleep(1);
							criaMensagem(3,Cliente[id].id,7,tempo_sim - Cliente[id].instante_fila_bar,Cliente[id].vip,Cliente[id].gender);
							sleep(1);
							desistiu_fila_vip_vip(id);
							Cliente[id].zona = random_zona();
							pthread_mutex_unlock(&trinco_comunicacao);
						}
					}
					
				}
				
				else{//cliente normal
					if(vips_na_pista_vip){//se existe um vip na pista
						int sem_value_vip_N;
						sem_getvalue(&fila_zona_vip,&sem_value_vip_N);
						if(sem_value_vip_N == 0){
							Cliente[id].desistiu = 0;
							sleep(1);
							Cliente[id].desistiu = checkDesistencia();
							if(Cliente[id].desistiu == 0){
							
								//////////////////////////////////////////////////////////////////////////////
                                //              	Clientes na fila da zona vip	 		         	   //
                                /////////////////////////////////////////////////////////////////////////////
							
								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(2,Cliente[id].id,7,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								entra_fila_vip_N(id);
								pthread_mutex_unlock(&trinco_comunicacao);
							}else if(Cliente[id].desistiu == 1){

										//////////////////////////////////////////////////////////////////////////////
                                		//             	      Clientes desistem da fila da zona vip                  //
                                		/////////////////////////////////////////////////////////////////////////////

								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(3,Cliente[id].id,7,tempo_sim - Cliente[id].instante_fila_bar,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								desistiu_fila_vip_N(id);
								Cliente[id].zona = random_zona();
								pthread_mutex_unlock(&trinco_comunicacao);
							}
						}else{

								//////////////////////////////////////////////////////////////////////////////
                                //             		     Clientes entram na zona vip	                    //
                                /////////////////////////////////////////////////////////////////////////////
						
					sem_wait(&fila_zona_vip);
					pthread_mutex_lock(&trinco_comunicacao);
					entra_na_zona_vip_N(id);
					sleep(1);
					criaMensagem(2,Cliente[id].id,8,0,Cliente[id].vip,Cliente[id].gender);
					sleep(1);
					pthread_mutex_unlock(&trinco_comunicacao);
				}
					
				}else{//nao tem nenhum vip na zona por isso têm de esperar
					
							Cliente[id].desistiu = 0;
							sleep(1);
							Cliente[id].desistiu = checkDesistencia();
							if(Cliente[id].desistiu == 0){
							
								//////////////////////////////////////////////////////////////////////////////
                                //              	Clientes na fila da zona vip	 		         	   //
                                /////////////////////////////////////////////////////////////////////////////
							
								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(2,Cliente[id].id,7,0,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								entra_fila_bar(id);
								pthread_mutex_unlock(&trinco_comunicacao);
							}else if(Cliente[id].desistiu == 1){

										//////////////////////////////////////////////////////////////////////////////
                                		//             	      Clientes desistem da fila da zona vip                  //
                                		/////////////////////////////////////////////////////////////////////////////

								pthread_mutex_lock(&trinco_comunicacao);
								sleep(1);
								criaMensagem(3,Cliente[id].id,7,tempo_sim - Cliente[id].instante_fila_bar,Cliente[id].vip,Cliente[id].gender);
								sleep(1);
								desistiu_fila_bar(id);
								Cliente[id].zona = random_zona();
								pthread_mutex_unlock(&trinco_comunicacao);
							}
						}	
				}	
			}
			else if(Cliente[id].zona == 9 && Cliente[id].id > 0){
				
					            //////////////////////////////////////////////////////////////////////////////
                                //              		   Clientes vai sair da disco                      //
                                /////////////////////////////////////////////////////////////////////////////
				
				pthread_mutex_lock(&trinco_comunicacao);
				sair_da_disco(id);
				sleep(1);
				criaMensagem(4,Cliente[id].id,3,tempo_sim - Cliente[id].instante_fila_pistaN,Cliente[id].vip,Cliente[id].gender);
				sleep(1);
				Cliente[id].zona = 10;
				pthread_mutex_unlock(&trinco_comunicacao);
				
			}
		}	
}

void cria_cliente(){
	total_clientes = capacidade_total;
	total_clientes_criados = 1;
	srand(time(NULL));
        while(tempo_sim < tempo_total || total_clientes_criados <= total_clientes){
                if(pthread_create(&(Clientes_Tarefas[total_clientes_criados]),NULL,(void *)&Event_Manager,(void *)total_clientes_criados) != 0){
                     printf("Error creating client!\n");
                        exit(1);
                }
                sleep(frequencia);
                total_clientes_criados++;
		clientes_na_fila_entrada++;
		
        }
}

void relogio(){
	while(disco_closed != 1){
		tempo_sim++;
		sleep(1);
	}
}


void envia_stream(char *buff){
	int n;
	char buffer[MAXLINE+1];

	if(strcpy(buffer,buff) !=0 ){
		n = strlen(buffer) + 1;
		if(write(sockfd,buffer,n) != n){
			perror("Erro cliente Write\n");
		}
	}

}


void criaMensagem(int estado, int id, int zona, int minutos, int vip, int gender){
	char buffer[MAXLINE];
	int tempo = tmpmedio;
	if(id > 0){
	switch(estado){
		case 1://chegada do cliente na fila
			sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
			envia_stream(buffer);//partir buffer em strings e mandar separado
			break;
		case 2://entrada do cliente zona 1
			sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
			envia_stream(buffer);
			break;
		case 3://desistencia do cliente
			sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
			envia_stream(buffer);
			break;
		case 4://saida do cliente
			sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
			envia_stream(buffer);
			break;
		case 5://cliente espera na fila
			sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
			envia_stream(buffer);
			break;
		case 6://cliente muda de zona
                        sprintf(buffer,"%d %d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo,gender);
                        envia_stream(buffer);
                        break;
		case 7://disco fechou
			sprintf(buffer,"%d %d %d %d %d %d", estado, id,zona,minutos,vip,tempo);
                        envia_stream(buffer);//partir buffer em strings e mandar separado
			break;
		default:
			printf("Erro estado\n");
	}
	}
}

void startSimulation(){
	if(pthread_create(&(simulacao),NULL,(void *)&relogio,NULL) != 0 ){
		printf("Error clock.\n");
		exit(1);
	}
}




int main(){
	leitura();

	if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		perror("ERROR: Error creating client!\n");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, UNIXSTR_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
		perror("client: can't connect to server\n");

	startSimulation();

	//Inicializacao de semaforos
	pthread_mutex_init(&trinco_wc,NULL);
	pthread_mutex_init(&trinco_comunicacao,NULL);
	sem_init(&pode_entrar, 0, 2);//inicializado a 2 porque a entrada e' feita de duas em duas pessoas
	sem_init(&fila_zona2,0,capacidade_zona + 20);
	sem_init(&fila_zona_vip,0,capacidade_zona);
	sem_init(&pode_entrar_bar,0,5);//capacidade maxima do bar sao 5 pessoas
	cria_cliente();
	sleep(2);
	int cli;
		for(cli = 0; cli < total_clientes_criados; cli++){
			pthread_join(Clientes_Tarefas[cli],NULL);
		}
	sleep(2);
	if(tempo_sim >= tempo_total){
		disco_closed = 1;
		pthread_join(simulacao,NULL);
		printf("Acabou Join Thread do Ciclo do Tempo de Simulacao.\n");
		sleep(2);

		//Mensagem de Fim de Simulacao
		pthread_mutex_lock(&trinco_comunicacao);

        	criaMensagem(7,0,0,tempo_sim,0,0);
		printf("** Fim de Simulacao **\n");
		pthread_mutex_unlock(&trinco_comunicacao);
		sleep(2);
	}
	close(sockfd);
	exit(0);
	return 0;
}
