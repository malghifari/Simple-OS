#define NOT_FOUND -1

int main()
{
	char pwd;
	char argc;
	int input[64];
	int result;

	getCurdir(pwd);
	getArgc(argc);
	if(argc==1){
		getCurdir(&pwd);
		getArgv(0, input);
		deleteFile(input, &result, pwd);
		if(result == NOT_FOUND)
			println("File Not Found");
	}else
		println("Command rm cannot be applied");
	terminateProgram(0);
}