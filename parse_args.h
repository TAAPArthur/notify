#ifndef NOTIFY_PARSE_ARGS_H
#define NOTIFY_PARSE_ARGS_H

#ifdef NO_ARG_PARSE
static inline char** parseArgs(char **argv) { return argv;}
#else
#define GET_ARG_STR argv[0][2] ? *argv +2 :*++argv
#define GET_ARG atoi(GET_ARG_STR );
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
                seq_num = GET_ARG;
                break;
#endif
            case 'h':
                height = GET_ARG;
                break;
            case 't':
                timeout = GET_ARG;
                break;
            case 'w':
                width = GET_ARG;
                break;
            case 'x':
                x = GET_ARG;
                break;
            case 'y':
                y = GET_ARG;
                break;
        }
    }
    return argv;
}
#endif
#endif

