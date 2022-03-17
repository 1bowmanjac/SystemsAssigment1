#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>
#include <stdio.h>
#include "Parser.h"
#include <memory.h>
#include <wait.h>
#include <fstream>

std::streambuf* stream_buffer_cout = std::cout.rdbuf(); //backup of cout stream buffer
bool isCoutNormal = true;
std::streambuf* stream_buffer_cin = std::cin.rdbuf(); //backup of cin stream buffer
bool isCinNormal = true;


int main(int args, char** argv) {
    struct stat status;
    fstat(STDIN_FILENO, &status);
    std::string input;




    // Display a character to show the user we are in an active shell.
    if (S_ISCHR(status.st_mode))
        std::cout << "? ";
        
    // Pipe files or read from the user prompt
    // TODO: Groups will need to swap out getline() to detect the tab character.
    while (getline(std::cin, input)) {

        std::cout << std::endl;
        
        // Add another case here if you are in a group to handle \t!
        if (input == "exit") {
            return 1;
        } else {
            // TODO: Put your command execution logic here
            std::list<Command>* commands = Parser::Parse(input);
            //iterate over linked list structure    
            for ( auto it = commands->begin(); it != commands->end(); it++){
                std::string commandName = "/usr/bin/" + it->name;
                // std::string commandName = it->name;
                std::cout << "name: " << commandName << std::endl;
                std::string input = it->input_file;
                std::string outut = it->output_file;
                // char const *argList[it->args.size()];
                char* argList[1000];

                //input redirect
                std::fstream newCin;
                if(it->input_file != ""){
                    std::cout << "redirecting input" << std::endl;
                    newCin.open(it->input_file, std::ios::out);
                    std::streambuf* stream_buffer_file = newCin.rdbuf();
                    std::cout.rdbuf(stream_buffer_file);
                } else
                {
                    argList[0] = strdup(it->name.c_str());
                    int i = 1;
                    for(auto it2 = it->args.begin(); it2 != it->args.end(); it2++){
                        char* nonConst = strdup(it2->c_str());
                        argList[i] = nonConst;
                        //std::cout << argList[i] << std::endl;
                        i++;
                    }
                    argList[i] = NULL;
                }
                

                char *newenviron[] = { NULL };

                // int fd1[2]; // Used to store two ends of first pipe
                // int fd2[2]; // Used to store two ends of second pipe
                                
                // char fixed_str[] = "forgeeks.org";
                // char input_str[100];
                // pid_t p;
                // if (pipe(fd1) == -1) {
                //     fprintf(stderr, "Pipe Failed");
                //     return 1;
                // }
                // if (pipe(fd2) == -1) {
                //     fprintf(stderr, "Pipe Failed");
                //     return 1;
                // }           


                //output redirect
                    int* fd = new int[2]; // first index is read, second is write
                    pipe(fd);

                //fork() a new proccess for each command
                // Create a child process
                 pid_t pid = fork();
                 pid_t wpid;
                if (pid > 0) { // if parent
                    dup2(fd[0], STDIN_FILENO);
                } else //if child
                {
                    int fd_out = dup2(fd[1], STDOUT_FILENO); 
                    if (it->output_file != "")
                    {
                        freopen("outputFile.txt", "w", stdout); 
                    }
                    
                    execve(commandName.c_str(),argList,newenviron);
                    return -1;
                }
                
                //wait for process to finish
                wait(NULL);
                std::string response;
                getline(std::cin, response);
                if (it->input_file == "" && it->output_file == ""){
                    std::cout << response << std::endl;
                }
                
                
            }
        }
             // Display a character to show the user we are in an active shell.
             if (S_ISCHR(status.st_mode))
                 std::cout << "? ";
    }
}