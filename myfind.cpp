#include <iostream>
#include <filesystem>
#include <assert.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;
/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "Usage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

void writeInTerminal(std::vector<std::string> foundPathes)
{
    if(!foundPathes.empty())
    {
        for(size_t i = 0; i < foundPathes.size(); i+=2)
        {
            std::cout <<  getpid() << ": " << foundPathes[i] << ": " << foundPathes[i+1] << '\n';
        }
    } else {
        std::cerr << "File not found!" << std::endl;
    }
}

void recursivFileSearchthroughDir(char* searchdir, std::string filename, bool case_insensitive, std::vector<std::string>& foundPathes)
{
    
    for(const auto& entry : fs::recursive_directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        if(fs::is_regular_file(entry)){
            if(case_insensitive) {
            //convertion from string to const char* because strncasecmp() compares two char* variables
                if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                    foundPathes.emplace_back(filenameStr);
                    foundPathes.emplace_back(fs::absolute(entry));
                }
            }
            else if (filenameStr == filename) {
                foundPathes.emplace_back(filenameStr);
                foundPathes.emplace_back(fs::absolute(entry));         
            }
        }
    }
    return;
}

void fileSearchthroughDir( char* searchdir, std::string filename, bool case_insensitive, std::vector<std::string>& foundPathes) 
{   
    for(const auto& entry : fs::directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        //convertion from string to const char* because strncasecmp() compares two char* variables
        if(case_insensitive) {
            if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                foundPathes.emplace_back(filenameStr);
                foundPathes.emplace_back(fs::absolute(entry));
            }
        }
        else if(filenameStr == filename) {
            foundPathes.emplace_back(filenameStr);
            foundPathes.emplace_back(fs::absolute(entry));
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
    bool recursivOption = false;
    std::vector<std::string> foundPathes;
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
            recursivOption = true;
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
        case '?':          /* ungueltiges Argument */
            error = 1;
            break;
        default:           /* unmoeglich */
            assert(0); 
        }
    }
    if (error) /* Optionen fehlerhaft ? */
    {
        print_usage();
    }
    if ((argc <= optind + 1))  /* falsche Anzahl an Optionen */
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
        pid_t pid = fork();
        switch(pid)
        {
            case -1:
                std::cerr << "You are a Failure\n";
                return EXIT_FAILURE;
            break;
            case 0:
            // child
            if(!recursivOption)
                fileSearchthroughDir( searchdir, argv[optind], case_insensitive, foundPathes);
            else
                recursivFileSearchthroughDir(searchdir, argv[optind], case_insensitive, foundPathes);
            writeInTerminal(foundPathes);
            return EXIT_SUCCESS;
            default:
            optind++;
            //parent
            break;
        }
    }
    wait(NULL);
    return EXIT_SUCCESS;
}