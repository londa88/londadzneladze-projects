using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct Cell{
	const void *val;
	const void *file;
};

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}
int actorComparision(const void *a1, const void *a2) {
    Cell *cellData = (Cell *)a1;
    int offset = *(int *)a2;
    char *playerName = (char *)cellData->val;
    char * fullName= (char *)cellData->file+offset;

    return strcmp(playerName, fullName);
}

bool imdb::getCredits(const string& player, vector<film>& films) const { 
    int numActors = *(int *)actorFile;
    Cell curr;
    curr.val = player.c_str();
    curr.file = actorFile;      
    int *offsetPtr = (int *)bsearch(&curr,(char *)actorFile + sizeof(int),numActors, sizeof(int),actorComparision);
    if (!offsetPtr) return false;
    char* actorPosition = (char*)actorFile + *offsetPtr;
    int curLen = strlen(actorPosition);
    int finalLen = curLen;
    if (curLen % 2 == 1) finalLen++;
    if (curLen % 2 == 0) finalLen += 2;
    short* numOfMovies = (short*)(actorPosition + finalLen);
    finalLen += sizeof(short);
    int *firstMov;
    if (finalLen % 4 == 0) firstMov=(int*)(numOfMovies+1);
    else firstMov=(int*)((char*)numOfMovies+sizeof(short)+2);
    int movieCount = *numOfMovies;
    for (int i = 0; i < movieCount; i++) {
        int movieOffset = *(firstMov + i);
        char *currentMovie = (char *)movieFile + movieOffset; 
        int len=strlen(currentMovie);
        int movieYear = *(currentMovie + len + 1)+1900;
        film currentFilm;
        currentFilm.title = currentMovie;
        currentFilm.year = movieYear;
        films.push_back(currentFilm);
    }

    return true;
}



int movieComparision(const void *a1, const void *a2) {
    Cell *cellData = (Cell *)a1;
    int offset = *(int *)a2;
    char* movie=(char*)((char*)cellData->file+offset);
    int len=strlen(movie);
    int fileMovieYear = *(char*)(movie + len + 1)+1900;
    film curFilm;
    curFilm.title=movie;
    curFilm.year=fileMovieYear;
    film &targetFilm=*(film*)cellData->val;
    if(targetFilm==curFilm) return 0;
    else if(targetFilm<curFilm) return -1;
    return 1;
    
}

bool imdb::getCast(const film& movie, vector<string>& players) const  {
    int numMovies = *(int *)movieFile;  
    Cell curr;
    curr.val = &movie;        
    curr.file = movieFile;    
    int *offsetPtr = (int *)bsearch(&curr,(char *)movieFile + sizeof(int),numMovies, sizeof(int), movieComparision);
    if (!offsetPtr) return false;
    char* moviePosition = (char*)movieFile + *offsetPtr;
    int curLen = strlen(moviePosition);
    int finalLen = curLen;
    if ((curLen+1) % 2 == 1) finalLen += 2;
    else finalLen += 3;
    short* numOfCast = (short*)(moviePosition + finalLen);
    finalLen += sizeof(short);
    int *firstPl;
    if(finalLen%4==0) firstPl=(int*)(numOfCast+1);
    else firstPl=(int*)((char*)numOfCast+sizeof(short)+2);
    int castCount = *numOfCast;
    for (int index = 0; index < castCount; index++) {
        int actorOffset = *(firstPl + index);
        char *actorName = (char *)actorFile + actorOffset;
        players.push_back(actorName);
    }

    return true; 
} 

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}