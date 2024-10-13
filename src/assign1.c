#include "../include/display.h"

// args work
#include <getopt.h>
#include <stdbool.h>

// std
#include <stdlib.h>

// r/w
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const int LOWER_UPPERCASE = 64;
static const int UPPER_UPPERCASE = 91;
static const int LOWER_LOWERCASE = 96;
static const int UPPER_LOWERCASE = 123;
static const int ALPHA_DIFF      = 32;

static int lower_filter(int ch)
{
    if(ch < UPPER_UPPERCASE && LOWER_UPPERCASE < ch)
    {
        ch += ALPHA_DIFF;
    }
    return ch;
}

static int upper_filter(int ch)
{
    if(ch < UPPER_LOWERCASE && LOWER_LOWERCASE < ch)
    {
        ch -= ALPHA_DIFF;
    }
    return ch;
}

static int null_filter(int ch)
{
    return ch;
}

static void parse_args(int argc, char *argv[], char **f_val, char **i_val, char **o_val)
{
    int       opt;
    const int ARGS_NEEDED = 7;

    if(argc != ARGS_NEEDED)
    {
        display("ERROR: INCORRECT NUMBER OF ARGUMENTS");
        exit(EXIT_FAILURE);
    }
    while((opt = getopt(argc, argv, "hi:o:f:")) != -1)
    {
        switch(opt)
        {
            case 'f':
                *f_val = optarg;
                break;
            case 'i':
                *i_val = optarg;
                break;
            case 'o':
                *o_val = optarg;
                break;
            default:
                display("ERROR: INVALID ARGUMENT");
                exit(EXIT_FAILURE);
        }
    }
    if(*f_val == NULL || (strcmp(*f_val, "lower") != 0 && strcmp(*f_val, "upper") != 0 && strcmp(*f_val, "null") != 0))
    {
        display("ERROR: INVALID FILTER");
        exit(EXIT_FAILURE);
    }
}

static void process_files(char **i_val, char **o_val, int (*operation)(int))
{
    int  input  = open(*i_val, O_RDONLY | O_CLOEXEC);
    int  output = open(*o_val, O_WRONLY | O_CLOEXEC | O_TRUNC | O_CREAT, S_IRWXU);
    char ch     = 'a';

    if(input != -1 && output != -1)
    {
        while(read(input, &ch, 1) > 0)
        {
            int chInt = (unsigned char)ch;

            chInt = operation(chInt);

            write(output, &chInt, sizeof(char));
        }
        close(output);
        close(input);
    }

    else
    {
        if(output != -1)
        {
            display("ERROR: INVALID INPUT");
            close(output);
            exit(EXIT_FAILURE);
        }
        else
        {
            display("ERROR: INVALID OUTPUT");
            close(input);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    char *f_val = NULL;
    char *i_val = NULL;
    char *o_val = NULL;

    parse_args(argc, argv, &f_val, &i_val, &o_val);

    if(i_val != NULL && o_val != NULL && f_val != NULL)
    {
        if(strcmp(i_val, o_val) == 0)
        {
            display("ERROR: INPUT AND OUTPUT ARE THE SAME FILE");
            exit(EXIT_FAILURE);
        }
        if(strcmp(f_val, "lower") == 0)
        {
            process_files(&i_val, &o_val, lower_filter);
        }
        if(strcmp(f_val, "null") == 0)
        {
            process_files(&i_val, &o_val, null_filter);
        }
        if(strcmp(f_val, "upper") == 0)
        {
            process_files(&i_val, &o_val, upper_filter);
        }
    }
    return EXIT_SUCCESS;
}
