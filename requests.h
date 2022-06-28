#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include <iostream>
#include <sstream>
#include <string>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

string POST(string host, string url, string data, string cookie, string token);
json GET(string host, string url, string query_param, string cookie, string token);
int DELETE(string host, string url, string query_param, string cookie, string token);
