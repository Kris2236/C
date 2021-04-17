#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

static int count, number, value = 0, pid_glob;
int n = 0;

void handler( int signo, siginfo_t *info, void * o ) {
    count++;
    number = signo;
    value = info->si_value.sival_int;
    pid_glob = info->si_pid;
    printf("Signal received: status = %d/%d, sig nr= %d, val= %d, PID= %d\n", count, n, number, value, pid_glob);

    // confirmation of receiving the signal
    union sigval sig;
    sig.sival_int = -777;  // set signal value
    sigqueue(pid_glob, SIGUSR1, sig);
}

void signal_queue() {
    // ignore all signals besides 10 and 12
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    // handle communication
    sigset_t sig_set;
    struct sigaction act;
    sigemptyset( &sig_set );
    sigaddset( &sig_set, SIGUSR1 );

    act.sa_flags = SA_SIGINFO;
    act.sa_mask = sig_set;
    act.sa_handler = (__sighandler_t) &handler;
    sigaction( SIGUSR1, &act, NULL );

    for(int i=0; i<n; i++) {
        wait(NULL);
    }

    printf("\n\n===================\n");
    printf("Received & send %d signals = 100 %%", n);
    printf("\n===================\n\n");
}

void init_sender() {
    char pid_str[12];
    sprintf(pid_str, "%d", getpid());

    char n_str[12];
    sprintf(n_str, "%d", n);

    char mode[2];
    sprintf(mode, "%d", 0);

    if((fork()) == 0) { // Child process
        char * args[] = {"../sender/sender", pid_str, n_str, mode, NULL};
        execvp("../sender/sender", args);
    }
}


int main( int argc, char * argv[] ) {

    if(argc != 2) {
        printf("Wrong number of arguments!");
        return 0;
    }

    n = atoi(argv[1]);

    init_sender();
    signal_queue();

    return 0;
}
