//
// A program that builds rooms for an Adventure-like game
// by Philip Jarrett
// CS 344, Winter 2018
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include<sys/stat.h>
#include <unistd.h>

// Function prototypes
int IsGraphFull();
void AddRandomConnection();
struct Room GetRandomRoom();
int CanAddConnectionFrom(struct Room x);
int ConnectionAlreadyExists(struct Room x, struct Room y);
void ConnectRoom(struct Room x, struct Room y);
int IsSameRoom(struct Room x, struct Room y);
void WriteOutput(char *directory);

// Creates a directory to store the files in with my OSU ID and the process id
char *createDir() {

    int pid = getpid();
    char* dirAddress = "jarrettp.rooms.";
    char *directory = malloc(30);

    sprintf(directory, "%s%d", dirAddress, pid);

    //Need to add 2nd arg 0700 for linux dev
    mkdir(directory, 0700);
    return directory;
}

// Takes two integers and generates a random number in that range inclusive
int GetRandomNum(int lower, int upper)
{
    int num = rand() % (upper + 1 - lower) + lower;
    return num;
}

#define MAX_CONNECTIONS 6

struct Room
{
    char* name;
    char* type;
    int numOutboundConnections;
    struct Room* outboundConnections[MAX_CONNECTIONS];
}RoomsUsed[7];

// Room Names
char* RoomNames[10] = {"CasterlyRock", "Riverlands", "Dorne", "Braavos", "TheWall", "TheVale", "Volantis", "VaesDothrak", "Winterfell", "IronIslands"};

char* RoomTypes[3] = {"START_ROOM", "MID_ROOM", "END_ROOM"};

int main()
{
    srand(time(0)); // seed random number generator

    // Make directory
    char *roomDir = createDir();

    // Make randomly named rooms
    int i;
    for(i = 0; i < 7; i++) {
        int taken, j, k;
        taken=1;
        while(taken) {
            j = rand()%10;
            taken = 0;
            for (k = 0; k < 7; k++) {
                if(RoomsUsed[k].name == RoomNames[j]) {
                    taken = 1;
                }
            }
        }
        RoomsUsed[i].name = RoomNames[j];
        RoomsUsed[i].numOutboundConnections = 0;
        RoomsUsed[i].type = RoomTypes[1];
    }
        RoomsUsed[0].type = RoomTypes[0];
        RoomsUsed[6].type = RoomTypes[2];

    // Create all connections in graph

    while (IsGraphFull() == 0) {
        AddRandomConnection();
    }

    WriteOutput(roomDir);
    free(roomDir);
    return 0;
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
    int IsGraphFull() {

        if (RoomsUsed[0].numOutboundConnections > 3 &&
            RoomsUsed[1].numOutboundConnections > 3 &&
            RoomsUsed[2].numOutboundConnections > 3 &&
            RoomsUsed[3].numOutboundConnections > 3 &&
            RoomsUsed[4].numOutboundConnections > 3 &&
            RoomsUsed[5].numOutboundConnections > 3 &&
            RoomsUsed[6].numOutboundConnections > 3) return 1;

        else {
            return 0;
        }
    }

// Adds a random, valid outbound connection from a Room to another Room
    void AddRandomConnection() {
        struct Room A;  // Maybe a struct, maybe global arrays of ints
        struct Room B;

        while (1) {
            A = GetRandomRoom();

            if (CanAddConnectionFrom(A) == 1)
                break;
        }

        do {
            B = GetRandomRoom();
        } while (CanAddConnectionFrom(B) == 0 || IsSameRoom(A, B) == 1 || ConnectionAlreadyExists(A, B) == 1);

        ConnectRoom(A, B);  // Add connection both to and from rooms A and B

    }

// Returns a random Room, does NOT validate if connection can be added
    struct Room GetRandomRoom() {
        struct Room randomRoom;
        // Generate random index to choose a room
        int r = rand() % 7;
        randomRoom = RoomsUsed[r];
        return randomRoom;
    }

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
    int CanAddConnectionFrom(struct Room x) {
        if(x.numOutboundConnections < MAX_CONNECTIONS){
            return 1;
        }
        else{
            return 0;
        }
    }
// Returns true if a connection from Room x to Room y already exists, false otherwise
    int ConnectionAlreadyExists(struct Room x, struct Room y) {
        int numOut = x.numOutboundConnections;
        int i;
        for (i=0; i < numOut; i++){
            if(strcmp(x.outboundConnections[i]->name, y.name) == 0)
                return 1;
        }

        return 0;
    }

// Connects Rooms x and y together, does not check if this connection is valid
    void ConnectRoom(struct Room x, struct Room y) {
        int i;
        int from, to;
        for(i=0; i < 7; i++){
            // Find from room name, copy index
            if(strcmp(x.name,RoomsUsed[i].name)==0){
                from = i;
            }
            // Find to room name, copy index
            if(strcmp(y.name,RoomsUsed[i].name)==0){
                to = i;
            }
        }
    RoomsUsed[from].outboundConnections[RoomsUsed[from].numOutboundConnections] = &RoomsUsed[to];
    RoomsUsed[from].numOutboundConnections++;
    RoomsUsed[to].outboundConnections[RoomsUsed[to].numOutboundConnections] = &RoomsUsed[from];
    RoomsUsed[to].numOutboundConnections++;
    }

// Returns true if Rooms x and y are the same Room, false otherwise
    int IsSameRoom(struct Room x, struct Room y) {
        if(strcmp(x.name, y.name)== 0) {// compare room names to see if same.
            return 1;
        }
        else{
            return 0;
        }
    }

void WriteOutput(char *directory)
{
    // change into the room file directory
    chdir(directory);

    int i, j;

    // Open a file for writing, write the room name first
    for(i = 0; i < 7; i++) {

        FILE *fp = fopen(RoomsUsed[i].name, "w");
        fprintf(fp, "ROOM NAME: %s\n", RoomsUsed[i].name);

        // Write
        for(j=0; j<RoomsUsed[i].numOutboundConnections; j++) {
            fprintf(fp, "CONNECTION %d: %s\n", j+1, RoomsUsed[i].outboundConnections[j]->name);
        }

        //Write room type
        fprintf(fp, "ROOM TYPE: %s\n", RoomsUsed[i].type);

        //Close file
        fclose(fp);
    }
}
