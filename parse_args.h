#ifndef NOTIFY_PARSE_ARGS_H
#define NOTIFY_PARSE_ARGS_H

#ifdef NO_ARG_PARSE
static inline char** parseArgs(char **argv) { return argv;}
#else
static inline char** parseArgs(char **argv) {
    for(; argv[0]; argv++){
        if(argv[0][0] != '-')
            break;
        if(argv[0][1] == '-') {
            argv++;
            break;
        }
        switch(argv[0][1]) {
#ifndef NO_MSD_ID
            case 'r':
                notify_id = *++argv;
                break;
            case 's':
                seq_num = atoi(*++argv);
                break;
#endif
            case 'h':
                height = atoi(*++argv);
                break;
            case 't':
                timeout = atoi(*++argv);
                break;
            case 'w':
                width = atoi(*++argv);
                break;
            case 'x':
                x = atoi(*++argv);
                break;
            case 'y':
                y = atoi(*++argv);
                break;
        }
    }
    return argv;
}
#endif
#endif

