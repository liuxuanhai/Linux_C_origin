#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#define MAXLINE 1024*10

char origin_ip[] = "10.0.0.149";
int origin_port = 2525;

#define LOCAL_PORT 8600
#define LOCAL_IP "10.0.0.12"

#define REMOTE_IP "10.0.0.74"
#define REMOTE_PORT 8686

void print_inner_pkt(char *note, char *inner_pkt, int len);
int make_message(char *mesg, char sendbuf[], int send_buf_len, uint32_t src_ip, u_int16_t src_port, uint32_t des_ip, u_int16_t des_port);

int main()
{
	int server_sockfd;
	char send_message[MAXLINE];
	char recv_message[MAXLINE];
	struct sockaddr_in server_address;

	server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(LOCAL_IP);
	server_address.sin_port = htons(LOCAL_PORT);

	int i;
	int mesg_len;
	for (i = 4010; i < 4030; i++) /*i表示内存数据包的源端口号*/
	{
		origin_port = i;
		bzero(&send_message, MAXLINE);
		mesg_len = make_message("hello world!", send_message, MAXLINE, inet_addr(origin_ip), origin_port, inet_addr(REMOTE_IP), REMOTE_PORT);

		/* 发送数据包 */
		sendto(server_sockfd, send_message, mesg_len, 0, (struct sockaddr *) &server_address, sizeof(server_address)); //将包发出去
		print_inner_pkt("send_inner_pkt", send_message, mesg_len);

		/* 接收数据包 */
		bzero(&recv_message, sizeof(recv_message));
		int len = recvfrom(server_sockfd, recv_message, MAXLINE, 0, NULL, NULL);
		print_inner_pkt("recv_inner_ptk", recv_message, len);
	}
	close(server_sockfd);
	return 0;
}

/*
 * 打印内层数据包的信息
 */
void print_inner_pkt(char *note, char *inner_pkt, int len)
{
	printf("%s: ", note);
	struct iphdr *ip;
	ip = (struct iphdr *) inner_pkt;
	struct udphdr *udp;
	udp = (struct udphdr *) (inner_pkt + sizeof(struct iphdr));
	struct in_addr ad;
	ad.s_addr = ip->saddr;
	printf("%s:%d->", inet_ntoa(ad), ntohs(udp->source));

	ad.s_addr = ip->daddr;
	printf("%s:%d,  ", inet_ntoa(ad), ntohs(udp->dest));
	printf(" datalen:%d, data:%s\n",  len-28, (inner_pkt+28));
}
