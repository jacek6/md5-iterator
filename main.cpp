#include <iostream>
#include <string>

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cctype>


// https://stackoverflow.com/questions/54871085/is-it-a-good-practice-to-call-pthread-sigmask-in-a-thread-created-by-stdthread
#include <signal.h>
#include <time.h>

#include "md5.h"

using std::string;

#define MD5_LEN 32
#define MD5_HASH char[MD5_LEN]

#define MAX_PASSWORDS     100 
#define PASSWORD_DESCRIPTION_LEN 1000
#define MAX_DIGITS 20

string md5_func(const std::string&  text) {
    return md5(text);
}

char *dictWords;
int* dictWordsIndexes;
int dictLen;

char passwords[MAX_PASSWORDS][MD5_LEN];
char passwordsDescritpions[MAX_PASSWORDS][PASSWORD_DESCRIPTION_LEN];
bool crackedPasswords[MAX_PASSWORDS];
int totalCrackedPasswords;
int loadedPasswords;
string decodedPasswords[MAX_PASSWORDS];

static volatile sig_atomic_t sig_caught = 0;

void handle_sighup(int signum) 
{
    /* in case we registered this handler for multiple signals */ 
    if (signum == SIGHUP) {
        sig_caught = 1;
    }
}

void loadDict(string inputFile) {
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    std::ifstream infile(inputFile);
    std::string line;
    
    int numberOfLines = 0;
    int newDictLen = 0;
    
    while (std::getline(infile, line))
    {
        numberOfLines++;
        newDictLen += line.length();
    }
    
    dictWords = (char*) malloc(newDictLen);
    dictWordsIndexes = (int*) malloc(4*(numberOfLines + 3));
    dictLen = numberOfLines;
    
    std::ifstream infile2(inputFile);
    int dictIndex = 0;
    int wordIndex = 0;
    
    while (std::getline(infile2, line))
    {
        dictWordsIndexes[wordIndex++] = dictIndex;
        for(int i=0; i<line.length(); i++) {
            dictWords[dictIndex++] = line.at(i);
        }
    }
    dictWordsIndexes[wordIndex] = dictIndex;
    
    dictWordsIndexes[wordIndex++] = dictIndex;
    dictWordsIndexes[wordIndex++] = dictIndex;
    dictWordsIndexes[wordIndex++] = dictIndex;
    
    //std::cout << " dictWordsIndexes[0] = " << dictWordsIndexes[0] << "\n";
    //std::cout << " dictWordsIndexes[1] = " << dictWordsIndexes[1] << "\n";
}

char to_lower_char(char ch) {
    if('A' <= ch and 'Z' >= ch) {
        return ch - ('A' - 'a');
    }
    return ch;
}

void loadPasswords(string inputFile) {
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    std::ifstream infile(inputFile);
    std::string line;
    
    int passwordIndex = 0;
    
    while (std::getline(infile, line))
    {
        if(passwordIndex >= MAX_PASSWORDS) {
            std::cout << "Reached max number of passwords";
            return;
        }
        if(line.length() == 0) continue;
        if(line.length() < MD5_LEN) {
            std::cout << "Too short line";
            return;
        }
        if(line.length() >= PASSWORD_DESCRIPTION_LEN) {
            std::cout << "Too long line";
            return;
        }
        int i;
        for(i=0; i<line.length(); i++) {
            passwordsDescritpions[passwordIndex][i] = line.at(i);
        }
        passwordsDescritpions[passwordIndex][i] = 0;
        for(i=0; i<MD5_LEN; i++) {
            passwords[passwordIndex][i] = to_lower_char(line.at(i));
        }
        crackedPasswords[passwordIndex] = false;
        passwordIndex++;
    }
    loadedPasswords = passwordIndex;
    totalCrackedPasswords = 0;
    std::cout << "Loaded " << passwordIndex << " passwords\n";
}

void iterDict() {
    for(int wordIndex=0; wordIndex<dictLen; wordIndex++) {
        //std::cout << " wordIndex = " << wordIndex << "\n";
        for(int i=dictWordsIndexes[wordIndex]; i<dictWordsIndexes[wordIndex+1]; i++) {
            std::cout << dictWords[i];
        }
        std::cout << "\n";
    }
}

bool isPasswordMatch(string passwordHash, int passwordIndex) {
    for(int i=0; i<MD5_LEN; i++) {
        if(passwordHash.at(i) != passwords[passwordIndex][i]) return false;
    }
    return true;
}

void consumerRegisterPasswordAsCracked(string password) {
    std::cout << "zalamano haslo " << password << "\n";
    
    string hash = md5_func(password);
    for(int i=0; i<loadedPasswords; i++) {
        if(crackedPasswords[i]) continue;
        if(isPasswordMatch(hash, i)) {
            //std::cout << "match!!";
            decodedPasswords[i] = password;
            crackedPasswords[i] = true;
            std::cout << "zlamane haslo '" << password << "' dla " << passwordsDescritpions[i] << "\n";
            totalCrackedPasswords++;
        }
    }
}


pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;
string crackedPassword;
bool canSendPasswordNow = false;

void sendCrackedPassword(string password) {
    while(true) {
        if(!canSendPasswordNow) {
            continue;
        }
        pthread_mutex_lock(&count_mutex);
        if(!canSendPasswordNow) {
            pthread_mutex_unlock(&count_mutex);
            continue;
        } else {
            crackedPassword = password;
            pthread_cond_signal(&count_threshold_cv);
            pthread_mutex_unlock(&count_mutex);
            break;
        }
    }
}

void *consumer(void *t) {
    while(true) {
        if (sig_caught) {
            sig_caught = 0;
            std::cout << "Zlamano " << totalCrackedPasswords << " z " << loadedPasswords << " wszystkich hasel\n";
        }
        
        pthread_mutex_lock(&count_mutex);
        canSendPasswordNow = true;
        
        timespec ts;
        ts.tv_sec = time(NULL) + 1;
        
        if(pthread_cond_timedwait(&count_threshold_cv, &count_mutex, &ts) == 0) {
            canSendPasswordNow = false;
            //std::cout << "GOT proper singal  ";
            consumerRegisterPasswordAsCracked(crackedPassword);
            pthread_mutex_unlock(&count_mutex);
        } else {
            // timeout
            pthread_mutex_unlock(&count_mutex);
        }
    }
    pthread_exit (NULL);
}

void tryOutPassword(string pass) {
    string hash = md5_func(pass);
    for(int i=0; i<loadedPasswords; i++) {
        if(crackedPasswords[i]) continue;
        if(isPasswordMatch(hash, i)) {
            //std::cout << "match " << hash << " and " << passwords[i] << "\n";
            sendCrackedPassword(pass);
        }
    }
}

bool producer0_killed = false;
bool producer1_killed = false;
bool producer2_killed = false;
pthread_t producer_threads[3];

void *producer0(void *t) {
    string password;
    int maxNumber = 1;
    for(int digitsNum=0; digitsNum<=MAX_DIGITS; digitsNum++) {
        for(int number=0; number<maxNumber; number++) {
            for(int numberPost=0; numberPost<maxNumber; numberPost++) {
                for(int wordIndex=0; wordIndex<dictLen; wordIndex++) {
                    
                    if(producer0_killed) {
                        std::cout << "procuder0 killed \n";
                        pthread_exit (NULL);
                        return 0;
                    }
                    
                    std::stringstream ss;
                    if(digitsNum > 0) {
                        ss << number;
                    }
                    for(int i=dictWordsIndexes[wordIndex]; i<dictWordsIndexes[wordIndex+1]; i++) {
                        ss << dictWords[i];
                    }
                    if(digitsNum > 0) {
                        ss << numberPost;
                    }
                    ss >> password;
                    //std::cout << " try pass " << password << "   ";
                    tryOutPassword(password);
                }
            }
        }
        maxNumber *= 10;
    }
    
    pthread_exit (NULL);
}

void *producer1(void *t) {
    string password;
    bool first;
    int maxNumber = 1;
    for(int digitsNum=0; digitsNum<=MAX_DIGITS; digitsNum++) {
        for(int number=0; number<maxNumber; number++) {
            for(int numberPost=0; numberPost<maxNumber; numberPost++) {
                for(int wordIndex=0; wordIndex<dictLen; wordIndex++) {
                    
                    if(producer1_killed) {
                        std::cout << "procuder1 killed \n";
                        pthread_exit (NULL);
                        return 0;
                    }
                    
                    std::stringstream ss;
                    if(digitsNum > 0) {
                        ss << number;
                    }
                    first = true;
                    for(int i=dictWordsIndexes[wordIndex]; i<dictWordsIndexes[wordIndex+1]; i++) {
                        if(first) {
                            ss << (char)toupper(dictWords[i]);
                            first = false;
                        } else {
                            ss << dictWords[i];
                        }
                    }
                    if(digitsNum > 0) {
                        ss << numberPost;
                    }
                    ss >> password;
                    //std::cout << " try pass " << password << "   ";
                    tryOutPassword(password);
                }
            }
        }
        maxNumber *= 10;
    }
    
    pthread_exit (NULL);
}

void *producer2(void *t) {
    string password;
    int maxNumber = 1;
    for(int digitsNum=0; digitsNum<=MAX_DIGITS; digitsNum++) {
        for(int number=0; number<maxNumber; number++) {
            for(int numberPost=0; numberPost<maxNumber; numberPost++) {
                for(int wordIndex=0; wordIndex<dictLen; wordIndex++) {
                    
                    if(producer2_killed) {
                        std::cout << "procuder2 killed \n";
                        pthread_exit (NULL);
                        return 0;
                    }
                    
                    std::stringstream ss;
                    if(digitsNum > 0) {
                        ss << number;
                    }
                    for(int i=dictWordsIndexes[wordIndex]; i<dictWordsIndexes[wordIndex+1]; i++) {
                        ss << (char)toupper(dictWords[i]);
                    }
                    if(digitsNum > 0) {
                        ss << numberPost;
                    }
                    ss >> password;
                    //std::cout << " try pass " << password << "   ";
                    tryOutPassword(password);
                }
            }
        }
        maxNumber *= 10;
    }
    
    pthread_exit (NULL);
}

void startProducerThreads() {
    producer0_killed = false;
    producer1_killed = false;
    producer2_killed = false;
  pthread_attr_t attr;
  int t1;


  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  //pthread_create(&threads[0], &attr, watch_count, (void *)t1);
  pthread_create(&producer_threads[0], &attr, producer0, &t1);
  pthread_create(&producer_threads[1], &attr, producer1, &t1);
  pthread_create(&producer_threads[2], &attr, producer2, &t1);
}

void *resetThread(void *t) {
    while(true) {
        sleep(1);
        for (std::string line; std::getline(std::cin, line);) {
            producer0_killed = true;
            producer1_killed = true;
            producer2_killed = true;
            for (int i = 0; i < 2; i++) {
                pthread_join(producer_threads[i], NULL);
            }
            
            std::cout << "load new passwords file " << line << std::endl;
            loadPasswords(line);
            
            startProducerThreads();
        }
    }
}

int runThreads()
{
  pthread_t internal_threads[2];
  pthread_attr_t attr;
  int t1;

  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&count_mutex, NULL);
  pthread_cond_init (&count_threshold_cv, NULL);

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  //pthread_create(&threads[0], &attr, watch_count, (void *)t1);
  pthread_create(&internal_threads[0], &attr, consumer, &t1);
  pthread_create(&internal_threads[1], &attr, resetThread, &t1);
  
  startProducerThreads();

  /* Wait for all threads to complete */
  for (int i = 0; i < 2; i++) {
    pthread_join(internal_threads[i], NULL);
  }

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mutex);
  pthread_cond_destroy(&count_threshold_cv);
  pthread_exit (NULL);

}


int main(int argc, char **argv) {
    string dictPath = "dict_small.txt";
    string passPath = "md5.txt";
    if(argc >= 2) {
        dictPath = argv[1];
    }
    if(argc >= 3) {
        passPath = argv[2];
    }
    signal(SIGHUP, handle_sighup);
    loadDict(dictPath);
    loadPasswords(passPath);
    runThreads();
    
    return 0;
}
