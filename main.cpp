#include <iostream>
#include <string>

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

using std::string;

#define MD5_LEN 32
#define MD5_HASH char[MD5_LEN]

#define MAX_PASSWORDS     100 
#define PASSWORD_DESCRIPTION_LEN 1000

string md5(string text) {
    if(text.compare("kota")) {
        return "31D9BB37999652D494BA78FEB642A73F";
    }
    return "8F8579A77926FA12B533F2E1A327F5F3"; // 'inny'
}

char *dictWords;
int* dictWordsIndexes;
int dictLen;

char passwords[MAX_PASSWORDS][MD5_LEN];
char passwordsDescritpions[MAX_PASSWORDS][PASSWORD_DESCRIPTION_LEN];
bool crackedPasswords[MAX_PASSWORDS];
int loadedPasswords;
string decodedPasswords[MAX_PASSWORDS];

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
            passwords[passwordIndex][i] = line.at(i);
        }
        passwordIndex++;
    }
    loadedPasswords = passwordIndex;
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

void tryOutPassword(string pass) {
    string hash = md5(pass);
    for(int i=0; i<loadedPasswords; i++) {
        if(crackedPasswords[i]) continue;
        if(hash.compare(passwords[i])) {
            //std::cout << "match!!";
            decodedPasswords[i] = pass;
            crackedPasswords[i] = true;
        }
    }
}


int main(int argc, char **argv) {
    std::cout << "Hello, world  777 !" << std::endl;
    loadDict("/home/jacek/Downloads/krystian-md5/proj2/dict_small.txt");
    iterDict();
    
    loadPasswords("/home/jacek/Downloads/krystian-md5/proj2/md5.txt");
    std::cout << "Hello, world  666 !" << std::endl;
    
    tryOutPassword("abc");
    tryOutPassword("kota");
    
    return 0;
}
