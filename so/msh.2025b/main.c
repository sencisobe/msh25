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


extern int obtain_order();		/* See parser.y for description */
// vamos a poner como se procesara cada mandato?
char * procesarCD (char** parametros ) {
	//caso cd
	char ret[256];

		if(parametros[1]==NULL){
			chdir(getenv("HOME"));
		}
		else{
			chdir(parametros[1]);
		}

	strcpy(ret,getcwd(ret,256));
	printf("%s\n",ret);
		return ret ;
	}
int main(void)
{
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

	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);

	while (1) {
		fprintf(stderr, "%s", "msh> ");	/* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0) break;		/* EOF */
		if (ret == -1) continue;	/* Syntax error */
		argvc = ret - 1;		/* Line */
		if (argvc == 0) continue;	/* Empty line */
		int conArgvv=0; //contador de mandato
		// tengo el 1er mandato , hay que hacer un hijo que lo ejecute
		pid=fork();
		int fd[2];
		switch(pid)
		{
			case(-1):
				perror("error fork 1");
				exit(1);
			case(0): //hijo
			// volver procesar señales
				signal(SIGINT,SIG_DFL);
				signal(SIGQUIT,SIG_DFL);
				// mandato a ejecutar? execvp?

				//procesar CD
				if (strcmp(argvv[conArgvv][0],"cd")==0){
				
					procesarCD(argvv[conArgvv]);

				}

				
			default:
				wait();

		}
		
//-----------------------------------------------





	}
	exit(0);
	return 0;
}
