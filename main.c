#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <termios.h>
#include <string.h>

#define printable(ch) (isprint((unsigned char) ch) ? ch : '#')

static struct termios initial_settings, new_settings;
static int peek_character = -1;


// kbhit INICIO --------------------------------------------------------------------------------------

void init_keyboard() {
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard() {
    tcsetattr(0, TCSANOW, &initial_settings);
}

int kbhit() {
    unsigned char ch;
    int nread;

    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int readch() {
    char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}


// kbhit FIN --------------------------------------------------------------------------------------

static void usageError(char *progName, char *msg, int opt) {
    if (msg != NULL && opt != 0) {
        fprintf(stderr, "%s (-%c)\n", msg, printable(opt));
    }
    fprintf(stderr, "Uso: %s [-v prog] [-V prog]\n", progName);
    exit(EXIT_FAILURE);
}


/*
 * Esta funcion llama al comando strace para rastrear las llamadas al sistema hechas por un programa. Para obtener
 * una tabla con los acumulados se llama a strace con los parametros -c y -f
 */
void traceCall(int argc, char *argv[]){


    int initialIndex = 2;
    int i = initialIndex;
    int argumentsIndex = 3;
    int argumentsSize = argumentsIndex + (argc - initialIndex) + 1;
    char *arguments[argumentsSize];

    if(strcmp(argv[1], "-V") == 0){
        close_keyboard();
    }

    arguments[0] = "strace"; arguments[1] = "-c"; arguments[2] = "-f";
    syslog(LOG_DEBUG, "Arg count = %d, i = %d, ArgSize = %d \n", argc, i, argumentsSize);

    while(i < argc){
        arguments[argumentsIndex] = argv[i];
        syslog(LOG_DEBUG, "AA[%d] %s \n", argumentsIndex, arguments[argumentsIndex]);
        argumentsIndex++;
        i++;
    }
    i++;
    arguments[i] = NULL;

    execvp("strace", arguments);
}

// MAIN  --------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {

    setlogmask (LOG_UPTO (LOG_DEBUG));
    openlog(">>rastreador>>", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_DEBUG, "Iniciando rastreador");

    int opt;
    while((opt = getopt(argc, argv, "-v:V")) != EOF) {


        switch (opt) {
            case 'v':
                syslog(LOG_DEBUG, "Llamando a traceCall con -v");
                traceCall(argc, argv);
                break;
            case 'V':
                syslog(LOG_DEBUG, "Llamando a traceCall con -V");
                puts("Presione cualquier tecla para continuar...");

                int ch = 0;
                init_keyboard();
                while(ch == 0) {
                    //Se comienza a monitorear la terminal cada segundo para saber si algo se presiono
                    sleep(1);
                    if(kbhit()) {
                        ch = readch();
                        syslog(LOG_DEBUG, "tecla presionada: %c\n",ch);
                        traceCall(argc, argv);
                    }
                }
                break;
            case ':':
                usageError(argv[0], "Falta argumento", optopt);
            case '?':
                usageError(argv[0], "Opcion invalida", optopt);
            default:
                usageError(argv[0], "Falta argumento", optopt);
        }

        syslog(LOG_DEBUG, "opt =%3d (%c); optind = %d", opt, printable(opt), optind);
        if (opt == '?' || opt == ':') {
            syslog(LOG_DEBUG, "; optopt =%3d (%c)", optopt, printable(optopt));
        }
    }
    closelog();

    return 0;
}
