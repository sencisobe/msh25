/*-
 * main.c
 * Minishell C source
 * Shows how to use "obtain_order" input interface function.
 *
 * Copyright (c) 1993-2002-2019, Francisco Rosales <frosal@fi.upm.es>
 * Todos los derechos reservados.
 *
 * Publicado bajo Licencia de Proyecto Educativo Práctico
 * <http://laurel.datsi.fi.upm.es/~ssoo/LICENCIA/LPEP>
 *
 * Queda prohibida la difusión total o parcial por cualquier
 * medio del material entregado al alumno para la realización
 * de este proyecto o de cualquier material derivado de este,
 * incluyendo la solución particular que desarrolle el alumno.
 *
 * DO NOT MODIFY ANYTHING OVER THIS LINE
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h>			/* NULL */
#include <stdio.h>			/* setbuf, printf */
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h> 
#include <string.h>
#include <fcntl.h> /*creat*/
#include <sys/wait.h> /*waitpid*/
#include <stdlib.h> 
#include <sys/stat.h>
#include <sys/resource.h>

extern int obtain_order();		/* See parser.y for description */

int status=0;

void guardar_descriptores(int saved[3]) {
    saved[0] = dup(STDIN_FILENO);
    saved[1] = dup(STDOUT_FILENO);
    saved[2] = dup(STDERR_FILENO);
}

void gestRedir(char * filev []){

	int fd;
	if(filev[0]!=NULL){ // <
	fd=open(filev[0],O_RDONLY);
		if (fd<0){
			perror("open err");
			exit(1);
		}
	dup2(fd,STDIN_FILENO);
	close(fd);
	}
	if(filev[1]!=NULL){ // >
	fd=creat(filev[1],0666);
	if (fd<0){
			perror("creat err");
			exit(1);
		}
	dup2(fd,STDOUT_FILENO);
	close(fd);
	}
	else if(filev[2]!=NULL){ // &> caso
	fd=creat(filev[2],0666);
	if (fd<0){
			perror("creat err");
			exit(1);
		}
	dup2(fd,STDERR_FILENO);
	close(fd);
	}
					

}
void restRedir(int saved[3]) {

    if (saved[0] != -1) {
        dup2(saved[0], STDIN_FILENO);
        close(saved[0]);
    }

    if (saved[1] != -1) {
        dup2(saved[1], STDOUT_FILENO);
        close(saved[1]);
    }

    if (saved[2] != -1) {
        dup2(saved[2], STDERR_FILENO);
        close(saved[2]);
    }
}
void procesarCD (char** parametros ) {
	//caso cd
	char ret[256];
	if (parametros[2]!=NULL){
		fprintf(stderr,"demasiado arg\n");
		status=1;
	}
	
		if(parametros[1]==NULL){
			char * home =getenv("HOME");
			if (!home){
				fprintf(stderr,"cd : HOME no definido\n");
				status=1;
			}

			if(chdir(home)==-1){
				perror("cd");
				status=1;
			}
		}
		else{
			if(chdir(parametros[1])==-1){
					perror("cd");
					status=1;
				}
		}

	strcpy(ret,getcwd(ret,256));
	printf("%s\n",ret);
		
}
void procesarUmask (char ** parametros){
	int res;
	char *err;
	if (parametros[2]!=NULL){
		fprintf(stderr, "Numero incorrecto de argumentos");
		status=1;
	}
	else if (parametros[1]!=NULL){
		res=strtol(parametros[1],&err,8);

		if (res < 0000 || res > 7777 ||*err!= '\0' ) {
			perror ( "Valor de umask no valido(debe ser en octal)");
			status=1;
			return;
		}
		umask(res);
		printf("%o\n",res);

	}
	else {
		res=umask(0);
		umask(res);
		printf("%o\n",res);

	}

}
void procesarLimit (char ** parametros){
	struct rlimit lim;
	
	if (parametros[1]==NULL){
		// presentar por salida estandar todos los limites
		getrlimit(RLIMIT_CPU,&lim);
		printf("%s\t%d\n","cpu",lim.rlim_cur);
		getrlimit(RLIMIT_FSIZE,&lim);
		printf("%s\t%d\n","fsize",lim.rlim_cur);
		getrlimit(RLIMIT_DATA,&lim);
		printf("%s\t%d\n","data",lim.rlim_cur);
		getrlimit(RLIMIT_STACK,&lim);
		printf("%s\t%d\n","stack",lim.rlim_cur);
		getrlimit(RLIMIT_CORE,&lim);
		printf("%s\t%d\n","core",lim.rlim_cur);
		getrlimit(RLIMIT_NOFILE,&lim);
		printf("%s\t%d\n","nofile",lim.rlim_cur);
	
	}
	else if (parametros[2]==NULL){ // caso maximo no aparece
		// presenar el limite del recurso actual
		char * cad= parametros[1];
		if (strcmp("cpu",cad)==0){
			getrlimit(RLIMIT_CPU,&lim);
			printf("%s\t%d\n","cpu",lim.rlim_cur);
		}else if (strcmp("fsize",cad)==0){
			getrlimit(RLIMIT_FSIZE,&lim);
			printf("%s\t%d\n","fsize",lim.rlim_cur);
		}else if (strcmp("data",cad)==0){
			getrlimit(RLIMIT_DATA,&lim);
			printf("%s\t%d\n","data",lim.rlim_cur);
		}else if (strcmp("stack",cad)==0){
			getrlimit(RLIMIT_STACK,&lim);
			printf("%s\t%d\n","stack",lim.rlim_cur);
		}else if (strcmp("core",cad)==0){
			getrlimit(RLIMIT_CORE,&lim);
			printf("%s\t%d\n","core",lim.rlim_cur);
		}else if (strcmp("nofile",cad)==0){
			getrlimit(RLIMIT_NOFILE,&lim);
			printf("%s\t%d\n","nofile",lim.rlim_cur);
		}
		else{
			perror ( "limit err");
			status=1;
			return;
		}
	}
	else { // caso normal de establecimiento de limites maximos 

		// un maximo de -1 es valor infinito? MAX_INT?
		char * cad = parametros[1];
		char * fin;
		int valor = strtol(parametros[2],&fin,10);//convertir valor
		// setear valor de rlim
		if (strcmp("cpu",cad)==0){
			getrlimit(RLIMIT_CPU,&lim);
		}else if (strcmp("fsize",cad)==0){
			getrlimit(RLIMIT_FSIZE,&lim);
		}else if (strcmp("data",cad)==0){
			getrlimit(RLIMIT_DATA,&lim);
		}else if (strcmp("stack",cad)==0){
			getrlimit(RLIMIT_STACK,&lim);
		}else if (strcmp("core",cad)==0){
			getrlimit(RLIMIT_CORE,&lim);
		}else if (strcmp("nofile",cad)==0){
			getrlimit(RLIMIT_NOFILE,&lim);
		}
		else{
			perror ( "limit err");
			status=1;
			return;
		}
		if (valor == -1)
        	lim.rlim_cur = RLIM_INFINITY;
   		else
        	lim.rlim_cur = valor;

		if (strcmp("cpu",cad)==0)
			setrlimit(RLIMIT_CPU,&lim);
		if (strcmp("fsize",cad)==0)
			setrlimit(RLIMIT_FSIZE,&lim);
		if (strcmp("data",cad)==0)
			setrlimit(RLIMIT_DATA,&lim);
		if (strcmp("stack",cad)==0)
			setrlimit(RLIMIT_STACK,&lim);
		if (strcmp("core",cad)==0)
			setrlimit(RLIMIT_CORE,&lim);
		if (strcmp("nofile",cad)==0)
			setrlimit(RLIMIT_NOFILE,&lim);
	}	
}
void procesarSet (char ** parametros){
	char * ret;
	extern char ** environ;
	int len;
	if (parametros[1] == NULL){
		  for (char **varEnt = environ; *varEnt != NULL; varEnt++) {
            printf("%s\n", *varEnt);
        }
        return;
	}
	if (parametros[2] == NULL){
		//caso no hay valor 
		len=strlen(parametros[1] + 1);
		ret=malloc(len);
		strcpy(ret,parametros[1]);

		if(getenv(parametros[1])!=NULL){
			printf("%s=%s\n",parametros[1],getenv(ret));
		}
		else {
			printf("%s=\n", parametros[1]);
		}
		return;
	}

	else {
		//valor es una lista de palabras separadas por blancos
		// se consigue los parametros valor y su tamaño de las palabras
	len = strlen(parametros[1]) + 1;
	for (int i = 2; parametros[i]!=NULL; i++){
		len = len + strlen(parametros[i]) + 1;
	}
	ret = malloc(len);

    if (ret == NULL) {
        perror("err malloc");
        return;
    }
	strcpy(ret, parametros[1]);
    strcat(ret, "=");

	for (int i = 2; parametros[i] != NULL; i++) {
        strcat(ret, parametros[i]);
        if (parametros[i+1] != NULL){ 
			strcat(ret, " "); 
		}
    }
	putenv(ret);
	}
}

int main(void){
	//bloquear señales 
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	
	char ***argvv = NULL;
	int argvc;
	char **argv = NULL;
	int argc;
	char *filev[3] = { NULL, NULL, NULL };
	int bg;
	int ret;


//----------------Codigo Dado -----------------

	pid_t pid;
	pid_t bgpid;
	int saved[3];
	printf("%d \n", getpid());

	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);

	while (1) {
		int masDeUnArgvv=0;
		int manInt=0;
		
		fprintf(stderr, "%s", "msh> ");	/* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0) break;		/* EOF */
		if (ret == -1) continue;	/* Syntax error */
		argvc = ret - 1;		/* Line */
		if (argvc == 0) continue;	/* Empty line */

		masDeUnArgvv=(argvc>1);
		int  pipesfd[argvc-1][2];
		pid_t pIDs[argvc];

		//si argvc > 1 hay mas de 1 mandato (2 min) xd

		//creamos pipes para los mandatos
		for(int i=0;i<argvc-1 && masDeUnArgvv;i++){
		if((pipe(pipesfd[i])<0)){
				perror("pipe"); return 1;
			}

		}
		// creo los hijos
		/*
		for(int i=0;i<argv;i++){
			pIDs[i]=fork();
		}
		*/
		guardar_descriptores(saved);
		
		if ((argvc==1) && (strcmp(argvv[0][0],"cd"))==0){
				//redireccion 
				gestRedir(filev);
				procesarCD(argvv[0]);
				//restaurar redirecciones	
				restRedir(saved);
			}
		else if ((argvc==1) && (strcmp(argvv[0][0],"umask"))==0){
			//redireccion 
			gestRedir(filev);
			procesarUmask(argvv[0]);	
			//restaurar redirecciones	
			restRedir(saved);

		}
		else if ((argvc==1) && (strcmp(argvv[0][0],"limit"))==0){
			//redireccion 
			gestRedir(filev);
			procesarLimit(argvv[0]);	
			//restaurar redirecciones	
			restRedir(saved);

		}
		else if ((argvc==1) && (strcmp(argvv[0][0],"set"))==0){
			//redireccion 
			gestRedir(filev);
			procesarSet(argvv[0]);	
			//restaurar redirecciones	
			restRedir(saved);

		}
		//alterar pipe en caso de que haya "|" 

		else {

		for(int conArgvv=0;conArgvv<argvc;conArgvv++){
		//hay que hacer un hijo que ejecute cada mandato
			


			pid=fork();
			if(conArgvv==argvc-1){
				bgpid=pid;
			}
			
			switch(pid)
			{
				case(-1):
					perror("error fork 1");
					exit(1);
				case(0): //hijo
					// redirecciones
					int fd;
					if(conArgvv==0){ //primer mandato
					if(filev[0]!=NULL){ // <
					fd=open(filev[0],O_RDONLY);
						if (fd<0){
							perror("open err");
							exit(1);
						}
					dup2(fd,STDIN_FILENO);
					close(fd);
					}
					}
					if(conArgvv==argvc-1){ // > ultimo mandato
					if(filev[1]!=NULL){
					fd=creat(filev[1],0666);
					if (fd<0){
							perror("creat err");
							exit(1);
						}
					dup2(fd,STDOUT_FILENO);
					close(fd);
					}
					else if(filev[2]!=NULL){ // &> caso
					fd=creat(filev[2],0666);
					if (fd<0){
							perror("creat err");
							exit(1);
						}
					dup2(fd,STDERR_FILENO);
					close(fd);
					}
					}
					
					//procesar secuencias
					if(masDeUnArgvv){
					if (conArgvv==0){ // primer (entrada)
						dup2(pipesfd[conArgvv][1],STDOUT_FILENO);
					}
					else if (conArgvv==argvc-1){ //ultimo (salida)
						dup2(pipesfd[conArgvv-1][0],STDIN_FILENO);
					}
					else {
						dup2(pipesfd[conArgvv][1],STDOUT_FILENO);
						dup2(pipesfd[conArgvv-1][0],STDIN_FILENO);
					}
					//cerrar al resto
					for (int i = 0; i < argvc - 1; i++) {
						close(pipesfd[i][0]);
						close(pipesfd[i][1]);
					}
					}
					
				// volver procesar señales
				if(!bg){
				signal(SIGINT,SIG_DFL);
				signal(SIGQUIT,SIG_DFL);
				}
				// mandato a ejecutar? execvp?
				//procesar CD
				if (strcmp(argvv[conArgvv][0],"cd")==0){
					
					procesarCD(argvv[conArgvv]);
					
					//caso de si hay "|"
					exit(status);
				}
				if (strcmp(argvv[conArgvv][0],"umask")==0){
					
					procesarUmask(argvv[conArgvv]);
					
					//caso de si hay "|"
					exit(0);
				}
				if (strcmp(argvv[conArgvv][0],"limit")==0){
					
					procesarLimit(argvv[conArgvv]);
					
					//caso de si hay "|"
					exit(0);
				}
				if (strcmp(argvv[conArgvv][0],"set")==0){
					
					procesarSet(argvv[conArgvv]);
					
					//caso de si hay "|"
					exit(0);
				}


				else {
					//caso mandato generico
					execvp(argvv[conArgvv][0],argvv[conArgvv]);
					perror("error execvp");
					exit(status);
				}	

					
				default:
				break;
			}
		}
	
		
		for (int i = 0; masDeUnArgvv && (i < argvc - 1) ; i++) {
			close(pipesfd[i][0]);
			close(pipesfd[i][1]);
		}
		if(!bg ){// esperar al ultimo hijo SOLO
			waitpid(bgpid,&status,0);
				if (WIFEXITED(status)) {   
            	printf("Hijo terminó con código %d\n", WEXITSTATUS(status));
				}
			}
		
		else { // no esperar CASO bg activo
			printf("[%d]\n",bgpid);
		}

		}	
//-----------------------------------------------
		
		restRedir(saved);
	}
	exit(0);
	return 0;
}
