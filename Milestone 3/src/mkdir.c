#include "helper.h"

int main()
{
    char argc;
    char pwd;
    char input[20];
    int i, j, result;
    enableInterrupts();
    getArgc(&argc);
    if (argc != 1){
        println("Invalid Number for args");
    }else{
        getCurdir(&pwd);
        getArgv(0, input);
        makeDirectory(input, &result, pwd);
        if (result == INSUFFICIENT_ENTRIES){
            println("Directory_sec is Full");
        }
    }
    terminateProgram(0);
}
