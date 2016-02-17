#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
    char buffer[] = "Fred male 25,John male 62,Anna female 16";
    int in = 0;
    char *p[20];
    char *buf=buffer;
    char *inner_ptr=NULL;
    char *outer_ptr=NULL;
    while((p[in] = strtok_r(buf, ",", &outer_ptr))!=NULL)
    {
        buf=p[in];
        while((p[in]=strtok_r(buf, " ", &inner_ptr))!=NULL)
        {
            in++;
            buf=NULL;
        }
        buf=NULL;
    }
    printf("Here we have %d strings\n",in); //in=9
    int j;
    //Fred male 25 John male 62 Anna female 16
    for (j=0; j<in; j++)
    {
        printf("%s\t",p[j]);
    }
    printf("\n");


    return 0;
}
