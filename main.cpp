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
#include <sstream>


std::streambuf* stream_buffer_cout = std::cout.rdbuf(); //backup of cout stream buffer
bool isCoutNormal = true;
std::streambuf* stream_buffer_cin = std::cin.rdbuf(); //backup of cin stream buffer
bool isCinNormal = true;
char* argList[1000];



void stringToArgv(std::string str){
    
    std::string word;
    int i = 1;
    std::istringstream lineStream(str);
    while (lineStream >> word)
    {
        argList[i] = strdup(word.c_str());
        i++;
    }
    argList[i] = NULL;
}

int main(int args, char** argv) {
    struct stat status;
    fstat(STDIN_FILENO, &status);
    std::string input;

    std::string pipeInput;

    //if you are the first command, run normally, save your input for the next command
    //if you are command n, use the previoues command's input and run normally
    //if you are comands 2-->n-1, use the previous command's input as your own, save your input for the next command
    

    // Display a character to show the user we are in an active shell.
    if (S_ISCHR(status.st_mode))
        std::cout << "? ";
        
    // Pipe files or read from the user prompt
    // TODO: Groups will need to swap out getline() to detect the tab character.
    while (getline(std::cin, input)) {
        
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
                std::string input = it->input_file;
                std::string outut = it->output_file;
                std::string argLine;

                //input redirect
                std::fstream newCin;
                if(it->input_file != "" && it == commands->begin()){ // if you are the first line and have an input redirect
                    std::cout << "redirecting input" << std::endl;
                    newCin.open(it->input_file, std::ios::in);
                    getline(newCin,argLine);

                    stringToArgv(argLine);

                    newCin.close();
                }  else if(it == commands->begin()) //if you are the first line but do not have an input redirect
                {
                    int i = 1;
                    for(auto it2 = it->args.begin(); it2 != it->args.end(); it2++){
                        char* nonConst = strdup(it2->c_str());
                        argList[i] = nonConst;
                        //std::cout << argList[i] << std::endl;
                        i++;
                    }
                    argList[i] = NULL;
                } else {//else you are to take the output of the previous command and use it as your input
                    stringToArgv(pipeInput);
                }
                
                    

                
                
                argList[0] = strdup(it->name.c_str());
                char *newenviron[] = { NULL };




                //output redirect
                    int old_stdin = dup(STDIN_FILENO);
                    int old_stout = dup(STDOUT_FILENO);
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
                    execve(commandName.c_str(),argList,newenviron);
                    return -1;
                }
                
                //wait for process to finish
                wait(NULL);

                std::string response;
                getline(std::cin, response);


                dup2(old_stdin, STDIN_FILENO);
               
                if (it->output_file != "" && std::next(it) != commands->end()){ // if there is an output file specified for the current file and it is not the last command
                    FILE* out = fopen(it->output_file.c_str(), "w");
                    int fd2 = fileno(out); // first index is read, second is write
                    dup2(fd2, STDOUT_FILENO);
                    std::cout << response << std::endl;
                    dup2(old_stout, STDOUT_FILENO);
                    fclose(out);
                    
                }
                if(it->output_file != "" && std::next(it) == commands->end()) {// if there is an output file and it is the end
                    FILE* out = fopen(it->output_file.c_str(), "w");
                    int fd2 = fileno(out); // first index is read, second is write
                    dup2(fd2, STDOUT_FILENO);
                    std::cout << response << std::endl;
                    dup2(old_stout, STDOUT_FILENO);
                    fclose(out);

                } else {
                    if ((std::next(it) == commands->end())){ // if it is the end and there is no outputfile
                            std::cout << response << std::endl;
                    } else { //it is not the end. save the response to be used as the args for the next command
                        pipeInput = response;
                    }
                }
                
                
            }
        }
             // Display a character to show the user we are in an active shell.
             if (S_ISCHR(status.st_mode))
                 std::cout << "? ";
    }
}