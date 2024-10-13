// args work
#include <getopt.h>
#include <stdbool.h>

// std
#include <stdio.h>
#include <stdlib.h>

// r/w
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#define SIZE 1023    // buffer woohoo

#define EXTRACHAR 2

#define DIVISOR 10

// #define STRINGSIZE 8

static int parse_args(int argc, char *argv[], char **f_val, char **m_val)
{
    int       opt;
    const int ARGS_NEEDED = 5;

    if(argc != ARGS_NEEDED)
    {
        printf("ERROR: INCORRECT NUMBER OF ARGUMENTS\n");
        return -1;
    }
    while((opt = getopt(argc, argv, "hf:m:")) != -1)
    {
        switch(opt)
        {
            case 'f':
                *f_val = optarg;
                break;
            case 'm':
                *m_val = optarg;
                break;
            default:
                printf("ERROR: INVALID ARGUMENT\n");
                return -1;
        }
    }
    if(*f_val == NULL || (strcmp(*f_val, "l") != 0 && strcmp(*f_val, "u") != 0 && strcmp(*f_val, "n") != 0))
    {
        printf("ERROR: INVALID FILTER\n");
        return -1;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char *f_val = NULL;
    char *m_val = NULL;

    if(parse_args(argc, argv, &f_val, &m_val) == -1)
    {
        exit(EXIT_FAILURE);
    }

    if(f_val != NULL && m_val != NULL)
    {
        char  *msg;
        int    input      = open("./inputFifo", O_WRONLY | O_CLOEXEC);
        int    output     = open("./outputFifo", O_RDONLY | O_CLOEXEC | O_NONBLOCK | O_TRUNC);
        size_t digits     = 0;
        size_t length     = strlen(m_val);
        size_t tempLength = length;
        asprintf(&msg, "%lu %s%s", length, m_val, f_val);

        if(strlen(m_val) > SIZE)
        {
            printf("ERROR: MESSAGE TOO LONG (>1023 characters)\n");
            exit(EXIT_FAILURE);
        }

        digits++;
        while(1)
        {
            if((tempLength / DIVISOR) > 0)
            {
                tempLength /= DIVISOR;
                digits++;
            }
            else
            {
                break;
            }
        }

        if(length == 0)
        {
            printf("ERROR: LENGTH MUST BE GREATER THAN 0\n");
            exit(EXIT_FAILURE);
        }

        if(input != -1 && output != -1)
        {
            char   outputMsg[SIZE];
            size_t counter = 0;

            write(input, msg, sizeof(char) * (length + EXTRACHAR + digits));

            while(counter < length)
            {
                if(read(output, &outputMsg, length) > 0)
                {
                    counter += strlen(outputMsg);
                }
            }

            for(size_t i = 0; i < length; i++)
            {
                printf("%c", outputMsg[i]);
            }
            printf("\n");
            close(input);
            close(output);
        }
        else
        {
            if(input != -1)
            {
                close(input);
            }
            else
            {
                close(output);
            }
        }
    }
    return EXIT_SUCCESS;
}
