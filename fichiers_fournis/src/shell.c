/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

#define FIFO_NAME "./fifo/fifo"

void clear_pid(){
	while(waitpid(-1,NULL,0)!=-1){}
}

void clear_redirection(struct cmdline *l , int doc_in, int doc_out){
	if(l->in != NULL){
		close(doc_in);
		l->in = NULL;
		dup2(5,0);
	}
	if(l->out != NULL){
		close(doc_out);
		l->out = NULL;
		dup2(6,1);
	}
}

void exec_quit(){
	printf("quit\n");
	clear_pid();
	exit(0);
}


int main()
{
	mkfifo(FIFO_NAME,0777);
	while (1) {
		struct cmdline *l;
		int i, j;
		int doc_in,doc_out;
		int fifo_fd, fifo_fd1;
		

		printf("kintama> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in != NULL) {
			printf("in: %s\n", l->in);
			doc_in = open(l->in,O_RDONLY,0);
			dup2(0,5);
			dup2(doc_in,0);

		}
		if (l->out != NULL) {
			printf("out: %s\n", l->out);
			doc_out = open(l->out,O_WRONLY | O_CREAT | O_TRUNC,0752);
			dup2(1,6);
			dup2(doc_out,1);
		}


		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];

			printf("seq[%d]: ", i);

			if(!strcmp(l->seq[i][0],"quit")){
				exec_quit();
			}

			if(Fork()==0){
				if (i!=0 && l->seq[i+1] != 0){
					fifo_fd=open(FIFO_NAME,O_RDWR);
					printf("read : %d \n write : %d\n",fifo_fd,fifo_fd);
					dup2(fifo_fd,0);
					dup2(fifo_fd,1);
					
				}
				else if (i!=0) {
					fifo_fd=open(FIFO_NAME,O_RDONLY);
					dup2(fifo_fd,0);
					printf("%d je suis bien ici ^^ %d ;)\n",fifo_fd,getpid());
				}
				else if (l->seq[i+1] != 0){
					fifo_fd=open(FIFO_NAME,O_WRONLY); 
					dup2(fifo_fd,1);
				}
				fprintf(stderr,"je passe ici ^^ %d\n",getpid());
				execvp(cmd[0],cmd);
				fprintf(stderr,"normalement pas ici ?\n");
				exit(0);
				
			}

			//clear_redirection(l,doc_in,doc_out);
			
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}

			printf("\n");
		}
		clear_pid();
	}
}