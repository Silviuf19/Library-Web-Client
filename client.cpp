#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
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

int main(){
    string command;
    string input_line;
    string cookie;
    string token;
    bool is_logged = false;
    bool logout = false;
    bool has_access = false;

    // rularea aplicatiei
    while (true){
        // citeste comanda data
        getline(cin, input_line);
        //verifica daca comanda e scrisa gresit
        if (hasSpaces(input_line)){
            invalidCommand();
        }
        else{

            // retine comanda si ia fiecare caz in parte
            string command = input_line.substr(0, input_line.find(" "));

            // register
            if (command.compare("register") == 0){

                // preia credentialele
                string username, password;
                cout << "username=";
                getline(cin, username);
                cout << "password=";
                getline(cin, password);

                //verifica daca credentialele sunt corecte
                if (!hasSpaces(username) && !hasSpaces(password)){

                    // register POST
                    json json_credentials;
                    json_credentials["password"] = password;
                    json_credentials["username"] = username;
                    string credentials = json_credentials.dump();

                    //face requestul si verifica mesajul
                    string response = POST("34.241.4.235:8080", "/api/v1/tema/auth/register", credentials, "", "");
                    if (!response.compare("error")){
                        cout << "Username already taken." << endl;
                    }
                    else{
                        cout << "User registered sucesfully." << endl;
                    }
                }
                else{
                    cout << "Invalid credentials." << endl;
                }
            }

            //login
            else if (command.compare("login") == 0){

                //verifica daca utilizatorul e logat, in caz contrar afiseaza un mesaj de eroare
                if (!is_logged){

                    //parseaza datele
                    string username, password;
                    cout << "username=";
                    getline(cin, username);
                    cout << "password=";
                    getline(cin, password);

                    //verifica daca credentialele sunt corecte
                    if (!hasSpaces(username) && !hasSpaces(password)){

                        //parseaza credentialele in josn
                        json json_credentials;
                        json_credentials["password"] = password;
                        json_credentials["username"] = username;
                        string credentials = json_credentials.dump();

                        //trimite requestul si in functie de response afiseaza mesaje corespunzatoare
                        string response = POST("34.241.4.235:8080", "/api/v1/tema/auth/login", credentials, "", "");
                        if (!response.compare("error")){
                            cout << "Invalid username or password." << endl;
                        }
                        else{
                            cookie = response;
                            is_logged = true;
                            logout = false;
                            cout << "User loged-in sucesfully." << endl;
                        }
                    }
                    else{
                        invalidCredentials();
                    }
                }
                else{
                    userLogged();
                }
            }

            //enter_library
            else if (command.compare("enter_library") == 0){
                if (is_logged){
                    if (!has_access){
                        
                        // trimite requestul si in functie de mesaj afiseaza ce trebuie
                        json response;
                        response = GET("34.241.4.235:8080", "/api/v1/tema/library/access", "", cookie, "");
                        if (response.contains("token")){
                            cout << "Access granted." << endl;
                            response["token"].get_to(token);
                            has_access = true;
                        }
                        else{
                            cout << "Something went wrong" << endl;
                        }
                    }
                    else{
                        userHasAccess();
                    }
                }
                else{
                    userNotLogged();
                }
            }

            //get_books
            else if (command.compare("get_books") == 0){
                
                //verifica utilizatorul e logat si are acces la biblioteca
                if (is_logged){
                    if (has_access){

                        // trimite requestul
                        json books;
                        books = GET("34.241.4.235:8080", "/api/v1/tema/library/books", "", cookie, token);

                        //afiseaza cartea sau faptul ca nu exista carti
                        if(books.size() != 0){
                            int id_json;
                            string title_json;
                            for(json data : books){
                                data["id"].get_to(id_json);
                                data["title"].get_to(title_json);
                                cout << title_json << "(id:" << id_json << ")" << endl;
                            } 
                        }else{
                            cout << "The library is empty." << endl;
                        }
                    }
                    else{ 
                        userNoAccess();
                    }
                }
                else{
                    userNotLogged();
                }
            }

            //get_book
            else if (command.compare("get_book") == 0){

                //verifica daca userul este logat si are acces la biblioteca
                if (is_logged){
                    if (has_access){

                        //primeste id ul 
                        string ID;
                        cout << "id=";
                        getline(cin, ID);

                        //verifica daca id ul e corect
                        if (isNumber(ID)){

                            //trimite request
                            json book_list;
                            book_list = GET("34.241.4.235:8080", "/api/v1/tema/library/books", ID, cookie, token);

                            //afiseaza cartea sau mesajul de eroare in functie de raspuns
                            if(book_list.contains("error")){
                                cout << "The book with id = " << ID << " is not in library." << endl;
                            }else{
                                string title, author, publisher, genre;
                                int page_count;
                                for(json book : book_list){
                                    book["title"].get_to(title);
                                    book["author"].get_to(author);
                                    book["publisher"].get_to(publisher);
                                    book["genre"].get_to(genre);
                                    book["page_count"].get_to(page_count);
                                
                                    cout << "title: " << title << endl;
                                    cout << "author: " << author << endl;
                                    cout << "publisher: " << publisher << endl;
                                    cout << "genre: " << genre << endl;
                                    cout << "page count: " << page_count << endl;
                                }
                            }
                        }
                        else{
                            invalidID();
                        }
                    }
                    else{
                        userNoAccess();
                    }
                }
                else{
                    userNotLogged();
                }
            }
            
            //add_book
            else if (command.compare("add_book") == 0){

                //verifica daca utilizatorul e logat si are access la biblioteca
                if (is_logged){
                    if (has_access){

                        //citeste datele de la utilizator
                        string title, author, genre, publisher, page_count;
                        cout << "title=";
                        getline(cin, title);
                        cout << "author=";
                        getline(cin, author);
                        cout << "genre=";
                        getline(cin, genre);
                        cout << "publisher=";
                        getline(cin, publisher);
                        cout << "page_count=";
                        getline(cin, page_count);

                        //verifica datele 
                        if (isNumber(page_count) && !title.empty() && !author.empty() && !genre.empty() && !publisher.empty()){

                            //parseaza datele
                            json json_book;
                            json_book["title"] = title;
                            json_book["author"] = author;
                            json_book["genre"] = genre;
                            json_book["publisher"] = publisher;
                            json_book["page_count"] = page_count;

                            //trimite requestul
                            string book = json_book.dump();
                            POST("34.241.4.235:8080", "/api/v1/tema/library/books", book, cookie, token);
                            cout << "Book added."<< endl;   
                        }
                        else{
                            //afiseaza mesaje de eroare
                            invalidBookInfo();
                        }
                    }
                    else{
                        userNoAccess();
                    }
                }
                else{
                    userNotLogged();
                }
            }

            //delete_book
            else if (command.compare("delete_book") == 0){

                //verifica daca utilizatorul are acces la biblioteca si daca e logat
                if (is_logged){
                    if (has_access){
                        
                        //primeste id ul
                        string ID;
                        cout << "id=";
                        getline(cin, ID);

                        //verifica datele, trimite request si in functie de raspuns afiseaza un mesaj
                        if (isNumber(ID)){
                            if(DELETE("34.241.4.235:8080", "/api/v1/tema/library/books", ID, cookie, token)){
                                cout << "Book deleted." << endl;
                            }else{
                                cout << "Book with id=" << ID << " not found." << endl;
                            }
                        }else{
                            invalidID();
                        }
                    }
                    else{
                        userNoAccess();
                    }
                }
                else{
                    userNotLogged();
                }
            }

            //logout
            else if (command.compare("logout") == 0){
                if (is_logged){
                    
                    //trimite requestul
                    GET("34.241.4.235:8080", "/api/v1/tema/auth/logout", "", cookie, "");
                    cout << "User logged-out succesfully." << endl;

                    //elimina acessul la biblioteca, goleste datele din token si cookie, 
                    //updateaza statusul is_logged ca false
                    has_access = false;
                    is_logged = false;
                    token.clear();
                    cookie.clear();
                    is_logged = false;

                    //variabila care verifica daca utilizatorul s-a delogat, pentru
                    //a verifica cazul in care doreste sa se delogheze de mai multe ori
                    logout = true;

                //mesaje de eroare
                }else if(logout){
                    cout << "User already logged-out" << endl;
                }else{
                    cout << "User not logged." << endl;
                }
            }
            
            //exit
            else if (command.compare("exit") == 0){
                
                //elimina acessul la biblioteca, goleste datele din token si cookie
                has_access = false;
                is_logged = false;
                token.clear();
                cookie.clear();
                break;
            }

            //afiseaza mesaj de eroare daca comanda data e incorecta
            else{
                invalidCommand();
            }
        }
    }
}