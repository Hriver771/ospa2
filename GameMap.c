// 21500344 서찬솔

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _City{
  int num;
  int neighbor;
  int *listN;
}City;

int myDP(City *c, int num, int i);

int main(){

  City *c;
  FILE *fp = fopen("input2.txt", "r");
  int num, link, buf = 0, first, second, max = 0;

  fscanf(fp, "%d %d", &num, &link);

  c = (City*)malloc(sizeof(City)*num);

  for(int i=0; i < num; i++){
    c[i].num = i;
    c[i].neighbor = 0;
    c[i].listN = (int*)malloc(sizeof(int)*(num-1));
    for(int j=0; j < num; j++){
      c[i].listN[j] = 0;
    }
  }
  for(int i=0; i < link; i++){
    fscanf(fp, "%d %d", &first, &second);
    c[first].neighbor++;
    c[first].listN[second] = 1;
    c[second].neighbor++;
    c[second].listN[first] = 1;
  }

  for(int i=0; i < num; i++){
    buf = myDP(c, num, i);
    if(buf>max) max = buf;
  }

  printf("%d", max);

  free(c);
  fclose(fp);
  return 0;
}

int myDP(City *c, int num, int i){

  int tmp = 0, min = i;

  for(int j = 0; j < num; j++){
    if((c[i].listN[j] == 1) && (c[i].neighbor < c[j].neighbor)) {
        tmp = j;
        if(min == i) min = tmp;
        if(c[tmp].neighbor < c[min].neighbor) min = tmp;
    }
  }

  if(min != i){
    return 1 + myDP(c, num, min);
  }

  return 1;
}
