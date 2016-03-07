/* File: main.c
 * ------------
 * 802.1X客户端命令行
 * 客户端成必须使用方式：
 * 1.直接从命令行调用.
 * 2.在OpenWrt环境下，通过luci的脚本加载执行，如项目https://github.com/ferstar/luci-njit中使用方法。
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

struct sigaction act;
char *DeviceName; 

/* 子函数声明 */
int Authentication(const char *UserName, const char *Password, const char *DeviceName);
void SendLogoffPkt(const char *DeviceName);

void exit_handler(int signo, siginfo_t * info, void * p)
{
    if(signo == SIGINT)
    {
        printf("\n接收到退出信号，准备退出。\n");
        if(DeviceName != NULL)
            SendLogoffPkt(DeviceName);
        printf("bye bye!\n");
        exit(0);
    }
}

/**
 * 函数：main()
 *
 * 检查程序的执行权限，检查命令行参数格式。
 * 允许的调用格式包括：
 * 	njit-client  username  password
 * 	njit-client  username  password  eth0
 * 	njit-client  username  password  eth1
 *  若没有从命令行指定网卡，则默认将使用eth0
 */
int main(int argc, char *argv[])
{
    //注册退出事件函数
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = exit_handler;
    act.sa_flags = SA_SIGINFO;

	char *UserName;
	char *Password;

	/* 检查当前是否具有root权限 */
	if (getuid() != 0) {
		fprintf(stderr, "抱歉，运行本客户端程序需要root权限\n");
		fprintf(stderr, "(RedHat/Fedora下使用su命令切换为root)\n");
		fprintf(stderr, "(Ubuntu/Debian下在命令前添加sudo)\n");
		exit(-1);
	}

	/* 检查命令行参数格式 */
	if (argc<3 || argc>4) {
		fprintf(stderr, "命令行参数错误！\n");
		fprintf(stderr,	"正确的调用格式例子如下：\n");
		fprintf(stderr,	"    %s username password\n", argv[0]);
		fprintf(stderr,	"    %s username password eth0\n", argv[0]);
		fprintf(stderr,	"    %s username password eth1\n", argv[0]);
		fprintf(stderr, "(注：若不指明网卡，默认情况下将使用eth0)\n");
		exit(-1);
	} else if (argc == 4) {
		DeviceName = argv[3]; // 允许从命令行指定设备名
	} else {
		DeviceName = "eth0"; // 缺省情况下使用的设备
	}
	UserName = argv[1];
	Password = argv[2];

	/* 调用子函数完成802.1X认证 */
	Authentication(UserName, Password, DeviceName);

	return (0);
}
