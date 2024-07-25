#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#ifdef _OPENMP
#include <omp.h>
#endif

double bucket_range_max;
//Method for checking the run-time
static double get_wall_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

//Method for checking the sorting of the final array.
void check_sort(double *arr, int N)
{   int i;
    for ( i = 0; i < N - 1; i++)
    {
        if (arr[i] > arr[i + 1])
        {
            printf("Sort was not successfull, found fault at %f", arr[i]);
            printf(" %f \n", arr[i + 1]);
            printf("At position %d \n", i);
            exit(EXIT_FAILURE);
        }
    }
    printf("Success, numbers of elements sorted %d \n", i + 1);
}

//Method for checking the sorting of the buckets.
void check_buckets(double **b_list, int *index_list, int k) {
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < index_list[i] - 1; j++) {
            if (b_list[i][j] > b_list[i][j + 1]) {
                printf("Sort was not successful, found fault at bucket %d, position %d \n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }
    printf("Success, numbers of buckets sorted correctly %d \n", k);
}

// Method for printing buckets, used for early problem solving
void print_buckets(double **b_list, int *index_list, int k) {
    for (int i = 0; i < k; i++) {
        printf("Bucket %d: ", i);
        for (int j = 0; j < index_list[i]; j++) {
            printf("%f ", b_list[i][j]);
        }
        printf("\n");
    }
}

//Method for generating an array based upon the executing input
void gen_arr(double *arr, int N, int array_type)
{
    int i;
    int max_value = 1000;
    srand(time(NULL));
    switch (array_type)
    {
    case 1: //The normal case for generating an array with range between 0-1000
        for (i = 0; i < N; i++)
        {   
            arr[i] = rand() % (max_value + 1);
            if(bucket_range_max < arr[i])
            {
                bucket_range_max = arr[i];
            }
        }
        break;
    case 2: //The exponential case for generating an array with exponential numbers as a base
        double e = exp(1);
        for (i = 0; i < N; i++)
        {  
            arr[i] = e + rand();
            if(bucket_range_max < arr[i])
            {
                bucket_range_max = arr[i];
            }
        }
        break;
    case 3: //The uniform case for generating an array with 1's
        for (i = 0; i < N; i++)
        {
            arr[i] = 1;
        }
        bucket_range_max = 1;
        break;

    default: 
                printf("Value must be entered for a randomly generated array to sort  \n");
                exit(EXIT_FAILURE);
        break;
    }
}


//Method for main execution
void bucket_srt(double *arr, int k,int N, int threadcount,int array_type)
{
    int count = 0;
    int i;
    int bucket_range;
 

     #ifdef _OPENMP
     omp_set_num_threads(threadcount);
     #endif
    
    //Array generating method
    gen_arr(arr, N, array_type);
    //Value range for buckets
    double temp_range = ceil(bucket_range_max / (double)k);
    bucket_range = temp_range;

    //Init bucket list
    double **b_list = (double **)malloc(k * sizeof(double *));

    //Allocate the memory for the buckets inside of the bucket list
    for (i = 0; i < k; i++) {
        b_list[i] = (double *)malloc(N * sizeof(double));
    }

    //Size tracker for the bucket
    int *index_list = (int *)calloc(k,sizeof(int));

    //Here we go
    double start = get_wall_time();

    int bucketindex;
    for (i = 0; i < N; i++) {
        //Find the placement bucketwise of the element depending on its value rounded up.
        double temp_br = ceil((double)arr[i] / (double)bucket_range);
        int bucketindex = temp_br;

        //If we end up outside the range put it in last bucket
        if (bucketindex >= k)
        {
            bucketindex = k - 1;
        }

        //Insert element into appropriate bucket
        b_list[bucketindex][index_list[bucketindex]] = arr[i];
        //Keep track of size
        index_list[bucketindex] += 1;
    }

    //Main sorting algorithm
    #pragma omp parallel for firstprivate(b_list) schedule(dynamic,100)
    for (i = 0; i < k; i++)
    {
        int size = index_list[i];

        if (size > 0)
        {   

            #pragma omp task
            for (int j = 1; j < size; j++)
            {   
                //Save current value
                int check_val = b_list[i][j];
                int c = j - 1;
                //Iterate backwards until we find a pos where the curr value is larger then the one before it
                while (b_list[i][c] > check_val)
                {   
                    //Shift the array to make room
                    b_list[i][c + 1] = b_list[i][c];
                    c = c - 1;
                }
                //Overwrite the value in its sorted pos
                b_list[i][c + 1] = check_val;
            }
        }
    }
double end_b = get_wall_time();

printf("Time to bucket sort : %f\n", end_b - start);

//Put the bucket into a full array again
int index = 0;
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < index_list[i]; j++) {
            arr[index++] = b_list[i][j];
        }
    }

double end = get_wall_time();
printf("Time adding into full array: %f\n", end - end_b);
printf("Time to total sort: %f\n", end - start);
//Check the sorting of the buckets
check_buckets(b_list, index_list, k);
//Check the finalized array
check_sort(arr, N);
free(b_list);
}

// Main function
int main(int argc, char *argv[])
{
    printf("Main prog starting... \n");
    int N = atoi(argv[1]); // Size of array
    int k = atoi(argv[2]); // Number of buckets
    int threadcount = atoi(argv[3]); // Number of threads
    int array_type;
    printf("Enter 1: Normal sort, 2: Exponential sort, 3: Uniform sort ");
    scanf("%d", &array_type);

    double *arr = (double *)malloc(N * sizeof(double));
    bucket_srt(arr, k, N, threadcount, array_type);

    free(arr);
    return 0;
}