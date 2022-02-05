// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

/*
SOURCES:
OSU, CS 344, Module 3, Exploration: Directories
OSU, CS 344, Module 3, Exploration: Files
OSU, CS 344, Module e, Exploration: Strings
student.c from module 1, provided by instructor
https://stackoverflow.com/questions/5889880/better-way-to-concatenate-multiple-strings-in-c/5889984
https://stackoverflow.com/questions/5428632/c-error-undefined-reference-to-itoa
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PREFIX "movies_"
#define ONID "rastellv.movies."

/* struct for movie information */
struct movie
{
    char *title;
    char *year;
    char *languages;
    char *rating;
    struct movie *next;
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*/
struct movie *createMovie(char *currLine)
{
    struct movie *currMovie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // The next token is the year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token);

    // The next token is the languages
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->languages, token);

    // The last token is the rating
    token = strtok_r(NULL, "\r", &saveptr);
    currMovie->rating = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->rating, token);

    // Set the next node to NULL in the newly created movie entry
    currMovie->next = NULL;

    return currMovie;
}

/*
* Return a linked list of students by parsing data from
* each line of the specified file.
*/
struct movie *processFile(char *filePath)
{
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    int count = 0;
    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    fscanf(movieFile, "%*[^\n]%*\n");
    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        // Get a new student node corresponding to the current line
        struct movie *newNode = createMovie(currLine);

        // Is this the first node in the linked list?
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }
    free(currLine);
    fclose(movieFile);
    return head;
}

void processYear(char* filePath, int year, struct movie *list)
{
    // used Module 3, Exploration: files for this code
    char rootPath[256] = "";
    int fd;
    char yearString[5];
    int first = 1;

    // convert year to string
    snprintf(yearString, 5, "%d", year);
    
    while (list != NULL)
    {
        // if year matches year of struct create file for year with movie titles
        if (strncmp(list->year, yearString, strlen(yearString)) == 0)
        {
            // create file path name and file with correct permissions
            snprintf(rootPath, sizeof(rootPath), "./%s/%s.txt", filePath, yearString);
            fd = open(rootPath, O_RDWR | O_CREAT | O_APPEND, 0640);
            
            // catch errors
            if (fd == -1){
                printf("open() failed on \"%s\"\n", rootPath);
                perror("Error");
                exit(1);
	        }
            
            //write titles to file
            char message[256] = "";
            snprintf(message, sizeof(message), "%s\n", list->title);
            write(fd, message, strlen(message));
        }
        
        list = list->next;
    }
}

void createDirectory(char *fileName)
{
    // random seed and create random number
    time_t t;
    srand((unsigned) time(&t));
    int r = rand() % 100000;
    char dest[] = ONID;
    char source[7];
    struct movie *list = processFile(fileName);

    //create directory name and create directory with correct permissions
    snprintf(source, 7, "%d", r);
    strcat(dest, source);
    mkdir(dest, 0750);

    printf("\nCreated directory with name %s", dest);

    // loop through possible years and call function to process year
    for(int i = 1900; i <= 2022; i++){

        processYear(dest, i, list);
    }
}

void processLarge()
{
    // Large parts of this code was taken from Module 3, Exploration: Directories
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    size_t fileSize;
    struct stat dirStat;
    char* fileName = malloc(sizeof(char) * 256);
    int i = 0;
    // const PREFIX = "movies_"

    // Go through all the entries in current directory
    while((aDir = readdir(currDir)) != NULL){

        //check prefix
        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){

            //check file type
            int len = strlen(aDir->d_name);
            char *last_four = &aDir->d_name[len-4];
            if(strncmp(last_four, ".csv", 4) == 0){

                // Get meta-data for the current entry
                stat(aDir->d_name, &dirStat); 

                // compare files sizes to get largest
                if(i == 0 || (dirStat.st_size > fileSize) ){

                    fileSize = dirStat.st_size;
                    memset(fileName, '\0', sizeof(fileName));
                    strcpy(fileName, aDir->d_name);
                }

                i++;

            }
        }

    }
    printf("\nNow processing the largest file named %s\n", fileName);
    createDirectory(fileName);
}

void processSmall()
{
    // Large parts of this code was taken from Module 3, Exploration: Directories
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    size_t fileSize;
    struct stat dirStat;
    char* fileName = malloc(sizeof(char) * 256);
    int i = 0;
    // const PREFIX = "movies_"

    // Go through all the entries in current directory
    while((aDir = readdir(currDir)) != NULL){

        //check prefix
        if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){

            //check file type
            int len = strlen(aDir->d_name);
            char *last_four = &aDir->d_name[len-4];
            if(strncmp(last_four, ".csv", 4) == 0){

                // Get meta-data for the current entry
                stat(aDir->d_name, &dirStat); 

                // compare files sizes to get largest
                if(i == 0 || (dirStat.st_size < fileSize) ){

                    fileSize = dirStat.st_size;
                    memset(fileName, '\0', sizeof(fileName));
                    strcpy(fileName, aDir->d_name);
                }

                i++;

            }
        }

    }
    printf("\nNow processing the smallest file named %s\n", fileName);
    createDirectory(fileName);
}

void processName()
{
    // Large parts of this code was taken from Module 3, Exploration: Directories
    int input;
    DIR* currDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;
    char fileName[256];

    while(input != 1){
        //get file name from user
        printf("Enter the complete file name: ");
        scanf("%256s", fileName);
        
        DIR* currDir = opendir(".");
        // Go through all the entries in current directory
        while((aDir = readdir(currDir)) != NULL){
            //check file name
            if(strncmp(fileName, aDir->d_name, strlen(aDir->d_name)) == 0){
                input = 1;
                break;
            }
        
        }
        // if no file found tell user
        if(input != 1){ 
            printf("\nNo file found by the name %s, try again.\n\n", fileName);
        }
    }
    printf("\nNow processing the chosen file named %s", fileName);
    createDirectory(fileName);
}

int promptUserSecond()
{
    int input;

    printf("\nWhich file do you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n");
    scanf("%d", &input);

    switch(input){
        case 1:
            processLarge();
            break;
        case 2:
            processSmall();
            break;
        case 3:
            processName();
            break;
        default:
            printf("Not a valid option, please select 1, 2 or 3 only.\n");
            break;
    }
}

int promptUser()
{
    int input;

    // prompts for for user
    while( input != 2 ){

        printf("\n1. Select file to process\n");
        printf("2. Exit the program\n\n");

        printf("Enter a choice 1 or 2: ");
        scanf("%d", &input);
        
        switch(input){
            case 1:
                promptUserSecond();
                break;
            case 2:
                break;
            default:
                printf("Not a valid option, please select 1 or 2 only.\n");
                break;
        }
        printf("\n");
    }
    return 0;

}

int main(int argc)
{
    promptUser();
}