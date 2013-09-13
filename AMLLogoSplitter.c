/*   AMLLogoSplitter.c
 *      
 *   Copyright 2013 Bartosz Jankowski
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#pragma pack(1)

struct AMLLogoHeader  // sizeof 0x40
{
	int magic;			//0x00 - 0x27051956
	int _2;				//0x04 - zera
	int filesize;		//0x08
	int address;		//0x0C
	int _6;				//0x10 - zera
	int nextlogofile;	//0x14
	int _8;				//0x18 - zera
	int fileindex;		//0x1C - 0x0B00, 0x0B01, 02, ...,  0x0B3B
	char filename[32];  //0x20 - 'bootup' , 
};

#pragma pack()

static void usage(char **argv)
{
	printf("Usage: (extract) %s file\n", argv[0]);
	printf("       (repack) %s directory\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int c, i_opt = 0;
	FILE * f;
	char *fname = 0;
	struct stat sb = {0};
	
	printf("===========================================\n"
		   "AMLogic Images Splitter v. 0.1 by lolet\n"
		   "===========================================\n");
		
	if (argc != 2 )
		usage(argv);
	
	fname = argv[1];
	
	
    if (stat(argv[1], &sb) == -1) 
	{
     		printf("File or directory doesn't exists!\n");
			exit(EXIT_FAILURE);
	}
	
    if ((sb.st_mode & S_IFMT) == S_IFREG)
    {
    // not a directory
    f = fopen(argv[1], "rb");  
    printf ("Input file: %s\n", fname);
	
    fseek (f , 0 , SEEK_END);
	int s = ftell (f);
	rewind (f);
	if(s<sizeof(struct AMLLogoHeader)) 
	{
		printf("File is too small!\n");
		fclose(f);
		exit(EXIT_FAILURE);
	}
		
	mkdir("out", 777);
	while(!feof(f))
	{
	struct AMLLogoHeader amlh = {0};
	fread(&amlh, sizeof(amlh), 1, f);
	
	if(amlh.magic != 0x27051956) 
	{
		printf("Wrong file signature!\n");
		fclose(f);
		exit(EXIT_FAILURE);
	}
	printf("-> Extracting: %d - '%s'\n",amlh.fileindex, amlh.filename);
	char * file = malloc(amlh.filesize);
	fread(file,amlh.filesize,1,f);
	
	char outfile[36] = {0};
	if(file[0] == 'B' && file[1] == 'M') sprintf(outfile,"./out/%s.bmp",amlh.filename);
	else  sprintf(outfile,"./out/%s",amlh.filename);
	
	FILE * e = fopen(outfile,"wb");
	fwrite(file,amlh.filesize,1,e);
	fclose(e);
	free(file);
	if(!amlh.nextlogofile) break;
	fseek(f, amlh.nextlogofile, SEEK_SET);
	}
	
	fclose(f);
  }
  else if((sb.st_mode & S_IFMT) == S_IFDIR)
  {
   printf ("Input directory: %s\n", fname);
	// assume its a dir
  }
  else
  {
	printf ("File isn't file or directory!\n");
	exit(EXIT_FAILURE);
  }
	
	
	printf("-->Done.\n");
	return EXIT_SUCCESS;
}
