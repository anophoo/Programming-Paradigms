using namespace std;
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

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
//shedarebis funqcia
int compar (const void* pkey, const void* pelem) {
  const void* actorF = (* (info *) pkey).addr;
  string str = (* (info *) pkey).name;
  int nBytes = * (int *) pelem;
  char *newStr = (char *) actorF + nBytes;
  string str2(newStr);
  if (str2 == str)
    return 0; 
  if (str < str2)
    return -1;
  else 
    return 1;
}

int compar2 (const void* pkey, const void* pelem) {
  const void* actorF = (* (info2 *) pkey).addr;
  string str = (* (info2 *) pkey).name;
  int nBytes = * (int *) pelem;
  char *newStr = (char *) actorF + nBytes;
  string str2(newStr);
  char year = *(char*) (newStr + str2.size() + 1 );
  int happyNewYear = (int) year + 1900;
  int curYear = (* (info2 *) pkey).year;
  if (str2 == str){
    if (curYear == happyNewYear)
      return 0; 
    else if (curYear > happyNewYear)
      return -1;
    else
      return 1;
  }
  if (str < str2)
    return -1;
  else 
    return 1;
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
  int *t = (int *) actorFile;
  size_t num = *t;
  info key;
  key.name = player;
  key.addr = (char*)actorFile;
  void* actorf =  bsearch (&key, t, num, sizeof(int), compar); // null
  if (actorf == NULL) {
    return false;
  }
  int offset = * (int *) actorf;
  void* actorRecord = (char*) actorFile + offset;
  int strLength = player.size(); // +-1
  if (strLength % 2 == 0) {
    strLength++; // aq vumateb \0-s
  }
  short numMovies = * (short *)((char*) actorRecord + strLength + 1);
  int moviesOffset = strLength + 1 + 2; // 1 for \0 and 2 for short 
  if (moviesOffset % 4 == 2)
    moviesOffset += 2;
  int* currentMovie = (int*) ((char*) actorRecord + moviesOffset); // int-ebis masivze mimtitebeli
  for (short i = 0; i < numMovies; i++)
  {
    int* curMovie = (int*)(currentMovie + i * sizeof(char));
    int curOffset = (* curMovie);
    void* current = (char*)((char*) movieFile + curOffset); // movieFile shemodis aq pirvelad
    string s((char*) current);
    int sizeOfMovieName = strlen((char*)current);
    int offsetOfYear = sizeOfMovieName + 1; // aq ar emateba

    char delta = * (char*)((char *) current + offsetOfYear); // aq ra vqna?
    int year = 1900 + (int) (delta); // es ra ari 
    string name((char*)current);

    film curFilm;
    curFilm.year = year;
    curFilm.title = name;
    films.push_back(curFilm);
  }
  if (films.empty())
    return false;
  else
    return true;
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int *t = (int *) movieFile;
  size_t num = *t;
  info2 key;
  key.name = movie.title;
  key.year = movie.year;
  key.addr = (char*)movieFile;
  void* movief =  bsearch (&key, t, num, sizeof(int), compar2); // null
  if (movief == NULL) {
    return false;
  }
  int offset = * (int *) movief;
  void* actorRecord = (char*) movieFile + offset;
  int strLength = (key.name).size(); // +-1
  if (strLength % 2 == 1) {
    strLength ++;
  }
  // year-s davikideb imito, ro ar mchirdeba saertod
  short numActors = * (short *)((char*) actorRecord + strLength + 1 + 1); // 1 for \0 and 1 for year
  int actorsOffset = strLength + 1 + 1 + 2; // 1 for \0, 1 for year and 2 for short 
  
  if (actorsOffset % 4 == 2)
    actorsOffset += 2;
  int* currentMovie = (int*) ((char*) actorRecord + actorsOffset); // int-ebis masivze mimtitebeli
  for (short i = 0; i < numActors; i++)
  {
    int* curActor = (int*)(currentMovie + i * sizeof(char));
    int curOffset = (* curActor);
    void* current = (char*)((char*) actorFile + curOffset); // actorFile shemodis aq pirvelad
    string s((char*) current);
    string name((char*)current);
    players.push_back(name);
  }
  if (players.empty()) 
    return false;
  else 
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
