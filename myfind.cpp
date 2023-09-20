#include <iostream>
#include <filesystem>
#include <assert.h>
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>


namespace fs = std::filesystem;
/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage()
{
    std::cerr << "Usage:" << program_name << "[-R] [-i] searchpath filename1 [filename2] ...[filenameN]" << std::endl;
    exit(EXIT_FAILURE);
}

void writeInPipe(const char* filenameStr, const char* absolutePath)
{
    FILE *fp;
    if(!fs::exists("foo"))
    {
        if (mkfifo("foo", 0660) == -1)
        {
            fprintf(stderr, "myfifo: Error creating fifo foo\n");
            return;
        }
    }

    if ((fp = fopen("foo", "w")) != NULL)
    {
        fprintf(fp, "%d: %s: %s \n", getpid(), filenameStr, absolutePath);
    }
    fclose(fp);

}

void recursivFileSearchthroughDir(char* searchdir, std::string filename, bool case_insensitive)
{
    for(const auto& entry : fs::recursive_directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        if(fs::is_regular_file(entry)){
            if(case_insensitive) {
            //convertion from string to const char* because strncasecmp() compares two char* variables
                if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                    writeInPipe(filenameStr.c_str(), fs::absolute(entry).c_str());
                }
            }
            else if (filenameStr == filename) {
                writeInPipe(filenameStr.c_str(), fs::absolute(entry).c_str());           
            }
            else
            {
                std::cerr << "File not found!" << std::endl;
                return;
            }
        }
    }
    return;
}

void fileSearchthroughDir( char* searchdir, std::string filename, bool case_insensitive) 
{   
    for(const auto& entry : fs::directory_iterator(searchdir)) {
        const auto filenameStr = entry.path().filename().string();
        //convertion from string to const char* because strncasecmp() compares two char* variables
        if(case_insensitive) {
            if(strncasecmp(filenameStr.c_str(), filename.c_str(), filenameStr.length()) == 0) {
                writeInPipe( filenameStr.c_str(), fs::absolute(entry).c_str());
                return;    
            }
        }
        if(filenameStr == filename) {
            writeInPipe( filenameStr.c_str(), fs::absolute(entry).c_str());
            return;
        }
    }
    std::cerr << "File not found!" << std::endl;
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
                fileSearchthroughDir( searchdir, argv[optind], case_insensitive);
            else
                recursivFileSearchthroughDir(searchdir, argv[optind], case_insensitive);
            optind++;
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