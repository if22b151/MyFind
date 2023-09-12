#include <iostream>
#include <filesystem>
#include <getopt.h>
#include <assert.h>

/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "Usage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[])
{
    char* searchdir = argv[1];

    if(searchdir == NULL)
    {
        print_usage();
    }

    int c;
    int error = 0;
    char *inputFile = NULL;
    int cOptionR = 0;
    int cOptioni = 0;
    program_name = argv[0];
    bool case_insensitive = false;

    while ((c = getopt(argc, argv, "Ri")) != EOF)
    {
       switch (c)
       {
       case 'R':        /* Option ohne Argument */
            if (cOptionR) /* mehrmalige Verwendung? */
            {
                error = 1;
                break;
            }
            cOptionR = 1;
            break;
        case 'i':                 /* Option mit Argument */
            if (cOptioni) /* mehrmalige Verwendung? */
            {
                error = 1;
                break;
            }
            cOptioni = 1;
            case_insensitive = true;
            break;
            case '?': /* ungueltiges Argument */
                error = 1;
                break;
            default: /* unmoeglich */
                assert(0); 
        }
    }
    if (error) /* Optionen fehlerhaft ? */
    {
       print_usage();
    }
    if ((argc < optind + 1) || (argc > optind + 2)) /* falsche Anzahl an Optionen */
    {
       print_usage();
    }

    /* Die restlichen Argumente, die keine Optionen sind, befinden sich in
    * argv[optind] bis argv[argc-1]
    */
    while (optind < argc)
    {
       /* aktuelles Argument: argv[optind] */
       printf("%s: parsing argument %s\n", program_name, argv[optind]);

       optind++;
    }
    return EXIT_SUCCESS;
}