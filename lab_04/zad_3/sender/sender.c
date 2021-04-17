#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

int pid;
int message = 0;

void parse_args(int argc, char *argv[]) {
    if(argc < 3) {
        printf("You have to specify input enough arguments.");
        exit(-1);
    }
    else if (argc == 4) {
        return;
    }
    else if(argc > 4) {
        printf("To many arguments.");
        exit(-1);
    }
}

void send_sig_queue(int pid) {
    union sigval sig;
    message++;

    printf("Sending: sig = %d \n", message);
    sig.sival_int = message;  // set signal value
    sigqueue(pid, SIGUSR1, sig);
}

void send_sig_kill(int pid) {
    kill(pid, SIGUSR1);
}

int synchronize = 0;

void handler( int signo, siginfo_t *info, void * o ) {
    printf("Synchronization;\t");
    synchronize = 1;
}

void run(int argc, char *argv[])
{
    parse_args(argc, argv);
    pid = atoi(argv[1]);
    int n = atoi(argv[2]);
    int mode = atoi(argv[3]);
    printf("Parent PID: %d, Number of signals to send: %d, Mode: %d", pid, n, mode);

    sigset_t sig_set;

    struct sigaction act;
    sigemptyset( &sig_set );
    sigaddset( &sig_set, SIGUSR1 );

    act.sa_flags = SA_SIGINFO;
    act.sa_mask = sig_set;
    act.sa_handler = (__sighandler_t) &handler;
    sigaction( SIGUSR1, &act, NULL );

    for(int i=0; i<n; i++) {
        if(mode == 0)
            send_sig_queue(pid);
        else if (mode == 1)
            send_sig_kill(pid);

        while(!synchronize){
        }
        synchronize=0;
    }
}


int main(int argc, char *argv[]) {
    run(argc, argv);
    return 0;
}