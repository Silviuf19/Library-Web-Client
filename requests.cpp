#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <iostream>
#include <sstream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

//functia pentru POST
string POST(string host, string url, string data, string cookie, string token){
    stringstream message;
    stringstream response;
    string temp_cookie;

    // adaugare tip request, host, content-type, content-length, cookie si token
    message << "POST " << url << " HTTP/1.1" << "\r\n";
    message << "Host " << host << "\r\n";
    message << "Content-Type: application/json" << "\r\n";
    message << "Content-Length: " << data.size() << "\r\n";
    if(!cookie.empty()){
        message << "Cookie: " << cookie << "\r\n";
    }
    if(!token.empty()){
        message << "Authorization: Bearer " << token << "\r\n";
    }

    //adaugare linie noua si date
    message << "\r\n";
    message << data;

    // deschidere conexiune
    int sockfd = open_connection((char *)"34.241.4.235\0", 8080, AF_INET, SOCK_STREAM, 0);

    //parsare mesaj pentru functia send_to_server
    string message_string = message.str();
    char *message_char = strcpy(new char[message_string.length() + 1], message_string.c_str());

    // trimite request-ul, primeste raspunsul si inchide conexiunea
    send_to_server(sockfd, message_char);
    response << receive_from_server(sockfd);
    close(sockfd);

    // register
    if (!url.compare("/api/v1/tema/auth/register")){
        int message_len;

        // trece prin mesaj pana gaseste lungimea contentului
        string temp;
        while (response >> temp){
            if (!temp.compare("Content-Length:")){
                response >> message_len;
                break;
            }
        }

        // daca nu am primit nici un mesaj de eroare de la server (content-length == 0) atunci
        // trimite un string gol, altfel trimit "eroare"
        if (!message_len){
            return "";
        }
        else{
            return "error";
        }
    }

    // login
    else if (!url.compare("/api/v1/tema/auth/login")){
        // trece prin fiecare linie a raspunsului
        string temp;
        string argument;
        while (getline(response, temp)){
            argument = temp.substr(0, temp.find(" "));

            // daca gaseste campul "Set-Cookie" atunci retine cookie-ul
            if (!argument.compare("Set-Cookie:")){
                temp_cookie = temp.substr(temp.find(" ") + 1, temp.find(";") - 12);
                break;
            }
        }

        // returneaza cookie ul daca s-a gasit, altfel returneaza eroare
        if (!temp_cookie.empty()){
            return temp_cookie;
        }
        else{
            return "error";
        }
    }

    // add_book
    else if (!url.compare("/api/v1/tema/library/books")){

        //verificarea datelor s-a realizat in client deci se returneaza
        //un mesaj gol
        return "";
    }

    return "";
}

json GET(string host, string url, string query_param, string cookie, string token){
    stringstream message;
    stringstream response;
    string temp_token;

    // deschidere conexiune
    int sockfd = open_connection((char *)"34.241.4.235\0", 8080, AF_INET, SOCK_STREAM, 0);

    // adaugare tip request, host, query-params, cookie si token
    if (query_param.empty()){
        message << "GET " << url << " HTTP/1.1"<< "\r\n";
    }else{
        message << "GET " << url << "/" << query_param << " HTTP/1.1" << "\r\n";
    }
    message << "Host " << host << "\r\n";

    if(!cookie.empty()){
        message << "Cookie: " << cookie << "\r\n";
    }
    if(!token.empty()){
        message << "Authorization: Bearer " << token << "\r\n";
    }
    message << "\r\n";

    //parsare mesaj pentru functia send_to_server
    string message_string = message.str();
    char *message_char = strcpy(new char[message_string.length() + 1], message_string.c_str());

    //trimite mesaj la server, primeste raspuns si inchide conexiunea
    send_to_server(sockfd, message_char);
    response << receive_from_server(sockfd);
    close(sockfd);

    //enter_library
    if (!url.compare("/api/v1/tema/library/access")){
        string temp;
        string testValue;
        string string_json;

        //cauta in mesaj tokenul primit ca raspuns, sau mesajul de eroare daca 
        //ceva nu e in regula
        while (getline(response, temp)){
            testValue = temp.substr(0, temp.find(":"));
            if (!testValue.compare("{\"token\"") || !testValue.compare("{\"error\"")){
                string_json = temp;
                break;
            }
        }

        //parsare token primit din mesaj
        json tok;
        tok = json::parse(string_json);

        //returneaza tokenul sau eroarea
        return tok;
    }

    //get_books
    else if (!url.compare("/api/v1/tema/library/books") && query_param.empty()){
        string temp;
        string testValue;
        string string_json;

        //cauta jsonul de carti
        while (getline(response, temp)){
            testValue = temp.substr(0, temp.find("[") + 1);
            if (!testValue.compare("[")){
                string_json = temp;
                break;
            }
        }

        //parseaza lista de carti
        json book_list;
        book_list = json::parse(string_json);

        //returneaza lista de carti parsata
        return book_list;
    }

    //get_book
    else if (!url.compare("/api/v1/tema/library/books")&& !query_param.empty()){
        string temp;
        string testValue;
        string string_json;

        //cauta obiectul carte in raspuns
        while (getline(response, temp)){
            testValue = temp.substr(0, temp.find("{") + 1);
            if (!testValue.compare("{") || !testValue.compare("[{")){
                string_json = temp;
                break;
            }
        }

        //parseaza si returneaza cartea
        json book;
        book = json::parse(string_json);
        return book;

    //logout    
    }else if(!url.compare("/api/v1/tema/auth/logout")){

        //a fost trimis corect mesajul catre server deci returnez
        //un json gol
        return json({});
    }else{
        return json({});
    }
}

int DELETE(string host, string url, string query_param, string cookie, string token){
    stringstream message;
    stringstream response;
    int message_len;
    
    // adaugare tip request, host, cookie, token
    message << "DELETE " << url << "/" << query_param << " HTTP/1.1" << "\r\n";
    message << "Host " << host << "\r\n";
    message << "Cookie: " << cookie << "\r\n";
    message << "Authorization: Bearer " << token << "\r\n";
    message << "\r\n";

    // deschidere conexiune
    int sockfd = open_connection((char *)"34.241.4.235\0", 8080, AF_INET, SOCK_STREAM, 0);

    // parsare date pentru functia send_to_server
    string message_string = message.str();
    char *message_char = strcpy(new char[message_string.length() + 1], message_string.c_str());

    // trimitere request si primire raspuns
    send_to_server(sockfd, message_char);
    response << receive_from_server(sockfd);
    close(sockfd);

    //verifica daca content-length e 0 sau nu
    string temp;
    while (response >> temp){
        if (!temp.compare("Content-Length:")){
            response >> message_len;
            break;
        }
    }

    //daca content-length e 0 atunci totul e in regula, altfel avem o eroare
    if(message_len == 0){
        return 1;
    }else{
        return 0;
    }

    return 0;
}