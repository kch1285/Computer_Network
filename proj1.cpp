#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <netinet/in.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>

#include <locale>

using namespace std;

#define PORT 9999

string HTM = "Content-Type: text/html/r/n";
string HTML = "Content-Type: text/html/r/n";
string GIF = "Content-Type: image/gif/r/n";
string JPEG = "Content-Type: image/jpeg/r/n";
string MP3 = "Content-Type: audio/mpeg/r/n";
string PDF = "Content-Type: application/pdf/r/n";

string STATUS_ERROR = "STATUS_ERROR";

void error(string msg)
{
    perror(msg.c_str());
    exit(1);
}


void handleSigchild(int sig){
	while(waitpid((pid_t)(-1), 0, WNOHANG) > 0);
	fprintf(stderr, "Child exited successfully with code %d.\n", sig);
}

string parseFileName(char *buf){
	string request(buf);
	if(request == "")	return "";

	size_t found = request.find(" ");
	size_t found2 = request.find(" HTTP/1.1/r/n", found + 1, 10);
	string tmp = (found2 - found - 2 > 0) ? request.substr(found + 2, found2 - found - 2) : "";

	if(tmp == "")	return "";
	
	size_t foundPeriod = tmp.find(".");
	if(foundPeriod == string::npos)	return tmp;

	string tmp2 = tmp.substr(0, foundPeriod);
	for(int i=foundPeriod;i<tmp.length();i++)	tmp += tolower(tmp[i]);
	return tmp2;
}

void writeResponse(int newsockfd){
	int n, fileFd;
	struct stat fileInfo;
	time_t t;
	struct tm *timeTm, *modifyTm;
	char currTime[100];
	char modifyTime[100];
	char *fileBuffer;
	char buf[8192];
	ifstream inFile;
	streampos fs;

	memset(buf, 0, 8192);
	n = read(newsockfd, buf, 8192);

	if(n < 0)	error("read error");

	cout << buf << '\n';
	string fileName = parseFileName(buf);
	if(fileName == ""){
		write(newsockfd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
		return;
	}

	for(string::size_type i = 0; (i = fileName.find("%20", i)) != string::npos;){
		fileName.replace(i, 3, " ");
		i++;
	}

	fileFd = open(fileName.c_str(), O_RDONLY);
	if(fileFd < 0){
		write(newsockfd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
		return;
	}

	
	inFile.open(fileName.c_str(), ios::in | ios::binary | ios::ate);
	if(inFile.is_open()){
		fs = inFile.tellg();
		fileBuffer = new char[(long long)(fs) + 1];
		inFile.seekg(0, ios::beg);
		inFile.read(fileBuffer, fs);
		inFile.close();
	}

	t = time(NULL);
	timeTm = gmtime(&t);
	strftime(currTime, sizeof(currTime), "Date : %a, %d %b %G %T GMT", timeTm);
	modifyTm = gmtime(&fileInfo.st_mtime);
	strftime(modifyTime, sizeof(modifyTime), "Last-Modified: %a, %d %b %G %T GMT", modifyTm);

	string responseStatus = "OKOKOK";
	string date(currTime);
	date += "/r/n";
	string server = "Server : Chi-Hoon Kang Server\r\n";
	string lastModified(modifyTime);
	lastModified += "\r\n";
	long long fileLength = fileInfo.st_size;
	char cLength[1000];

	sprintf(cLength, "Content-length : %lld\r\n", fileLength);
	string contentLength(cLength);
	string closeConnection = "Connection : close\r\n";
	string contentDisposition = "Content Disposition : inline\r\n";
	string contentType ="";// parseFileType(fileName);

	string respHeader = responseStatus + date + server + lastModified + contentLength
	 + contentType + contentDisposition + closeConnection + "\r\n";
	
	cout << respHeader << '\n';

	write(newsockfd, respHeader.c_str(), respHeader.length());
	write(newsockfd, fileBuffer, fileLength);
	close(fileFd);
	delete[] fileBuffer;
}

string parseFileType(string inputFile){
	string file = "";

	for(int i=0;i<inputFile.length();i++){
		file += tolower(inputFile[i]);
	}

	if(file.find(".html") != string::npos)	return HTML;
	if(file.find(".htm") != string::npos)	return HTML;
	if(file.find(".gif") != string::npos)	return GIF;
	if(file.find(".jpeg") != string::npos)	return JPEG;
	if(file.find(".mp3") != string::npos)	return MP3;
	if(file.find(".pdf") != string::npos)	return PDF;
	return HTML;
}
int main(){
	int sockfd, newsockfd, n;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)	error("socket");

	memset((char *)&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5556);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
		error("bind");
	
	if(listen(sockfd, 10) < 0)
		error("listen");
	
	clilen = sizeof(cli_addr);

	struct sigaction sa;
	sa.sa_handler = &handleSigchild;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

	if(sigaction(SIGCHLD, &sa, 0) == -1)
		error("Sig action");
	

	while(true){
		if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0)
			error("ERROR on accept");
		
		pid_t pid = fork();

		if(pid == 0){
			close(sockfd);
			// char buf[8192];
			// memset(buf, 0, 8192);
			// n = read(newsockfd, buf, 8192);
			// if (n < 0) error("ERROR reading from socket");
			// printf("Here is the message: %s\n",buf);
			writeResponse(newsockfd);
			close(newsockfd);
			exit(0);
		}
		else if(pid > 0){
			close(newsockfd);
			continue;
		}
		else
			error("Fork failed");
	}
	return 0;
}
