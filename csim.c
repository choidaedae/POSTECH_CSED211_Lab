// 20210716 최대현
// daehyeonchoi@postech.ac.kr

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include "cachelab.h"

int s; int E; int b;
int S;

int hit_num;
int miss_num;
int eviction_num;

char* filename;
int time = 0;

typedef struct {

   int valid;
   int tag;
   int LRU;

} block;

typedef block * line;
line* cache;

void cacheing(unsigned long address){

    int tag = address >> (s+b);
    int set_index = (address>>b) - (tag<<s);
    int t_evict = 0;
    int index = 0;
    int eviction = 1;
    int hit = 0;
    int i;

    time++;
    t_evict=time;

    for(i=0; i<E; i++){

        if(cache[set_index][i].LRU< t_evict){

            t_evict = cache[set_index][i].LRU;
            index = i;

        }

        if(cache[set_index][i].valid == 0){

            index = i;
            eviction=0;
            break;

        }

        else {

           if(cache[set_index][i].tag==tag) {

                hit=1;
                break;

            }

        }

    }

    if(hit) {

        hit_num++;
        cache[set_index][i].LRU=time;

    }

    else {

        if(eviction) eviction_num++;
        miss_num++;

        cache[set_index][index].valid=1;
        cache[set_index][index].tag=tag;
        cache[set_index][index].LRU=time;

    }

}

int main(int argc, char *argv[])
{
    int i, j;     
    FILE *file;
    char op;
    unsigned long address;
    int size;   
    int opt;


    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){


        switch (opt){

            case 'h':
                printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
                return 0;

            case 'v':
                break;

            case 's':
                s=atoi(optarg);
                S = 1 << s;
                break;

            case 'E':
                E= atoi(optarg);
                break;

            case 'b':
                b = atoi(optarg);
                break;

            case 't':
                filename = optarg;
                break;

            default:
                exit(-1);

      }
    }

    cache = (line*)malloc(sizeof(line) * S);

    //printf("%d %d %d %lx\n", s, S, E, sizeof(cache));

    for(i=0; i < S; i++){

        cache[i] = (block *)malloc(sizeof(block) * E);

        for(j = 0; j < E; j++){

            cache[i][j].valid=0;

            cache[i][j].tag=0;

            cache[i][j].LRU=0;

        }

    }

    file = fopen(filename, "r");
    if(file == NULL) return -1;


    while(fscanf(file, " %c %lx, %d", &op, &address, &size) != -1){
         
         switch(op){

            case 'I':
                break;

            case 'L':
                
                cacheing(address);
                break;

            case 'S':
                
                cacheing(address);
                break;

            case 'M':
                
                cacheing(address); 
                cacheing(address);
                break;

            default:
                break;
         }

   }

    printSummary(hit_num, miss_num, eviction_num);
    
    fclose(file);

    return 0;

}

