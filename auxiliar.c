#include "stdio.h"
#include "stdlib.h"

int main()
{
    int i,aux;
    FILE *arq = fopen("banana.txt","wt"),*arq2;
    for(i=0;i<900;i+=5)
    {
        fprintf(arq,"%d 0\n",i);
    }
    fclose(arq);
    /*for(i=0;i<128;i++)
    {
        fprintf(arq,"0\n-50\n0\n40\n50\n40\n");
    }
    fclose(arq);
    arq = fopen("banana.txt","rt");
    if(arq==NULL)
    {
        printf("Erro");
    }
    arq2 = fopen("final.txt","wt");
    for(i=0;i<900;i+=5)
    {
        fscanf(arq,"%d\n",&aux);
        printf("%d",aux);
        fprintf(arq2,"%d %d\n",i,aux);
    }*/
}
