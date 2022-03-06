#include <stdio.h>
#include <string.h>
#include <utmp.h>
#include <stdlib.h>
#include <time.h>
#define PATH "/var/log/wtmp"
#define METHOD "rb"
void showtime(struct utmp * log){
        char *cp;
        long t = log->ut_time;
        cp = ctime(&t);
        if(*log->ut_line !='~' ){printf("              ");}
        printf("%12.16s", cp);
}
void show_created_time(struct utmp * log){
         char *cp;
        long t = log->ut_time;
        cp = ctime(&t);
        printf("\nwtmp begins %s", cp);
}

void showline(char * str){
        if(*str == '~'){
                printf("system boot");
        }
        else{printf("%s         ", str);}

}

void utmpprint(struct utmp *log){
        printf("%-8.8s", log->ut_name);
        printf(" ");
        showline(log->ut_line);
        printf(" ");
        printf("%s", log->ut_host);
        printf(" ");
        showtime(log);
        printf("\n");
}

int is_valid(short stat){
        return  stat == BOOT_TIME || stat == USER_PROCESS;
        }


int main(int argc, char ** argv) {
        int x;
        if(argc > 2){printf("[-] please enter just 1 argument\n"); exit(0);}
        FILE *file;
        struct utmp log;
        file = fopen(PATH, METHOD);
        if(!file){printf("\a[-] Error while openning the file\n"); exit(0);}  
        //find how many utmp structs there are in wtmp file
        if(fseek(file, 0L, SEEK_END)){printf("[-] Error with the fseek, Please try again\a"); exit(0);}
        long file_size = ftell(file)/sizeof(struct utmp);
        if(argc == 1){x = file_size;}
        else{
                x = atoi(argv[1]);
                if(x<0){printf("\a[-] Argument should be greater or equal to 0\n"); exit(0);}   
        }
        x = (x < file_size) ? x:file_size;        
        fseek(file, 0L, SEEK_SET);
        int count =1;
        int chunk_size = 1;
        while(count < x + 1 && chunk_size <= file_size){
                if(fseek(file, -chunk_size*sizeof(struct utmp), SEEK_END)){printf("[-] Error with the fseek, Please try again\a"); exit(0);}
                if(fread(&log, sizeof(struct utmp), 1, file)== 1){ 
                 if(is_valid(log.ut_type)){
                        utmpprint(&log);
                        count++;
                        }
                        chunk_size++;
                }else{printf("\a[-] Error with fread function, Please try again\n"); exit(0);}  
        }
        rewind(file);
        if(fread(&log, sizeof(struct utmp), 1, file)== 1){
                show_created_time(&log);
        }else{printf("\a[-] Error with fread function, Please try again\n"); exit(0);}  

    return(0);

}
