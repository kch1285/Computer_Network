#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

using namespace std;

#define PORT 9999

string HTM = "Content-Type: text/html/r/n";
string HTML = "Content-Type: text/html/r/n";
string GIF = "Content-Type: image/gif/r/n";
string JPEG = "Content-Type: image/jpeg/r/n";
string MP3 = "Content-Type: audio/mpeg/r/n";
string PDF = "Content-Type: application/pdf/r/n";

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

int main(){


	return 0;
}
