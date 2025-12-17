#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//C:\Windows\System32\drivers\etc\hosts -> 127.0.0.1  [ALIAS]
#pragma comment(lib, "ws2_32.lib")

typedef struct{
    char* command;
    char* result;
}cr;//commandresult calc is shjort for calculatro
typedef struct{
    cr* maparray;
    size_t length;
}mapa;
mapa m;

// Coisas que modificam mapeamento comando -> link

void saveMapa(){
    FILE* f = fopen("mapa.txt", "wb");
    if (!f){
        return;
    }

    for (int i = 0 ; i < m.length ; i++){
        fprintf(f, "%s %s\n",m.maparray[i].command, m.maparray[i].result);
    }
    fclose(f);
}

void insertCr(char* str, int start, int end){ //poe 'comm resp' no mapa 
    int l = end-start;

    int sindex = -1;
    for (int i = start; i < end; i++){
        if (str[i] == ' '){
            sindex = i-start;
            break;
        }
    }
    if (sindex <= 0){
        return;
    }
    sindex += start;
    char* command = malloc(sindex-start + 1);
    char* result = malloc(end-sindex + 1); 
    if (!command || !result){
        free(command);
        free(result);
        return;
    }

    memcpy(command, str+start, sindex-start);
    command[sindex-start] = '\0';

    memcpy(result, str+sindex+1, end-sindex);
    result[end-sindex] = '\0';

    
    for (int i = 0; i < m.length; i++){
        if (strcmp(m.maparray[i].command, command) == 0){
            free(m.maparray[i].result);
            m.maparray[i].result = result;
            free(command);
            return;
        }
    }

    m.length++;
    cr* temp = realloc(m.maparray, m.length*sizeof(cr));
    if (!temp){
        free(command);
        free(result);
        return;
    }
    m.maparray = temp;

    m.maparray[m.length-1].command = command;
    m.maparray[m.length-1].result = result;
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
    else{
        fclose(f);
        return;
    }
    fclose(f);

    int startindex = 0;
    for(int i = 0; i< size; i++){
        if (content[i] == '\n'){
            if (content[i-1] == '\r') insertCr(content, startindex, i-2);
            else insertCr(content, startindex, i-1);
            startindex = i+1;
        }
    }
    insertCr(content, startindex, size);
    free(content);
}

char* newConfigCommand(char* command){
    if (!command) return NULL;

    for(int i = 2; i < strlen(command); i++){
        if(command[i] == '/'){
            command[i] = ' ';
            insertCr(command, 2, strlen(command));
            saveMapa();
            char* crcommand = malloc(i-1);
            if (!crcommand){
                return NULL;
            }

            crcommand = memcpy(crcommand, command+2, i-2);
            crcommand[i-2] = '\0';
            return crcommand;
        }
    }
    return NULL;
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
    if (!command){
        return NULL;
    }
    memcpy(command, a+5, length);
    command[length] = '\0';
    return command;
}

char* translate(char* command){
    if (!command){
        return "https://erro";
    }

    for (int i = 0; i < m.length; i++){
        if (strcmp(command, m.maparray[i].command) == 0){
            return m.maparray[i].result;
        }
    }
    return "https://youtube.com";
}

////

int main(){
    m.length = 0;
    m.maparray = NULL;

    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(80);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(s, (struct sockaddr*)&address, sizeof(address));
    listen(s, 1);

    printf("run");
    updateMapa();
    while (1){
        SOCKET c = accept(s, 0, 0);
        char buf[1024];
        int n = recv(c, buf, sizeof(buf)-1, 0);
        buf[n] = '\0';

    
        if (n > 0) {
            char* command;
            command = getCommand(buf);
            char* loc;
            if (strncmp("c/", command, 2) == 0 && strlen(command)>2){
                loc = translate(newConfigCommand(command));
            }
            else if(strncmp("sair/", command, 2) == 0){
                return 0;
            }
            else{
                loc = translate(command);
            }
            free(command);
            char rbuf[512];
            int len = 0;
            len+=sprintf(rbuf, "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n", loc);

            const char *respf = rbuf;
            send(c, respf, len, 0);
           
        }
        closesocket(c);
    }

    WSACleanup();
    return 0;
}