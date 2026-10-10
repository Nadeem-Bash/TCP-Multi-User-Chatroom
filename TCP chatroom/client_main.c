#include"chatroom.h"

int login_flag = 0;

int main()
{
    struct sockaddr_in client_addr;                              
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT_NO);
    inet_pton(AF_INET,"127.0.0.1",&client_addr.sin_addr);
    printf("Trying to connect...\n");
    if(connect(sock_fd,(struct sockaddr*)&client_addr,sizeof(client_addr)) < 0)
    {
        perror("connect");
    }
    else
    {
        printf("Connected to server\n");
    }

    while(1)
    {
        int choice;
        printf("1.Register\n2.Login\n3.Single Chat\n4.Multi Chat\n5.Logout & Exit\n");
        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                register_client(sock_fd);
                break;
            case 2: // login
                login_client(sock_fd);
                break;
            case 3: // single chat
                if(!login_flag)
                {
                    printf("User not logged in\n");
                    break;
                }
                single_chat(sock_fd);
                break;
            case 4: // multichat
                if(!login_flag)
                {
                    printf("User not logged in\n");
                    break;
                }
                multi_chat(sock_fd);
                break;
            case 5:
                logout(sock_fd);
                close(sock_fd);
                exit(0);
        }
    }
}