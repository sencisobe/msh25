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

extern int obtain_order();		/* See parser.y for description */
// vamos a poner como se procesara cada mandato?
void procesarCD (char** parametros ) {
	//caso cd
	char ret[256];
	struct stat st;
	if (parametros[2]!=NULL){
		perror("Mucho Argumento");
		exit(1);
	}
	
		if(parametros[1]==NULL){
			if(chdir(getenv("HOME"))){
			perror("no existe dir");
			exit(1);
			}
		}
		else{
			if (stat(parametros[1], &st) == 0 && S_ISDIR(st.st_mode)) {
    		
			chdir(parametros[1]);
			} 
			else {
        		perror("No existe o no es un directorio.\n");
				exit(1);
   			 }
	
			
		}

	strcpy(ret,getcwd(ret,256));
	printf("%s\n",ret);
		
}
void procesarUmask (char ** parametros){
	int res;
	char *err;
	if (parametros[2]!=NULL){
		perror("Numero incorrecto de argumentos");
		exit(1);
	}
	else if (parametros[1]!=NULL){
		res=strtol(parametros[1],&err,8);
		if ( *err!= '\0' ) {
			perror ( "Valor de umask no valido(debe ser en octal)");
			exit(1);
		}

		umask(res);
	}
	else {
		res=umask(0);
		umask(res);
	}
	printf("%o\n",res);

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
	int status;



	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);

	while (1) {
		int masDeUnArgvv=0;
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

		//alterar pipe en caso de que haya "|" 
		for(int conArgvv=0;conArgvv<argvc;conArgvv++){
		//hay que hacer un hijo que ejecute cada mandato
			
		
		
			if(conArgvv==argvc-1){ // caso cd ultimo 
					
				if (strcmp(argvv[conArgvv][0],"cd")==0){
					
					procesarCD(argvv[conArgvv]);
					
					//caso de si hay "|"
					break;
				}

			}
				
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
					if(filev[0]!=NULL){
					fd=open(filev[0],O_RDONLY);
						if (fd<0){
							perror("open err");
							exit(1);
						}
					dup2(fd,STDIN_FILENO);
					close(fd);
					}
					}
					if(conArgvv==argvc-1){ //ultimo mandato
					if(filev[1]!=NULL){
					fd=creat(filev[1],0666);
					if (fd<0){
							perror("creat err");
							exit(1);
						}
					dup2(fd,STDOUT_FILENO);
					close(fd);
					}
					else if(filev[2]!=NULL){
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
					exit(0);
				}
				if (strcmp(argvv[conArgvv][0],"umask")==0){
					
					procesarUmask(argvv[conArgvv]);
					
					//caso de si hay "|"
					exit(0);
				}


				else {
					//caso mandato generico
					execvp(argvv[conArgvv][0],argvv[conArgvv]);
					perror("error execvp");
					exit(1);
				}	

					
				default:
				break;
			}
		}
		
		for (int i = 0; masDeUnArgvv && (i < argvc - 1) ; i++) {
			close(pipesfd[i][0]);
			close(pipesfd[i][1]);
		}
		if(!bg){// esperar al ultimo hijo SOLO
			waitpid(bgpid,&status,0);
				if (WIFEXITED(status)) {   
            	printf("Hijo terminó con código %d\n", WEXITSTATUS(status));
				}
			}
		
		else{ // no esperar CASO bg activo
			printf("[%d]\n",bgpid);
		}

		
//-----------------------------------------------
		

	}
	exit(0);
	return 0;
}
