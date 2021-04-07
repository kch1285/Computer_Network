#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

#define PORT 9999

string HTM = "Content-Type: text/html/r/n";
string HTML = "Content-Type: text/html/r/n";
string GIF = "Content-Type: image/gif/r/n";
string JPEG = "Content-Type: image/jpeg/r/n";
string MP3 = "Content-Type: audio/mpeg/r/n";
string PDF = "Content-Type: application/pdf/r/n";

string STATUS_ERROR = "STATUS_ERROR";

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

void writeResponse(int newFd){
	int n, fileFd;
	struct stat fileInfo;
	time_t t;
	struct tm *TimeTm, *modifyTm;
	char currTime[100];
	char modifyTime[100];
	char *fileBuf;
	char buf[8192];
	ifstream inFile;
	streampos fs;
	string fileName;
	memset(buf, 0, 8192);
	n = read(newFd, buf, 8192);

	if(n < 0)	perror("read Error");

	cout << buf << '\n';
	fileName = parseFileName(buf);
	if(fileName == ""){
		write(newFd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
		return;
	}

	for(string::size_type i = 0; (i = fileName.find("%20", i)) != string::npos;){
		fileName.replace(i, 3, " ");
		i++;
	}

	fileFd = open(fileName.c_str(), O_RDONLY);
	if(fileFd < 0){
		write(newFd, STATUS_ERROR.c_str(), STATUS_ERROR.length());
		return;
	}

	
	inFile.open(fileName.c_str(), ios::in | ios::binary | ios::ate);
	if(inFile.is_open()){
		fs = inFile.tellg();
		filebuf = new char[(long long)(fs) + 1];
		inFile.seekg(0, ios::beg);
		inFile.read(filebuf, fs);
		inFile.close();
	}
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


	return 0;
}
