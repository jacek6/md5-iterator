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
bool crackedPasswords[MAX_PASSWORDS];

void loadDict(string inputFile) {
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    std::ifstream infile(inputFile);
    std::string line;
    
    int numberOfLines = 0;
    int newDictLen;
    
    while (std::getline(infile, line))
    {
        numberOfLines++;
        newDictLen += line.length();
        printf("wczytuje linie: \n\n");
        std::cout << line;
    }
    
    dictWords = (char*) malloc(newDictLen);
    dictWordsIndexes = (int*) malloc(numberOfLines + 1);
    dictLen = newDictLen;
    
    std::ifstream infile2(inputFile);
    int dictIndex = 0;
    int wordIndex = 0;
    while (std::getline(infile2, line))
    {
        dictWordsIndexes[wordIndex] = dictIndex;
        for(int i=0; i<line.length(); i++) {
            dictWords[dictIndex++] = line.at(i);
        }
    }
}

void iterDict() {
    for(int wordIndex=0; wordIndex<dictLen; wordIndex++) {
        for(int i=dictWordsIndexes[wordIndex]; i<dictWordsIndexes[wordIndex+1]; i++) {
            std::cout << dictWords[i];
        }
        std::cout << "\n";
    }
}


int main(int argc, char **argv) {
    std::cout << "Hello, world  555 !" << std::endl;
    loadDict("/home/jacek/Downloads/krystian-md5/proj2/dict_small.txt");
    iterDict();
    std::cout << "Hello, world  666 !" << std::endl;
    return 0;
}
