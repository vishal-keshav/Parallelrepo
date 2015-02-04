#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(){
	
	
	int m,n,f_err;
	FILE *fp;
	fp = fopen("matrix.txt", "r");
  f_err = fscanf(fp, "%d %d", &m,&n);
	int **mat = (int **)malloc(sizeof(int*)*m);
	for(int i=0;i<m;i++){
		mat[i] = (int *)malloc(sizeof(int)*n);
	}
	
	for(int i=0;i<m;i++){
		for(int j=0;j<n;j++){
			f_err = fscanf(fp,"%d",&mat[i][j]);
		}
	}
	
	
	for(int k=0;k<n;k++){
		for(int i=0;i<m;i++){
			for(int j=0;j<n;j++){
				mat[i][j] = (mat[i][j]<(mat[i][k]+mat[k][j]))?mat[i][j]:(mat[i][k]+mat[k][j]);
			}
		}
	}
	
	for(int i=0;i<m;i++){
			for(int j=0;j<n;j++){
				printf("%d\t",mat[i][j]);
		}
		printf("\n");
		}
			
	
return 0;
}


