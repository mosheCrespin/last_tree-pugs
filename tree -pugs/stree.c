#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

typedef struct entry {
  char* name;
  char* details;
  int is_dir;
  struct entry *next;
} _entry;





void mode_to_letters(int mode,char str[]){
    strcpy(str,"----------");
    if(S_ISDIR(mode)){str[0]='d';}
    if(S_ISCHR(mode)){str[0]='c';}
    if(S_ISBLK(mode)){str[0]='b';}

    if(mode & S_IRUSR){str[1] ='r';}
    if(mode & S_IWUSR){str[2] ='w';}
    if(mode & S_IXUSR){str[3] ='x';}

    if(mode & S_IRGRP){str[4] ='r';}
    if(mode & S_IWGRP){str[5] ='w';}
    if(mode & S_IXGRP){str[6] ='x';}

    if(mode & S_IROTH){str[7] ='r';}
    if(mode & S_IWOTH){str[8] ='w';}
    if(mode & S_IXOTH){str[9] ='x';}
    str[10]='\0';



}

static int can_free_uid =0;
char * uid_to_name(uid_t uid){
    struct passwd *getpwuid(), *pw_ptr;
    if((pw_ptr = getpwuid(uid)) == NULL){
        can_free_uid = 1;
        char * numstr = (char*) malloc (10);
        if(!numstr){perror("can't malloc");exit(1);}
        
        sprintf(numstr,"%d", uid);
        return numstr;
    }
    else{
        can_free_uid = 0;
        return pw_ptr->pw_name;}
}
static int can_free_gid=0;
char* gid_to_name(gid_t gid){
    struct group *getggrid(), *grp_ptr;
    if((grp_ptr = getgrgid(gid)) ==NULL){
        char* numstr = (char*) malloc(10);
        if(!numstr){perror("can't malloc");exit(1);}
        can_free_gid =1;
        sprintf(numstr,"%d",gid);
        return numstr;
    }else{
        can_free_gid =0;
        return grp_ptr->gr_name;}
}


void find_details(_entry *det , char *path){
    char *name = det->name;
    
    if(strcmp(".",path)!=0){
        name = (char*)malloc(strlen(path) + strlen(det->name)+2);
        if(!name){perror("can't malloc");exit(1);}
        sprintf(name,"%s/%s",path,det->name);
    }

    char *uid,*gid;
    struct stat info;
    if(stat(name, &info) == -1){perror(name);exit(-1);}
    
    struct stat * info_p = &info;
    char modestr[11];
    mode_to_letters(info_p->st_mode,modestr);
    long size = (long) info_p->st_size;
    uid = uid_to_name(info_p->st_uid);
    gid = gid_to_name(info_p->st_gid);
    char l_bracket ='[';
    char r_bracket =']';
    det->details = malloc(sizeof(char)+strlen(modestr) + strlen(uid) + strlen(gid)+ sizeof(long)+ sizeof(char) + 6);
    if(!det->details){perror("can't malloc");exit(1);}
    sprintf(det->details,"%c%s %-8s%-10s%8ld%c",l_bracket,modestr,uid,gid,size,r_bracket);
 
    if(can_free_uid){free(uid);}
    if(can_free_gid){ free(gid);}
    if(strcmp(".",path)!=0){free(name);}
}





void walk(char *path, char * history, size_t *dirs, size_t *files){
    (*dirs)++;
    DIR *dir;
    size_t count =0;

    dir = opendir(path);//open the curr dir
    if(!dir){perror("[-] can't open dir");return;}
    struct dirent *s_file;//struct of a file
    _entry *curr, *head = NULL, *ptr;
 
    while((s_file = readdir(dir))!=NULL){
        
        if(s_file->d_name[0] == '.'){continue;}
               
        curr = (_entry*) malloc(sizeof(_entry));
        if(!curr){perror("can't malloc");exit(1);}

                

        curr->name = (char*) malloc(strlen(s_file->d_name)+1);
        if(!curr->name){perror("can't malloc");exit(1);}

        

        strcpy(curr->name, s_file->d_name);//includes \0

        curr->is_dir = (s_file->d_type==DT_DIR)? 1 :0;
        

        curr->next = NULL;
           

        find_details(curr, path);
           
        if(!head){head= curr; ptr =curr;}
        
        else{
            ptr->next = curr;
            ptr=ptr->next;
        }


        count++;
    }

    if(closedir(dir)){perror("can't close directory"); exit(1);}

    char * f_d__ptr;
    char * to_history;
    ptr = head;    
    for(size_t i = 0;i<count;++i){
        if(i<count - 1){
            f_d__ptr = "├── ";
            to_history= "│   ";
        }
        else{
            f_d__ptr = "└── ";
            to_history = "   ";
        }
        
        printf("%s%s%s  %s\n", history, f_d__ptr ,ptr->details, ptr->name);
        if(ptr->is_dir){
            char *new_history = (char*) malloc(strlen(history) + strlen(to_history) +2);
            if(!new_history){perror("can't malloc");exit(1);}
            sprintf(new_history, "%s%s",history,to_history);
            char* new_path = (char*)malloc(strlen(path) + strlen(ptr->name)+2);
            if(!new_path){perror("can't malloc");exit(1);}
            sprintf(new_path,"%s/%s",path, ptr->name);
            walk(new_path,new_history,dirs, files);
            free(new_history);
            free(new_path);
        }else{(*files)++;}
            _entry *to_free= ptr;
            ptr = ptr->next;
            free(to_free->name);
            free(to_free->details);
            free(to_free);
    }
    
}




int main(int argc, char ** argv)
{
    size_t dirs =0, files=0;
    
    char * dir = (argc>1)? argv[1]:".";
    printf("%s\n",dir);
    walk(dir,"",&dirs,&files);
    printf("\n%ld directories, %ld files\n",dirs-1,files);
    return 0;
}
