#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string.h>
#include <cstdlib>

using namespace std;

using seconds = chrono::seconds;
using check_time = std::chrono::high_resolution_clock;
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
struct Parameter{
  string sync;
  float a;
  float b;
  float step_size;
  int begin;
  int end;
  int functionid;
  int intensity;
  int n;
};


int step_left = 0, n,global_begin=0, global_end=0, granularity=0;
pthread_mutex_t mut,mut1;
float final_res = 0.0, step_size =0.0;

bool done()
{
  pthread_mutex_lock(&mut1);
  if(step_left == 0)
 {
    pthread_mutex_unlock(&mut1);

    return true;
  }
  else
  {
      if(step_left < granularity)
      {
         step_left = 0;
      }
      else
      {
          step_left = step_left - granularity;
      }
       pthread_mutex_unlock(&mut1);
       return false;
   }

}
void getnext (int *begin, int *end)
{
pthread_mutex_lock(&mut1);
*begin = global_begin;
*end = global_end;
if(granularity <= (n-global_end))
{
	global_begin = *end;
	global_end = global_begin + granularity;
}

pthread_mutex_unlock(&mut1);

}





void* iteration_level_integration(void* p)
{

int begin, end;
  while(!done()){
   Parameter *params = (Parameter*)p;

   getnext(&begin, &end);

   for (int i=begin; i<end; i++){

     float x = (params->a + (i + 0.5) * params->step_size);

     switch(params->functionid){
       case 1:

	 pthread_mutex_lock(&mut);
         final_res +=(f1(x, params->intensity) * params->step_size);

	pthread_mutex_unlock(&mut);
         break;

       case 2:
	pthread_mutex_lock(&mut);
         final_res +=(f2(x, params->intensity) * params->step_size);

	pthread_mutex_unlock(&mut);
         break;

       case 3:
	pthread_mutex_lock(&mut);
         final_res +=(f3(x, params->intensity) * params->step_size);

	pthread_mutex_unlock(&mut);
         break;

       case 4:
	pthread_mutex_lock(&mut);
         final_res +=(f4(x, params->intensity) * params->step_size);

	pthread_mutex_unlock(&mut);
         break;
     }
   }
  }
}

void* chunk_level_integration(void* p)
{
 while(!done()){

   Parameter *params = (Parameter*)p;
   int begin, end;
   float sum = 0.0;
   getnext(&begin, &end);
   for (int i=begin; i<end; i++){
     float x = (params->a + (i + 0.5) * params->step_size);

     switch(params->functionid){
       case 1:
         sum +=(f1(x, params->intensity));
         break;

       case 2:
         sum +=(f2(x, params->intensity));
         break;

       case 3:
         sum +=(f3(x, params->intensity));
         break;

       case 4:
         sum +=(f4(x, params->intensity));
         break;
     }
   }
   pthread_mutex_lock(&mut);

   final_res += sum;
   pthread_mutex_unlock(&mut);
  }

}
void* thread_level_integration(void* p)
{

float sum = 0.0;
   int begin, end;
   Parameter *params = (Parameter*)p;
  while(!done()){
   getnext(&begin, &end);
   for (int i=begin; i<end; i++){
     float x = (params->a + (i + 0.5) * params->step_size);

     switch(params->functionid){
       case 1:
         sum +=(f1(x, params->intensity));
         break;

       case 2:
         sum +=(f2(x, params->intensity));
         break;

       case 3:
         sum +=(f3(x, params->intensity));
         break;

       case 4:
         sum +=(f4(x, params->intensity));
         break;
     }
   }
  }
    pthread_mutex_lock(&mut);
    final_res = final_res + sum;
    pthread_mutex_unlock(&mut);
}

int main (int argc, char* argv[]) {

  if (argc < 9) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <num_threads> <sync> <granularity>"<<std::endl;
    return -1;
  }
    
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
 int functionid = atoi(argv[1]);
  float a = atof(argv[2]);
  float b = atof(argv[3]);
  n = atoi(argv[4]);
  int intensity = atoi(argv[5]);
  float step_size = ((b - a) / n );
  int num_threads = atoi(argv[6]);
  string sync = argv[7];
  granularity = atoi(argv[8]);
  global_end = granularity;
  struct Parameter params[num_threads];
  pthread_t ths[num_threads];
  step_left = n;
  
  // do your calculation here

pthread_mutex_init(&mut, NULL);
    pthread_mutex_init(&mut1, NULL);

    if(0 == sync.compare("iteration"))
    {
    for(int i= 0; i<num_threads;i++)
    {
      params[i].a = a;
      params[i].b = b;
      params[i].functionid = functionid;
      params[i].intensity = intensity;

      params[i].step_size = step_size;
      params[i].sync = sync;
      params[i].n = n;
     pthread_create(&ths[i], NULL, iteration_level_integration, (void*) &params[i]);
    }
    }
    else if(0 == sync.compare("thread"))
    {
     for(int i= 0; i<num_threads;i++)
      {
        params[i].a = a;
        params[i].b = b;
        params[i].functionid = functionid;
        params[i].intensity = intensity;

        params[i].step_size = step_size;
        params[i].sync = sync;
        params[i].n = n;
          
        pthread_create(&ths[i], NULL, thread_level_integration,(void*) &params[i]);
      }

    }
    else if(0 == sync.compare("chunk"))
    {
     for(int i= 0; i<num_threads;i++)
      {
        params[i].a = a;
        params[i].b = b;
        params[i].functionid = functionid;
        params[i].intensity = intensity;

        params[i].step_size = step_size;
        params[i].sync = sync;
        params[i].n = n;
          
        pthread_create(&ths[i], NULL, chunk_level_integration, (void*) &params[i]);
      }

    }

    for(int i=0; i <num_threads; i++)
    {
     pthread_join(ths[i], NULL);
    }

    pthread_mutex_destroy(&mut);
    pthread_mutex_destroy(&mut1);

  // report reult and time
  std::cout<<final_res*step_size<<std::endl;
  std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end_time-start_time;
  std::cerr<<elapsed_seconds.count()<<std::endl;

  return 0;
}
