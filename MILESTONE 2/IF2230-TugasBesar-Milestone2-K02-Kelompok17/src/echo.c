int main(){

	char argc;
	int input[64];

	getArgc(argc);
	if(argc > 1){
		argc = 1;
		// get argv 
		getArgv(0, input);
		// print argv
		println(input);
	}
	terminateProgram(0);
}