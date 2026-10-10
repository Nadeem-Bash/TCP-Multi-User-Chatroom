#include"chatroom.h"

int main()
{
    //making the server socket
    struct sockaddr_in srvr_addr;                              
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);

    //updating the ip adress and portnumber to the structure
    srvr_addr.sin_port = htons(PORT_NO);
    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = INADDR_ANY;

    //checking if binding happens properly
    if(bind(sock_fd,(struct sockaddr*)&srvr_addr,sizeof(srvr_addr)) < 0)    
    {
        perror("bind");
        exit(1);
    }

    listen(sock_fd,5);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    //initializing the mutex because multiple threads will be accessing the username database and the login database.
    pthread_mutex_init(&lock,NULL);

    while(1)
    {
        int new_sockfd = -1;    
        pthread_t tid;
        new_sockfd = accept(sock_fd,(struct sockaddr*)&client_addr,&addr_len);
        if(new_sockfd >= 0)
        {
            printf("connect suxwjen\n");
        }
        int *ptr = malloc(sizeof(int));
        *ptr = new_sockfd;
        pthread_create(&tid,NULL,handle_client,ptr);
        pthread_detach(tid);
    }

}