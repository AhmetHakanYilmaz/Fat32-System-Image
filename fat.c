#include <unistd.h>
#include <linux/msdos_fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define SECTORSIZE 512
#define CLUSTERSIZE 1024

unsigned char sector[512];
struct fat_boot_sector *bp;
struct fat_boot_fsinfo *deneme;
unsigned char num_fats, sectors_per_cluster;
unsigned int num_sectors, sectors_per_fat;
unsigned int root_start_cluster;
unsigned char *buffer;

unsigned int reserved_no;
unsigned int cluster_count;
unsigned int data_start_sector;



char opToChar(char* op){
	if(op[0]=='-'){
		return op[1];
	}
	return '-';
}


int readsector(int fd, unsigned char *buf,
               unsigned int snum)
{
    off_t offset;
    int n;
    offset = snum * SECTORSIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, SECTORSIZE);

    if (n == SECTORSIZE)
        return (0);
    else
        return (-1);
}

int readcluster(int fd, unsigned char *buf,
                unsigned int cnum)
{
    off_t offset; 
    int n;
    unsigned int snum; // sector number

    snum = data_start_sector +
           (cnum - 2) * sectors_per_cluster;

    offset = snum * SECTORSIZE;
    lseek(fd, offset, SEEK_SET);
    n = read(fd, buf, CLUSTERSIZE);
    if (n == CLUSTERSIZE)
        return (0); // success
    else
        return (-1);
}

unsigned char* getNameFromClusterOrder( unsigned char* cluster, unsigned int order, unsigned char* result, unsigned char* chr)
{
    memset(result,0,11);
    int j;
    int exist = -1;
    for ( j = 7; j>=0 ; j-- )
    {
        if ( cluster[32*order+j] != 32 )
        {
            exist =1;
            break;
        }
    }

    if ( exist == -1)
    {
        j = -1;
    }
    if ( j != -1)
    {
    strncpy(result,cluster+32*order,j+1);
    }

exist = -1;

    for ( j = 10; j>= 8; j-- )
    {
        if ( cluster[32*order+j] != 32 )
        {
            exist = 1;
            break;
        }
    }
    if ( exist == -1)
    {
        j = 7;
    }
    if ( exist == 1)
    {
        strncat(result,chr,1);
    }
    if ( j != 7)
    {
    strncat(result,cluster+32*order+8,j-7);
    }
    //strncpy(result,cluster+32*order+8,j-7);
    return result;
}

unsigned char* getPathFromClusterNo( unsigned char* cluster, unsigned int order, unsigned char* result, unsigned char* chr)
{}


unsigned char* getTypeFromClusterOrder( unsigned char* cluster, unsigned int order, unsigned char* result)
{
    memset(result,0,11);
    if ( cluster[32*order+11]==8)
    {
        strcpy(result,"VOLUME");
    }

    else if ( cluster[32*order+11 ]== 16)
    {
        strcpy(result,"DIRECTORY");
    }
    else if ( cluster[32*order+11 ] == 32)
    {
        strcpy(result,"FILE");
    }
    return result;
}


unsigned int *findFinishOfStartCLuster(int fd, unsigned char *buf, unsigned int startClusterNo, unsigned int *resultArr)
{
    unsigned int sectorToRead;
    unsigned int orderInSector;
    sectorToRead = startClusterNo / 128;
    orderInSector = startClusterNo % 128;
    // unsigned char buf[5];
    // printf ("%d %d ",sectorToRead,startClusterNo);
    int a = 1;

    unsigned int result = 0;
    resultArr[result] = startClusterNo;
    while (a != -1)
    {
        readsector(fd, buf, sectorToRead + 32);

        if (buf[4 * orderInSector] == 255 && buf[4 * orderInSector + 1] == 255 && buf[4 * orderInSector + 2] == 255 && buf[4 * orderInSector + 3] == 15 ||
            (buf[4 * orderInSector] == 248 && buf[4 * orderInSector + 1] == 255 && buf[4 * orderInSector + 2] == 255 && buf[4 * orderInSector + 3] == 15))
        {
            a = -1;
        }
        else
        {
            result++;
            resultArr[result] = buf[4 * orderInSector] + buf[4 * orderInSector + 1] * 256 + buf[4 * orderInSector + 2] * 256 * 256 + buf[4 * orderInSector + 3] * 256 * 25 * 256;
            orderInSector = resultArr[result] % 128;
            sectorToRead = resultArr[result] / 128;
        }
    }
    return resultArr;
}

unsigned int findFirstOfClustersOnTheFile( unsigned int clusterNo,unsigned char *buf,int fd)
{
    unsigned int sectorNo = 32 + clusterNo/128;
    unsigned int exist =1;
  //  unsigned int exist2 = -1;
    while ( exist == 1)
    {
        exist = -1;
        for ( unsigned int i = 0; i <  1 +clusterNo/128; i++)
        {
            readsector(fd,buf,32+i);
        for ( unsigned int j = 0; j < 128 ; j++)
        {
            if ( buf[4*j] + buf[4*j+1]*256 + buf[4*j+2]*256*256+ buf[4*j+3]*256*256*256 == clusterNo)
            {
               // printf("1oldu");
                exist = 1;
               // exist2 = 1;
                clusterNo = i*128+j;
                i= 0;
                break;
            }
        }
        }

    }
    
    return clusterNo;
}

void readclusterForDirectoryAndFiles(int fd, unsigned char *cluster,
                                     unsigned int n, unsigned char *path)
{
    // readcluster(fd, cluster,n);
    //   printf("aaa %d", n);
    char oldPath[100];
    strcpy(oldPath, path);
    unsigned char *c;
    c = (unsigned char *)calloc(12, sizeof(unsigned char));
    //      c[0] =0;
    for (int i = 0; i < 32; i++)
    {
        readcluster(fd, cluster, n);
        if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 32)
        {
            // dosya
            printf("(f)");
            printf("%s", path);
            printf("/");
            for (int k = 0; k < 8; k++)
            {

                if (cluster[32 * i + k] == 32)
                {
                    break;
                }

                printf("%c", cluster[32 * i + k]);
            }
            printf(".");
            for (int k = 8; k < 11; k++)
            {

                printf("%c", cluster[32 * i + k]);
            }
            //   printf("%s",path);
            // char addingPath[30];
            // strncpy(addingPath,cluster+32*i,8);
            // strcat(path,"/");
            // printf("%s",path);
            printf("\n");
        }
        else if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 16)
        {
            // klasor
            // printf("/");

            // for (int k = 0; k < 11; k++)
            // {
            //   printf("%c",cluster[32 * i + k]);
            // }
            // char addingPath[30] =
            int r = 2;
            for (int k = 7; k >= 1; k--)
            {
                if (cluster[32 * i + k] != 32)
                {
                    r = k + 1;
                    break;
                }
            }
            strcat(path, "/");

            memcpy(c, &cluster[32 * i], r);
            // printf(" c: %s %d" ,c ,strlen(c));
            strcat(path, c);

            printf("(d)");
            printf("%s", path);

            printf("\n");
            unsigned int a;
            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
            //     printf("a: %d\n",a);
            unsigned int b1 = 1;
            unsigned int b2 = 1;
            if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 32 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
            {
                b1 = 0;
            }

            if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 46 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
            {
                b2 = 0;
            }
            //printf (" b2:%d b1:%d a:%d n:%d \n ",b2,b1,a,n);
            if ((b2 && b1) && (a != n))
            {
                // char addingPath2[30];
                //   strncpy(addingPath2,cluster+32*i,8);

                // path = strcat(path,"/");
                // path++;
                readclusterForDirectoryAndFiles(fd, cluster, a, path);

                // path--;
            }
            strcpy(path, oldPath);
            // printf("\n");
        }
    }
    free(c);
}



void readclusterForDirectoryAndFilesForQuestion12(int fd, unsigned char *cluster,
                                     unsigned int n, unsigned char *path,unsigned int clusterNo,unsigned int* exist)
{
    
    char oldPath[100] ="";
   
    
    

    int icerde = -1;
    strcpy(oldPath, path);
    unsigned char *c;
    c = (unsigned char *)calloc(12, sizeof(unsigned char));

    unsigned char *chr;
    chr = (unsigned char *)calloc(12, sizeof(unsigned char));
    *chr = '.';
    unsigned int a;
   
    for (int i = 0; i < 32; i++)
    {
        
        readcluster(fd, cluster, n);
       // printf( "n:%d   i:%d  clusterNo:%d \n",n,i,clusterNo);
        if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 32)
        {
            
             
              a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
            
            strcat(path, "/");
            getNameFromClusterOrder(cluster,i,c,chr);
            strcat(path, c);
            if ( clusterNo == a)
            {
                //getNameFromClusterOrder(cluster,i,c,chr);
                if ( ! ( cluster[32 * i+28] == 0 && cluster[32 * i +29] == 0 && cluster[32 * i + 30] == 0 && cluster[32 * i + 31] == 0 ))
                    {
               // getNameFromClusterOrder(cluster,i,c,chr);
                     printf("%s \n",path);
                     *exist =1;
                     strcpy(path, oldPath);
                     break;
                    }
                
        
            }
            strcpy(path, oldPath);
           // printf("\n");
        }
        else if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 16)
        {
            // klasor
            
            int r = 2;
            for (int k = 7; k >= 1; k--)
            {
                if (cluster[32 * i + k] != 32)
                {
                    r = k + 1;
                    break;
                }
            }
           
            
            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
             //    printf("a: %d    clusterNo: %d \n",a,clusterNo);
            strcat(path, "/");
            getNameFromClusterOrder(cluster,i,c,chr);
            strcat(path, c);
            
            unsigned int b1 = 1;
            unsigned int b2 = 1;
            if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 32 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
            {
                b1 = 0;
            }

            if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 46 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
            {
                b2 = 0;
            }
            
            if ((b2 && b1) && (a != n))
            {
                if ( clusterNo == a)
                {
                    
                    
               // getNameFromClusterOrder(cluster,i,c,chr);
                     printf("%s \n",path);
                    *exist = 1;
                    strcpy(path, oldPath);
                     break;
                    
              //  printf("%s",path);
                }
                // char addingPath2[30];
                //   strncpy(addingPath2,cluster+32*i,8);
                else
                {
                    icerde = 1;
                    readclusterForDirectoryAndFilesForQuestion12(fd, cluster, a, path,clusterNo,exist);
                }
                // path = strcat(path,"/");
                // path++;
              //  readclusterForDirectoryAndFilesForQuestion12(fd, cluster, a, path,clusterNo,exist);
                //strcpy(path, oldPath);
                // path--;
                
            }
            
            // printf("\n");
        }
        else if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 8)
        {
            //printf("/\n");
            
        }
        strcpy(path, oldPath);
    }
    free(c);
    free(chr);
    //free(c);
}





void readclusterForDirectoryAndFiles2(int fd, unsigned char *cluster, unsigned char *sector,
                                      unsigned int n, unsigned int m, unsigned char *path)
{
    char oldPath[100];
    strcpy(oldPath, path);
    unsigned char *c;
    c = (unsigned char *)calloc(12, sizeof(unsigned char));
    //      c[0] =0;
    for (int j = n; j <= m; j++)
    {
        for (int i = 0; i < 32; i++)
        {
            readcluster(fd, cluster, j);
            if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 32)
            {
                // dosya
                printf("(f)");
                printf("%s", path);
                printf("/");
                for (int k = 0; k < 8; k++)
                {

                    if (cluster[32 * i + k] == 32)
                    {
                        break;
                    }

                    printf("%c", cluster[32 * i + k]);
                }
                printf(".");
                for (int k = 8; k < 11; k++)
                {

                    printf("%c", cluster[32 * i + k]);
                }
                //   printf("%s",path);
                // char addingPath[30];
                // strncpy(addingPath,cluster+32*i,8);
                // strcat(path,"/");
                // printf("%s",path);
                printf("\n");
            }
            else if (cluster[32 * i] != 229 && cluster[32 * i + 11] == 16)
            {
                // klasor
                // printf("/");

                // for (int k = 0; k < 11; k++)
                // {
                //   printf("%c",cluster[32 * i + k]);
                // }
                // char addingPath[30] =
                int n = 2;
                for (int k = 7; k >= 1; k--)
                {
                    if (cluster[32 * i + k] != 32)
                    {
                        n = k + 1;
                        break;
                    }
                }
                strcat(path, "/");
                memcpy(c, &cluster[32 * i], n);
                // printf(" c: %s %d" ,c ,strlen(c));
                strcat(path, c);

                printf("(d)");
                printf("%s", path);

                printf("\n");
                unsigned int a;
                a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                //     printf("a: %d\n",a);
                unsigned int b1 = 1;
                unsigned int b2 = 1;
                if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 32 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                    cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
                {
                    b1 = 0;
                }

                if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 46 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
                    cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
                {
                    b2 = 0;
                }
                if ((b2 && b1) && (a != n))
                {
                    // char addingPath2[30];
                    //   strncpy(addingPath2,cluster+32*i,8);

                    // path = strcat(path,"/");
                    // path++;
                    // unsigned int c = findFinishOfStartCLuster(fd,sector,a);
                    readclusterForDirectoryAndFiles2(fd, cluster, sector, a, a, path);

                    // path--;
                }
                strcpy(path, oldPath);
                // printf("\n");
            }
        }
    }
    free(c);
}

void readDirectoryAndFiles(int fd, unsigned char *cluster,
                           unsigned int n)
{

    for (int i = 0; i < 32; i++)
    {

        if (cluster[32 * i] == 46 && cluster[32 * i + 1] == 46 && cluster[32 * i + 2] == 32 && cluster[32 * i + 3] == 32 &&
            cluster[32 * i + 5] == 32 && cluster[32 * i + 6] == 32 && cluster[32 * i + 7] == 32 && cluster[32 * i + 4] == 32)
        {
            printf("/");
            for (int k = 0; k < 11; k++)
            {
                printf("%c", cluster[32 * i + k]);
            }
            unsigned int a;
            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
            if (a != 0)
            {
                readDirectoryAndFiles(fd, cluster, a);
            }
        }
    }
}

void listFilesAndDirectoriesForCluster(  unsigned char* cluster)
{
    unsigned char *c;
    unsigned char *chr;
    chr = (unsigned char *)calloc(1, sizeof(unsigned char));
    *chr = '.';
    struct msdos_dir_entry *dep;
    dep = (struct msdos_dir_entry *) cluster;
    for( int i = 0; i <32; i++)
    {
        
    
        //int seconds; (time & 31) * 2;
        int minute ;
        int hour;  
        int day;   
        int month; 
        int year;  
        unsigned int a;

        
        c = (unsigned char *)calloc(12, sizeof(unsigned char));
        if ( cluster[32*i+11] == 16)
        {
            printf("(d)  name:");

            getNameFromClusterOrder(cluster,i,c,chr);

            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
            printf("%s    fcn= %d    ",c,a);

            year = (dep->date>>9 & 127) +1980; 
            month = (dep->date>>5 & 15) ; 
            day = (dep->date  & 15);
            hour = (dep->time  >> 11) & 31;
            minute = (dep->time  >> 5) & 63;
                printf("size(bytes)= %d  ",  dep->size);
          //  printf("date: %d ",    dep->date);
            //printf("date: %d ",    year);
            //printf("time: %d \n",  dep->time);

            printf("date= %d-%d-%d:%d:%d\n",day,month,year,hour,minute);
        }
        else if ( cluster[32*i+11] == 32)
        {
            printf("(f)  name:");

            getNameFromClusterOrder(cluster,i,c,chr);

            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
            printf("%s    fcn= %d    ",c,a);


               // printf("size(bytes)= %d  ",  dep->size);
            year = (dep->date>>9 & 127) +1980; 
            month = (dep->date>>5 & 15) ; 
            day = (dep->date  & 15);
            hour = (dep->time  >> 11) & 31;
            minute = (dep->time  >> 5) & 63;
                printf("size(bytes)= %d  ",  dep->size);
          //  printf("date: %d ",    dep->date);
            //printf("date: %d ",    year);
            //printf("time: %d \n",  dep->time);

            printf("date= %d-%d-%d:%d:%d\n",day,month,year,hour,minute);
       
        }
        else if ( cluster[32*i+11] == 8)
        {
            printf("(v)  name:");

            getNameFromClusterOrder(cluster,i,c,chr);

            a = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
          //  printf("%s    fcn= %d    ",c,a);

            printf("%s    fcn= 2    ",c);
             //   printf("size(bytes)= %d  ",  dep->size);
            year = (dep->date>>9 & 127) +1980; 
            month = (dep->date>>5 & 15) ; 
            day = (dep->date  & 15);
            hour = (dep->time  >> 11) & 31;
            minute = (dep->time  >> 5) & 63;
                printf("size(bytes)= %d  ",  dep->size);
          //  printf("date: %d ",    dep->date);
            //printf("date: %d ",    year);
            //printf("time: %d \n",  dep->time);

            printf("date= %d-%d-%d:%d:%d\n",day,month,year,hour,minute);
       
        }
        dep++;
        free(c);

    }
    free(chr);



}

int main(int argc, char *argv[])
{
    unsigned char sector[SECTORSIZE] ="";
    unsigned char sector2[SECTORSIZE] = ""; // sector for info
    unsigned char cluster[CLUSTERSIZE] = "";

    unsigned char sector3[SECTORSIZE] = "";



    if(argv[1][1]=='h'){
		printf("Operations: \n");
		printf("fat DISKIMAGE -v                   : Display some summary information about the specified FAT32 volume DISKIMAGE.\n");
		printf("fat DISKIMAGE -s SECTORNUM         : Display the content (byte sequence) of the specified sector to screen in hex form\n");
		printf("fat DISKIMAGE -c CLUSTERNUM        : Display the content (byte sequence) of the specified cluster to the screen in hex form.\n");
		printf("fat DISKIMAGE -t                   : Display all directories and their files and subdirectories starting from the root directory\n");
		printf("fat DISKIMAGE -a PATH              : Display the content of the ascii text file indicated with PATH to the screen as it is.\n");
		printf("fat DISKIMAGE -b PATH              : Display the content (byte sequence) of the file indicated with PATH to the screen in hex form \n");
		printf("fat DISKIMAGE -l PATH              : Display the names of the files and subdirectories in the directory indicated with PATH.\n");
		printf("fat DISKIMAGE -n PATH              : Display the numbers of the clusters storing the content of the file or directory indicated with PATH.\n");
		printf("fat DISKIMAGE -d PATH              : Display the content of the directory entry of the file or directory indicated with PATH.\n");
		printf("fat DISKIMAGE -f COUNT             : Display the content of the FAT table. The first COUNT entries will be printed out.\n");
		printf("fat DISKIMAGE -r PATH OFFSET COUNT : Display the bytes read COUNT bytes from the file indicated with PATH starting at OFFSET \n");
		printf("fat DISKIMAGE -m COUNT             : Display map of the volume \n");
		printf("fat -h                             : Display all 13 operations, their usage and explanation\n");
		return 0;		
	}

    char* diskImage = argv[1];	
	char* op = argv[2];
    int sectornum;
	int clusternum;
	char* pathInput ;
	int countInput;
	int offsetInput;

	int arg = -1;

	//printf("disk image: %s\n",diskImage);
	//printf("op: %s\n",op);


    

	switch (opToChar(op)){
	case 'v':
		arg = 1;
		break;

	case 's':
		arg = 2;
		sectornum = atoi(argv[3]);
	//	printf("sectornum: %d\n",sectornum);
		break;

	case 'c':
		arg = 3;
		clusternum = atoi(argv[3]);
	//	printf("clusternum: %d\n",clusternum);
		break;

	case 't':
		arg = 4;
		break;

	case 'a':
		arg = 5;
		pathInput = argv[3];
	//	printf("path: %s\n",pathInput);
		break;

	case 'b':
		arg = 6;
		pathInput = argv[3];
	//	printf("path: %s\n",pathInput);
		break;

	case 'l':
		arg = 7;
		pathInput = argv[3];
	//	printf("path: %s\n",pathInput);
		break;

	case 'n':
		arg = 8;
		pathInput = argv[3];
	//	printf("path: %s\n",pathInput);
		break;

	case 'd':
		arg = 9;
		pathInput = argv[3];
	//	printf("path: %s\n",pathInput);
		break;

	case 'f':
		arg = 10;
		countInput = atoi(argv[3]);
	//	printf("count: %d\n",countInput);
		break;

	case 'r':
		arg = 11;
		pathInput = argv[3];
		offsetInput = atoi(argv[4]);
		countInput = atoi(argv[5]);

	//	printf("path: %s\n",pathInput);
	//	printf("offset: %d\n",offsetInput);
	//	printf("count: %d\n",countInput);
		break;

	case 'm':
		arg = 12;
		countInput = atoi(argv[3]);
	//	printf("count: %d\n",countInput);
		break;
	
	default:
		printf("invalid operation \n");
		return -1;
		break;
	}












    int fd = open(diskImage, O_SYNC | O_RDONLY); // disk fd

    readsector(fd, sector, 0); // read sector #0

    bp = (struct fat_boot_sector *)sector;

    readsector(fd, sector2, bp->fat32.info_sector);

    deneme = (struct fat_boot_fsinfo *)sector2;

    sectors_per_cluster = bp->sec_per_clus;
    num_sectors = bp->total_sect;
    num_fats = bp->fats;
    sectors_per_fat = bp->fat32.length;
    root_start_cluster = bp->fat32.root_cluster;
    reserved_no = bp->reserved;

    cluster_count = (sectors_per_fat * 512 / 4);
    //int arg = 12;
    data_start_sector = 32 + 2 * sectors_per_fat;
    if (1 == arg)
    {
        printf("File System Type: ");
        for (int i = 0; i < 8; i++)
        {

            printf("%c", bp->fat32.fs_type[i]);
        }

        printf("\nVolume Label: ");
        for (int i = 0; i < 11; i++)
        {

            printf("%c", bp->fat32.vol_label[i]);
        }

        printf("\nNumber of Sectors in disk: %d\n", num_sectors);

        printf("Sector size in bytes: %d\n", SECTORSIZE);

        printf("Number Of Reserved Sectors: %d\n", reserved_no);

        printf("Number of sectors per Fat table: %d\n", bp->fat32.length);

        printf("Number of Fat Tables: %d\n", num_fats);

        printf("NUmber of sectors per cluster: %d\n", sectors_per_cluster);

        printf("Number of Clusters: %d\n", cluster_count);

        printf("Data Region Starts at sector: %d\n", data_start_sector);

        printf("Root Directory Starts at sector: %d\n", data_start_sector);

        printf("Root Directory Starts at cluster: %d\n", bp->fat32.root_cluster);

        printf("Disk Size in Bytes: %d bytes \n", (num_sectors * SECTOR_SIZE));

        printf("Disk Size in megabytes: %d megabytes \n", (num_sectors * SECTOR_SIZE) / (1024 * 1024));

        printf("Number of used clusters: %d \n", cluster_count - deneme->free_clusters);

        printf("Number of free clusters: %d \n", deneme->free_clusters);
    }


    int numFor2 = sectornum;
    readsector(fd, sector, numFor2);
    if (2 == arg)
    {
        for (int i = 0; i < SECTORSIZE / 16; i++)
        {

            printf("%06x :", numFor2 * 32 + i);
            for (int j = 0; j < 16; j++)
            {
                if (sector[16 * i + j] == 0)
                {
                    printf("00 ");
                }
                else if (sector[16 * i + j] < 16)
                {
                    printf("0%x ", sector[16 * i + j]);
                }
                else
                {
                    printf("%x ", sector[16 * i + j]);
                }
            }

            printf("    ");
            for (int j = 0; j < 16; j++)
            {
                if (isprint(sector[16 * i + j]))
                {
                    printf("%c", sector[16 * i + j]);
                }
                else
                    printf(".");
            }
            printf("    \n");
        }
    }
    printf("\n");

    int numFor3 = clusternum;
    readcluster(fd, cluster, numFor3);
    if (3 == arg)
    {
        for (int i = 0; i < CLUSTERSIZE / 16; i++)
        {

            printf("%06x :", (numFor3 - 2) * 32 + i);
            for (int j = 0; j < 16; j++)
            {
                if (cluster[16 * i + j] == 0)
                {
                    printf("00 ");
                }
                else if (cluster[16 * i + j] < 16)
                {
                    printf("0%x ", cluster[16 * i + j]);
                }
                else
                {
                    printf("%x ", cluster[16 * i + j]);
                }
            }

            printf("    ");
            for (int j = 0; j < 16; j++)
            {
                if (isprint(cluster[16 * i + j]))
                {
                    printf("%c", cluster[16 * i + j]);
                }
                else
                    printf(".");
            }
            printf("    \n");
        }
    }
    unsigned char path[1024] = " ";

    unsigned int path2 = 0;
    if (4 == arg)
    {

        readclusterForDirectoryAndFiles(fd, cluster, 2, path);
    }

    //    readsector(fd, sector23, bp->fat32.info_sector);
    // printf("sayı: %d", findFinishOfStartCLuster(fd,sector3,2));
    









    char arr[11][10] ;

    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            arr[i][j] = 0;
        }
    }

    if (5 == arg)
    {
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] = "";  
        strcpy(example,pathInput);
       // *example = pathInput;
       // unsigned char example[110] =  *pathInput;//"/DIR2/F1.TXT";
        unsigned int resultArr[32] ;


        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }

        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
           //     printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;
        int sayac = 0;
        unsigned int debug =3;
      //  printf("calıssssss\n");
        for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 10; j++)
        {
       //     printf("arr %c \n",arr[i][j]);
        }
    }

        unsigned char result[11]="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            sayac++;
            readcluster(fd, cluster, numForLoop);
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
                //printf("debug:%d numforloop: %d \n" ,debug,numForLoop);
               // printf("arrcounter: %s\n",arr[counter]);
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
           //      printf("result:%s\n",result);
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
            //    printf("\narr counter: %s %d\n", arr[counter], strlen(arr[counter]));
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    if (cluster[32 * i + 11] == 16)
                    {

                        exist = 1;
                        counter++;
                        break;
                    }
                    else
                    {
                        exist =1;
                        counter++;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                }
            }
            if (exist == -1)
            {
                //printf("wrong path \n");
            }
        }
        int j = 0;
        while (resultArr[j] != 0)
        {
            readcluster(fd,cluster,resultArr[j]);
            for ( int k = 0; k < CLUSTERSIZE; k++)
            {

                printf( "%c" ,cluster[k]);
            }
            j++;
        }
        
    }




    if ( 6== arg)
    {
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] = "";
        strcpy(example,pathInput);
        unsigned int resultArr[32];



        
        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }

        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
           //     printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;
        int sayac = 0;
        unsigned int debug =3;


        unsigned char result[11]="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            sayac++;
            readcluster(fd, cluster, numForLoop);
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
                
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
               //  printf("result:%s\n",result);
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
               // printf("\narr counter: %s %d\n", arr[counter], strlen(arr[counter]));
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    if (cluster[32 * i + 11] == 16)
                    {

                        exist = 1;
                        counter++;
                        break;
                    }
                    else
                    {
                        exist =1;
                        counter++;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                }
            }
            if (exist == -1)
            {
                //printf("wrong path \n");
            }
        }
        int j = 0;
        while (resultArr[j] != 0)
        {
            readcluster(fd,cluster,resultArr[j]);
            

            for (int i = 0; i < CLUSTERSIZE / 16; i++)
        {

            printf("%06x :", (resultArr[j] - 2) * 32 + i);
            for (int j = 0; j < 16; j++)
            {
                if (cluster[16 * i + j] == 0)
                {
                    printf("00 ");
                }
                else if (cluster[16 * i + j] < 16)
                {
                    printf("0%x ", cluster[16 * i + j]);
                }
                else
                {
                    printf("%x ", cluster[16 * i + j]);
                }
            }

            printf("    ");
            for (int j = 0; j < 16; j++)
            {
                if (isprint(cluster[16 * i + j]))
                {
                    printf("%c", cluster[16 * i + j]);
                }
                else
                    printf(".");
            }
            printf("    \n");
        }



            j++;
        }
    }
    // always check the return values



    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            arr[i][j] = 0;
        }
    }
    if ( 7== arg)
    {
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] = "";
        strcpy(example,pathInput);
        unsigned int resultArr[32];

        if( strlen(example) == 1)
        {

            readcluster(fd,cluster,2);
         
             listFilesAndDirectoriesForCluster(cluster);
        }
        else
        {
        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }
    //    printf( "length: %d\n",strlen(example));
        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
             //   printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;

        unsigned int debug =3;


        unsigned char result[11]="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            
            readcluster(fd, cluster, numForLoop);
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
             //   printf("debug:%d numforloop: %d \n" ,debug,numForLoop);
              //  printf("arrcounter: %s\n",arr[counter]);
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
               //  printf("result:%s\n",result);
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
               // printf("\narr counter: %s %d\n", arr[counter], strlen(arr[counter]));
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    counter ++;
                    exist =1;
                }
            }
            if (exist == -1)
            {
                //printf("wrong path \n");
            }
        }
        
            readcluster(fd,cluster,numForLoop);
         
        listFilesAndDirectoriesForCluster(cluster);

        }
    }


    if (8 == arg)
    {
        
        
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] = "";
        strcpy(example,pathInput);
        unsigned int resultArr[32];
        if( strlen(example) == 1)
        {

            printf("cindex:0  clusterNum:2\n");

        }
        else
        {
        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }

        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
             //   printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;
        int sayac = 0;
        unsigned int debug =3;


        unsigned char result[11]="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            sayac++;
            readcluster(fd, cluster, numForLoop);
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
                
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
                 
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
                
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    if (cluster[32 * i + 11] == 16)
                    {

                        exist = 1;
                        counter++;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                    else
                    {
                        exist =1;
                        counter++;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                }
            }
            if (exist == -1)
            {
               // printf("wrong path \n");
            }
        }





        for (int i = 0; i < 32; i++)
        {
            if ( resultArr[i] == 0)
            {
                break;
            }
            printf("cindex:%d  clusterNum:%d\n", i,resultArr[i]);
        }
        }
        //  readclusterForDirectoryAndFiles2(fd, cluster,sector3, 2,finishCluster,path);
        // printf("sayı: %d", findFinishOfStartCLuster(fd,sector3,17));
    }


    if ( 9 == arg)
    {
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] ="" ;
        strcpy(example,pathInput);
        unsigned int resultArr[32];
        readcluster(fd,cluster,2);
        struct msdos_dir_entry *dep;
          dep = (struct msdos_dir_entry *) cluster;
          int minute ;
        int hour;  
        int day;   
        int month; 
        int year;  
        if( strlen(example) == 1)
        {
          printf("name = %s\n",dep->name);

        printf("type = VOLUME\n");
        printf("first cluster = 2\n");

        printf("size(bytes)= 0\n");
        year = (dep->date>>9 & 127) +1980; 
            month = (dep->date>>5 & 15) ; 
            day = (dep->date  & 15);
            hour = (dep->time  >> 11) & 31;
            minute = (dep->time  >> 5) & 63;
        //printf("time %d\n",dep->time);
        printf("date = %d-%d-%d\n",day,month,year);
        printf("time = %d:%d\n",hour,minute);
       // printf("date %d\n",dep->date);
            //printf("You should enter FILE or DIRECTORY for that question \n");
        }
        else
        {
        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }

        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
             //   printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;
        int sayac = 0;
        unsigned int debug =3;

        struct msdos_dir_entry *dep;
                    dep = (struct msdos_dir_entry *) cluster;
        unsigned char result[11]="";
        unsigned char fileType[11]="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            sayac++;
            readcluster(fd, cluster, numForLoop);
            dep = (struct msdos_dir_entry *) cluster;
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
                
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
                 
                 getTypeFromClusterOrder(cluster,i,fileType);
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
                
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    
                    
                    if (cluster[32 * i + 11] == 16)
                    {

                        exist = 1;
                        counter++;
                        i=32;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                    else
                    {
                        exist =1;
                        counter++;
                        i =32;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                }
                dep++;
            }
            if (exist == -1)
            {
                //printf("wrong path \n");
            }
        }


        printf("name = %s\n",result);

        printf("file type = %s\n",fileType);
        printf("first cluster = %d\n",resultArr[0]);

        for (int i = 0; i < 32; i++)
        {
            if ( resultArr[i] == 0)
            {
                printf("cluster count = %d\n", i);
                break;
            }
            
        }

        printf("size(bytes) = %d\n",dep->size);
        year = (dep->date>>9 & 127) +1980; 
            month = (dep->date>>5 & 15) ; 
            day = (dep->date  & 15);
            hour = (dep->time  >> 11) & 31;
            minute = (dep->time  >> 5) & 63;
        //printf("time %d\n",dep->time);
        printf("date = %d-%d-%d\n",day,month,year);
        printf("time = %d:%d\n",hour,minute);
       // printf("time %d\n",dep->time);

       // printf("date %d\n",dep->date);
        
        }
        //  readclusterForDirectoryAndFiles2(fd, cluster,sector3, 2,finishCluster,path);
        // printf("sayı: %d", findFinishOfStartCLuster(fd,sector3,17));
    }



    unsigned int firstNEntries = 150;
    if ( countInput != -1 )
    {
    firstNEntries = countInput;
    }
    else
    {
        firstNEntries = 1016*128-1;
    }
    if( 10 == arg)
    {
        unsigned int finishSector;
        unsigned int remain;
        finishSector = 32 + (firstNEntries/128);
        remain = firstNEntries % 128;
        unsigned int entryNoPerSector = 128;
        for ( unsigned int i = 32; i <= finishSector; i++ )
        {
            readsector(fd,sector,i);
            if ( i == finishSector)
            {
                entryNoPerSector = remain;
            }
            for ( unsigned int j = 0; j <entryNoPerSector; j++)
            {
             printf("%06d :", j+ (i-32)*128);
             if( sector[4*j] == 255 && sector[4*j+1] ==255 &&  sector[4*j+2]==255 && sector[4*j+3]== 15
              || ( sector[4*j] == 248 && sector[4*j+1] ==255 &&  sector[4*j+2]==255 && sector[4*j+3]== 15))
             {
                printf("EOF\n");
             }
             else
             printf("%d \n",sector[4*j]+ sector[4*j+1]*256+ sector[4*j+2]*256*256 + sector[4*j+3]*256*256*256);
            }
        }

    }


    unsigned int OFFSET = offsetInput;
    unsigned int COUNT = countInput;
    unsigned int firstClusterToRead;
    unsigned int startByte;
    unsigned int finishByte;
    unsigned int lastClusterToRead;
    firstClusterToRead = OFFSET/CLUSTERSIZE;
    startByte = OFFSET%CLUSTERSIZE;
    lastClusterToRead = (COUNT + OFFSET)/CLUSTERSIZE;
    finishByte = (COUNT + OFFSET)%CLUSTERSIZE;
    if ( 11 == arg)
    {
        unsigned int oldcount = 0;
        unsigned int count = 0;
        unsigned char example[110] = "";
        strcpy(example,pathInput);
        unsigned int resultArr[32];



        
        for (int i = 0; i < 32; i++)
        {
            resultArr[i] = 0;
        }

        for (int i = 0; i < 110; i++)
        {

            if (i != 0 && example[i] == '/')
            {
                memcpy(arr[oldcount], &example[count + 1], i - count - 1);
               // printf("i:%d", i);
                oldcount++;
                count = i;
            }
        }
        memcpy(arr[oldcount], &example[count + 1], strlen(example) - count - 1);
        oldcount++;
        int counter = 0;
        // printf("l%ld",strlen(example));
        int finishLoop = 1;
        unsigned int numForLoop = 2;
        int exist = -1;
        int sayac = 0;
        unsigned int debug =3;


        unsigned char result[11] ="";
         unsigned char chr ='.'; 
        while (counter < oldcount ) //&& sayac < 3)
        {
            sayac++;
            readcluster(fd, cluster, numForLoop);
            for (int i = 0; i < 32; i++)
            {
                exist = -1;

                int sameDir = -1;
                
              //  chr = '.';
                 getNameFromClusterOrder(cluster,i,result,&chr);
               //  printf("result:%s\n",result);
                 if ( strcmp(result,arr[counter]) == 0 )
                 {
                     sameDir = 1;
                 }
               // printf("\narr counter: %s %d\n", arr[counter], strlen(arr[counter]));
                if (sameDir == 1)
                {
                    debug  ++;
                    numForLoop = 256 * 256 * cluster[32 * i + 20] + 256 * 256 * 256 * cluster[32 * i + 21] + 1 * cluster[32 * i + 26] + 256 * cluster[32 * i + 27];
                    if (cluster[32 * i + 11] == 16)
                    {

                        exist = 1;
                        counter++;
                        break;
                    }
                    else
                    {
                        exist =1;
                        counter++;
                        findFinishOfStartCLuster(fd, sector3, numForLoop, resultArr);
                        break;
                    }
                }
            }
            if (exist == -1)
            {
                //printf("wrong path \n");
            }
        }
        int j = firstClusterToRead;
        while (resultArr[j] != 0 && j<= lastClusterToRead)
        {
            readcluster(fd,cluster,resultArr[j]);
            
            if ( j == firstClusterToRead && j == lastClusterToRead)
            {
                for ( int i = 0; i< 1+(finishByte-startByte)/16; i++)
                {
                for ( int k = 0; k < 16; k++)
                {
                    if ( k % 16 == 0)
                    {
                        printf("%06x :",  startByte+16*i);
                    }
                    if ( finishByte > startByte+i*16+k )
                    printf("%02x ",cluster[startByte+i*16+k]);

                    else
                    {
                        printf("   ");
                    }
                    
                }
                
                printf("    ");
                for ( int k = 0; k < 16; k++)
                {
                    if ( finishByte > startByte+i*16+k )
                   {
                    if (isprint(cluster[startByte+i*16+k]))
                {
                    printf("%c", cluster[startByte+i*16+k]);
                }
                else
                    printf(".");
                }
                }
                printf("\n");
                }
            }
            else if  ( j == firstClusterToRead && j != lastClusterToRead)
            {
                
                for ( int i = 0; i< 1+(1023-startByte)/16; i++)
                {
                for ( int k = 0; k < 16; k++)
                {
                    if ( k % 16 == 0)
                    {
                        printf("%06x :",  startByte+16*i);
                    }
                    if ( 1023 > startByte+i*16+k )
                    printf("%02x ",cluster[startByte+i*16+k]);

                    else
                    {
                        printf("   ");
                    }
                    
                }
                
                printf("    ");
                for ( int k = 0; k < 16; k++)
                {
                    if ( 1023 > startByte+i*16+k )
                   {
                    if (isprint(cluster[startByte+i*16+k]))
                {
                    printf("%c", cluster[startByte+i*16+k]);
                }
                else
                    printf(".");
                }
                }
                printf("\n");
                }
            }
            else if ( j != firstClusterToRead && j == lastClusterToRead)
            {
                for ( int i = 0; i< 1+(finishByte-0)/16; i++)
                {
                for ( int k = 0; k < 16; k++)
                {
                    if ( k % 16 == 0)
                    {
                        printf("%06x :",  0+16*i);
                    }
                    if ( finishByte > 0+i*16+k )
                    printf("%02x ",cluster[0+i*16+k]);

                    else
                    {
                        printf("   ");
                    }
                    
                }
                
                printf("    ");
                for ( int k = 0; k < 16; k++)
                {
                    if ( finishByte > 0+i*16+k )
                   {
                    if (isprint(cluster[0+i*16+k]))
                {
                    printf("%c", cluster[0+i*16+k]);
                }
                else
                    printf(".");
                }
                }
                printf("\n");
                }

                
            }
            else
            {
                for ( int i = 0; i< 1+(1023-0)/16; i++)
                {
                for ( int k = 0; k < 16; k++)
                {
                    if ( k % 16 == 0)
                    {
                        printf("%06x :",  0+16*i);
                    }
                    if ( 1023 > 0+i*16+k )
                    printf("%02x ",cluster[0+i*16+k]);

                    else
                    {
                        printf("   ");
                    }
                    
                }
                
                printf("    ");
                for ( int k = 0; k < 16; k++)
                {
                    if ( 1023 > 0+i*16+k )
                   {
                    if (isprint(cluster[0+i*16+k]))
                {
                    printf("%c", cluster[0+i*16+k]);
                }
                else
                    printf(".");
                }
                }
                printf("\n");
                }
            }


            j++;
        }

    }


    //printf("dadad: %d\n" , findFirstOfClustersOnTheFile(130,cluster,fd));
    if ( 12 == arg)
    {
        unsigned char pathFor12[1024] = "";
       // strcpy(pathFor12,"");
        unsigned int b;
        unsigned int exist = -1;
        if( countInput == -1)
        {
           countInput = 1016*128-1; // all clusters
           
        }
        for ( int i = 0; i <countInput; i++)
        {
            if ( i < 2)
            { 
                printf( "%06d :  ",i);
                printf("EOF\n\n");
            }
            else if ( i == 2)
            {
                printf( "%06d :  ",i);
                printf("/ \n\n");
            }
            else
            {
    //    printf("i %d: %d\n",i,findFirstOfClustersOnTheFile(i,sector,fd));
             b = findFirstOfClustersOnTheFile(i,sector,fd);
            printf( "%06d :  ",i);
            memset(pathFor12,1024,0);
            exist = -1;
            readclusterForDirectoryAndFilesForQuestion12(fd,cluster,2,pathFor12,b,&exist);
            if ( exist == -1)
            {
                printf("free\n");
            }
            printf("\n");
            }

        }
    }


}



