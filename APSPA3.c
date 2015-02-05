#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//Only root process have right to read and write on data text file
//Root reads data from file, one portion at a time and distribute to other process
//Each process computes and broadcast
//Finally root starts receiving data from non-root processes and writes in text file

//Advantage - Space requirement reduced by a factor of no. of process
//Disadvantage - Reading and writing may take time (More overhead), increases running time

//TODO - Non Divisible number of processes
//TODO - Time analysis


/******************************************Global variables**************************************************/
int totalnodes,myid;
int mpi_err;
int n_Rows,n_Cols,c_Rows;//n_Rows = number of rows, c_Rows = no. of rows in which computation is done by a process
int *mat;//1D structure of matrix
int **b;//pointer to different rows of the matrix
int *b_Row;//broadcast row

/*******************************Non-algorithmic function declaration****************************************/
void MPI_initialize(int *, char ***);
void file_seeker(FILE *fp,int n);
void show_data(int *data,int n);
int minimum(int a,int b);

/*******************************************Main function***************************************************/
int main(int argc,char *argv[]){

	//Variables Declaration
	int i,k,j;

	int f_err;//File error msg
	int otherid;//Id of the process for which root have to wait for broadcast data
	MPI_Status status;
	int tag = 007;//a random tag

	//File pointers(Single file is pointed)
	FILE *file_reader;
	FILE *file_writer;

	MPI_initialize(&argc,&argv);


//If program is running on root processor
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	if(myid==0){
	//File pointers to read and write
    file_reader = fopen("matrix.txt","r");
    file_writer = fopen("matrix.txt","a");
    f_err = fscanf(file_reader, "%d %d\n", &n_Rows,&n_Cols);

   //Division of work - Work should be integer divisible
   c_Rows = n_Rows/totalnodes;
   b_Row = (int *)malloc(sizeof(int)*n_Cols);
   
   //Sending data to each process after scanning required portion of file
   
   //First portion is reserved for root processor
   file_seeker(file_reader,c_Rows*n_Cols);
   for(i=1;i<totalnodes;i++){
		 	mpi_err = MPI_Send(&n_Rows,1,MPI_INT,i,tag,MPI_COMM_WORLD);
		 	mpi_err = MPI_Send(&n_Cols,1,MPI_INT,i,tag,MPI_COMM_WORLD);
		 	b=(int **)malloc(sizeof(int *)*c_Rows);
		 	mat=(int *)malloc(sizeof(int)*c_Rows*n_Cols);//Space Efficient
		 	for(j=0;j<c_Rows;j++){
      	b[j] = &mat[j*n_Cols];
      }
   		//Scanning for data in file
		 	for(j=0;j<c_Rows;j++){
		 		for(k=0;k<n_Cols;k++){
		 			f_err = fscanf(file_reader,"%d",&b[j][k]);
		 		}
		 	}
		 	//Sending to process i
		 	mpi_err = MPI_Send(mat,c_Rows*n_Cols,MPI_INT,i,tag,MPI_COMM_WORLD);
	 }

		 //Now retrieve data for own
	 fseek(file_reader,0,0);//Seeks to start of the file
	 file_seeker(file_reader,2);
	 for(j=0;j<c_Rows;j++){
		 for(k=0;k<n_Cols;k++){
		 		f_err = fscanf(file_reader,"%d",&b[j][k]);
		 }
	 }
	 /*sleep(myid);
	 printf("%d shows received data\n",myid);
   	 show_data(mat,c_Rows*n_Cols);*/
  }
   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//   
//If program is running on non-root processor   
   else{
   	mpi_err = MPI_Recv(&n_Rows,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
   	mpi_err = MPI_Recv(&n_Cols,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
   	c_Rows = n_Rows/totalnodes;
   	b=(int **)malloc(sizeof(int *)*c_Rows);
    mat=(int *)malloc(sizeof(int)*c_Rows*n_Cols);
    b_Row = (int *)malloc(sizeof(int)*n_Cols);
    for(j=0;j<c_Rows;j++){
   		b[j] = &mat[j*n_Cols];
   	}
   	//receives data to work on
   	mpi_err = MPI_Recv(mat,c_Rows*n_Cols,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
   	/*sleep(myid);
   	printf("%d shows received data\n",myid);
   	show_data(mat,c_Rows*n_Cols);*/
   	}
   	 /*********************Processing*******************************/
   	for (k=0;k<n_Rows;k++)
   	{
        //If Data is needed to be sent
		 if(k<=myid*c_Rows+c_Rows - 1 && k>=myid*c_Rows)
		 {
		 	//printf("%d says:Broadcasting row %d from process %d \n",myid,k,myid);
		  	//show_data(b[k%c_Rows],n_Cols);
		 	mpi_err = MPI_Bcast(b[k%c_Rows],n_Cols,MPI_INT,myid,MPI_COMM_WORLD);
		 	for(i=0;i<c_Rows;i++)
		 	{
            	for(j=0;j<n_Cols;j++)
            	{
                	b[i][j] = minimum(b[i][j],b[i][k]+b[k%c_Rows][j]);
            	}
      		}
		 }
		 
        //If data is needed
		 else
		 {
		 	otherid = k/c_Rows;
		 	//printf("%d says:Require row %d from process %d \n",myid,k,otherid);
		 	mpi_err = MPI_Bcast(b_Row,n_Cols,MPI_INT,otherid,MPI_COMM_WORLD);
		 	//show_data(b_Row,n_Cols);

		 	for(i=0;i<c_Rows;i++)
		 	{
            	for(j=0;j<n_Cols;j++)
            	{
                	b[i][j] = minimum(b[i][j],b[i][k]+b_Row[j]);
            	}
        	}
       	}
     }
     //sleep(3);
     //printf("%d is my id\n",totalnodes);
     //show_data(mat,c_Rows*n_Cols);
     
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
/*****************Taking data back*************************************/
     MPI_Barrier(MPI_COMM_WORLD);
     if(myid==0)
     {
     	for(i=0;i<c_Rows;i++)
     	{
			for(j=0;j<n_Cols;j++)
			{
				fprintf(file_writer,"%d ",b[i][j]);
				//printf("%d\t",b[i][j]);
			}
		fprintf(file_writer,"\n");
		//printf("\n");
		}
        for(k=1;k<totalnodes;k++)
        {
        	
        	//printf("\nReceiving from %d \n",k);
            mpi_err = MPI_Recv(mat,c_Rows*n_Cols,MPI_INT,k,tag,MPI_COMM_WORLD,&status);
            //show_data(mat,c_Rows*n_Cols);
            for(i=0;i<c_Rows;i++)
            {
                for(j=0;j<n_Cols;j++)
                {
                    fprintf(file_writer,"%d ",b[i][j]);
                    //printf("%d\t",b[i][j]);
                }
            fprintf(file_writer,"\n");
            //printf("\n");
        	}
    	}
    
    	fclose(file_reader);
		fclose(file_writer);

    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//Non root processor sends back computed data
     else
     {
     	//printf("%d is sending data\n",myid);
     	mpi_err = MPI_Send(mat,c_Rows*n_Cols,MPI_INT,0,tag,MPI_COMM_WORLD);
     }

    mpi_err = MPI_Finalize();
  	return 0;


}
/*******************************************Main function ends***************************************************/

/********************Function defenition************************/
void MPI_initialize(int *argc,char ***argv)
{
	mpi_err = MPI_Init(argc,argv);
	mpi_err = MPI_Comm_size( MPI_COMM_WORLD,&totalnodes);
	mpi_err = MPI_Comm_rank(MPI_COMM_WORLD,&myid);
}

void file_seeker(FILE *fp,int n)
{
	int i,temp,f_err;
	for(i=0;i<n;i++)
	{
		f_err=fscanf(fp,"%d",&temp);
	}
}

void show_data(int *data,int n){
	int i;
	for(i=0;i<n;i++){
		printf("%d\t",data[i]);
	}
	printf("\n");
}

int minimum(int a,int b){
	if(a<b){
		return a;
	}
	else{
		return b;
	}
}
/*********************Function defenition ends*******************/
