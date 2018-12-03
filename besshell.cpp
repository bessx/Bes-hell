// Adam Bess
// CSC7103 - Programming Project
// Programming Project 1/3
// Project: PA-1(Programming)
// Instructor: Feng Chen
// Class: cs7103-au18
// LogonID: cs710313

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <queue>
#include <stdlib.h> 
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
// #include <set>
// #include <algorithm>
// #include <functional>

using namespace std;

// Count of commands
int h;
// Original History char array
char history[100][50];
// History 2 String Deque
deque <string> h2;
// Final History String Deque
deque <string> hq;

// By Default show Current Working Directory
// ***NOTE TO SELF*** Don't forget to change this back to true1!1!!!
bool scwd = true;

int inputwords;

map<string, int> freqmap;

template <typename A, typename B> multimap<B, A> flip_map(map<A,B> & src) {
    multimap<B,A> dst;

    for(typename map<A, B>::const_iterator it = src.begin(); it != src.end(); ++it)
        dst.insert(pair<B, A>(it -> second, it -> first));

    return dst;
}

void getinput(char* cmdtok, char* cmdlst[], char* input){	
  // Cool Bess Shell input prompt
  cout << "Bes§hell";

  // Get Current Working Directory and print out
  if(scwd){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        cout << " (" << cwd << ")";
    } else {
      cout << " (getcwd error)";
    }
  }
  
  // Get input from bess shell prompt
  cout << ">";

  // input as char
  cin.getline(input,50);

  // History1 breaks if h > 100
  if(h<99){
    strcpy(history[h], input);
  }else{
    strcpy(history[99], input);
  }

  // History 2 Queue
  // Push into back of history
  // If history is larger than 100, delete oldest commands from memmory
  if(h2.size() < 100){
    h2.push_back(input);
  } else{
    h2.pop_front();
    h2.push_back(input);
  }

  // History *Final* Attmpt
  // Convert h to string s
  string s = to_string(h+1);
  // Convert input to string r
  string r(input);
  // Concat h and String
  string hqnext = s + " " + r;

  // Find first word to parse command list
  string firstword = r.substr(0, r.find(" "));
  // Create Map of each first word and word frequency
  if (freqmap.find(firstword) == freqmap.end()){
    freqmap[firstword] = 1;
  } else{
    freqmap[firstword]++;
  }

  // Push into back of history if history is less than 100
  // If history is larger than 100, delete oldest commands from memory
  if(hq.size() < 100){
    hq.push_back(hqnext);
  } else{
    hq.pop_front();
    hq.push_back(hqnext);
  }


  // break up input into tokens by space
	cmdtok = strtok(input, " ");

  // run through each token
	inputwords = 0;

  do{
    // FIX EXECV because EXECV doesnt have a PATH Search like execvp()
    // Have to add /bin/ for full path to commands
    // if(i == 0){
    //   string t(cmdtok);
    //   string bin = "/bin/" + t;
    //   char *cbin = new char[bin.length() + 1];
    //   strcpy(cbin, bin.c_str());
    //   cmdlst[0] = cbin;
    // } else{
    //   cmdlst[i] = cmdtok;
    // }
    cmdlst[inputwords] = cmdtok;
    
    inputwords++;
    
    // cout << "cmdtoken: " << cmdtok << "\n";
    cmdtok = strtok(NULL, " ");
    // if(cmdtok == NULL){
      // cout << "cmdtoken: NULL\n";
    // }
    // oof(cmdtok, cmdtok);
    }while(cmdtok != NULL);
}

void reset(char* cmdlst[]){
  // reset getinput
  for(int a=0; a < 40; a++){
        cmdlst[a] = NULL;
  }
}

char* cmdlst2[40];

void run(char* cmdlst[]){
    pid_t baby, daddy;
    int status = 0;

    int pipefd[2];
    pipe(pipefd);

    baby = fork();

    if(baby < 0){
      // Fork Failed
      cout << "MY FORKKKK!! --ε\n";
      exit(-1);
    }else if(baby == 0){
          // Returns 0 to the child process.
          // Execv version: Execute command after adding /bin/ to execv()
          //print output to file
          for(int i=0; i<inputwords; i++){
            if(strcmp(cmdlst[i], ">") == 0){
              int fd = open(cmdlst[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
              if(fd < 0){
                  perror("cant open file");
                  exit(0);
              }
              // dup2(fd, 1);
              dup2(fd,STDOUT_FILENO);
              close(fd);
              cmdlst[i] = NULL;
            }else if(strcmp(cmdlst[i], "|") == 0){
              // If Pipe in Command List
              // Split into secondary (pipe) Command List
              cout << "\nPipe Command: ";
              for(int j = i; j < inputwords-1; j++){
                cmdlst2[j-i] = cmdlst[j+1];
                cout << cmdlst2[j-i] << " ";
              }
              cout << "\n\n";
              close(pipefd[0]);
              dup2(pipefd[1], 1);
              dup2(pipefd[1], 2);
              close(pipefd[1]);
              cmdlst[i] = NULL;
            }
          }
          int lost = execvp(cmdlst[0], cmdlst);

          // These won't show up if execv(p) works.
          // cout << "cmdlst[0] =" << *cmdlst[0] << "\n";
          // cout << "cmdlst =" << *cmdlst << "\n";
          // cout << "Child's PID is " << (int)getpid() << ". Parent's PID is " << (int)getppid() <<"\n";
          // If command not available or failed:
          if(lost == -1){
              printf("ERROR 404: %s.\n", strerror(errno));
              exit(0);
          }
    }else{
      // It returns a process ID to the parent process.
      // Parent process

        char buffer[1024];
        close(pipefd[1]);

        while (read(pipefd[0], buffer, sizeof(buffer)) != 0){
          // Buffer output from pipe 1
          cout << "***** Pipe Input Buffer *****\n" << buffer << "\n*****************************\n";
          
          // Display second Command
          // cout << "Command 2: " << *cmdlst2 << "\n";
          
          // Run Pipe Command 2
          // execvp(cmdlst2[0], cmdlst2);
        }

      do {
        // If fork exits with a weird status, print error.
            daddy = waitpid(baby, &status, WUNTRACED | WCONTINUED);
            if (daddy == -1) {
                perror("Daddy failure.");
                exit(EXIT_FAILURE);
            }
            // Exited Normally
            // if (WIFEXITED(status)) {
            //   printf("Baby exited hapilly. (status %d)\n", WEXITSTATUS(status));
            // } else 
            if (WIFSIGNALED(status)) {
                printf("Baby killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("Baby stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("Baby continued...\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      // cout << "Current PID is " << (int)getpid() << ". Parent PID is " << (int)getppid() <<"\n";
    }
}

int main(){
   char* cmdtok;
   char* cmdlst[40];
   char input[50]; 
   h = 0;

   while(cmdlst[0] != NULL){
        streambuf *buffer, *ogcout;
        ofstream filestr;
        ogcout = cout.rdbuf();

        reset(cmdlst);
   	    getinput(cmdtok, cmdlst, input);
   	    h++;
         
        // cout to file on > 
        for(int i = 0; i < inputwords; i++){
          if(strcmp(cmdlst[i], ">") == 0){
            filestr.open(cmdlst[i+1]);
            buffer = filestr.rdbuf();
            cout.rdbuf(buffer);
          }
        }

   	    if(strcmp(cmdlst[0], "quit") == 0 || strcmp(cmdlst[0], "q") == 0  || strcmp(cmdlst[0], "exit") == 0){
          // Goodbyeeee~
          cout << "Goodbye fren. ( ╥ ﹏ ╥)づ\n";
           break;
   	    } else if(strcmp(cmdlst[0], "historyold") == 0 || strcmp(cmdlst[0], "h1") == 0) {
          // My first attempt at History
          // Breaks after 100 inputs, too complicated to rearrange array
           cout <<"History (Last 100 Commands):\n";
   	        for(int m = 0; m < 100; m++){
                if(*history[m]){
                    cout << m+1 << " " << history[m] << "\n";
                }
            }
   	    } else if(strcmp(cmdlst[0], "history2") == 0 || strcmp(cmdlst[0], "h2") == 0) {
          // History Attempt #2
          // Pushes properly from deque and shows last 100 commands, however the count is wrong.
          for(int n = 0; n < 100; n++){
            if(n < h2.size()){
              cout << n+1 << " " << h2.at(n) << "\n";
            }else{
              n = 100;
            }
          }
   	    } else if(strcmp(cmdlst[0], "history") == 0 || strcmp(cmdlst[0], "h") == 0) {
          for(int n = 0; n < 100; n++){
            if(n < hq.size()){
              cout << hq.at(n) << "\n";
            }else{
              n = 100;
            }
          }
   	    } else if(strcmp(cmdlst[0], "cwd") == 0) {
          // Get Current Working Directory
          char cwd[PATH_MAX];
          if (getcwd(cwd, sizeof(cwd)) != NULL) {
              cout << cwd << "\n";
          }
   	    } else if(strcmp(cmdlst[0], "histat") == 0 || strcmp(cmdlst[0], "hs") == 0) {
          // Get History Statistics
          cout << "Histat: Top 10 Commands\n";
          int z = 0;
          multimap<int, string> sortedmap = flip_map(freqmap);
          for(multimap<int, string>::const_reverse_iterator it = sortedmap.rbegin(); it != sortedmap.rend(); it++){
            if(z<10){
              cout << it -> first << " " << it -> second << "\n";
            }
            z++;
          }
   	    } else if(strcmp(cmdlst[0], "showcwd") == 0 || strcmp(cmdlst[0], "scwd") == 0) {
          // Show/Hide Current Working Directory
          if(scwd){
            cout << "Show Current Working Directory = False\n";
            scwd = false;
          }else{
            cout << "Show Current Working Directory = True\n";
            scwd = true;
          }
   	    } else if(strcmp(cmdlst[0], "_") == 0) {
          // Some fun commands
            cout <<"~~~~~~~~~~ SECRET_BLANK_CODES (shhh.encrpyted) ~~~~~~~~~~\n";
            cout <<"xoxo\n";
            cout <<"The Answer.\n";
            cout <<"cola-konomi\n";
            cout <<"It's a bird!\n";
            cout <<"h*****2\n";
            cout <<"help\n";
            cout <<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   	    } else if(strcmp(cmdlst[0], "42") == 0) {
          // Some fun commands
           cout <<"The Answer to the Ultimate Question of Life, the Universe, and Everything.\n";
   	    } else if(strcmp(cmdlst[0], "hug") == 0 || strcmp(cmdlst[0], "hugs") == 0) {
          // Some fun commands
           cout <<"(づ•ᴗ•)づ ღ\n";
   	    } else if(strcmp(cmdlst[0], "kiss") == 0 || strcmp(cmdlst[0], "kisses") == 0) {
          // Some fun commands
          cout <<"( ˘ ³˘) ~♥\n";
   	    } else if(strcmp(cmdlst[0], "superman") == 0) {
          // Some fun commands
           cout <<"Your character becomes sickly, but will never die.\n";
   	    } else if(strcmp(cmdlst[0], "plz") == 0 || strcmp(cmdlst[0], "helpplz") == 0 || strcmp(cmdlst[0], "helplz") == 0) {
          // Some fun commands
           cout <<"¯\\_(ツ)_/¯\n";
   	    } else if(strcmp(cmdlst[0], "*") == 0 || strcmp(cmdlst[0], "hunter2") == 0) {
          // Some fun commands
           cout <<"<form action=\"/passwordlist.html\"  method=\"post\">\n";
           cout <<"  <input type=\"password\" name=\"hunter2\">\n";
           cout <<"</form>\n";
   	    } else if(strcmp(cmdlst[0], "uuddlrlrbastart") == 0 || strcmp(cmdlst[0], "uuddlrlrba") == 0 || strcmp(cmdlst[0], "UUDDLRLRBA") == 0 || strcmp(cmdlst[0], "UUDDLRLRBAS") == 0 || strcmp(cmdlst[0], "uuddlrlrbas") == 0) {
          // Some fun commands
            cout <<"Coca-Cola Source Code:\n";
            cout <<"         __                              ___   __        .ama     ,\n";
            cout <<"      ,d888a                          ,d88888888888ba.  ,88\"I)   d\n";
            cout <<"     a88']8i                         a88\".8\"8)   `\"8888:88  \" _a8'\n";
            cout <<"   .d8P' PP                        .d8P'.8  d)      \"8:88:baad8P'\n";
            cout <<"  ,d8P' ,ama,   .aa,  .ama.g ,mmm  d8P' 8  .8'        88):888P'\n";
            cout <<" ,d88' d8[ \"8..a8\"88 ,8I\"88[ I88' d88   ]IaI\"        d8[\n";
            cout <<" a88' ]P \"bm8mP8'(8'.8I  8[      d88'    `\"         .88\n";
            cout <<",88I ]P[  .I'.8     88' ,8' I[  ,88P ,ama    ,ama,  d8[  .ama.g\n";
            cout <<"[88' I8, .I' ]8,  ,88B ,d8 aI   (88',88\"8)  d8[ \"8. 88 ,8I\"88[\n";
            cout <<"]88  `8888\"  '8888\" \"88P\"8m\"    I88 88[ 8[ ]P \"bm8m88[.8I  8[\n";
            cout <<"]88,          _,,aaaaaa,_       I88 8\"  8 ]P[  .I' 88 88' ,8' I[\n";
            cout <<"`888a,.  ,aadd88888888888bma.   )88,  ,]I I8, .I' )88a8B ,d8 aI\n";
            cout <<"  \"888888PP\"'        `8\"\"\"\"\"\"8   \"888PP'  `8888\"  `88P\"88P\"8m\"\n\n";

   	    } else if(strcmp(cmdlst[0], "help") == 0) {
          // Help command
           cout <<"~~~~~~~~~~ plz_help.txt ~~~~~~~~~~\n";
           cout <<"cwd\n";
           cout <<"echo\n";
           cout <<"showcwd (scwd)\n";
           cout <<"history (h) [historyold (h1), history2 (h2)]\n";
           cout <<"histat (hs)\n";
           cout <<"quit/exit (q)\n";
   	    } else if(strcmp(cmdlst[0], "checksymbols") == 0) {
          // Check UTF-8 symbols
           cout <<"~~~~~~~~~~ allemoji.ttf ~~~~~~~~~~\n";
            cout <<"¯\\_(ツ)_/¯\n";
            cout <<"( ˘ ³˘) ~♥\n";
            cout <<"(づ•ᴗ•)づ ღ\n";
            cout << "Goodbye fren. ( ╥ ﹏ ╥)づ\n";
            cout << "MY FORKKKK!! --ε\n";
   	    } else {        
           run(cmdlst);
	       }
        cout.rdbuf(ogcout);
        filestr.close();
   }
  cout << "(End of the line, buddy.)\n";
}