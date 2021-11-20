#include <stdio.h>
#include <time.h>
#define NCY (100*1000000/(SIZE*SIZE))

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

struct wstr {
  float w;
  float wed;
  struct unit *sp;
} wstr;

struct unit {
  float act;
  float net;
  struct wstr wt[SIZE];
} unit[SIZE];

main() {
  clock_t t2,t1;
  int i,nflops;
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
  struct unit *up;
  struct wstr *wp,*end;

  for (up = &(unit[0]); up < &(unit[0]) + SIZE; up++) {
    for (wp = up->wt,end=up->wt+SIZE; wp < end; wp++) {
      wp->w += up->act  * wp->sp->act;
    }
  }
}

compute_acts() {
  int i,j;
  struct unit *up;
  struct wstr *wp,*end;
   
  for (up = &(unit[0]); up < &(unit[0]) + SIZE; up++) {
    up->net = up->act = (float) 0.1;
    for (wp = up->wt,end=up->wt+SIZE; wp < end; wp++) {
      up->net += wp->w * wp->sp->act;
    }
    up->act = (up->net > 1.0) ? up->net - ((float) ((int) up->net)) : up->net;
  }
}

set_weights() {
  int i,j,ii;
  struct unit *up;
  struct wstr *wp,*end;
  int h = SIZE/2;

  for (j = 0,up = &(unit[0]); up < &(unit[0]) + SIZE; up++,j++) {
    up->act = j/((float) SIZE);
    for (i = 0,wp=up->wt,end=up->wt+SIZE; wp < end; 
	 i++,wp++) {
      ii = (i%2)*h + i%h;
      wp->sp = &(unit[ii]);
      wp->w = (ii+j)/((float) SIZE*SIZE);
      wp->wed = (float) 0.0;
    }
  }
}
