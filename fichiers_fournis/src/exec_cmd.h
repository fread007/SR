#ifndef __EXEC_CMD__
#define __EXEC_CMD__

//fonction qui gere les erreur
void debbug(int val_retour, char* nom);
//fonction de redefinition du sigchild
void handler(int sig);
//suprime les processus zombie de tout les fils
void clear_pid();
//remer STDIN et STDOUT a leur place initial
void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout);
//execute la commande quit
void exec_quit();
//execute une commande
void execut_commande (struct cmdline *l,char* fifo_actuel_name, int fifo_fd,int fifo_fd1, char **cmd,int i);
#endif