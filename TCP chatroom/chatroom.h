#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PORT_NO 6969


typedef struct packet
{
    int type;
    union data
    {
        
        struct                          //structure for the register packets
        {
            char username[50];
            char password[50];
            int status;
        }reg;
        struct                          //structure for the login packets
        {
            char username[50];
            char password[50];
            int status;
        }log;
        struct                          //structure for the single chat packets
        {
            char username[50];
            char message[100];
        }single_chat;
        struct                          //structure for the multichat packets
        {
            char username[50];
            char message[100];
        }multi_chat;
        struct                          //structure for the get online users packet for single chat
        {
            char username[50];
        }userlist;
        struct
        {   
            int status;
        } group;

    }data; 
}packet;

enum packet_type
{
    LOGIN,
    REGISTER,
    SINGLE_CHAT,
    MULTI_CHAT,
    LOGOUT,
    GET_ONLINE_USERS,
    JOIN_GROUP,
    LEAVE_GROUP
};

typedef struct users
{
    char username[50];
    int sock_fd;
    struct users* next;
    int in_group; // 0 = normal, 1 = multichat 
}Online_users;


//SERVER FUNCTION PROTOTYPES
void* handle_client(void* sock_fd);
void add_to_list(Online_users** head,char username[],int sockfd);
int check_list(Online_users* head,char username[]);
void remove_from_list(Online_users** head, int new_sockfd);

//CLIENT FUNCTION PROTOTYPES
void register_client(int sock_fd);
void login_client(int sock_fd);
void single_chat(int sock_fd);
void multi_chat(int sock_fd);
void* recieve_chats(void* arg);
void logout(int sock_fd);


extern pthread_mutex_t lock;
extern int login_flag;
extern volatile int list_done_flag;
extern volatile int no_users_flag;

#endif