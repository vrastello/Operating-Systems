// If you are not compiling with the gcc option --std=gnu99, then
// uncomment the following line or you might get a compiler warning
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


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

void printByYear(char *year, struct movie *list)
{
    int count = 0;

    while (list != NULL)
    {
        if (strcmp(list->year, year) == 0)
        {
            printf("%s\n", list->title);
            count++;
        }
        
        list = list->next;
    }
    if (count == 0)
    {
        printf("No data about movies released in the year %s\n", year);
    }
    printf("\n");
}

void processLarge()
{
    printf("you have processed large");
}

void processSmall()
{
    printf("you have processed small");
}

void processName()
{
    printf("you have processed name");
}

int promptUserSecond()
{
    int input;
    char* fileName = malloc(sizeof(char) * 20);

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