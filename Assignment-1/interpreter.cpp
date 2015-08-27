#include<stdio.h>
#include<fstream>
#include<iostream>
#include<vector>
#include<string>
#include<map>
using namespace std;
string transform(string line) {
	map<string, string> mapping;
	mapping["%ebp"] = "%rbp";
	mapping["%esp"] = "%rsp";
	mapping["pushl"] = "pushq";
	mapping["8"] = "16";
	mapping["5"] = "6";
	mapping["-8"] = "-16";
	mapping["-5"] = "-6";
	mapping["leave"] = "popq    %rbp";
	size_t pos = 0;
	string newline;
	string token;
	if(line.find(".cfi") != string::npos) {
		pos = line.find(" ");
		string f = line.substr(0,pos);
		string s = line.substr(pos+1);
		if (mapping.find(s) != mapping.end())
		{
			s = mapping[s];
		}
		if(s.find(", ")!=string::npos) {
			size_t p = s.find(", ");
			s = mapping[s.substr(0,p)] + ", " + mapping[s.substr(p+2)];
		}
		return f+" "+s;
	}
	while((pos = line.find("\t")) != string::npos) {
		token = line.substr(0, pos);
		if (mapping.find(token) != mapping.end()) {
			newline = newline + mapping[token] + "\t";
		}
		else 
			newline = newline + token + "\t";
		line.erase(0, pos + 1);
	}
	token = line.substr(0,pos);
	if(token.find(", ") != string::npos) {
		size_t p = token.find(", ");
		string first = token.substr(0, p);
		token.erase(0, p+2);
		string second = token;
		if(mapping.find(first) != mapping.end())
			first = mapping[first];
		if(mapping.find(second) != mapping.end())
			second = mapping[second];
		if(first.find("(") != string::npos) {
			size_t t = first.find('(');
			first = first.substr(0,t) + "("+mapping[first.substr(t+1,4)] + ")" ;
		}
		if(second.find("(") != string::npos) {
			size_t t = second.find('(');
			second = second.substr(0,t) + "("+mapping[second.substr(t+1,4)] + ")" ;
		}
		newline = newline + first + ", " + second + "\0";
	}
	else if (mapping.find(token) != mapping.end()) {
		newline = newline + mapping[token] + "\0";
	}
	else
		newline = newline + token + "\0";
	cout<<newline<<endl;
	return newline;	
}
int main()
{
	string line;
	int flag = 0;
	int flag_leave = 0;
	ifstream ia32_file("ia32.s");
	ofstream ofs;
	ofs.open("output.s", ofstream::out);
	if(ia32_file.is_open()) {
		while( getline(ia32_file, line)) {
			if(line.find("subl	$16, %esp") != string::npos)
				continue;
			if(flag == 0) {
				ofs << line;
				ofs << "\n";
			}
			if(flag_leave == 1 && line.find("ret") != string::npos) {
				ofs << "\t.cfi_def_cfa 7, 8\n";
				ofs << "\tret\n";
				ofs << "\t.cfi_endproc\n";
				flag_leave = 0;
			}
			else if (flag_leave == 0) {
				if(flag == 1 && line.find(".cfi_endproc") == string::npos) {
					ofs << transform(line);
					ofs<<"\n";
				}
				if(line.find("leave") != string::npos)
					flag_leave=1;
				if(line.find(".cfi_startproc") != string::npos) 
					flag=1;
				else if(line.find(".cfi_endproc") != string::npos) {
					flag=0;
				}
			}
		}
	}
	ofs.close();
	ia32_file.close();
	return 0;
}
