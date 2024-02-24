#ifndef __EXEC_CMD__
#define __EXEC_CMD__

// fonction qui gere les erreurs
void debbug(int val_retour, char* nom);
// fonction de redefinition du sigchild
void handler(int sig);
// supprime les processus zombis de tous les fils
void clear_pid();
// remet STDIN et STDOUT a leur place initiale
void clear_redirection(struct cmdline *l , int doc_in, int doc_out,int new_stdin, int new_stdout);
// execute la commande quit
void exec_quit();
// execute une commande
void execut_commande (struct cmdline *l,char* fifo_actuel_name, int fifo_fd,int fifo_fd1, char **cmd,int i);
#endif