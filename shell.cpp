#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <cstdlib>
#include <sys/utsname.h>
#include <pwd.h> 
#include<dirent.h>
#include <limits.h>
#include<sys/stat.h>
#include<cstring>
#include<grp.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include<algorithm>
#include<csignal>
using namespace std;
pid_t foregroundPid=-1;
struct termios terminal;
string prevDirectory="";
string display(int homeLength)
{
    const char* username = getlogin();  
    struct utsname unameData;
    uname(&unameData);
    string systemName=unameData.nodename;
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    const char* home = getenv("HOME");
    if (home == nullptr) {
        home = getpwuid(getuid())->pw_dir;
    }
    return (string)username+"@"+systemName+":~"+(cwd+homeLength)+">";
}

void cd(char** argv,int argc,int homeLength,char home[]) {
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    if(argc==1)
    {
        chdir(home);
    }
    else
    {   
        string targetDirectory;
        if(strcmp(argv[1],"~")==0)
            targetDirectory=home;
        else if(strcmp(argv[1],"-")==0)
            targetDirectory=prevDirectory;
        else
        {   
            if(argv[1][0]!='/') targetDirectory=(string)cwd+"/"+(string)argv[1];
            else  targetDirectory=(string)home+argv[1];
        }       
            if(targetDirectory.substr(0,homeLength)==home)
            if(chdir(targetDirectory.c_str())==-1)
                cerr<<"No such directory\n";
            else
                 prevDirectory=cwd;

    }
   
}
void echo(char **argv, int argc) {
    
    for(int i=1;i<argc;i++)
    {   
        cout<<argv[i]<<" ";
    }
    cout<<'\n';
}
void pwd(int homeLength)
{   
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    cout<<"~"<<(cwd+homeLength)<<endl;
}
void ls(char *path,bool l,bool a)
{   
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    strcat(cwd,"/");
    strcat(cwd,path);
    cout<<cwd<<endl;
    struct dirent* file;
    DIR* directory=opendir(cwd);
    if(directory==nullptr)
    {
        cerr<<"could not open directory\n";
        exit(1);
    }
    off_t total=0;
    while((file=readdir(directory))!=NULL)
    {   
        struct stat filestat;
        stat((file->d_name),&filestat);
             
        if(file->d_name[0]!='.' || a)
            total+=filestat.st_blocks;
    }
    if(l)
        cout<<"total "<<total<<endl;
    rewinddir(directory);
    while((file=readdir(directory))!=NULL)
    {  if(file->d_name[0]!='.' || a)
        if(l==false)
         {
            cout<<file->d_name<<" ";
         }
        else{
                struct stat filestat;
                stat((file->d_name),&filestat);
                cout << ((S_ISDIR(filestat.st_mode)) ? "d" : "-");
                cout << ((filestat.st_mode & S_IRUSR) ? "r" : "-");
                cout << ((filestat.st_mode & S_IWUSR) ? "w" : "-");
                cout << ((filestat.st_mode & S_IXUSR) ? "x" : "-");
                cout << ((filestat.st_mode & S_IRGRP) ? "r" : "-");
                cout << ((filestat.st_mode & S_IWGRP) ? "w" : "-");
                cout << ((filestat.st_mode & S_IXGRP) ? "x" : "-");
                cout << ((filestat.st_mode & S_IROTH) ? "r" : "-");
                cout << ((filestat.st_mode & S_IWOTH) ? "w" : "-");
                cout << ((filestat.st_mode & S_IXOTH) ? "x" : "-");
                struct passwd* user=getpwuid(filestat.st_uid);
                struct group* group=getgrgid(filestat.st_gid);
                struct tm* time=localtime(&filestat.st_mtime);
                char timeBuffer[100];
                strftime(timeBuffer,sizeof(timeBuffer),"%b %d %H:%M",time);
                cout<<" "<<filestat.st_nlink<<" "<<user->pw_name<<" "<<group->gr_name<<" "<<filestat.st_size<<" "<<timeBuffer<<" "<<file->d_name;

                cout<<endl;

                    
            }
    }
    cout<<'\n';
    closedir(directory);

}
 void pinfo(int argc,char** argv,int homeLength,bool background)
 {  
    int pid;
    if(argc ==1) pid=getpid();
    else pid=stoi(argv[1]);
    string status="/proc/"+to_string(pid)+"/status";
    string stat="/proc/"+to_string(pid)+"/stat";
    string exec="/proc/"+to_string(pid)+"/exe";
    string statusInfo,dummy,statInfo;
    char execInfo[PATH_MAX];
    ifstream statfile(stat);
    ifstream statusfile(status);
    if(statfile.is_open())
    {
        statfile >> dummy;
        statfile >> dummy;
        statfile >> statInfo;
    }
    else
    {
        cerr<<"no such process";
        exit(1);
    }
      if(statusfile.is_open())
    {
        while(getline(statusfile,dummy))
        {
            if(dummy.find("VmSize:")==0)
            {
                statusInfo=dummy.substr(8);
                break;
            }

        }
    }
    else
    {
        cerr<<"no such process";
        exit(1);
    }
    
    size_t length = readlink(exec.c_str(),execInfo,sizeof(execInfo));
    cout<<"pid --"<<pid<<endl;
    cout<<"process status -- "<<statInfo<<endl;
    cout<<"memory -- "<<statusInfo<<endl;
    cout<<"Executable Path -- "<<execInfo+homeLength<<endl;
    statusfile.close();
    statfile.close();
 }

 bool search(string cwd,char* path)
 {
    DIR* directory=opendir(cwd.c_str());
    struct stat statfile;
    struct dirent *file;
    while((file=readdir(directory))!=nullptr)
    {   string filepath=cwd+"/"+file->d_name;
        stat(filepath.c_str(),&statfile);
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0) 
        {
            continue;
        }
        if(strcmp(file->d_name,path)==0)
        {   
            closedir(directory);
            return true;
        }
        
        if(S_ISDIR(statfile.st_mode))
        {
            if(search(filepath,path))
            {
            closedir(directory);
            return true;
            }
        }
    }
    return false;
 }
 void processPipes(char **argv) {
    int pipe1[2], pipe2[2];
    int commandCount = 1;
    int index = 0, cmdIndex = 0, pipeIndex = 0;

    char *currentCommand[256];
    pid_t childPid;

    for (int pos = 0; argv[pos] != nullptr; pos++) {
        if (strcmp(argv[pos], "|") == 0) {
            commandCount++;
        }
    }

    while (cmdIndex < commandCount) {
        pipeIndex = 0;

        while (argv[index] != nullptr && strcmp(argv[index], "|") != 0) {
            currentCommand[pipeIndex++] = argv[index++];
        }
        currentCommand[pipeIndex] = nullptr;
        index++;

        if (cmdIndex % 2 == 0) {
            pipe(pipe2);
        } else {
            pipe(pipe1);
        }

        childPid = fork();
        if (childPid == -1) {
            std::cerr << "Failed to create child process\n";
            return;
        } else if (childPid == 0) {
            if (cmdIndex == 0) {
                dup2(pipe2[1], STDOUT_FILENO);
            } else if (cmdIndex == commandCount - 1) {
                if (commandCount % 2 == 0) {
                    dup2(pipe2[0], STDIN_FILENO);
                } else {
                    dup2(pipe1[0], STDIN_FILENO);
                }
            } else {
                if (cmdIndex % 2 == 0) {
                    dup2(pipe1[0], STDIN_FILENO);
                    dup2(pipe2[1], STDOUT_FILENO);
                } else {
                    dup2(pipe2[0], STDIN_FILENO);
                    dup2(pipe1[1], STDOUT_FILENO);
                }
            }

            if (execvp(currentCommand[0], currentCommand) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        } else {
            if (cmdIndex == 0) {
                close(pipe2[1]);
            } else if (cmdIndex == commandCount - 1) {
                if (commandCount % 2 == 0) {
                    close(pipe2[0]);
                } else {
                    close(pipe1[0]);
                }
            } else {
                if (cmdIndex % 2 == 0) {
                    close(pipe1[0]);
                    close(pipe2[1]);
                } else {
                    close(pipe2[0]);
                    close(pipe1[1]);
                }
            }

            waitpid(childPid, nullptr, 0);
        }

        cmdIndex++;
    }
}


 void addToHistory(char* cmd)
 {
    ifstream log("history.log");
    string line;
    vector<string> history;
    while(getline(log,line))
        history.push_back(line);
        history.push_back(cmd);
    if(history.size()>20)
        {
            history.erase(history.begin());
        }
    log.close();
    ofstream logOf("history.log");
    for(string l : history)
    {
        logOf<<l<<endl;
    }
    logOf.close();
 }
  void undoRawMode()
 {
     tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal);
 }
 void rawMode()
 {
    tcgetattr(STDIN_FILENO,&terminal);
    struct termios raw = terminal;
    raw.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
 }
bool autocomplete(char* cmd,int &length,char* realcmd,int &cmdlen)
{   
    //printf("\n%s",cmd);
    //cout<<length;
    char cwd[PATH_MAX];
    getcwd(cwd,sizeof(cwd));
    DIR *directory = opendir(cwd);
    struct dirent *file;
    char *match[100];
    int matches=0;
    while((file=readdir(directory))!=NULL)
    {
        if(strncmp(file->d_name,cmd,length)==0)
        {      
            match[matches]=strdup(file->d_name);
            matches++;
            
        }
    }
    closedir(directory);
    if(matches>1)
    {   cout<<endl;
        for(int i=0;i<matches;i++)
    {
        cout<<match[i]<<"  ";
    }
    //cout<<endl;
    return false;
    }
    else if (matches == 1)
    {   
        for(int i = length;i<strlen(match[0]);i++)
            putchar(match[0][i]);
         realcmd[cmdlen - length] = '\0';
        cmdlen -= length;
        strcat(realcmd, match[0]);
        cmdlen += strlen(match[0]);
        strcpy(cmd, match[0]);
        length = strlen(match[0]); 
        free(match[0]);

        //cout << endl << realcmd << " " << cmdlen << endl;
        return true;
    }
    return true;
}
void signal_handler(int signal)
{
    if(signal == SIGINT)
    {
         if(foregroundPid!=-1)
           { kill(foregroundPid,1);
            foregroundPid=-1;
           }
    }
    // if(signal == SIGTSTP)
    // {
    //      if(foregroundPid!=-1)
    //         {kill(foregroundPid,SIGTSTP);
    //         cout<<"process in background"<<endl;
    //         foregroundPid=-1;
    //         }
    // }


}
int main(void) {
     signal(SIGINT,signal_handler); 
     signal(SIGTSTP,signal_handler);
 
    char home[PATH_MAX];
    size_t n=0;
    getcwd(home,sizeof(home));
    int homeLength=strlen(home);
    int pipes[2];
    pipe(pipes);
    while(true){
    string start=display(homeLength);

    cout<<start;
    fflush(stdout);
    int pid = fork();

    if(pid==0){
    foregroundPid=getpid();
    close(pipes[0]);
    size_t n = 1024;
    char *cmd = (char*)malloc(n*sizeof(char)),*cmd_cpy=(char*)malloc(n*sizeof(char)),*temp=(char*)malloc(n*sizeof(char));

    char **argv;
    char delimiters[]=" \t";
    int i=0;
     rawMode();
    int len=0,templen=0;
    char ch;
    bool flag=false;
    vector<string> historyLines;
    ifstream history("history.log");
    string line;
    while(getline(history,line))
        {   
          historyLines.push_back(line);
        }
    int num=historyLines.size();
    int num2=num;
    while(1)
    {   
        
        ch=getchar();
            if(ch=='\x04')
            {   
                close(pipes[1]);
                exit(4);
            }
            if(ch=='\n')
            {   
                cout<<endl;
                break;
            }
            else if(ch==127)
            {
                if(len>0)
                {
                    
                    cmd[--len]='\0';
                    temp[--templen]='\0';
                    printf("\b \b");
                }
            }
            else if(ch==' ')
            {   
                cmd[len++]=ch; 
                putchar(ch);
                templen=0;
                
            }
            else if(ch=='\t')
            {   

               if(!autocomplete(temp,templen,cmd,len))
                {   
                   
                    flag=true;
                    break;
                }
                //cout<<endl<<temp<<" "<<templen<<endl;
            }
            else if(ch=='\033')
            {   
                
                
                char seq[3];
                seq[0]=ch;
                seq[1]=getchar();
                seq[2]=getchar();

                if(seq[2]=='A')
                {   if(num>0)  
                        num--;
                    printf("\r%s%s", start.c_str(), cmd);
                    strcpy(cmd,historyLines[num].c_str());
                    len=historyLines[num].length();
                }
                if(seq[2]=='B')
                {
                    if(num<num2-1)  
                        num++;
                    printf("\r%s%s", start.c_str(), cmd);
                    strcpy(cmd,historyLines[num].c_str());
                    len=historyLines[num].length();
                }
            }
            else
            {
                cmd[len++]=ch; 
                temp[templen++]=ch;
                putchar(ch); 
            }
        
    }
    //cout<<endl<<cmd<<" "<<len<<endl;
    if(flag) {
        cout<<endl; 
        close(pipes[1]);
        exit(1);}
    cmd[len]='\0';
    addToHistory(cmd);
    cmd_cpy=strdup(cmd);
    if (cmd[len ] == '\n') {
        cmd[len] = '\0';
        cmd_cpy[len]='\0';
    }
    char *token = strtok(cmd, delimiters);
    int argc = 0;

    while (token != NULL) {

        argc++;
        token = strtok(NULL, delimiters);
    }

  

    argv = (char**)malloc(sizeof(char*)*(argc+1));
    token=strtok(cmd_cpy,delimiters);
    while(token)
    {   argv[i]=token;
        token=strtok(NULL,delimiters);
        i++;
    }
    argv[i]=NULL;
    bool background=false;
    if(strcmp(argv[i-1],"&")==0)  {
          background=true;
          argc--;
          argv[i-1]=NULL;
    }
    write(pipes[1],&background,sizeof(background));
        //close(pipes[1]);
        int saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout == -1) {
        perror("dup");
        return 1;
    }
    int saved_stdin = dup(STDIN_FILENO);
    if (saved_stdin == -1) {
        perror("dup");
        return 1;
    }
    int handlepipe=0;
    for(int i= 0;i<argc;i++)
    {   if(strcmp(argv[i],"|")==0)
        {
            handlepipe=1;
            break;
        }
        else if(strcmp(argv[i],">>")==0)
        {

              if(i+1<argc)
            {
            int fd = open(argv[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0644);
           
            
            dup2(fd,STDOUT_FILENO);
            close(fd);
                argc -= 2;
                for (int j = i; j < argc; j++) {
                argv[j] = argv[j + 2];
            }
            }
        }
        else if(strcmp(argv[i],">")==0)
        {
        if(i+1<argc)
            {
            int fd = open(argv[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
           
            
            dup2(fd,STDOUT_FILENO);
            close(fd);
                argc -= 2;
                for (int j = i; j < argc; j++) {
                argv[j] = argv[j + 2];
            }
        }
        }
        else if(strcmp(argv[i],"<")==0)
        {

             if (i + 1 < argc) {
                int fd = open(argv[i + 1], O_RDONLY);
                

                dup2(fd, STDIN_FILENO);
                close(fd);

                argc -= 2;
                for (int j = i; j < argc; j++) {
                argv[j] = argv[j + 2];
            }
        argv[argc] = NULL;
    }}
        
    }
    if(handlepipe==1)
    {
        processPipes(argv);
    }
    else
    if(strcmp(argv[0],"cd")==0)
    {   
        if(argc>2) 
        {
            cerr<<"too many arguments";
            exit(0);
        }
        cd(argv,argc,homeLength,home);
    }
    else if(strcmp(argv[0],"echo")==0)
    {
        echo(argv,argc);
    }
    else if(strcmp(argv[0],"pwd")==0)
    {   
        pwd(homeLength);
    }
    else if(strcmp(argv[0],"history")==0)
    {   
        vector<string> historyLines;
        ifstream history("history.log");
        string line;
        int num;
        if(argc==1)
            num=10;
        else
        num = stoi(argv[1]);
        int x=0;
        while(getline(history,line))
        {   x++;
            historyLines.push_back(line);
        }
        int start=max(0,(int)historyLines.size()-num);
        for(int i=start;i<x;i++)
        {
            cout<<historyLines[i]<<endl;
        }
    }
    else if(strcmp(argv[0],"ls")==0)
    {   
        bool a=false,l=false;
        vector<char*> dirBuffer;
        for(int i=1;i<argc;i++)
        {
            if(strcmp(argv[i],"-l")==0)
            l=true;
        else if(strcmp(argv[i],"-a")==0)
            a=true;
        else if(strcmp(argv[i],"-la")==0|| strcmp(argv[i],"-al")==0)
            {
                a=true;
                l=true;
            }
        else{
                dirBuffer.push_back(argv[i]);
        }
        }
        if(dirBuffer.size()>0)
        for(int i=0;i<dirBuffer.size();i++)
            ls(dirBuffer[i],l,a);
        else
        {
            ls((char*)"",l,a);
        }
    }
    else if(strcmp(argv[0],"pinfo")==0)
    {   
        pinfo(argc,argv,homeLength,background);
    }
    else if(strcmp(argv[0],"search")==0)
    {
        if(argc ==1) {cerr<<"enter the file name"; exit(1);}
        if(argc >2) {cerr<<"too many arguments";  exit(1);}
        char cwd[PATH_MAX];
        getcwd(cwd,sizeof(cwd));
        if(search((string)(cwd),argv[1]))
            {
                cout<<"true \n";
            } 
        else
        {
            cout<<"false \n";
        }
    }
    else
    {   
        
        execvp(argv[0],argv);
        perror("execvp");
        exit(1);
    }

    free(cmd),free(argv),free(cmd_cpy),free(temp);
        if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
        perror("dup2");
        return 1;
    }
      if (dup2(saved_stdin, STDIN_FILENO) == -1) {
        perror("dup2");
        return 1;
    }
    close(saved_stdout);
    close(saved_stdin);

    }
            else if (pid > 0) {
                int status;
                waitpid(pid,&status,0);
                 if (WIFEXITED(status)) {
             int exitstatus = WEXITSTATUS(status);
                if (exitstatus == 4) exit(1);
                 }
//             } 

//         close(pipes[1]);
//         wait(0);
//         bool x;
//         ssize_t bytesRead = read(pipes[0], &x, sizeof(x));
// if (bytesRead != sizeof(x)) {
//     cerr << "Error reading from pipe" << endl;
// }
//         cout <<"prarent"<<x<<endl;
//         if (!x) {
//             int status;
//             waitpid(pid, &status, 0);
//             if (WIFEXITED(status)) {
//                 int exitstatus = WEXITSTATUS(status);
//                 if (exitstatus == 4) exit(1);
//             } else {
//                 cout << "Process running in background with PID: " << pid << endl;
//             }
//         }
//         close(pipes[0]);
    }
else
{
     cerr << "Fork failed!" << endl;
}

    }

    return 0;
}