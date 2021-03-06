#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용
#include <arpa/inet.h>

#define MSG_SIZE 1000000

void error_print(char *error_msg); // 에러문 출력 및 서버 종료
void request_print(char *response_msg, char *request_msg); // response message를 만드는 함수 (프로젝트 Part1)

int main(int argc, char *argv[]){
    if (argc < 2) { // 명령 인수 개수가 부족하면
        fprintf(stderr, "Argument is out of quntity.\n"); // 에러문 출력후 
        exit(0); // 클라이언트 종료
    }

    int port_number = atoi(argv[1]); // 포트번호를 정수로 변환 
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // client와 똑같은 socket 생성
    if( server_socket_fd < 0 ){ // 소켓을 만들기 실패하면 종료
        error_print("Fail to open socket.\n"); 
    }

    struct sockaddr_in server_address, client_addr; // 소켓 주소를 담는 구조체, 서버와 클라이언트 2개 필요하다

    memset (&server_address, 0, sizeof(server_address)); // 구조체를 0으로 초기화
    server_address.sin_family = AF_INET; // IPv4로 설정
    server_address.sin_port = htons(port_number); // htons함수를 통해 포트번호를 네트워크 바이트 순서로 변환해준다.
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // p주소를 자동으로 대입해주는 함수 사용 long형이라 htonl사용

    //소켓에 포트번호 배정
    if ( bind( server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address) ) < 0) {
        error_print("error to bind");
    }
    /* 
        첫번쨰 인자 : 서버 소켓 디스크립터
        두번째 인자 : 서버의 주소 정보
        세번째 인자 : 서버으 주소 정보의 크기
        성공시  0 실패시 -1 반환
    */

    //클라이언트 요청 대기
    printf("server is waiting......\n");
    listen(server_socket_fd, 7); // 소켓의 연결 대기열 만들어서 대기상태. 연결 요청 대기 함수.
    // backlog는 요청 대기 큐의 크기 
    
    int socket_fd;
    //요청이 들어오면 받아 줘야 한다.
    socklen_t req_client = sizeof(client_addr); // 클라이언트 주소 정보 길이를 미리 지정해준다.
    
    char *request_msg; // 클라이언트에서 받은 메시지
    char *response_msg; // 클라이언트로 보낼 메시지 
    request_msg = malloc( sizeof(char)*MSG_SIZE ); // 공간 할당.
    response_msg = malloc( sizeof(char)*MSG_SIZE ); // 공간 할당.
    
    while(1){
        memset(response_msg, 0 ,MSG_SIZE); // 클라이언트로 보낼 메시지 초기화
        memset(request_msg, 0 ,MSG_SIZE); // 클라이언트에서 받은 메시지 초기화
        if( (socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client)) < 0){ // 요청 대기 큐에서 연결을 갖고와 연결된 소켓을 만든다.
            error_print("Fail to accpet");
        }
            /*
                첫번째 인자 : 서버 소켓 디스크립터
                두번쨰 인자 : 클라이언트 주소 정보
                세번쨰인자 : 클라이언트 주소 정보 크기
            */
        printf("connection is successful : address: %s, port = % d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); // 연결된 클라이언트의 주소와 포트 번호
        
        if( read(socket_fd, request_msg, MSG_SIZE) < 0 ){ // 클라이언트로 부터 메시지를 읽어 request_msg에 저장한다.
            error_print("Fail to read");
        }
        request_print(response_msg, request_msg);  // response message를 만드는 함수 (프로젝트 Part1)
        if ( write(socket_fd, response_msg, strlen(response_msg))  < 0 ) {  // response_msg를 클라이언트에 그 길이 만큼 보낸다.
            error_print("Fail to writing to socket.");
        }
        
        close(socket_fd); // 연결된 소켓을 닫는다.
    }

    free(response_msg); // 메모리 해제
    free(request_msg); // 메모리 해제
    close(server_socket_fd); //서버측 소켓을 닫아준다.
    return 0; // 프로그램 종료
} 


void request_print(char *response_msg, char *request_msg){ // response message를 만드는 함수 (프로젝트 Part1)
    char *p = strtok(request_msg, "\n"); // request message를 html형태로 변환시키기 위해 줄바꿈 문자를 <br> 태그로 변경해 주었다.
    char *req = malloc(sizeof(char)*MSG_SIZE);
    memset(req,0,MSG_SIZE);
    while( p!= NULL){
        strcat(req,p);
        strcat(req,"<br>"); // 줄 바꿈 구현
        p = strtok(NULL, "\n");
    }
    int content_length = 7 + strlen(req); // content-length field를 위해 값을 구하였다.
    sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %d\r\n\r\n<h>%s</h>",content_length,req); // response_msg를 만들고 response 메시지의 길이를 구했다.
    free(req); // 메모리 해제
}

void error_print(char *error_msg){ // 에러문을 출력하고 종료하는 함수
    perror(error_msg);
    exit(1);
}