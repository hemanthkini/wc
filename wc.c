#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct fileinfo {
    int id;
    char* filepath;
} info;


int *arrayCount = NULL;
static int numOfThreads;
static int bytesPerThread;
static int totalBytes;
pthread_t *arrayThreads = NULL;

void *threadFunction(void * vargp);

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("%d", argc);
        printf("needs a file as argument");
        exit(1);
    }
    FILE * theFile = fopen(argv[1], "r");
    numOfThreads = atoi(argv[2]);
    int fd = fileno(theFile);
    struct stat filestats;
    fstat(fd, &filestats);
    totalBytes = filestats.st_size;
    bytesPerThread = totalBytes / numOfThreads;
    arrayCount = calloc(numOfThreads, sizeof(int));
    fclose(theFile);

    if (arrayCount == NULL)
    {
        printf("couldn't initialize array of counts");
        exit(1);
    }

    arrayThreads = calloc(numOfThreads, sizeof(pthread_t));
    if (arrayThreads == NULL)
    {
        printf("couldn't initialize thread array");
        exit(1);
    }

    int len = strlen(argv[1]) + 1;
    int i;
    for (i = 0; i < numOfThreads; i++)
    {
        info* currInfo = malloc(sizeof(info));
        currInfo->id = i;
        currInfo->filepath = malloc(len);
        strcpy(currInfo->filepath, argv[1]);
        pthread_create(&arrayThreads[i], NULL, &threadFunction, (void *)currInfo);
    }
    int sum = 0;
    for (i = 0; i < numOfThreads; i++)
    {
        pthread_join(arrayThreads[i], NULL);
        sum = sum + arrayCount[i];
    }
    printf("word count: %d\n", sum);

    free(arrayCount);
    free(arrayThreads);
    exit(0);

}

void *threadFunction(void * vargp)
{
    info* currInfo = (info *)vargp;
    FILE * theFile = fopen(currInfo->filepath, "r");
    fseek(theFile, bytesPerThread * currInfo->id, SEEK_SET);
    printf("thread %d starts at offset %d\n", currInfo->id, currInfo->id * bytesPerThread);
    int i = ((currInfo->id == (numOfThreads - 1)) ? (totalBytes + 1) : bytesPerThread);
    int words = 0;
    int test;
    int startWhiteSpace = 0;
    while (i != 0 )
    {
        i--;
        test = fgetc(theFile);
        printf("DEBUG: char:%c\n", test);
        if ((unsigned)test < 33 || (unsigned)test == 127)
        {
            if (!startWhiteSpace)
                startWhiteSpace = 1;
            if (feof(theFile) || test == EOF)
            {
                printf("eof!");
                words++;
                i = 0;
            }
        } /* NEEDS A STRUCT WITH space at beginning, space at end, and words
             in middle  and then reduce across it, like parens problem */
        else
        {
            if (feof(theFile) || test == EOF)
            {
                printf("eof!");
                i = 0;
            }
            if (startWhiteSpace)
            {
                startWhiteSpace = 0;
                words++;
            }
        }

    }
    arrayCount[currInfo->id] = words;
    fclose(theFile);
    free(currInfo->filepath);
    free(currInfo);
    return NULL;
}
