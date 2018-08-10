#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <exception>

#include "MatcherFactory.hpp"
#include "MatcherOptions.hpp"
#include "CipherSuggester.hpp"
#include "Subject.hpp"
#include "Wordlist.hpp"
#include "Worker.hpp"

void usage(char* progname)
{
    printf("Usage: %s -w <word_list> -i <input_file> -c <ciphers_list> -m <matcher_name> [-t <num_threads>] [-o <param=value> -o <param=value> ...] \n", progname);
    printf("    -w <word_list>:     a wordlist containing passwordz\n");
    printf("    -i <input_file>:    the encrypted file\n");
    printf("    -c <ciphers_list>:  list with ciphers to probe for\n");
    printf("    -t <num_threads>:   change number of concurrent threads\n");
    printf("                        (optional, default: 4)\n");
    printf("    -m <matcher_name>:  use the specified matcher module\n");
    printf("    -o <param=value>:   set the matcher parameter param to value value\n"); 
    printf("                        (can be used multiple times for multiple values\n");
    printf("    -s                  suggest and outpt ciphers for given subject only and exit\n");
    printf("    -h                  this help\n");

    exit(1);
}

int main(int argc, char** argv)
{
    int i;
    bool suggestOnly = false;

    char* passwordFile = nullptr;
    char* cipherFile = nullptr;
    char* inputFile = nullptr;
        
    Subject subject;
    MatcherOptions matcherOptions;
    
    std::vector<Worker*> workers;
    std::string matcherName;
    
    int numThreads = 4;

    while ((i = getopt(argc, argv, "i:w:t:c:m:o:hs")) != -1) {
        switch(i) {
            case 'i':
                inputFile = optarg;
                break;
            case 'w':
                passwordFile = optarg;
                break;
            case 't':
                numThreads = std::atoi(optarg);
                break;
            case 'c':
                cipherFile = optarg;
                break;
            case 'm':
                matcherName = std::string(optarg);
                break;
            case 'o':
                {
                    char * opt = optarg;
                    char * p = strstr(opt, "=");
                    *p = 0;
                    char * key = opt;
                    char * val = p+1;
                    printf("* added matcher option: %s=%s\n", key, val);
                    matcherOptions.addOption(key, val);
                }
                break;
            case 's':
                suggestOnly = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    Wordlist passwordList(numThreads);
    Wordlist cipherList(1);
    
    if (matcherName == "h") {
        MatcherFactory::printMatcherHelp();
        return 1;
    }
    
    try {
        subject.loadFile(inputFile);
        printf("* Loaded %lu bytes ciphertext\n", subject.getCipherTextLength());
    } catch (std::exception& e) {
        fprintf(stderr, "! ERROR: Could not load input file: %s\n", inputFile);
        return 1;
    }
    
    // if no ciphers list is provided, use suggested list
    if (cipherFile == nullptr || suggestOnly == true) {
        CipherSuggester::suggest(subject, cipherList);
        if (suggestOnly) {
            printf("Suggested ciphers:\n");
            for (const auto& it : cipherList.getList(0))
                printf("%s\n", it.c_str());
            return 0;
        } else {
            printf("* Warning: no cipher list provided, using auto-suggested list\n");
        }
    } else {
        try {
            cipherList.loadFile(cipherFile);
        } catch (std::exception& e) {
            fprintf(stderr, "! ERROR: Could not load ciphers list: %s\n", cipherFile);
            return 1;
        }
    }
    printf("* Loaded %lu cipher(s)\n", cipherList.getSize());

    try {
        passwordList.loadFile(passwordFile);
        printf("* Loaded %lu password(s)\n", passwordList.getSize());
    } catch (std::exception& e) {
        fprintf(stderr, "! ERROR: Could not load password list: %s\n", passwordFile);
        return 1;
    }

    if (!MatcherFactory::checkAvailability(matcherName)) {
        fprintf(stderr, "Error: invalid matcher option.\n");
        fprintf(stderr, "Please specify a valid matcher with the -m option.\n");
        fprintf(stderr, "Use -mh to get a list of available matchers\n");
        return 1;
    } else {
        MatcherFactory::setDefaultValues(matcherName, matcherOptions);
    }
    
    printf("* Running %d thread(s)...\n", numThreads);
    for (i = 0; i < numThreads; i++) {
        Worker* worker = new Worker(subject, passwordList, cipherList, i, numThreads, matcherName, matcherOptions);
        workers.push_back(worker);
        worker->run();
    }

    for (auto it = workers.begin(); it != workers.end(); it++) {
        (*it)->getThread().join();
        delete *it;
    }

    printf("* Finished, all threads ended\n");
}


