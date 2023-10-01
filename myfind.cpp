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
#include <limits.h>

namespace fs = std::filesystem;

char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "Usage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

void writeInPipe(const std::vector<std::string>& foundPathes, int& fd1) 
{
    FILE* writing;
    if ((writing = fdopen(fd1, "w")) == NULL) //dateideskriptor wird als dateipuffer geöffnet und mit writing verbunden
    {
        std::cerr << "Could not open file for writing\n";
        perror("fdopen");
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> allPathes;
    if(foundPathes.empty()) //wenn nichts gefunden ist, ist der vector leer
        allPathes.emplace_back(std::to_string(getpid()) + ": File not found!");

    for(size_t i = 0; i < foundPathes.size(); i+=2) //alles was gefunden wurde, wird im vektor gespeichert
    {
        allPathes.emplace_back(std::to_string(getpid()) + ": " + foundPathes[i] + ": " + foundPathes[i+1]);
    }

    for(size_t i = 0; i < allPathes.size(); i++)
    {
        /* write into pipe */
        std::fprintf(writing, "%s\n", allPathes[i].c_str());
        std::fflush(writing); //alle Daten im Puffer werden sofort in die Pipe geschrieben durch flush, sonst bleibt es im puffer
    }
    /* close pipe */
    fclose(writing);
}

void fileSearch(const std::string& filename, bool case_insensitive, const std::string& filenameStr, std::string absolutePath, std::vector<std::string>& foundPathes)
{
    //convertion from string to const char* because strncasecmp() compares two char* variables
    if(case_insensitive) {
        if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
            foundPathes.emplace_back(absolutePath);
            foundPathes.emplace_back(filenameStr);   
        }
    }
    else if(filenameStr == filename) {
        foundPathes.emplace_back(absolutePath);
        foundPathes.emplace_back(filenameStr);
    }
}

void fileSearchthroughDir(char* searchdir, const std::string& filename, bool recursivOption, bool case_insensitive, std::vector<std::string>& foundPathes)
{   
    if(recursivOption){ //recursive variante
        for(const auto& entry : fs::recursive_directory_iterator(searchdir)) {
            const auto filenameStr = entry.path().filename().string();
            if(fs::is_regular_file(entry))
                fileSearch(filename, case_insensitive, filenameStr, fs::absolute(entry), foundPathes);
        }
    }
    else{
        for(const auto& entry : fs::directory_iterator(searchdir)) {
            const auto filenameStr = entry.path().filename().string();
            fileSearch(filename, case_insensitive, filenameStr, fs::absolute(entry), foundPathes);
        }
    }
    return;
}

/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[])
{
    int c, error = 0, cOptionR = 0, cOptioni = 0;
    bool case_insensitive = false, recursivOption = false;
    program_name = argv[0];
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

    int fd[2];
    char puffer[PIPE_BUF];
    FILE *reading;

    /* create a pipe */
    if (pipe(fd) < 0)
    {
        perror("pipe");
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
                close(fd[0]); //Kinderprozess schließt lesen
                fileSearchthroughDir(searchdir, argv[optind], recursivOption, case_insensitive, foundPathes);
                writeInPipe(foundPathes, fd[1]);
                return EXIT_SUCCESS;
            default:
            optind++;
            //parent
            break;
        }
    }

    close(fd[1]); //Elternprozess schließt schreiben
    if ((reading = fdopen(fd[0], "r")) == NULL) //dateideskriptor wird als dateipuffer geöffnet und mit reading verbunden
    {
        std::cerr << "Could not open file for reading\n";
        perror("fdopen");
        return EXIT_FAILURE;
    }
    while (fgets(puffer, PIPE_BUF, reading) != NULL) //liest aus pipe und schreibt in stdout
    {
        fputs(puffer, stdout);
        fflush(stdout);
    }
    fclose(reading);

    pid_t childpid;
    while((childpid = waitpid(-1, NULL, WNOHANG))) {    //fängt alle Kinderprozesse
        if(childpid == -1 && (errno != EINTR)) {
            break;
        }
    }
    
    return EXIT_SUCCESS;
}