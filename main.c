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
