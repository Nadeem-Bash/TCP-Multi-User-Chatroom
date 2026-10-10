#include"chatroom.h"

Online_users* head = NULL;
pthread_mutex_t lock;

void* handle_client(void* ptr) //function to handle cleints
{
    int new_sockfd = *(int*)ptr;
    free(ptr);

    packet data_client;
    packet data_server;

    while(1)
    {
        // printf("checkingg");
        int n = recv(new_sockfd,&data_client,sizeof(data_client),0);

        if (n <= 0) //if disconnected or closed
        {
            pthread_mutex_lock(&lock);
            remove_from_list(&head,new_sockfd);  
            pthread_mutex_unlock(&lock);
            close(new_sockfd);             // release socket
            break;                         // exit loop (end thread)
        }
        // printf("n is %d",n);

        if(data_client.type == REGISTER)
        {
            //locking the critical section so that fultiple access of database.txt is avoided
            pthread_mutex_lock(&lock);

            FILE* fptr_r = fopen("Database.txt","r");
            int status= 0;
            char line[100];
            while(fgets(line,sizeof(line),fptr_r) != NULL)         //to get line by line
            {
                char* token1 = strtok(line," ");
                // char* token2 = strtok(NULL,"\n"); //no need we are only comparing username
                if(strcmp(data_client.data.reg.username,token1)==0)
                {
                    status = 1;
                }
            }
            fclose(fptr_r);
            if(status == 0)
            {
                FILE* fptr_a = fopen("Database.txt","a");
                fprintf(fptr_a,"%s %s\n",data_client.data.reg.username,data_client.data.reg.password);
                fclose(fptr_a);
            }
    
            data_server.type = REGISTER;
            data_server.data.reg.status = status;
            printf("before send\n");
            send(new_sockfd,&data_server,sizeof(data_server),0);
            printf("aftersend\n");

            //unlocking the mutex
            pthread_mutex_unlock(&lock);
        }

        else if(data_client.type == LOGIN)
        {
            //locking the critical section so that fultiple access of database.txt is avoided
            pthread_mutex_lock(&lock);
            int status = 0;

            if(check_list(head,data_client.data.log.username))
            {
                status = 2;
            }
            else
            {
                FILE* fptr = fopen("Database.txt","r");
                char line[100];
                while(fgets(line,sizeof(line),fptr) != NULL)         //to get line by line
                {
                    char* token1 = strtok(line," ");
                    char* token2 = strtok(NULL,"\n");
                    if(token1 && token2 && strcmp(data_client.data.log.username,token1) == 0 && strcmp(data_client.data.log.password,token2) ==0)
                    {
                        status = 1;
                        add_to_list(&head,data_client.data.log.username,new_sockfd);            //if username and password match then store username and sock fd in the online users list.
                        break;
                    }
                }
                fclose(fptr);
            }
            data_server.type = LOGIN;
            data_server.data.log.status = status;
            send(new_sockfd,&data_server,sizeof(data_server),0);
            //unlock mutex
            pthread_mutex_unlock(&lock);

        }

        else if(data_client.type == SINGLE_CHAT)
        {
            pthread_mutex_lock(&lock);

            int send_sockfd = -1;

            Online_users* temp = head;
            packet data_server;

            data_server.type = SINGLE_CHAT;
            strcpy(data_server.data.single_chat.message,data_client.data.single_chat.message);

            while(temp != NULL)
            {
                if(strcmp(data_client.data.single_chat.username,temp->username) == 0)
                {
                    send_sockfd = temp->sock_fd;
                }
                if(temp->sock_fd == new_sockfd)
                {
                    strcpy(data_server.data.single_chat.username,temp->username);
                }
                temp = temp->next;
            }
            if(send_sockfd != -1)
            {
                send(send_sockfd,&data_server,sizeof(data_server),0);
            }
            pthread_mutex_unlock(&lock);
        }

        else if(data_client.type == MULTI_CHAT)
        {
            pthread_mutex_lock(&lock);
            packet data_server;
            data_server.type = MULTI_CHAT;
            strcpy(data_server.data.multi_chat.message,data_client.data.multi_chat.message);
            Online_users* temp = head;
            while(temp!=NULL)                           //traverse the list to get senders username
            {
                if(temp->sock_fd == new_sockfd)
                {
                    strcpy(data_server.data.multi_chat.username,temp->username);
                }
                temp = temp->next;
            }
            temp = head;
            while(temp != NULL)                         //traverse the list to send message to every cleint in the multichat
            {
                if(temp->in_group == 1)
                {
                    send(temp->sock_fd,&data_server,sizeof(data_server),0);
                }
                temp = temp->next;
            }
            pthread_mutex_unlock(&lock);
        }

        else if(data_client.type == LOGOUT)
        {
            pthread_mutex_lock(&lock);
            remove_from_list(&head,new_sockfd);
            pthread_mutex_unlock(&lock);
        }

        else if(data_client.type == JOIN_GROUP)
        {
            pthread_mutex_lock(&lock);

            packet data_server;
            data_server.type = JOIN_GROUP;
            data_server.data.group.status = 0;

            int in_group_flag = 0;

            Online_users * temp = head;             //To traverse the online users and enable multichat

            while(temp != NULL)
            {
                if(temp->sock_fd == new_sockfd)
                {
                    temp->in_group = 1;
                    in_group_flag = 1;
                }
                temp = temp->next;
            }
            if(in_group_flag)
            {
                data_server.data.group.status = 1;
            }
            send(new_sockfd,&data_server,sizeof(data_server),0);
            pthread_mutex_unlock(&lock);
        }  

        else if(data_client.type == LEAVE_GROUP)
        {
            pthread_mutex_lock(&lock);

            packet data_server;
            data_server.type = LEAVE_GROUP;
            data_server.data.group.status = 0;

            int in_group_flag = 0;

            Online_users * temp = head;             //To traverse the online users and enable multichat

            while(temp != NULL)
            {
                if(temp->sock_fd == new_sockfd)
                {
                    temp->in_group = 0;
                    in_group_flag = 1;
                }
                temp = temp->next;
            }
            if(in_group_flag)
            {
                data_server.data.group.status = 1;
            }
            send(new_sockfd,&data_server,sizeof(data_server),0);
            pthread_mutex_unlock(&lock);
        }  

            
        else if(data_client.type == GET_ONLINE_USERS)
        {
            pthread_mutex_lock(&lock);

            Online_users* temp = head;
            packet data_server;
            data_server.type = GET_ONLINE_USERS; 
            while(temp != NULL)
            {
                strcpy(data_server.data.userlist.username,temp->username);
                send(new_sockfd,&data_server,sizeof(data_server),0);
                temp = temp->next;
            }

            strcpy(data_server.data.userlist.username,"END");
            send(new_sockfd,&data_server,sizeof(data_server),0);

            pthread_mutex_unlock(&lock);
        }
    }
    
}


//function to check if user is already logged in
int check_list(Online_users* head,char username[])
{
    if(head == NULL)
    {
        return 0;
    }
    while(head != NULL)
    {
        if(strcmp(head->username,username) == 0)
        {
            return 1;
        }
            head = head->next;
    }
    return 0;
}


//function to add the client username and sockfd to the online user list
void add_to_list(Online_users** head,char username[],int sockfd)
{
    Online_users* new = malloc(sizeof(Online_users));
    new->sock_fd = sockfd;
    strcpy(new->username,username);
    new->next = NULL;
    new->in_group = 0;

    if(*head == NULL)
    {
        *head = new;
        return;
    }
    Online_users* temp = *head;
    while(temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new;
}


//function to remove the client username and sock fd from the online user list
void remove_from_list(Online_users** head, int new_sockfd)
{
    Online_users* temp = *head;
    Online_users* prev = NULL;

    while (temp != NULL)
    {
        if (temp->sock_fd == new_sockfd)
        {
            // 🔹 Case 1: deleting head
            if (prev == NULL)
            {
                *head = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }

            free(temp);
            return;
        }

        prev = temp;
        temp = temp->next;
    }
}