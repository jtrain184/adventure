/*
 * jarrettp.adventure.c - A version of the game Adventure created by Philip Jarrett
 * Must run jarrettp.buildrooms.c before running this program.
 * CS 344 - OS, Winter 2018
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

/* Function protypes */
char * FindDir();
int GetData();
int RunGame();
char * Room();
int FindRoom(char* name);
int FindRoomType(char* type);
int ValidConnection(int index, char* name);
void p_thread();
void* writeTime();
void printTime();

pthread_mutex_t mutex;

/*
 * Struct to hold room data
 */

#define MAX_CONNECTIONS 6


struct Room
{
    char name[32];
    char type[32];
    int numOutboundConnections;
    char outboundConnections[MAX_CONNECTIONS][32]; /*Max connection number of 32 byte strings*/
}RoomsUsed[7];


int main(){
	/* Initialize rooms */
	int i;
	for (i = 0; i < 7; i++){
		memset(RoomsUsed[i].name, '\0', 32);
		RoomsUsed[i].numOutboundConnections = 0;
		memset(RoomsUsed[i].type, '\0', 32);
	}
    /* User introduction */
    printf("Welcome to the game of adventure.\n");
    writeTime();

	/* Build up array of Room structs from input files */
	GetData();

	/* Start game interface */
	RunGame();

    return 0;
}

char * FindDir()
{
    int newestDirTime = -1; // Modified timestamp of newest subdir examined
    char targetDirPrefix[32] = "jarrettp.rooms."; // Prefix we're looking for
    char newestDirName[256]; // Holds the name of the newest dir that contains prefix
    memset(newestDirName, '\0', sizeof(newestDirName));
    char *buffer = malloc(sizeof(char) * 256);


    DIR* dirToCheck; // Holds the directory we're starting in
    struct dirent *fileInDir; // Holds the current subdir of the starting dir
    struct stat dirAttributes; // Holds information we've gained about subdir

    dirToCheck = opendir("."); // Open up the directory this program was run in

    if (dirToCheck != NULL) // Make sure the current directory could be opened
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix (searches for substring)
			{
                stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

                if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }

    closedir(dirToCheck); // Close the directory we opened
	free(fileInDir);

    memset(buffer, '\0', 256);
    strcpy(buffer, newestDirName);
    return buffer;
}

int GetData()
{
	/*
	 * I modified the code I found on this post to loop through the input files
	 * https://stackoverflow.com/a/11737506
	 */
	DIR* FD;
	struct dirent* in_file;
	FILE    *entry_file;
	char    buffer[BUFSIZ];
	char	*dirName;	
	dirName = FindDir(); 

	char filename[100];
	int i = 0; /* Counter for looping through RoomsUsed Array */

	/* Scanning the in directory */
	if (NULL == (FD = opendir (dirName)))
	{
		fprintf(stderr, "Error : Failed to open input directory -\n");
		return 1;
	}
	while ((in_file = readdir(FD)))
	{
		/* On linux/Unix we don't want current and parent directories
         * On windows machine too, thanks Greg Hewgill
         */
		if (!strcmp (in_file->d_name, "."))
			continue;
		if (!strcmp (in_file->d_name, ".."))
			continue;
		/* Open directory entry file for common operation */
		/* TODO : change permissions to meet your need! */
		//Concatenate full directory name using starting room
		sprintf(filename, "./%s/%s", dirName, in_file->d_name);
		entry_file = fopen(filename, "r");
		if (entry_file == NULL)
		{
			fprintf(stderr, "Error : Failed to open entry file - \n");
			return 1;
		}

		/* Doing some stuff with entry_file : */
		/* For example use fgets */

		while (fgets(buffer, BUFSIZ, entry_file) != NULL)
		{
			/* Here I'm using strstr to parse the lines and act accordingly */
			if(strstr(buffer,"ROOM NAME: "))
				sscanf(buffer, "%*s %*s %s", RoomsUsed[i].name);

			if(strstr(buffer,"CONNECTION")){
				sscanf(buffer, "%*s %*s %s", RoomsUsed[i].outboundConnections[RoomsUsed[i].numOutboundConnections]);
				RoomsUsed[i].numOutboundConnections++;
			}


			if(strstr(buffer,"ROOM TYPE:"))
				sscanf(buffer, "%*s %*s %s", RoomsUsed[i].type);
		}
		/* Increment counter for RoomsUsed */
		i++;

		/* When you finish with the file, close it */
		fclose(entry_file);
	}

    free(dirName);
    return 0;
}

int RunGame()
{
	int endRoom = 0; // Flag for reaching the END_ROOM
	int roomNum = FindRoomType("START_ROOM");
	char roomChoice[100];
	char path[50][16];
	int steps = 0;
	int i;
	do{
		printf("\nCURRENT LOCATION: %s\n", RoomsUsed[roomNum].name);
		printf("POSSIBLE CONNECTIONS:");
		for(i = 0; i < RoomsUsed[roomNum].numOutboundConnections; i++) {   //Run through room connections
			printf(" %s,", RoomsUsed[roomNum].outboundConnections[i]);
		}

		printf(". \nWHERE TO? >");
		scanf("%s", roomChoice);

		if(strcmp(roomChoice, "time") == 0){  // User asks for time, call time functions
			p_thread();
			printTime();
		}

		else if(ValidConnection(roomNum, roomChoice) == -1){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		}

		else{
			strcpy(path[steps],roomChoice);
			steps++;
			roomNum = FindRoom(roomChoice);
		}

		if (strcmp(RoomsUsed[roomNum].type,"END_ROOM")==0)
			endRoom = 1;

	} while(endRoom != 1);
	int j;

	/* Print out victory message, num steps, and path to victory. */
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	for(j = 0; j < steps; j++){
		printf("%s\n", path[j]);
	}

	return 0;
}

int FindRoom(char* name)
{
	int i;
	for(i=0; i < 7; i++){
		if(strcmp(RoomsUsed[i].name, name)==0)
			return i;
	}

	return -1;
}
int FindRoomType(char* type)
{
	int i;
	for(i=0; i < 7; i++){
		if(strcmp(RoomsUsed[i].type, type)==0)
			return i;
	}

	return -1;
}

int ValidConnection(int index, char* name){
	int i;
	for (i = 0; i< RoomsUsed[index].numOutboundConnections;i++){
		if (strcmp(RoomsUsed[index].outboundConnections[i],name)==0)
			return 1;
	}
	return -1;
}

void p_thread() {
	pthread_t thread2;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
	//thread creation
	int tid = pthread_create(&thread2, NULL, writeTime, NULL);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	//Sleep function for unlocking time
	usleep(50);
}

void* writeTime() {


	FILE *fp;
	fp = fopen("currentTime.txt", "w+");
	char buff[26];
	struct tm *sTm;

	time_t now = time (0);
	sTm = gmtime (&now);

	strftime (buff, 26, "%Y-%m-%d %H:%M:%S", sTm);
	fputs(buff, fp);
	fclose(fp);

	//return 0;
}

void printTime() {

	char buffer[50];
	FILE *fp;
	fp = fopen("currentTime.txt", "r");
	if(fp == NULL) {
		perror("Error\n");
	}
	else {
		//print buffer line
		fgets(buffer, 50, fp);
		printf("\n%s\n", buffer);
		fclose(fp);
	}
}
