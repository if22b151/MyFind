#include <iostream>
#include <filesystem>
#include <assert.h>
#include <strings.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "Usage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

void writeInPipe(std::vector<std::string>& foundPathes) {
    FILE* fp;
    if((fp = fopen("foundpathes", "w")) != NULL) {
        for(size_t i = 0; i < foundPathes.size(); i+=2) {
            fprintf(fp, "%d: %s: %s\n", getpid(), foundPathes[i].c_str(), foundPathes[i+1].c_str());
        }
    fclose(fp); 
    } else {
        std::cerr << "Could not open file for writing\n";
    }
    return;
}

void recursivFileSearchthroughDir(char* searchdir, std::string filename, bool case_insensitive, std::vector<std::string>& foundPathes)
{
    
    for(const auto& entry : fs::recursive_directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        if(fs::is_regular_file(entry)){
            if(case_insensitive) {
            //convertion from string to const char* because strncasecmp() compares two char* variables
                if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                    foundPathes.emplace_back(fs::absolute(entry));
                    foundPathes.emplace_back(filenameStr);
                }
            }
            else if (filenameStr == filename) {
                foundPathes.emplace_back(fs::absolute(entry));
                foundPathes.emplace_back(filenameStr);       
            }
        }
    }
    writeInPipe(foundPathes);
    return;
}

void fileSearchthroughDir( char* searchdir, std::string filename, bool case_insensitive, std::vector<std::string>& foundPathes) 
{   
    for(const auto& entry : fs::directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        //convertion from string to const char* because strncasecmp() compares two char* variables
        if(case_insensitive) {
            if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                foundPathes.emplace_back(fs::absolute(entry));
                foundPathes.emplace_back(filenameStr);   
            }
        }
        else if(filenameStr == filename) {
            foundPathes.emplace_back(fs::absolute(entry));
            foundPathes.emplace_back(filenameStr);
        }
    }
    writeInPipe(foundPathes);
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

    if(mkfifo("foundpathes", 0660) == -1) {
        std::cerr << "Could not make pipe\n";
        return EXIT_FAILURE;
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
                std::cerr << "Could not make child\n";
                return EXIT_FAILURE;
            break;
            case 0:
            // child
            if(!recursivOption)
                fileSearchthroughDir(searchdir, argv[optind], case_insensitive, foundPathes);
            else
                recursivFileSearchthroughDir(searchdir, argv[optind], case_insensitive, foundPathes);
            return EXIT_SUCCESS;
            default:
            optind++;
            //parent
            break;
        }
    }
    pid_t childpid;
    while((childpid = waitpid(-1, NULL, WNOHANG))) {
        if(childpid == -1 && (errno != EINTR)) {
            break;
        }
    }
    FILE* fp;
    char buff[2000];
    if((fp = fopen("foundpathes", "r")) != NULL) {
        while(fgets(buff, sizeof(buff), fp) != NULL) {
            fputs(buff, stdout);
        }
        fclose(fp);

        //remove pipe
        if(remove("foundpathes") == -1){
            std::cerr << "Could not remove pipe\n";
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << "Could not open pipe for reading\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}