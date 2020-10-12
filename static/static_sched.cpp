#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <cmath>
using std::string;
#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif
struct Parameter
{
  float result;
  float sum;
  int ub;
  int lb;
  int functionid;
  int intensity;
  string sync;
  float a;
  float b;
  float step_size;
  float global_result;

};
pthread_mutex_t mut;
float total_sum = 0;

void *static_worker_thread(void *p){
    Parameter *m = (Parameter *) p;
    if(0 == m->sync.compare("iteration"))
{
  switch(m->functionid)
  {
    case 1:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
          //lock mutex
        pthread_mutex_lock(&mut);

        total_sum +=f1(x, m->intensity);
          //unlock mutex
          pthread_mutex_unlock(&mut);
      }
      break;

    case 2:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
          //lock mutex

        pthread_mutex_lock(&mut);
          //unlock mutex

        total_sum +=f2(x, m->intensity);
	pthread_mutex_unlock(&mut);
      }
      break;

    case 3:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        pthread_mutex_lock(&mut);
          //lock mutex


        total_sum +=f3(x, m->intensity);
          //unlock mutex

          pthread_mutex_unlock(&mut);
      }
      break;

    case 4:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        pthread_mutex_lock(&mut);
          //lock mutex


        total_sum +=f4(x, m->intensity);
          //unlock mutex after writer
	pthread_mutex_unlock(&mut);
      }
      break;
   }
}
else{
  switch(m->functionid){
    case 1:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        m->sum +=f1(x, m->intensity);
      }
      break;

    case 2:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        m->sum +=f2(x, m->intensity);
      }
      break;

    case 3:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        m->sum +=f3(x, m->intensity);
      }
      break;

    case 4:
      for (int i=m->lb; i<=m->ub; i++){
        float x = (m->a + (i + 0.5) * m->step_size);
        m->sum +=f4(x, m->intensity);
      }
      break;
   }
   }

  return NULL;

}

/*
void *static_worker_iter(void *p){
    Parameter *m = (Parameter *) p;
    pthread_mutex_lock(&mut);
    m.global_result += tmp;
    pthread_mutex_unlock(&mut);
}*/

int main (int argc, char* argv[]) {
    
    if (argc < 8) {
        std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<std::endl;
        return -1;
    }
    
    int functionid = atoi(argv[1]);
    float a = atof(argv[2]);
    float b = atof(argv[3]);
    float n = atof(argv[4]);
    
    int intensity = atoi(argv[5]);
    int num_threads = atoi(argv[6]);
    string sync = argv[7];
    
    float result = 0;
    float step_size = (b - a) / n;
	
    if(0 == sync.compare("iteration")){
    pthread_mutex_init(&mut, NULL);
    }
	
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    
    //calculate your result here
    pthread_t ths[num_threads];
    Parameter params[num_threads];
    
    int steps_each_thread = int(n / num_threads);
   // if ((int) n % num_threads != 0)
    //    steps_each_thread += 1;
    int ni=0;
    for (int i = 0; i < num_threads; i++){
		if(ni<n){
    params[i].a = a;

    params[i].b = b;
    params[i].functionid = functionid;
    params[i].intensity = intensity;

    params[i].lb = ni;
    params[i].step_size = step_size;
    params[i].sum = 0;
    params[i].sync = sync;

    if((i+1) >= num_threads){

      params[i].ub = n - 1;

    }


   else{

     params[i].ub = ni + (steps_each_thread-1);

    }

    pthread_create(&ths[i], NULL, static_worker_thread, (void*) &params[i]);
  }
  ni+=steps_each_thread;
     /*   params[i].intensity = intensity;
        params[i].step_size = step_size;
        params[i].start = a + i * step_size * steps_each_thread;
        params[i].end = a + (i + 1) * step_size * steps_each_thread;
        if (i == num_threads - 1)
            params[i].end = b;
        
        params[i].local_result = 0;
        params[i].global_result = &result;
        
        pthread_create(&ths[i], NULL, static_worker, (void *) &params[i]);
		*/
    }
    
    for (int i = 0; i < num_threads; i++){
        pthread_join(ths[i], NULL);
    }
    if(0 == sync.compare("iteration"))
   {
      pthread_mutex_destroy(&mut);
  }
  else
  {

    for (int i = 0; i < num_threads; i++){
        total_sum+= params[i].sum;
    }
  }
 
  
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    
    std::chrono::duration<double> elapsed_seconds = end-start;
    
    
    // display result and time
    std::cout<<total_sum*step_size<<std::endl;
    std::cerr<<elapsed_seconds.count()<<std::endl;
    
    return 0;
}
