char currentDir = 0xFF;
char currentState[1000];
currentState = "$/";

int main(){
    char command[1000];
    while(1){
        interrupt(0x21, 0x00, currentState, 0, 0);
        interrupt(0x21, 0x01, command, 0, 0);
        handleCommand(command);
    }
    return 0;
}

void handleCommand(char* command){
    int i;
    char argument[1000];
    if(command[0] == 'c' && command[1] == 'd'){
        i = 3;
        while(command[i] != '\0'){
            argument[i-3] = command[i];
        }
    // belum selesai cok
    }
}