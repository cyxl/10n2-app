#include <t_test.h>
#include <stdio.h>
#include <nuttx/signal.h>


static bool running = false;

void* do_stuff(void* args)
{

    while(running)
    {
        nxsig_usleep(500000); /* usecs (arbitrary) */
        printf("running\n");

    }
}

int t_test_start(pthread_t* t)
{
    if (!running)
    {
        running = true;
        if (!pthread_create(t,NULL,&do_stuff,NULL))
        {
            return -1;
        }
    }
    else{
        return -1;
    }
    return 0;

}
int t_test_stop(pthread_t* t)
{
    if (running)
    {
        running = false;
        if (!pthread_join(t,NULL))
        {
            printf("error joining!!\n");
        }
    }
    return 0;
}