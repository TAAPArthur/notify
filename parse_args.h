#ifndef NOTIFY_PARSE_ARGS_H
#define NOTIFY_PARSE_ARGS_H

#ifdef NO_ARG_PARSE
static inline char** parseArgs(char **argv) { return argv;}
#else
#define GET_ARG_STR argv[0][2] ? *argv +2 :*++argv
#define GET_ARG atoi(GET_ARG_STR );
static inline char** parseArgs(char **argv) {
    if(getenv("NOTIFY_FONT"))
        FONT = getenv("NOTIFY_FONT");
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
                NOTIFY_ID = *++argv;
                break;
            case 's':
                SEQ_NUM = GET_ARG;
                break;
#endif
            case 'h':
                HEIGHT = GET_ARG;
                break;
            case 't':
                TIMEOUT = GET_ARG;
                break;
            case 'w':
                WIDTH = GET_ARG;
                break;
            case 'x':
                X = GET_ARG;
                break;
            case 'y':
                Y = GET_ARG;
                break;
        }
    }
    return argv;
}
#endif
#endif

