#include <android/log.h>
#include <string.h>
#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>

#define LOG_TAG "zcgames:Daemon=="

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#define PROC_DIR          "/proc/"
#define DAEMON_LOOP_FLAG  "/data/data/cn.zcgames.ghostclaw/daemonRunFlag"
#define DAEMON_PROCESS    "/data/data/cn.zcgames.ghostclaw/daemon"
#define TARGET_SERVICE_PROCESS "cn.zcgames.ghostclaw:MainLoop"
#define TARGET_PROCESS    "cn.zcgames.ghostclaw"
#define TARGET_SERVICE    "cn.zcgames.ghostclaw.service.InputManagerService"
/**
 * 判断是否是数字
 */
int isNumeric(const char* pidStr) {
    for (; *pidStr; pidStr++)
        if (*pidStr < '0' || *pidStr > '9')
            return 0;
    return 1;
}

void killRunningPorcess(const char *processNameStr)
{
    DIR* dirProc = NULL;
    struct dirent* entry = NULL;
    char cmdLinePath[256];
    char processName[256];

    dirProc = opendir(PROC_DIR);
    if (dirProc == NULL) {
        LOGD("Open proc directory Failed");
        return;
    }

    while((entry = readdir(dirProc))){
        for(int i = 0; i < 256; i++)cmdLinePath[i]=0;
        for(int i = 0; i < 256; i++)processName[i]=0;
        if (entry->d_type == DT_DIR){
            if (isNumeric(entry->d_name)){
                strcpy(cmdLinePath, PROC_DIR);
                strcat(cmdLinePath, entry->d_name);
                strcat(cmdLinePath, "/cmdline");
                FILE* fp = fopen(cmdLinePath, "rt");
                if(fp != NULL){
                    fscanf(fp, "%s", processName);
                    fclose(fp);
                    if(strcmp(processNameStr, processName) == 0){
                        pid_t pid = (pid_t)atoi(entry->d_name);
                        if(pid != getpid()){
                            kill(pid, SIGKILL);
                        }
                    }
                }
            }
        }
    }
    closedir(dirProc);
    return;//false
}

int serviceIsRunning(const char* serviceName){
    DIR* dirProc = NULL;
    struct dirent* entry = NULL;
    char cmdLinePath[256];
    char processName[256];

    dirProc = opendir(PROC_DIR);
    if (dirProc == NULL) {
        LOGD("Open proc directory Failed");
        return 0;
    }

    while((entry = readdir(dirProc))){
        for(int i = 0; i < 256; i++)cmdLinePath[i]=0;
        for(int i = 0; i < 256; i++)processName[i]=0;
        if (entry->d_type == DT_DIR){
            if (isNumeric(entry->d_name)){
                strcpy(cmdLinePath, PROC_DIR);
                strcat(cmdLinePath, entry->d_name);
                strcat(cmdLinePath, "/cmdline");
                FILE* fp = fopen(cmdLinePath, "rt");
                if(fp != NULL){
                    fscanf(fp, "%s", processName);
                    fclose(fp);
                    if(strcmp(serviceName, processName) == 0){
                        return 1;//true
                    }
                }
            }
        }
    }
    closedir(dirProc);
    return 0;//false
}

void execCommandWithPopen(char *command)
{
    FILE *fp;
    char result[1024] = {0};
    fp = popen(command, "r");
    if(fp){
        while(fgets(result, 1024-1, fp)){
            LOGD("result info: %s", result);
        }
    }else{
        LOGD("popen Failed");
    }
}

void checkAndRestart(const char *serviceName){
    char command[256] = {0};
    char result[1024] = {0};

    if(!serviceIsRunning(serviceName)){
        strcpy(command, "am startservice -n ");
        strcat(command, TARGET_PROCESS);
        strcat(command, "/");
        strcat(command, TARGET_SERVICE);
        strcat(command, "  2>&1");
        LOGD("command  %s", command);
        //system("/system/xbin/su -c \"am startservice -n cn.zcgames.ghostclaw/cn.zcgames.ghostclaw.service.InputManagerService\"");
        execCommandWithPopen(command);
    }
}

int main(int argc, char** argv){
    pthread_t id;
    int ret;

    killRunningPorcess(DAEMON_PROCESS);

    while(access(DAEMON_LOOP_FLAG, F_OK) == 0){
        sleep(20);
        checkAndRestart(TARGET_SERVICE_PROCESS);
    }
}
