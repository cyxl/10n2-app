
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>

extern "C"
{
#include <asmp/asmp.h>
#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>
}

/* Include worker header */
#include "worker.h"

/* Worker ELF path */
#define WORKER_FILE "/mnt/spif/asmp_test"

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)


extern "C" int main(int argc, FAR char *argv[])
{
  mptask_t mptask;
  mpmutex_t mutex;
  mpshm_t shm;
  mpmq_t mq;
  uint32_t msgdata;
  int data = 0x1234;
  int ret, wret;
  char *buf;

  /* Initialize MP task */

  ret = mptask_init(&mptask, WORKER_FILE);
  if (ret != 0)
    {
      err("mptask_init() failure. %d\n", ret);
      return ret;
    }

  ret = mptask_assign(&mptask);
  if (ret != 0)
    {
      err("mptask_assign() failure. %d\n", ret);
      return ret;
    }

  /* Initialize MP mutex and bind it to MP task */

  ret = mpmutex_init(&mutex, ASMP_TEST_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&mptask, &mutex);
  if (ret < 0)
    {
      err("mptask_bindobj(mutex) failure. %d\n", ret);
      return ret;
    }

  /* Initialize MP message queue with assigned CPU ID, and bind it to MP task */

  ret = mpmq_init(&mq, ASMP_TEST_MQ, mptask_getcpuid(&mptask));
  if (ret < 0)
    {
      err("mpmq_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&mptask, &mq);
  if (ret < 0)
    {
      err("mptask_bindobj(mq) failure. %d\n", ret);
      return ret;
    }

  /* Initialize MP shared memory and bind it to MP task */

  ret = mpshm_init(&shm, ASMP_TEST_SHM, 1024);
  if (ret < 0)
    {
      err("mpshm_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&mptask, &shm);
  if (ret < 0)
    {
      err("mptask_binobj(shm) failure. %d\n", ret);
      return ret;
    }

  /* Map shared memory to virtual space */

  buf = (char*) mpshm_attach(&shm, 0);
  if (!buf)
    {
      err("mpshm_attach() failure.\n");
      return ret;
    }
  message("attached at %08x\n", (uintptr_t)buf);
  memset(buf, 0, 1024);

  /* Run worker */

  ret = mptask_exec(&mptask);
  if (ret < 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }

  /* Send command to worker */
  ret = mpmq_send(&mq, MSG_ID_TEST, (uint32_t) &data);
  if (ret < 0)
    {
      err("mpmq_send() failure. %d\n", ret);
      return ret;
    }

  /* Wait for worker message */

  ret = mpmq_receive(&mq, &msgdata);
  if (ret < 0)
    {
      err("mpmq_recieve() failure. %d\n", ret);
      return ret;
    }
  message("Worker response: ID = %d, data = %x\n",
          ret, *((int *)msgdata));

  /* Show worker copied data */

  /* Lock mutex for synchronize with worker after it's started */

  mpmutex_lock(&mutex);

  message("Worker said: %s\n", buf);

  mpmutex_unlock(&mutex);

  /* Destroy worker */

  wret = -1;
  ret = mptask_destroy(&mptask, false, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() failure. %d\n", ret);
      return ret;
    }

  message("Worker exit status = %d\n", wret);

  /* Finalize all of MP objects */

  mpshm_detach(&shm);
  mpshm_destroy(&shm);
  mpmutex_destroy(&mutex);
  mpmq_destroy(&mq);

  return 0;
}
