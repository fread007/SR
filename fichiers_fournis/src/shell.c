/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"
#include "exec_cmd.h"

int main()
{
	Signal(SIGCHLD,handler);
	while (1) {
		struct cmdline *l;	
		int i, j;
		int doc_in,doc_out;		//descipteur des fichier d'entree et de sortie
		int fifo_fd, fifo_fd1;	//descripteur pour les fifo 
		int new_stdin,new_stdout;	//stoque stdin et stdout pour les remetre a leur place
		int back_cmd=0;				//est egal a 1 si ont doit executer la commande en arriere plan
		char fifo_actuel_name[50];	//chaine de caractere pour cree les fifo
		

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

		//test si la commande doit etre fait en arriere plan
		if(l->background==1){
			//cree un fils qui executera la commande
			if (Fork()!=0){
				//change la valeur de back_cmd pour que le pere n'execute pas la commande
				back_cmd=1;
			}
		}

		//teste si il doit effectuer la commande
		if(back_cmd==0){
			/* Display each command of the pipe */
			for (i=0; l->seq[i]!=0; i++) {
				char **cmd = l->seq[i];

				printf("seq[%d] : ", i,getpid());

				//teste est execution de la commande special quit
				if(!strcmp(l->seq[i][0],"quit")){
					exec_quit();
				}
				
				//redirection de la sortie, ont verifie que l'on est bien a la derniere commande si dans un pipe
				if (l->out != NULL && l->seq[i+1]==0) {
					printf("out: %s\n", l->out);
					//ouvre en ecriture , cree le fichier si besoin , si non le remet a 0
					doc_out = open(l->out,O_WRONLY | O_CREAT | O_TRUNC,0752);
					//deplace STDOUT
					new_stdout = dup(0);
					//change la sortie standare par le fichier
					dup2(doc_out,1);
				}

				//redirection de l'entree, ont verifie que l'on est bien a la derniere commande si dans un pipe
				if (l->in != NULL && l->seq[i+1]==0) {
					printf("in: %s\n", l->in);
					doc_in = open(l->in,O_RDONLY,0);
					if (doc_in == -1) {
						debbug(2, l->in);
					}
					new_stdin = dup(0);
					dup2(doc_in,0);

				}

				execut_commande(l,fifo_actuel_name,fifo_fd,fifo_fd1,cmd,i);

				if(l->seq[i+1]==0){
					clear_redirection(l,doc_in,doc_out,new_stdin,new_stdout);
				}
				
				for (j=0; cmd[j]!=0; j++) {
					printf("%s ", cmd[j]);
				}

				printf("\n");
			}
			if (l->background==1){
				exit(0);
			}
			clear_pid();
			
		}
		l->background=0;
	}
}