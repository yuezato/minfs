#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define TV2SEC(tv) ((double)((tv).tv_sec) + (double)((tv).tv_usec / 1000000.0))

int main(int argc, char **argv){
   int fd, flag, i, bufsize, count;
   char *buf;
   char *filename;
   struct timeval tv1, tv2;

   if(argc != 5){
      printf("usage: out_file direct_or_not[1/0], bufsize(x512), count\n");
      exit(1);
   }

   if(atoi(argv[2])){
     flag = O_WRONLY | O_CREAT | O_DIRECT;
      puts("Enable O_DIRECT");
   }else if(atoi(argv[2]) == 0){
      flag = O_WRONLY | O_CREAT;
      puts("Disable O_DIRECT");
   }else{
      printf("usage: out_file direct_or_not[1/0], bufsize(x512), count\n");
      exit(1);
   }

   filename = argv[1];
   bufsize = atoi(argv[3]);
   count = atoi(argv[4]);

   printf("flag: %d, bufsize: %d, count: %d\n", atoi(argv[2]), bufsize, count);

   fd = open(filename, flag, S_IREAD | S_IWRITE);
   if(fd < 0){
      printf("open fail\n");
      exit(1);
   }

   posix_memalign((void **)&buf, 512, bufsize * 512);
   memset((void*)buf, 'a', bufsize * 512);

   gettimeofday(&tv1, 0);
   for(i = 0; i < count ; i++){
      write(fd, buf, bufsize * 512);
   }
   gettimeofday(&tv2, 0);
   printf("time: %f\n", TV2SEC(tv2) - TV2SEC(tv1));

   close(fd);

   return 0;
}
