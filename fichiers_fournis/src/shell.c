/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

#define FIFO_NAME "./fifo/fifo%d"

void debbug(int val_retour, char* nom){
	switch (val_retour) {
		case 1:
			fprintf(stderr,"%s: command not found.\n", nom);
			break;
		case 2:
			switch (errno) {
				case EACCES:
					fprintf(stderr,"%s: Permission denied.\n", nom);
					break;
				case ENOENT:
					fprintf(stderr,"%s: File not found.\n", nom);
					break;
				default : 
					fprintf(stderr,"%s: Error number %d.\n", nom, errno);
					break;
			}
			break;
		case 3:
			fprintf(stderr,"%s: Closing file error number %d.\n", nom, errno);
	}	
}

void handler(int sig){
	pid_t pid;
	while((pid=waitpid(-1,NULL,WNOHANG))>0){}
}

void clear_pid(){
	while(waitpid(-1,NULL,0)!=-1){}
}


void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout){
	if(l->in != NULL){
		int e = close(doc_in);
		if (e == -1) {
			debbug(3, l->in);
		}		
		l->in = NULL;
		dup2(new_stdin,0);
	}
	if(l->out != NULL){
		int e = close(doc_out);
		if (e == -1) {
			debbug(3, l->in);
		}
		l->out = NULL;
		dup2(new_stdout,1);
	}
}

void exec_quit(){
	printf("quit\n");
	clear_pid();
	exit(0);
}


int main()
{
	Signal(SIGCHLD,handler);
	while (1) {
		struct cmdline *l;
		int i, j;
		int doc_in,doc_out;
		int fifo_fd, fifo_fd1;
		int new_stdin,new_stdout;
		int childpid=1;
		char fifo_actuel_name[50];
		

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

		if(l->background==1){
			printf("ici ^^\n");
			if (Fork()!=0){
				printf("%d\n",getpid());
				childpid=0;
			}
			else{
				printf("child : %d\n",getpid());
			}
		}

		if(childpid==1){
			/* Display each command of the pipe */
			for (i=0; l->seq[i]!=0; i++) {
				char **cmd = l->seq[i];

				printf("seq[%d] %d: ", i,getpid());

				if(!strcmp(l->seq[i][0],"quit")){
					exec_quit();
				}
				if (l->out != NULL && l->seq[i+1]==0) {
					printf("out: %s\n", l->out);
					doc_out = open(l->out,O_WRONLY | O_CREAT | O_TRUNC,0752);
					new_stdout = dup(0);
					dup2(doc_out,1);
				}

				if (l->in != NULL && l->seq[i+1]==0) {
					printf("in: %s\n", l->in);
					doc_in = open(l->in,O_RDONLY,0);
					if (doc_in == -1) {
						debbug(2, l->in);
					}
					new_stdin = dup(0);
					dup2(doc_in,0);

				}

				if(Fork()==0){
					if (i!=0 && l->seq[i+1] != 0){
						sprintf(fifo_actuel_name, FIFO_NAME, i-1);
						mkfifo(fifo_actuel_name,0777);
						fifo_fd=open(fifo_actuel_name,O_RDONLY);

						sprintf(fifo_actuel_name, FIFO_NAME, i);
						mkfifo(fifo_actuel_name,0777);
						fifo_fd1=open(fifo_actuel_name,O_WRONLY);
						

						dup2(fifo_fd,0);
						dup2(fifo_fd1,1);
						
					}
					else if (i!=0) {
						sprintf(fifo_actuel_name, FIFO_NAME, i-1);
						mkfifo(fifo_actuel_name,0777);
						fifo_fd=open(fifo_actuel_name,O_RDONLY);
						dup2(fifo_fd,0);
					}
					else if (l->seq[i+1] != 0){
						sprintf(fifo_actuel_name, FIFO_NAME, i);
						mkfifo(fifo_actuel_name,0777);
						fifo_fd=open(fifo_actuel_name,O_WRONLY); 
						dup2(fifo_fd,1);
					}
					int e=execvp(cmd[0],cmd);
					if (e == -1) {
						debbug(1, cmd[0]);
					}
					exit(0);
					
				}

				if(l->seq[i+1]==0){
					clear_redirection(l,doc_in,doc_out,new_stdin,new_stdout);
				}
				
				for (j=0; cmd[j]!=0; j++) {
					printf("%s ", cmd[j]);
				}

				printf("\n");
			}
		if(l->background==1 && childpid==1){
			clear_pid();
			exit(0);
		}
		}
		l->background=0;
	}
}