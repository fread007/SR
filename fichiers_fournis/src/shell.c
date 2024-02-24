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
		int doc_in, doc_out;		// descipteur des fichiers d'entree et de sortie
		int fifo_fd=0, fifo_fd1=0;	// descripteur pour les fifo 
		int new_stdin,new_stdout;	// stocke stdin et stdout pour les remettre a leurs places
		int back_cmd=0;				// est egal a 1 si on doit executer la commande en arriere plan
		char fifo_actuel_name[50];	// chaine de caracteres pour creer les fifo
		

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

		// teste si la commande doit etre faite en arriere plan
		if (l->background == 1){
			// cree un fils qui executera la commande
			if (Fork() != 0){
				// change la valeur de back_cmd pour que le pere n'execute pas la commande
				back_cmd = 1;
			}
		}

		// teste s'il doit effectuer la commande
		if (back_cmd == 0){
			/* Display each command of the pipe */
			for (i=0; l->seq[i]!=0; i++) {
				char **cmd = l->seq[i];

				printf("seq[%d] : ", i);

				// teste si est execution de la commande speciale "quit"
				if (!strcmp(l->seq[i][0], "quit")){
					exec_quit();
				}
				
				// redirection de la sortie, on verifie que l'on est bien a la derniere commande si dans un pipe
				if (l->out != NULL && l->seq[i+1] == 0) {
					printf("out: %s\n", l->out);
					// ouvre un fichier en ecriture , le cree si besoin sinon le reecrit de 0
					doc_out = open(l->out, O_WRONLY | O_CREAT | O_TRUNC, 0752);
					// teste s'il ya eu un probleme
					if (doc_out == -1) {
						debbug(2, l->in);
					}
					// stocke STDOUT
					new_stdout = dup(0);
					// change la sortie standard
					if (dup2(doc_out, 1) == -1){
						debbug(4, "dup2");
					}
				}

				// redirection de l'entree, on verifie que l'on est bien a la derniere commande si dans un pipe
				if (l->in != NULL && l->seq[i+1] == 0) {
					printf("in: %s\n", l->in);
					// ouvre un fichier en lecture
					doc_in = open(l->in, O_RDONLY, 0);
					// teste s'il ya eu un probleme
					if (doc_in == -1) {
						debbug(2, l->in);
					}
					// stocke STDIN
					new_stdin = dup(0);
					// change la sortie standard
					if (dup2(doc_in, 0) == -1){
						debbug(4, "dup2");
					}

				}

				// execute la commande actuelle
				execut_commande(l, fifo_actuel_name, fifo_fd, fifo_fd1, cmd, i);

				// si on est a la fin remet a l'etat initial l'entree et la sortie standard
				if (l->seq[i+1] == 0){
					clear_redirection(l, doc_in, doc_out, new_stdin, new_stdout);
				}
				
				for (j=0; cmd[j] != 0; j++) {
					printf("%s ", cmd[j]);
				}

				printf("\n");
			}
			// si on execute en arriere plan on s'arrete
			if (l->background == 1){
				exit(0);
			}
			clear_pid();
			
		}
		// on remet a 0 le fait de devoir executer en arriere plan 
		l->background = 0;
	}
}