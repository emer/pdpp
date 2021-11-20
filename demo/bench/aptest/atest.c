#include <stdio.h>
#include <time.h>
#define NCY (100*1000000/(SIZE*SIZE))

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

float warray[SIZE][SIZE];
float wedarray[SIZE][SIZE];
float act[SIZE];
float net[SIZE];

main() {
  clock_t t1,t2 ;
  int i,nflops,et;
  float mflops,secs;
  set_weights();
  t1 = clock();
  for (i = 0; i < NCY; i++) {
    compute_acts();
    compute_weds();
  }
  t2 = clock();
  secs = ((float)(t2 - t1))/ (float)CLOCKS_PER_SEC;
  mflops = 2*2*NCY*(SIZE*SIZE/((float) 1000000.0)) ;
  printf("%d cycles, %d weights, %f mflops in %f secs.\n",
	 NCY,SIZE*SIZE,mflops,secs);
  printf("%f Megaflops\n",mflops/secs) ;

}

compute_weds() {
  int i,j;

  for (i = 0; i < SIZE; i++) {
    for (j = 0; j < SIZE; j++) {
      wedarray[i][j] += act[i]*act[j];
    }
  }
}

compute_acts() {
  int i,j;

   
  for (i = 0; i < SIZE; i++) {
    net[i] = act[i] = (float) 0.1;
    for (j = 0; j < SIZE; j++) {
      net[i] += act[j]*(warray[i][j]);
    }
    act[i] = (net[i] > 1.0) ? net[i] - ((float) ((int) net[i])) : net[i];
  }
}

set_weights() {
  int i,j;

  for (j = 0; j < SIZE; j++) {
    act[j] = j/(float) SIZE;
    for (i = 0; i < SIZE; i++) {
      warray[i][j] = (i+j)/((float) SIZE*SIZE);
      wedarray[i][j] = (float) 0.0;
    }
  }
}
