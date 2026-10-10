#include"chatroom.h"

volatile int list_done_flag = 0;
volatile int no_users_flag = 0;
volatile int join_status = -1;
volatile int leave_status = -1;

//function to register the cleint 
void register_client(int sock_fd)
{
    packet data_client;
    packet data_server;

    data_client.type = REGISTER;

    printf("Enter username: ");
    scanf("%s", data_client.data.reg.username);

    printf("Enter password: ");
    scanf("%s", data_client.data.reg.password);

    // send to server
    send(sock_fd, &data_client, sizeof(data_client), 0);

    // receive response (status)
    recv(sock_fd, &data_server, sizeof(data_server), 0);

    // check status
    if (data_server.data.reg.status == 0)
    {
        printf("Registration successful\n");
    }
    else
    {
        printf("Username already exists\n");
    }
}


//function to login the cleint
void login_client(int sock_fd)
{
    packet data_client;
    packet data_server;

    pthread_t tid;
    int* ptr = malloc(sizeof(int));
    *ptr = sock_fd;

    data_client.type = LOGIN;

    printf("Enter username: ");
    scanf(" %[^\n]",data_client.data.log.username);

    printf("Enter password: ");
    scanf(" %[^\n]",data_client.data.log.password);

    //send the username and password to server
    send(sock_fd,&data_client,sizeof(data_client),0);

    //receive response (status)
    recv(sock_fd,&data_server,sizeof(data_server),0);

    //check status
    if(data_server.data.log.status == 2)
    {
        printf("User already logged in\n");
    }
    else if(data_server.data.log.status == 0)
    {
        printf("Invalid credentials\n");
    }
    else
    {
        login_flag = 1;
        pthread_create(&tid,NULL,recieve_chats,ptr);
        pthread_detach(tid);
        printf("Login successfull\n");
    }
}


//function for single chat
void single_chat(int sock_fd)
{
    packet data_client;
    // packet data_server;

    data_client.type = GET_ONLINE_USERS;

    send(sock_fd,&data_client,sizeof(data_client),0);
    
    while(list_done_flag == 0);   // wait for list  (wait till all the online members are recieved)
    
        list_done_flag = 0;   // reset for next use
        
        if(no_users_flag)
        {
            no_users_flag = 0;
            printf("No users online\n");
            return;
        }

        printf("Enter username: ");
        scanf("%s", data_client.data.single_chat.username);

        printf("Enter message: ");
        scanf(" %[^\n]", data_client.data.single_chat.message);

        data_client.type = SINGLE_CHAT;
        send(sock_fd, &data_client, sizeof(data_client), 0);        
}

void multi_chat(int sock_fd)
{
    packet data_client;
    data_client.type = JOIN_GROUP;
    send(sock_fd, &data_client, sizeof(data_client), 0);
    while(join_status == -1);
    if(join_status == 1)
    {
        data_client.type = MULTI_CHAT;
        join_status = -1;
        printf("=====CHATROOM=====\n");
        printf("type '/exit' to exit the chatroom\n");
        while(1)
        {
            scanf(" %[^\n]",data_client.data.multi_chat.message);
            if(strcmp(data_client.data.multi_chat.message,"/exit") == 0)
            {
                data_client.type = LEAVE_GROUP;
                send(sock_fd, &data_client, sizeof(data_client), 0);
                while(leave_status == -1);                      // wait till the server removes cleint from the multichat list
                printf("leaving chatroom\n");
                break;
            }
            send(sock_fd, &data_client, sizeof(data_client), 0);
        }
    }
    else
    {
        join_status = -1;
        printf("Failed to join group\n");
        return;
    }
}


void logout(int sock_fd)
{
    packet data_client;
    data_client.type = LOGOUT;
    send(sock_fd, &data_client, sizeof(data_client), 0);
}



//thread function to recieve messages
void* recieve_chats(void* arg)
{
    int sock_fd = *(int*)arg;
    free(arg);
    packet data_server;

    while(login_flag)
    {
        static int printed = 0;
        static int count = 0;
        int n = recv(sock_fd,&data_server,sizeof(data_server),0);
        if(n <= 0)
        {
            if(n == 0)

                printf("Server disconnected\n");
            else
                perror("recv");
        login_flag = 0;   // stop thread
        break;  
        }
        if(data_server.type == SINGLE_CHAT)
        {
            printf("\n%s : %s\n",data_server.data.single_chat.username,data_server.data.single_chat.message);
        }

        else if(data_server.type == MULTI_CHAT)
        {
            printf("\n%s : %s\n",data_server.data.multi_chat.username,data_server.data.multi_chat.message);
        }

        else if(data_server.type == GET_ONLINE_USERS)
        {
            if(printed == 0)                                                //to print the heading only once
            {
                printf("ONLINE USERS:\n");
                printed = 1;
                count = 0;
            }
            if(strcmp(data_server.data.userlist.username, "END") == 0)      //checking the last username is received
            {
                if(count == 0)
                {
                    // printf("No users Online\n");
                    no_users_flag = 1;
                }
                list_done_flag = 1;  
                printed = 0;                                       //signal main thread
            }
            else
            {
                printf("%s\n", data_server.data.userlist.username);         //print the online usernames
                count++;
            }
        }

        else if(data_server.type == JOIN_GROUP)
        {
            join_status = data_server.data.group.status;
        }

        else if(data_server.type == LEAVE_GROUP)
        {
            leave_status = data_server.data.group.status;
        }
    }
    return NULL;
}