#include <winsock2.h>
#include <stdio.h>

//C:\Windows\System32\drivers\etc\hosts -> 127.0.0.1  [ALIAS]
#pragma comment(lib, "ws2_32.lib")

typedef struct{
    char* command;
    char* result;
}cr;
typedef struct{
    cr* maparray;
    int length;
}mapa;
mapa m;

// Coisas que modificam mapeamento comando -> link

void insertCr(char* str, int start, int end){
    int l = end-start;
    char* s = malloc(l+1);
    s = memcpy(s, str+start, l);
    s[l] = '\0';

    int sindex = -1;
    for (int i = 0; i < l; i++){
        if (s[i] == ' '){
            sindex = i;
            break;
        }
    }
    if (sindex <= 0){
        return;
    }
    printf("%d", sindex);

    printf("(%s, %d, %d)\n", s, start, end);
}

void updateMapa(){
    FILE* f = fopen("mapa.txt", "rb");
    if (!f){
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* content = malloc(size+1);
    if (content){
        size = fread(content, 1, size, f);
        content[size] = '\0';
    }
    fclose(f);

    int startindex = 0;
    for(int i = 0; i< size; i++){
        if (content[i] == '\n'){
            if (content[i-1] == '\r') insertCr(content, startindex, i-1);
            else insertCr(content, startindex, i);
            startindex = i+1;
        }
    }
    insertCr(content, startindex, size);

}

////

// Leitura de comando e tals

char* getCommand(char a[]){
    //GET /COISA HTTP/1.1 
    int length = 0;
    for(int i = 5; i < 1024; i++){
        if (a[i] == ' ') break;
        length++;
    }
    char* command = malloc(length+1);
    memcpy(command, a+5, length);
    command[length] = '\0';
    return command;
}

char* translate(char* command){
    return "https://google.com";
}

////

int main(){
    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(80);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(s, (struct sockaddr*)&address, sizeof(address));
    listen(s, 1);

    printf("RUN");
    while (1){
        SOCKET c = accept(s, 0, 0);
        char buf[1024];
        int n = recv(c, buf, sizeof(buf)-1, 0);
        system("cls");
        updateMapa();
        

        
        if (n > 0) {
            char* command;
            command = getCommand(buf);

            if (strncmp("c/", command, 2) == 0){
                printf("sim");
            }
            else{
                char* loc = translate(command);
                char rbuf[512];
                int len = 0;
                len+=sprintf(rbuf, "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n", loc);

                const char *respf = rbuf;
                send(c, respf, len, 0);
            }
           
        }
        closesocket(c);
    }

    WSACleanup();
    return 0;
}
