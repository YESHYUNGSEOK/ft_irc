#include "../includes/Server.hpp"

Server::Server(int port, std::string password) : port(port), password(password) {
    std::cout << CYAN << SERVER_PREFIX << "Server created with port: " << port << ", password: " << password << RESET << std::endl;
    serv_addr_init();
	socket_init();
	fd_set_init();
}

Server::~Server() {
	close(server_sockfd); // 서버 소켓 닫기

    std::cout << CYAN << SERVER_PREFIX << "Server is closed" << RESET << std::endl;
}

int Server::get_sockfd() {
	return server_sockfd;
}

int Server::get_port() {
    return port;
}

std::string Server::get_password() {
    return password;
}

sockaddr_in Server::get_serv_addr() {
    return serv_addr;
}

void Server::serv_addr_init() {
    std::memset(&serv_addr, 0, sizeof(serv_addr)); // 구조체를 0으로 초기화
    serv_addr.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스의 주소를 사용
    serv_addr.sin_port = htons(port); // 호스트 바이트 순서를 네트워크 바이트 순서로 변환

	std::cout << CYAN << SERVER_PREFIX << "Server address initialized" << RESET << std::endl;
}

void Server::socket_init() {
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성

	if (server_sockfd == ERROR) { // 소켓 생성 실패 시
		std::cerr << RED << SERVER_PREFIX << "Error opening socket" << RESET << std::endl;
		exit(EXIT_FAILURE); // 오류 메시지 출력 후 프로그램 종료
	}

	if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == ERROR) { // 소켓에 주소 할당
		std::cerr << RED << SERVER_PREFIX << "Error on binding" << RESET << std::endl;
		exit(EXIT_FAILURE); // 바인딩 실패 시 오류 메시지 출력 후 종료
	}

    if (fcntl(server_sockfd, F_SETFL, O_NONBLOCK) == ERROR) { // 소켓을 논블로킹으로 설정
        std::cerr << RED << SERVER_PREFIX << "Error on setting non-blocking" << RESET << std::endl;
        exit(EXIT_FAILURE); // 논블로킹 설정 실패 시 오류 메시지 출력 후 종료
    }

	if (listen(server_sockfd, SOMAXCONN) == ERROR) { // 연결 요청 대기열 생성
		std::cerr << RED << SERVER_PREFIX << "Error on listening" << RESET << std::endl;
		exit(EXIT_FAILURE); // 리스닝 실패 시 오류 메시지 출력 후 종료
	}

	std::cout << CYAN << SERVER_PREFIX << "Server socket initialized" << RESET << std::endl;
}

void Server::fd_set_init() {
	FD_ZERO(&current_sockets); // current_sockets 세트를 0으로 초기화
	FD_SET(server_sockfd, &current_sockets); // 소켓 파일 디스크립터를 current_sockets 세트에 추가

	std::cout << CYAN << SERVER_PREFIX << "File descriptor set initialized" << RESET << std::endl;
}

void Server::run() {
	std::cout << CYAN << SERVER_PREFIX << "Server is running..." << RESET << std::endl;

	int max_fd = server_sockfd; // 최대 파일 디스크립터 번호 초기화

 	while (true) { // 무한 루프
        ready_sockets = current_sockets; // ready_sockets를 current_sockets로 복사

        if (select(max_fd + 1, &ready_sockets, nullptr, nullptr, nullptr) == ERROR) { // 준비된 소켓 검사. 변화 없으면 블로킹
            std::cerr << CYAN << SERVER_PREFIX << "Error on select" << RESET << std::endl;
            exit(EXIT_FAILURE); // select 실패 시 오류 메시지 출력 후 종료
        }

        for (int sockfd = 0; sockfd <= max_fd; sockfd++) { // 준비된 모든 파일 디스크립터 확인
            if (FD_ISSET(sockfd, &ready_sockets)) { // 파일 디스크립터가 준비되었는지 확인
                if (sockfd == server_sockfd) { // 새로운 연결 요청이면
                    add_client(server_sockfd, &max_fd); // 클라이언트 추가
                } else { // 기존 연결에서 데이터가 도착한 경우
                    char buffer[BUF_SIZE]; // 데이터 수신을 위한 버퍼
                    int nbytes = read(sockfd, buffer, sizeof(buffer)); // 데이터 읽기
                    if (nbytes <= 0) { // 읽기 실패 또는 연결 종료
                        remove_client(sockfd); // 클라이언트 제거
                    } else if (client_manager.get_nickname(sockfd) == "Anonymous") { // 닉네임이 설정되지 않은 경우
                        set_client_nickname(sockfd, std::string(buffer, nbytes - 1)); // 닉네임 설정, -1은 개행 지우기 위함
                    } else {
						for (int _sockfd = 0; _sockfd <= max_fd; _sockfd++) {
							if (_sockfd != sockfd && _sockfd != server_sockfd && FD_ISSET(_sockfd, &current_sockets)) { 
                        		if (write(_sockfd, buffer, nbytes) == ERROR) { // 받은 데이터를 다른 소켓에 전송
									std::cerr << CYAN << SERVER_PREFIX <<  "Error on write" << RESET << std::endl;
								}	
							}
						}
                    }
                }
            }
        }
    }
}

void Server::add_client(int sockfd, int *max_fd) {
	std::cout << CYAN << SERVER_PREFIX << "New connection request!" << RESET << std::endl;

    struct sockaddr_in cli_addr; // 클라이언트 주소 정보 구조체
    socklen_t clilen = sizeof(cli_addr); // 주소 정보의 크기
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // 연결 수락
    if (newsockfd == ERROR) { // 연결 수락 실패 시
        std::cerr << CYAN << SERVER_PREFIX <<  "Error on accept" << RESET << std::endl;
        return; // 다음 반복으로 넘어감
    }

    FD_SET(newsockfd, &current_sockets); // 새 소켓을 파일 디스크립터 세트에 추가
    *max_fd = std::max(*max_fd, newsockfd); // 최대 파일 디스크립터 번호 업데이트

    client_manager.add_client(newsockfd, cli_addr); // 클라이언트 매니저에 클라이언트 추가
    
    write(newsockfd, "Welcome! Please enter your nickname ;)\n", 39); // 닉네임 설정 요청
}

void Server::remove_client(int sockfd) {
    std::string nickname = client_manager.get_nickname(sockfd);

    close(sockfd); // 소켓 닫기
    FD_CLR(sockfd, &current_sockets); // 세트에서 소켓 제거

    client_manager.remove_client(sockfd); // 클라이언트 매니저에서 클라이언트 제거

    std::cout << CYAN << SERVER_PREFIX << nickname << " quit the server" << RESET << std::endl;
}

void Server::set_client_nickname(int sockfd, std::string nickname) {
    client_manager.set_nickname(sockfd, nickname); // 클라이언트 매니저에서 닉네임 설정

    std::cout << CYAN << SERVER_PREFIX << nickname << " entered the server" << RESET << std::endl;
}