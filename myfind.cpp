#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;
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
    //char tmp[256];
   std::string currentpath = std::filesystem::current_path();
   std::cout << currentpath << std::endl;
   int c;
   int error = 0;
   char *inputFile = NULL;
   int cOptionA = 0;

   program_name = argv[0];

   while ((c = getopt(argc, argv, "Ri")) != EOF)
   {
      switch (c)
      {
      case 'R':        /* Option ohne Argument */
         if (cOptionA) /* mehrmalige Verwendung? */
         {
            error = 1;
            break;
         }
         cOptionA = 1;
         printf("%s: parsing option a\n", program_name);
         break;
      case 'i':                 /* Option mit Argument */
         if (inputFile != NULL) /* mehrmalige Verwendung? */
         {
            error = 1;
            break;
         }
         inputFile = optarg; /* optarg zeigt auf Optionsargument */
         printf("%s: parsing option f, argument: %s\n", program_name, inputFile);
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