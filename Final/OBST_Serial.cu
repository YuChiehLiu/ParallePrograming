#include <iostream>
#include "CycleTimer.h"
#include <time.h>
#define node 2000 
#define key (node+1)
using namespace std;

void generate_data(int *p, int *q){
    
    srand(0);    
   
    for(int i=0; i<=node; i++){
        p[i] = rand() % 10;
        q[i] = rand() % 10;
    }
}

int main(){
    double start_time, end_time;
    start_time = currentSeconds();
    int *w, *s;
    w = (int*)calloc((node+2) * (node+1), sizeof(int));
    s = (int*)calloc((node+2) * (node+1), sizeof(int));

    /*
    int w[node+2][node+1] = {0};
    int s[node+2][node+1] = {0};
    */
    int *p = (int*)malloc(key * sizeof(int));
    int *q = (int*)malloc(key * sizeof(int));
    //int p[node+1] = {0,25,20,5,20,30};
    //int q[node+1] = {0};
    //int p[node+1] = {0, 5, 2, 4, 3};
    //int q[node+1] = {3, 2, 3, 4, 2};
    
    generate_data(p,q);
    for(int i=1; i<=node+1; i++){
        for(int j=0; j<=node; j++){
            if(i-j < 2){
                for(int k = i-1; k <= j; k++)
                    w[i*(node+1)+j] += q[k];
                for(int k = i; k <= j; k++)
                    w[i*(node+1)+j] += p[k];
            }
        }
    }

    for(int k=-1; k<= node-1; k++){
        for(int i=1;i<=node+1; i++){
            if(i+k > node)
                break;
            //s[i][i+k]
            int min = INT_MAX;
            if(k == -1)
                s[i*(node+1)+i+k] = q[i+k];
            else{
                for(int r = i; r<=i+k; r++){
                    s[i*(node+1)+i+k] = s[i*(node+1)+r-1] + s[(r+1)*(node+1)+i+k] +  w[i*(node+1)+i+k];
                    if(s[i*(node+1)+i+k] < min)
                        min = s[i*(node+1)+i+k];
                }
                s[i*(node+1)+i+k] = min;
            }
        }
    }
    end_time = currentSeconds();
    

    //for(int i=1; i<=node+1; i++){
    //    for(int j=0; j<=node; j++){
    //        cout << w[i][j] << " ";
    //    }
    //    cout << endl;
    //}

    //for(int i=1; i<=node+1; i++){
    //    for(int j=0; j<=node; j++){
    //        cout << w[i * (node+1) +j] << " ";
    //    }
    //    cout << endl;
    //}

    //cout<<endl;
    //    
    //for(int i=1; i<=node+1; i++){
    //    for(int j=0; j<=node; j++){
    //        cout << s[i * (node+1) +j] << " ";
    //    }
    //    cout << endl;
    //}
    
    

    cout << s[1*(node+1)+key-1] << endl;
    double total_time = end_time - start_time;

    printf("\n[Serial]:\t\t[%.3f] ms\n", total_time * 1000);
    free(w);
    free(s);
    free(q);
    free(p);
}
