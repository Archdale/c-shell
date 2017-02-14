#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void toggleDir(char** prevWorkDir, char** currWorkDir)
{
    char buffWorkDir[2048];
    
    strcpy(buffWorkDir,prevWorkDir);
    strcpy(prevWorkDir,currWorkDir);
    strcpy(currWorkDir,buffWorkDir);
    chdir(currWorkDir);
    setenv("PWD", currWorkDir, 1);
    setenv("OLDPWD", prevWorkDir, 1);
}

void changeDir(char** prevWorkDir, char** currWorkDir)
{
    getcwd(prevWorkDir, sizeof(prevWorkDir));
    chdir(commandArgs[1]);
    getcwd(currWorkDir, sizeof(currWorkDir));
    setenv("PWD", currWorkDir, 1);
    setenv("OLDPWD", prevWorkDir, 1);
}

void homeDir()
{
    getcwd(prevWorkDir, sizeof(prevWorkDir));
    chdir(getenv("HOME"));
    setenv("PWD", getenv("HOME"), 1);
    setenv("OLDPWD", prevWorkDir, 1);
}