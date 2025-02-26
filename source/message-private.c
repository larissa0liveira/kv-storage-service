// Grupo 49
// Larissa Oliveira - 59830
// Nickolas Ficker - 60810
// Tiago Lopes - 60745

#include <errno.h>
#include <unistd.h>
#include <stdio.h>

int write_all(int sock, void *buf, int len){

    int bufsize = len;

    while(len>0) {
        int res = write(sock, buf, len);

        if(res<0) {
            if(errno==EINTR) continue;
            perror("write failed:");
            return res; /* Error != EINTR */
        }
        if(res==0) return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, void *buf, int len){
    int bufsize = len;

    while(len>0) {
        int res = read(sock, buf, len);

        if(res<0) {
            if(errno==EINTR) continue;
            perror("read failed:");
            return res; /* Error != EINTR */
        }
        if(res==0) return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}