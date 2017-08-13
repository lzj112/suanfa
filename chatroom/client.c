/*************************************************************************
	> File Name: slient.c
	> Author: 
	> Mail: 
	> Created Time: 2017年08月10日 星期四 08时20分22秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<signal.h>

#define MAXLEN 4096     //聊天最长输入
#define PORT 4507
#define IP "127.0.0.1"
        

//用户信息
typedef struct a{ 
    int fd;
    int flag;           //标志是登录还是注册
    int login;          //标志各种功能 2为登录成功 其他看menu
    int power;          //权限 能否禁言 踢人
    char name[50];      //昵称
    char number[10];    //账号
    char passwd[20];    //密码
    char buf[MAXLEN];   //输入
}user;
 
 
//消息管理
typedef struct b{
    int flag;   //标志是哪一种消息请求
    int fd;
    char buf[MAXLEN];
    struct b *next;
}news;


int flag;  //判断客户端是否收到了服务器发来的消息

int log_in();
void set_in();
void denglu();
int menu();
void *request();
void baocun( user *guy );
void xiaoxi();

pthread_t tid;
int s_fd;
user guy;
news *head,*p1,*p2; 

int main()
{
    signal( SIGPIPE,SIG_IGN );
    struct sockaddr_in sin;
    int n,choose;
    
    memset( &guy,0,sizeof(guy) );   //两个结构体置0
    memset( &sin,0,sizeof(sin) );
    
    head = (news *)malloc( sizeof(news) );  //初始化头指针
    head->next = NULL;
    p1 = head;
    p2 = head;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    inet_aton(IP,&sin.sin_addr);

     if( (s_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 )   //建立套接字
    {
        printf( "socket error\n" );
        exit(0);
    }
    
  //  printf( "-->%d<--\n",s_fd ); //
    if( connect(s_fd,(struct sockaddr *)&sin,sizeof(sin)) )     //连接
    {
        printf( "connect error\n" );
        exit(0);
    }
    
    denglu();  //登录

    pthread_create( &tid,NULL,request,NULL );

    while( choose = menu() )    //主界面，各种功能
    {
        
        switch(choose)
        {
            case 0:
                break;
            case 1:
            {
                printf( "\n请输入添加账号：" );
                scanf( "%s",guy.buf );       //输入要添加的账号
                guy.login = 1;
                if( send( s_fd,(void *)&guy,sizeof(guy),0 ) < 0 )
                {
                    printf( "case 1 send error\n" );
                    exit(0);
                }
                break;
            }
            case 5:
            {
                xiaoxi();
            }
        }
    }
    return 0;
}

int menu()      //主界面
{
    int n;

        printf( "\n\t*****************************************\n" );
        printf( "\t\t1.增删好友\n" );
        printf( "\t\t2.打开好友列表\n" );
        printf( "\t\t3.发起私聊\n" );
        printf( "\t\t4.发起群聊\n" );
        printf( "\t\t5.消息管理\n" );
        printf ("\t\t6.摇一摇(附近的人)\n");
        printf( "\t\t按0退出聊天室\n" );
        printf( "\t*****************************************\n" );

        printf("menu===\n");
        scanf( "%d",&n );
       while( n>6 || n<0 )
       {
           printf( "错误选项，重新选择\n" );
           scanf( "%d",&n );
       }
       return n;
}

int log_in()   //登录
{
    int flag;
    guy.flag = 1;
    guy.login = 2;

    printf( "\t\n请输入账号：\n" );
    scanf( "%s",guy.number );
    printf( "\t\n请输入密码：\n" );
    scanf( "%s",guy.passwd );

    if( send(s_fd,(void *)&guy,sizeof(user),0) < 0 )         //发送
    {
        printf( "client send error\n" );
        exit(0);
    }

    if( recv(s_fd,(void *)&flag,sizeof(flag),0) < 0 )       //接收
    {
        printf( "client recv error\n" );
        exit(0);
    }

    if( flag == 1 )
    {
        printf( "\n登陆成功\n" );
        return 1;
    }
    else
        return 0;

}

void set_in()       //注册
{
    int flag;
    guy.flag = 2;
    guy.login = 2;

    char passwd[20];
    printf( "\t\n请输入账号：" );
    scanf( "%s",guy.number );
    printf( "\t\n请输入密码：" );
    scanf( "%s",guy.passwd );
    printf( "\t\n请确认密码：" );
    scanf( "%s",passwd );
    if( (strcmp(passwd,guy.passwd)) == 0 )
    {
        if( send(s_fd,(void *)&guy,sizeof(user),0) < 0)      //发送
        {
            printf( "set_in send error\n" );
            exit(0);
        }
        if( recv(s_fd,(void *)&flag,sizeof(flag),0) < 0 )    //接收
        {
            printf( "recv error\n" );
            exit(0);
        }
       
      //  printf( "--->%d<---\n",flag ); //  
        if( flag == 1 )
        {
            printf( "\n\t注册成功\n" );
            return ;
        }
        else
        {
            if( flag == 0 )
                printf( "\n账号已存在 注册失败\n" );
            memset( &guy,0,sizeof(guy) );
            return ;
        }
    }
    else
    {
        printf( "两次密码不同\n" );
        return ;
    }

}



void denglu()             //登录界面
{
    int choose;
    while(1)
    {
        int n=0;
        printf( "\n*********************\n\t1.登录\n\t2.注册\n\t3.按0退出\n*********************\n" );
        scanf( "%d",&choose );
        while( choose > 2 || choose < 0 )
        {
            printf( "错误选项，重新选择\n" );
            scanf( "%d",&choose );
        }
        if( choose == 1 )   //登录
        {
            n = log_in();
            if( n == 1 )
            {
                //guy.login = 2;
                break;
            }
            else
                memset( &guy,0,sizeof(guy) );
        }

        if( choose == 2 ) //注册
        {
            set_in();
        }
        if( choose == 0 )
        {
            break;
        }
    }
}

void *request()    //接收别的客户端发来的请求 添加好友 聊天什么的
{
    while(1)
    {
        if( recv( s_fd,(void *)&guy,sizeof(user),0 )  )
        {
            printf( "\n--->消息+1>" );
            if( guy.login == 1 )
            {
                //printf( "%s wangs to be friend with u~...<---",guy.buf );
                baocun( &guy );
            }
        }
    }

   /* if( guy.login == 1 )
    {
    

       /* printf( "%s,请按y或n\n",guy.buf );

        send( s_fd,(void *)&guy,sizeof(user),0 );
	    if( recv( s_fd,(void *)&guy,sizeof(user),0 ) < 0 )
        {
            printf( "case 1 recv error\n" );
            exit(0);
        }            
        pthread_create( &tid,NULL,request,NULL ); 
            
        printf( "%s\n",guy.buf ); 
        if( strcmp(guy.buf,"number error") == 0 )
        {
            printf( "\nnumber error\n" );
        }
        else if( strcmp(guy.buf,"yes") == 0 )
        {
            printf( "成功添加\n" );
            
            //保存好友到本地文件
        
        }
        else if( strcmp(guy.buf,"no") == 0 )
        {
            printf( "对方拒绝加你为好友\n" );
        }
        else 
        {
		    printf( "\n%s,请按y或n\n",guy.buf );
            memset( guy.buf,0,sizeof(guy.buf) );
            printf( "\n" );
            //strcpy( guy.buf,"你特么怎么回事" );
            scanf( "%s\n",guy.buf);
            printf("buf = %s\n",guy.buf);
            printf("------------\n");
            getchar();
            printf( "\n  asdasdasd    %s    asdasdasd \n",guy.buf );
            if( send( s_fd,(void *)&guy,sizeof(user),0 ) < 0)
            {
                printf( "我就没有发\n" );
            }
        }
    }*/
}

void baocun( user *guy )
{
    p1 = (news *)malloc( sizeof(news) );
    p2->next = p1;
    p1->flag = guy->login;
    p1->fd = guy->fd;
    strcpy( p1->buf,guy->buf );
    p2 = p1;
    p1->next = NULL;
}

void xiaoxi()   //在主线程处理服务器发来的消息
{
    int n;
    while(1)
    {
        printf("\n\t1.好友添加\n\t2.私聊\n\t3.群聊\n\t4.按零退出返回上一级菜单\n");
        scanf( "%d",&n );
        getchar();
        while( n>3 || n<0 )
        {
            scanf( "%d",&n );
            getchar();
        }
        
        if( n == 0 )   //按零退出
        { 
            break;
        }
        
        if( n == 1 )   //处理好友添加的请求
        {
            int t;
            news *p = head->next;
            while( p->next )
            {
                if( p->flag == 1 )
                {
                    printf( "\n编号:%d\n%s\n",p->fd,p->buf );
                }
                p = p->next;
            }
            printf( "请输入对应编号处理:(或按零返回上层页面)\n" );
            scanf( "%d",&t );
            if( t == 0 )
            {
                return ;
            }
            while( p->next )
            {
                if( p->fd == t )
                {
                    break;
                }
                p = p->next;
            }
            printf( "\n%s\n",p->buf );
            memset(guy.buf,0,sizeof(guy.buf));
            scanf( "%s",guy.buf );
            //guy.fd = p->fd;
            printf( "--------a----->%s\n",guy.buf );
            send( s_fd,(void *)&guy,sizeof(user),0 );
        }
    }
}
