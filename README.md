# library
这是一个ping的simple而已，非常简单，有利于理解icmp，ip和原始套接字编程
实现结构为：            
 reference--- unix网络编程

      main--------------sig处理函数-----------sig_alrm
      |                                         |
      |                                         |
    readloop                                    send_v4/send_v6
  /       \
recvfrom   pro_v4/pro_v4 
