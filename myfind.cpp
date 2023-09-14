#include <iostream>
#include <filesystem>
#include <getopt.h>
#include <assert.h>
#include <strings.h>
#include <string>

namespace fs = std::filesystem;
/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "\nUsage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

void fileSearchthroughDir(char* searchdir, char* filename, bool case_insensitive) {
    for(const auto& entry : fs::directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        //TODO: if directory and option -R recursive search
        //TODO: filesearch works down e.g. if i have another dir after myfind but it doesnt work upwards e.g. if the file is in ../ directories
        //std::cout << "Filename in directory: " << filenameStr << " to search filename: " << filename << " absolute filepath to filname " << fs::path(filename) << std::endl;
        
        //convertion from string to const char* because strncasecmp() compares two char* variables
        const char* filenameStrconv = filenameStr.c_str();
        if(case_insensitive) {
            if(strncasecmp(filenameStrconv, filename, filenameStr.length()) == 0) {
                std::cout << filename << " " << fs::absolute(filename) << std::endl;
                return;    
            } else {
                std::cout << "File not found!" << std::endl;
            }
        }
        if(filenameStr == filename) {
            std::cout << filename << " " << fs::absolute(filename) << std::endl;
        } else {
            std::cout << "File not found!" << std::endl;
        }
    }
    return;
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[])
{
    int c;
    int error = 0;
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
    char* searchdir = argv[optind];

    /* Die restlichen Argumente, die keine Optionen sind, befinden sich in
    * argv[optind] bis argv[argc-1]
    */
    optind++;

    while (optind < argc)
    {
       /* aktuelles Argument: argv[optind] */
       fileSearchthroughDir(searchdir, argv[optind], case_insensitive);

       optind++;
    }
    return EXIT_SUCCESS;
}