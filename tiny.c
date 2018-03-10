/*
    tiny.c - A simple,iterative HTTP/1.0 Web server that uses the GET
    method to serve static and dynamic content.
*/
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri,char *filename,char *cgiargs);
void serve_static(int fd,char *filename,int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd,char *filename,char *cgiargs);
void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg);


void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char filename[MAXLINE],cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio,fd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version);
    if(strcasecmp(method,"GET"))
    {
        clienterror(fd,method,"501","Not Implemented","Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);
∂
    /* Parse URI from GET request*/
    is_static=parse_uri(uri,filename,cgiargs);
    //利用stat函数，通过filename文件名来获取文件的相关信息并存储到sbuf中
    if(stat(filename,&sbuf)<0)
    {
        clienterror(fd,filename,"404","Not found","Tiny couldn't find this file");
        return;
    }

    /* Serve static content */
    if(is_static)
    {
        //判断是否为可读的静态文件
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(fd,filename,"403","Forbidden","Tiny couldn't read the file");
            return;
        }
        serve_static(fd,filename,sbuf.st_size);
    }
    /* Serve dynamic content */
    else
    {
        //判断是否为可执行的动态文件
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd,filename,"403","Forbidden","Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd,filename,cgiargs);
    }
}

void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg)
{
    char buf[MAXLINE],body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
    sprintf(body,"%s%s: %s\r\n",body,errnum,shortmsg);
    sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
    sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body);

    /* Print the HTTP response */
    sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,shortmsg);
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-type: text/html\r\n");
    Rio_writen(fd,buf,strlen(buf));
    sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body));
    Rio_writen(fd,buf,strlen(buf));
    Rio_writen(fd,body,strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    //读取请求，由于只是说明发送的内容，所以可以忽略请求报头
    Rio_readlineb(rp,buf,MAXLINE);
    while(strcmp(buf,"\r\n"))
    {
        Rio_readlineb(rp,buf,MAXLINE);
        printf("%s",buf);
    }
    return;
}

int parse_uri(char *uri,char *filename, char *cgiargs)
{
    char *ptr;

    /* Static content */
    //strstr:判断后面一个字符串是否是前面一个字符串的子串
    //即判断是动态内容，还是静态内容
    if(!strstr(uri,"cgi-bin"))
    {
        strcpy(cgiargs,"");
        strcpy(filename,".");
        strcat(filename,uri);
        if(uri[strlen[uri]-1]=='/')
            strcat(filename,"home.html");
        return 1;
    }
    /* Dynamic content */
    else
    {
        //读取参数,进行分割判断
        ptr=index(uri,'?');
        if(ptr){
            strcpy(cgiargs,ptr+1);
            *ptr='\0';
        }
        else
            strcpy(cgiargs,"");
        strcpy(filename,".");
        strcat(filename,uri);
        return 0;
    }
}

void serve_static(int fd,char *filename,int filesize)
{
    int srcfd;
    char *srcp,filetype[MAXLINE],buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename,filetype);
    sprintf(buf,"HTTP/1.0 200 OK\r\n");
    sprintf(buf,"%sServer: Tiny Web Server\r\n",buf);
    sprintf(buf,"%sContent-length: %d\r\n",buf,filesize);
    sprintf(buf,"%sContent-type: %s\r\n\r\n",buf,filetype);
    Rio_writen(fd,buf,strlen(buf));

    /* Send response body to client */
    srcfd=Open(filename,O_RDONLY,0);
    //利用mmap函数将打开的文件映射到虚拟存储器空间中，srcfd表示的是文件描述符
    srcp=Mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);
    Close(srcfd);
    Rio_writen(fd,srcp,filesize);
    //奖
    Munmap(srcp,filesize);
}

/*
* get_filetype-derive file type from file name
*/
void get_filetype(char *filename,char *filetype)
{
    if(strstr(filename,".html"))
        strcpy(filetype,"text/html");
    else if(strstr(filename,".gif"))
        strcpy(filetype,"image/gif");
    else if(strstr(fliename,".jpg"))
        strcpy(filename,".jpeg");
    else
        strcpy(filetype,"text/plain");
}


int main(int argc,char **argv)
{
    int listenfd,connfd,port,clientlen;
    struct sockaddr_in clientaddr;

    /* Check command line args */
    if(argc!=2)
    {
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }
    port=atoi(argv[1]);

    listenfd=Open_listenfd(port);
    while(1)
    {
        clientlen=sizeof(clientaddr);
        connfd=Accept(listenfd,(SA *)&clientaddr,&clientlen);
        doit(connfd);
        Close(connfd);
    }
}
