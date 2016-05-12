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

struct udp_front
{
        uint32_t srcip;
        uint32_t desip;
        u_int8_t zero;
        u_int8_t protocol;
        u_int16_t len;

};

u_int16_t in_chksum(u_int16_t *addr, int len);
u_int16_t udp_check(char *sendbuf, int len, const struct udp_front front);
int make_message(char *mesg, char sendbuf[], int send_buf_len, uint32_t src_ip, u_int16_t src_port, uint32_t des_ip, u_int16_t des_port);


/*拼接IP数据报*/
int make_message(char *mesg, char sendbuf[], int send_buf_len, uint32_t src_ip, u_int16_t src_port, uint32_t des_ip, u_int16_t des_port)
{
        char message[MAXLINE];
        bzero(message, sizeof(message));
        strcpy(message,mesg);
        //printf("message len:%d\n",strlen(message));
        struct iphdr *ip;
        ip = (struct iphdr *)sendbuf;
        ip->ihl = sizeof(struct iphdr) >> 2; //首部长度
        ip->version = 4;   //ip协议版本
        ip->tos = 0;   //服务类型字段
        ip->tot_len = 0;   //总长度
        ip->id = 1000;   //
        ip->frag_off = 0;
        ip->ttl = 128;
        ip->protocol = IPPROTO_UDP;
        ip->check = 0;  //内核会算相应的效验和
        ip->saddr = src_ip;
        ip->daddr = des_ip;

        struct udp_front front;
        front.srcip = src_ip;
        front.desip = des_ip;
        front.len = htons(8+strlen(message));
        front.protocol = 17;
        front.zero = 0;

        struct udphdr *udp;
        udp = (struct udphdr *)(sendbuf + sizeof(struct iphdr));
        udp->source = htons(src_port);  //源端口
        udp->dest = htons(des_port);    //目的端口
        udp->check = 0;   //效验和，效验整个udp数据报
        bcopy(message, (sendbuf+20+8), strlen(message));
        udp->len = htons(8+strlen(message)); //udp数据报总长度

        udp->check = udp_check((sendbuf+20), 8+strlen(message), front);

        ip->tot_len = (20 + 8 + strlen(message));   //总长度
        printf("ip->tot_len:%d\n",ip->tot_len);
        ip->check = in_chksum((unsigned short *)sendbuf, 20);

        return (ip->tot_len);

}

/*计算udp效验和*/
unsigned short udp_check(char *sendbuf, int len, const struct udp_front front)
{
        char str[MAXLINE];
        bzero(&str, MAXLINE);
        bcopy(&front, str, sizeof(front));
        bcopy(sendbuf, str+sizeof(front), len);
        struct udp_front *ptr;
        ptr = (struct udp_front *)str;
        char *s;
        s = (str+20);
        return in_chksum((unsigned short *)str, sizeof(front)+len);

}

/*效验和算法*/
uint16_t in_chksum(uint16_t *addr, int len)
{
        int nleft = len;
        uint32_t sum = 0;
        uint16_t *w = addr;
        uint16_t answer = 0;
        /*把ICMP报头二进制数据以2字节为单位累加起来*/
        while (nleft > 1)
    {
                sum += *w++;
                nleft -= 2;

    }
        if (nleft == 1)
    {
                *(unsigned char *)(&answer) = *(unsigned char *)w;
                sum += answer;

    }
        sum = (sum>>16) + (sum&0xffff);
        sum += (sum>>16);
        answer = ~sum;
        return answer;

}
