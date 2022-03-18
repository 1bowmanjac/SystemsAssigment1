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

/**
 * Jack Bowman - 100752381
 * Systems assignment #1
 * mini shell with IO redirection and piping
 */

char* argList[1000];


//takes a string of arguments seperated by spaces and converts them to c_strings before adding them to the argList
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
                std::string commandName = "/usr/bin/" + it->name; // this is needed to execute commands from the PATH instead of from the local folder
                std::string argLine; // line of arguments seperated by spaces. used to read from file

                // input redirect --> read from file rather than command line
                // std::fstream newCin;
                // if(it->input_file != "" && it == commands->begin()){ // if you are the first line and have an input redirect
    
                //     newCin.open(it->input_file, std::ios::in);
                //     getline(newCin,argLine);

                //     stringToArgv(argLine);

                //     newCin.close();
                
                if(it == commands->begin()) //if you are the first command
                {
                    //goes through the argument list and converts the strings to c_strings before adding them to the argList
                    int i = 1;
                    for(auto it2 = it->args.begin(); it2 != it->args.end(); it2++){
                        std::cout << "converting arglist" << std::endl;
                        char* nonConst = strdup(it2->c_str());
                        argList[i] = nonConst;
                        //std::cout << argList[i] << std::endl;
                        i++;
                    }
                    argList[i] = NULL;

                } else {//else you are to take the output of the previous command and use it as your input
                        //i first first command
                        //as such, it takes the arguments from the previous command's output rather than from the commands argument list
                    int old_stdin = dup(STDIN_FILENO);
                    FILE* in = fopen("tmp", "r"); // open the output file
                    int fdIn = fileno(in); // save the output file descriptor 
                    dup2(fdIn, STDIN_FILENO); //redirect the output to the file
                    std::string tempString;
                    getline(std::cin,tempString);
                    stringToArgv(tempString);
                    dup2(old_stdin, STDIN_FILENO);//undo the redirect
                    fclose(in);
                }
                
                    

                
                //set the first argument in the argList to the command
                argList[0] = strdup(it->name.c_str());

                char *newenviron[] = { NULL };



                    //save the original STDIN and STDOUT to revert IO redirects
                    int old_stdin = dup(STDIN_FILENO);
                    int old_stout = dup(STDOUT_FILENO);


                    //file descriptor for the pipline
                    int* fd = new int[2]; // first index is read, second is write

                    int fdIn;
                    if (it->input_file != "")
                    {
                        // std::cout << "making file desc" << std::endl;
                        FILE* in = fopen(it->input_file.c_str(), "r"); // open the output file
                        fdIn = fileno(in);
                    }
                    
                    pipe(fd);

                //fork() a new proccess for each command
                // Create a child process
                 pid_t pid = fork();
                 pid_t wpid;
                if (pid > 0) { // if parent, redirect the STDIN to the pipe
                    // std::cout << "parent redirecting cin to pipe" << std::endl;
                    dup2(fd[0], STDIN_FILENO);
                } else //if child, redirect the output of the command from the console to the pipe
                {
                    int fd_out = dup2(fd[1], STDOUT_FILENO); 
                    if(it->input_file != ""){
                        // std::cout << "got here";
                        dup2(fdIn, STDIN_FILENO); //redirect the input to the file
                        close(fdIn);
                    }
                    
                    execve(commandName.c_str(),argList,newenviron); //execute the command
                    return -1;
                }
                
                //wait for process to finish
                wait(NULL);

                //get the responce from the command run by the child proccess
                // std::string response = "";
                //  while (!std::cin.eof())
                // {
                //     std::string line;
                //     getline(std::cin, line);

                //     if (std::cin.fail())
                //     {
                //         //error
                //         break;
                //     }

                //     response += line + "\n";
                // }

                std::string response;
                getline(std::cin, response);
                
                dup2(old_stdin, STDIN_FILENO);//revert the STD file redirect
               
               //this is needed to cover the case where a file is part of the pipeline but also has an output file
                if (it->output_file != "" && std::next(it) != commands->end()){ // if there is an output file specified for the current file and it is not the last command
                    FILE* out = fopen(it->output_file.c_str(), "w"); // open the output file
                    int fd2 = fileno(out); // save the output file descriptor 
                    dup2(fd2, STDOUT_FILENO); //redirect the output to the file
                    std::cout << response << std::endl;
                    dup2(old_stout, STDOUT_FILENO);//undo the redirect
                    fclose(out);
                    
                }
                //this if statment covers the case that a command is at the end of a pipeline or is not part of one and needs to output to a file
                if(it->output_file != "" && std::next(it) == commands->end()) {// if there is an output file and it is the end
                    FILE* out = fopen(it->output_file.c_str(), "w");
                    int fd2 = fileno(out); 
                    dup2(fd2, STDOUT_FILENO);
                    std::cout << response << std::endl;
                    dup2(old_stout, STDOUT_FILENO);
                    fclose(out);

                } else {
                    if ((std::next(it) == commands->end())){ // if it is the end and there is no outputfile output the command result to the commandline
                            std::cout << response << std::endl;
                    } else { //else the current command is part of a pipeline and the result of the command must be saved for the next line
                        FILE* out = fopen("tmp", "w");
                        int fd2 = fileno(out); 
                        dup2(fd2, STDOUT_FILENO);
                        std::cout << response << std::endl;
                        dup2(old_stout, STDOUT_FILENO);
                        fclose(out);
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