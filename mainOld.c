#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include "mergesort.h"
#include <sys/stat.h>

/*
  Dir Traversal(char* filename, char* headerTitle, char* outputDir)
  filename: folder being processed
  headerTitle: the header each csv is sorted by
  outputDir: the folder which will hold the sorted csv files 

 */

int dirTraversal(char* filename, char* headerTitle, char* outputDir)
{   
  int childStatus[255];
  pid_t childProcess[255];
  int processCounter = 0;
  pid_t myPID = getpid();
  struct stat sb;
  if ( outputDir != NULL )
    {
      if( stat(outputDir,&sb) != 0 || !S_ISDIR(sb.st_mode))
	{
	  mkdir(outputDir,0700);	  
	}
    }

  DIR * base = opendir(filename);
  struct dirent * entry = readdir(base);
  
  char nextDir[4095] = "\0";
  char*  output = (char*)malloc(sizeof(char)*4095);
  while( entry != NULL )
    {
      switch( (int)entry->d_type)
	{
	case DT_DIR:
	  memset( nextDir, '\0', 4095);
	  if ( strcmp( entry->d_name, ".\0")  != 0 &&
	       strcmp( entry->d_name, "..\0") != 0)
	    {
	      strcat( nextDir, filename);
	      strcat( nextDir, "/");
	      strcat( nextDir, entry->d_name);

	      childProcess[processCounter] = fork();	     

	      if( childProcess[processCounter] == 0)
		{
		  _exit(dirTraversal( nextDir, headerTitle,outputDir));

		}
	      processCounter++;
	    }
	  break;
	case DT_REG:
	  childProcess[processCounter] = fork();
	   
	  if( childProcess[processCounter] == 0)
	    {
	      myPID = getpid();
	      node* head = (node*)malloc(sizeof(node));
	      head->next = NULL;
	      node* rows = (node*)malloc(sizeof(node));
	      int numberOfHeaders = 0;
	      int chosenField =-1;
	      char filePath[4095];
	      memset( filePath, '\0', 4095);
	      if( strcmp(nextDir,filename) != 0 )
		{
		  memset( filePath, '\0', 4095);		  
		  strcat(filePath,filename);
		  strcat(filePath,"/");
		  strcat(filePath,entry->d_name);
		}
	      strcat( nextDir, filename);
	      strcat( nextDir, "/");
	      strcat( nextDir, entry->d_name);

	      char * extention = (char*)malloc(sizeof(char)*5);
	      extention[4] = '\0';
	      int i =0;
	      int c =3;

	      FILE * unSorted = NULL;
	      FILE * sorted   = NULL;
	      for ( i = sizeof(filePath); i > 0; i--)
		{
		  if ( filePath[i] == '\0')
		    continue;
		  else
		    {
		      extention[c] = filePath[i];
		      c--;
		    }
		  if ( c < 0)
		    break;
		}
	      if ( strcmp(extention,".csv") != 0 )
		{
		  printf("File is not correct\n");
		  _exit(-4);
		}
	      unSorted = fopen(filePath,"r");
	      chosenField = getHeader(head, headerTitle, &numberOfHeaders,&unSorted);
	      if(chosenField == -1)
		{
		  printf("header DOES NOT EXIST\n");
		  _exit(-100);
		}
	      if ( numberOfHeaders != 28)
		{
		  printf("PID: % number of headers wrong\n",myPID);
		  _exit(-2);//File not compatible
		}
	      char* unsortedFile = (char*)malloc(sizeof(char)*1500);
	      memset( unsortedFile, '\0', 1500);


	      int z =0;
	      char* fNAME = (char*)malloc(sizeof(char)*4095);
	      memset( fNAME, '\0', 4095);

	      if(outputDir != NULL)
		{
		  for(z=0; z<strlen(entry->d_name)-4; z++)
		    {
		      fNAME[z] = entry->d_name[z];
		    }
		  strcat( unsortedFile,outputDir);
		  strcat( unsortedFile,"/");
		  strcat( unsortedFile,fNAME);
		  strcat( unsortedFile,"-");
		  strcat( unsortedFile,"sorted");
		  strcat( unsortedFile,"-");
		  strcat( unsortedFile,headerTitle);
		  strcat( unsortedFile,".csv");
		  sorted = fopen( unsortedFile, "w");
		}
	      else
		{
		  if ( filePath == NULL)
		    {
		      for(z=0; z<strlen(nextDir)-4; z++)
			{
			  fNAME[z] = nextDir[z];
			}
		    }
		  else
		    {
		      for(z=0; z<strlen(filePath)-4; z++)
			{
			  fNAME[z] = filePath[z];
			}
		    }
		  strcat( unsortedFile,fNAME);
		  strcat( unsortedFile,"-");
		  strcat( unsortedFile,"sorted");
		  strcat( unsortedFile,"-");
		  strcat( unsortedFile,headerTitle);
		  strcat( unsortedFile,".csv");
		  sorted = fopen(unsortedFile,"w");
		}
	      readData( rows, numberOfHeaders, chosenField, &unSorted);
	      mergeSort(&rows, chosenField, checkString); 
	      head->next = rows;

	      printData(head,numberOfHeaders,&sorted);
	      freeNode(head);
	      _exit(1);
	      
	    }
	  break;
	    
	}
      processCounter++;


      entry = readdir(base);
    }
  waitpid(-1,NULL,0);
  return 0;

}

int main( int argc, char ** argv )
{
    
  int options;
  int flagC =0;
  int flagD =0;
  int flagO =0;
  char * headerTitle = (char*)malloc(sizeof(char)*1000);
  char * searchDir   = (char*)malloc(sizeof(char)*4095);
  char * outputDir   = (char*)malloc(sizeof(char)*4095);
  memset( headerTitle, '\0',(sizeof(char)*1000));
  memset( searchDir, '\0', (sizeof(char)*4095));
  memset( outputDir, '\0', (sizeof(char)*4095));

  printf("BEFORE WHILE\n");
  while( (options = getopt(argc,argv,"c:d:o:")) != -1 )
    {
      switch(options)
	{
	case 'c':
	  headerTitle = optarg;
	  flagC = 1;
	  break;
	case 'd':
	  flagD = 1;
	  searchDir = optarg;
	  break;
	case 'o':
	  flagO = 1;
	  outputDir = optarg;
	  break;
	}
    }
  if( flagC == 0 )
    {
      printf( "No -c option: Please try again with a header to sort by\n");
      return -1;
    }
  if ( flagD != 1 || searchDir == NULL)
    {
      searchDir = getcwd(NULL,0);
      printf("flag d\n");
    }
  if ( flagO != 1 || outputDir == NULL)
    {
      outputDir = 0;
    }
  
  printf("%s  %s\n",searchDir,headerTitle);
  dirTraversal(searchDir, headerTitle,outputDir);

  return 0;
}
