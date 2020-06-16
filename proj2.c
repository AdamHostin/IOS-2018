/*Adam Hostin xhosti02
  29.4.2018
  implementacia algoritmu bus problem solution #2
  IOS 2.projekt
*/
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include<ctype.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/mman.h>


#include <fcntl.h>

void generator_pasazierov(int r,int art);
void cestujuci_pasazier(int moje_id);
void BUS(int r, int abt,int c);
void loging(char* meno,char* akcia, int cislo_procesu,int pocet);


FILE *f;
int *a;
int *rada = 0;
int *pocet_prevezenych;

sem_t *pristup_k_rade;
sem_t *pristup_k_autobusu;
sem_t *nastupovanie;
sem_t *pristup_k_a;
sem_t *vystupovanie;
sem_t *vystupenie_z_autobusu;

void loging(char* meno,char* akcia, int cislo_procesu,int pocet){

  if ((cislo_procesu==-1)&&(pocet==-1)){
    sem_wait(pristup_k_a);
    fprintf(f,"%d:%s:%s\n",*a,meno,akcia );
    fflush(f);
    (*a)++;
    sem_post(pristup_k_a);
  }
  else if ((cislo_procesu!=-1)&&(pocet==-1)){
    sem_wait(pristup_k_a);
    fprintf(f,"%d:%s %d:%s\n",*a,meno, cislo_procesu,akcia );
    fflush(f);
    (*a)++;
    sem_post(pristup_k_a);
  }
  else if ((cislo_procesu==-1)&&(pocet!=-1)){
    sem_wait(pristup_k_a);
    fprintf(f,"%d:%s:%s: %d \n",*a,meno,akcia,pocet );
    fflush(f);
    (*a)++;
    sem_post(pristup_k_a);
  }
  else {
    sem_wait(pristup_k_a);
    fprintf(f,"%d:%s %d:%s: %d \n",*a,meno,cislo_procesu,akcia,pocet );
    fflush(f);
    (*a)++;
    sem_post(pristup_k_a);
  }

}

void BUS(int r, int abt,int c) {

//  int pocet_prevezenych=0;
  int pocet_nastupujucich=0;

  loging("BUS","start",-1,-1);

  while (r > (*pocet_prevezenych)) {


    sem_wait(pristup_k_rade);
    loging("BUS","arrival",-1,-1);

    //zisti kolko pasazierov bude nastupovat
    if ((*rada)-(*pocet_prevezenych)<c){
      pocet_nastupujucich=(*rada)-(*pocet_prevezenych);
      (*pocet_prevezenych)+=pocet_nastupujucich;
    }
    else{
      pocet_nastupujucich=c;
      (*pocet_prevezenych)+=pocet_nastupujucich;
    }

    //Nastupovanie
    if (pocet_nastupujucich!=0){

      loging("BUS","start boarding",-1,pocet_nastupujucich);

      for (int i=0;i<pocet_nastupujucich;i++){
        sem_post(pristup_k_autobusu);
        sem_wait(nastupovanie);
      }
    }
    if (pocet_nastupujucich!=0){
      loging("BUS","end boarding",-1,(*rada)-(*pocet_prevezenych));
    }
    loging("BUS","depart",-1,-1);
    sem_post(pristup_k_rade);


    //jazdim
    if (abt!=0) {
      usleep(rand() % abt);
    }
    //jazda konci

    loging("BUS","end",-1,-1);

    //vystupovanie

    if (pocet_nastupujucich!=0){

      for (int i=0;i<pocet_nastupujucich;i++){
        sem_post(vystupenie_z_autobusu);
        sem_wait(vystupovanie);
      }
    pocet_nastupujucich=0;
    }
  }
  loging("BUS","finish",-1,-1);

}

void cestujuci_pasazier(int moje_id) {

    loging("RID","start",moje_id,-1);

    //pasazier sa postavi na zastavku
    sem_wait(pristup_k_rade);

    (*rada)++;

    loging("RID","enter",moje_id,(*rada)-(*pocet_prevezenych));

    sem_post(pristup_k_rade);

  //boardovanie pasazierov
    sem_wait(pristup_k_autobusu);

    loging("RID","boarding",moje_id,-1);

    sem_post(nastupovanie);

    //ukoncenie ridera
    sem_wait(vystupenie_z_autobusu);

    loging("RID","finish",moje_id,-1);

    sem_post(vystupovanie);

    exit(0);
}

void generator_pasazierov(int r,int art) {
    int i;
    pid_t pole[r];
    pid_t kontrolny_pid;

    for (i=0; i<r; i++) {
         pole[i]=fork();

        if (pole[i]==0) {
            cestujuci_pasazier(i+1);


            exit(0);
        } else if (pole[i-1]==-1) {
            fprintf(stderr, "chyba pri forkovani\n" );
            exit(EXIT_FAILURE);
        } else {
            //kod generatoru_pasazierov
        }
        if (art!=0){
          usleep(rand() % art);
        }
    }
    //cakam na ukoncenie pasazierov
    for (i=0; i<r; i++){
      while ((kontrolny_pid=wait(&pole[i]))>0) {}
    }
}

int main(int argc, char *argv[]) {

    int r;
    int c;
    int art;
    int abt;
    time_t t;
    pid_t kontrolny_pid;
    long test;
    char *arg_test_string;

    if (argc!=5){
      fprintf(stderr, "neplatny pocet argumentov\n" );
      exit(1);
    }
    //r=atoi(argv[1]);
    test=strtol(argv[1],&arg_test_string,10);
    if ((test<=0)||((*arg_test_string)!='\0')){
      fprintf(stderr, "neplatny vstup poctu riderov\n" );
      exit(1);
    }
    else{
      r= (int)test;
    }
    //c=atoi(argv[2]);
    test=strtol(argv[2],&arg_test_string,10);
    if ((test<=0)||((*arg_test_string)!='\0')){
      fprintf(stderr, "neplatny vstup capacity autobusu\n" );
      exit(1);
    }
    else{
      c= (int)test;
    }
    //art=atoi(argv[3]);
    test=strtol(argv[3],&arg_test_string,10);
    if ((test<0) || (test>1000)||((*arg_test_string)!='\0')){
      fprintf(stderr, "neplatny vstup art\n" );
      exit(1);
    }
    else{
      art=(int)test;
    }
    //abt=atoi(argv[4]);
    test=strtol(argv[4],&arg_test_string,10);
    if ((test<0) || (test>1000)||((*arg_test_string)!='\0')){
      fprintf(stderr, "neplatny vstup abt\n" );
      exit(1);
    }
    else{
      abt=(int)test;
    }
    art=art*1000;
    abt=abt*1000;

    srand((unsigned) time(&t));

    //alokacia zdrojov


    pristup_k_a = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (pristup_k_a== MAP_FAILED){
      fprintf(stderr, "chyba pri vytvarani semaforu\n" );
      exit(EXIT_FAILURE);
      }
    if(sem_init(pristup_k_a, 1, 1)==-1){
      fprintf(stderr, "chyba pri vytvarani semaforu\n" );
      exit(EXIT_FAILURE);
    }

    pristup_k_rade = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        if (pristup_k_rade== MAP_FAILED){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
          }
        if (sem_init(pristup_k_rade, 1, 1)==-1){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }

    pristup_k_autobusu = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        if (pristup_k_autobusu== MAP_FAILED){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }
        if(sem_init(pristup_k_autobusu, 1, 0)==-1){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }

    nastupovanie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        if (nastupovanie== MAP_FAILED){
            fprintf(stderr, "chyba pri vytvarani semaforu\n" );
                exit(EXIT_FAILURE);
          }
        if (sem_init(nastupovanie, 1, 0)==-1){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }

    vystupovanie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        if (vystupovanie== MAP_FAILED){
            fprintf(stderr, "chyba pri vytvarani semaforu\n" );
                exit(EXIT_FAILURE);
          }
        if (sem_init(vystupovanie, 1, 0)==-1){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }

    vystupenie_z_autobusu = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
        if (vystupenie_z_autobusu== MAP_FAILED){
            fprintf(stderr, "chyba pri vytvarani semaforu\n" );
                exit(EXIT_FAILURE);
          }
        if(sem_init(vystupenie_z_autobusu, 1, 0)==-1){
          fprintf(stderr, "chyba pri vytvarani semaforu\n" );
          exit(EXIT_FAILURE);
        }


    int rada_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    rada = (int *)shmat(rada_id, NULL, 0);

    int a_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    a = (int*)shmat(a_id, NULL, 0);
    (*a)++;

    int pocet_prevezenych_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    pocet_prevezenych = (int*)shmat(pocet_prevezenych_id, NULL, 0);

    f=fopen("proj2.out", "w");
    if(f == NULL){
        exit(EXIT_FAILURE);
    }

    pid_t bus_pid=fork();

    //BUS
    if (bus_pid==0) {
      BUS(r,abt,c);

        exit(0);
    } else if (bus_pid==-1) {
        fprintf(stderr, "chyba pri forkovani\n" );
        exit(EXIT_FAILURE);
    } else {
      //DO nothing SUCCESSfully
    }

    pid_t rider_pid=fork();

    //generator_pasazierov
    if (rider_pid==0) {

        generator_pasazierov(r,art);

        exit(0);
    } else if (rider_pid==-1) {
        fprintf(stderr, "chyba pri forkovani\n" );
        exit(EXIT_FAILURE);

    } else {
      //DO nothing SUCCESSfully
    }
        //cakam na BUS
    while ((kontrolny_pid=wait(&bus_pid))>0) {
    }
    //cakam na generator_pasazierov
    while ((kontrolny_pid=wait(&rider_pid))>0) {
    }

    sem_destroy(pristup_k_rade);
    sem_destroy(pristup_k_autobusu);
    sem_destroy(pristup_k_a);
    sem_destroy(nastupovanie);
    sem_destroy(vystupovanie);
    sem_destroy(vystupenie_z_autobusu);

    shmdt(pocet_prevezenych);
    shmdt(rada);
    shmdt(a);

    shmctl(pocet_prevezenych_id,IPC_RMID,NULL);
    shmctl(a_id,IPC_RMID,NULL);
    shmctl(rada_id,IPC_RMID,NULL);

    fclose(f);

    return 0;
}
