#include "../include/display.h"
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFERSIZE 1024
#define MULTIPLIER 10

static volatile sig_atomic_t exiting = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

struct info
{
    int    inFD;
    int    outFD;
    size_t length;
    char   buffer[BUFFERSIZE];
};

static void signal_handler(int sig)
{
    if(sig == SIGINT)
    {
        exiting = 1;
        printf("\nExiting 'gracefully'...");
    }
}

static void setup_signals(void);

static void setup_signals(void)
{
    struct sigaction action;
#if defined(__clang__)
    {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    }
#endif
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signal_handler;
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
    sigaction(SIGINT, &action, NULL);
}

void *client_request(void *infoArg);

void *client_request(void *infoArg)
{
    const struct info *info   = (struct info *)infoArg;
    size_t             length = info->length;
    int                inFD   = info->inFD;
    int                outFD  = info->outFD;
    char               newBuffer[BUFFERSIZE];
    char               c;

    printf("GOT INFO FROM STRUCT\n");
    printf("INPUT:%d\n", inFD);
    printf("OUTPUT:%d\n", outFD);
    printf("LENGTH:%zu\n", length);
    printf("buffer:%s\n", info->buffer);

    c = info->buffer[length];    // gets the filter character
    printf("filter:%c\n", c);
    switch(c)
    {
        case 'u':
            for(size_t i = 0; i < length; i++)
            {
                newBuffer[i] = (char)toupper(info->buffer[i]);
            }
            break;
        case 'l':
            for(size_t i = 0; i < length; i++)
            {
                newBuffer[i] = (char)tolower(info->buffer[i]);
            }
            break;
        case 'n':
            break;
        default:
            display("ERROR: INVALID FILTER\n");
            break;
    }
    write(outFD, newBuffer, length);    // everything is good, output :D
    free(infoArg);
    return NULL;
}

int main(void)
{
    int  input  = open("./inputFifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    int  output = open("./outputFifo", O_WRONLY | O_CLOEXEC);
    char c      = 'a';

    setup_signals();

    display("CODE IS FUCKING RUNNING");
    if(input != -1 && output != -1)
    {
        printf("INPUT: %d\nOUTPUT: %d\n", input, output);
        while(!exiting)
        {
            if(read(input, &c, sizeof(char)) > 0)    // only run if there is a char detected
            {
                if(isdigit(c))    // if the character is a number
                {
                    int       cInt   = c - '0';
                    size_t    length = (size_t)cInt;
                    pthread_t thread;

                    void        *ptr     = (struct info *)calloc(1, sizeof(struct info));
                    struct info *infoPtr = (struct info *)ptr;

                    read(input, &c, sizeof(char));
                    while(1)    // loop to check for more digits
                    {
                        if(isdigit(c))
                        {
                            length *= MULTIPLIER;
                            cInt = c - '0';
                            length += (size_t)cInt;
                            read(input, &c, sizeof(char));
                        }
                        else
                        {
                            break;
                        }
                    }    // after the loop ends, the next character read shud be part of the text

                    if(length > BUFFERSIZE - 1)    // temp buffer overflow prevention
                    {
                        display("ERROR: PROVIDED LENGTH BIGGER THAN BUFFER\n");
                        free(infoPtr);
                        continue;
                    }

                    if(infoPtr != NULL)
                    {
                        read(input, infoPtr->buffer, sizeof(char) * BUFFERSIZE);
                        infoPtr->inFD   = input;
                        infoPtr->outFD  = output;
                        infoPtr->length = length;
                        pthread_create(&thread, NULL, client_request, (void *)infoPtr);
                    }
                }
            }
        }
        return EXIT_SUCCESS;
    }
}
