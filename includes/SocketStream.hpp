#pragma once
#ifndef SOCKETSTREAM_HPP
#define SOCKETSTREAM_HPP

#include <exception>
#include <string>
#include <vector>

#include "ft_irc.hpp"

class SocketStream {
 private:
  static const int BUFFER_SIZE = 1024;
  static const int LINE_SIZE_MAX = 512;

  struct sockaddr_in _addr;
  socklen_t _addr_len;
  const int _fd;
  std::string _read_buffer;
  std::string _write_buffer;
  char *_raw_buffer;

  // 사용 X
  SocketStream();  // 사용하지 않는 생성자
  SocketStream(__unused const SocketStream &src);  // 사용하지 않는 생성자
  SocketStream &operator=(
      __unused const SocketStream &src);  // 사용하지 않는 연산자

 public:
  SocketStream(const int server_fd);
  ~SocketStream();

  void recv();
  void send();

  int get_fd() const;

  SocketStream &operator<<(const std::string &data);
  SocketStream &operator>>(std::string &data);

  class ConnectionClosedException : public std::exception {
   public:
    ConnectionClosedException();
    ConnectionClosedException(__unused const ConnectionClosedException &src);
    ConnectionClosedException &operator=(
        __unused const ConnectionClosedException &src);
    virtual ~ConnectionClosedException() throw();
    virtual const char *what() const throw();
  };
  class MessageTooLongException : public std::exception {
   public:
    MessageTooLongException();
    MessageTooLongException(__unused const MessageTooLongException &src);
    MessageTooLongException &operator=(
        __unused const MessageTooLongException &src);
    virtual ~MessageTooLongException() throw();
    virtual const char *what() const throw();
  };
};

#endif  // SOCKETSTREAM_HPP
