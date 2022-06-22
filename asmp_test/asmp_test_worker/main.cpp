#include <errno.h>

extern "C"
{
#include <asmp/types.h>
#include <asmp/mpshm.h>
#include <asmp/mpmutex.h>
#include <asmp/mpmq.h>
}

#include "asmp.h"
#include "include/worker.h"

#define ASSERT(cond) \
  if (!(cond))       \
  wk_abort()

static char *strcopy(char *dest, const char *src)
{
  char *d = dest;
  while (*src)
    *d++ = *src++;
  *d = '\0';

  return dest;
}

extern "C" int main(int argc, FAR char *argv[])
{
  mpmutex_t mutex;
  mpshm_t shm;
  mpmq_t mq;
  uint32_t msgdata;
  char *buf;
  int ret;

  /* Initialize MP Mutex */

  ret = mpmutex_init(&mutex, ASMP_TEST_MUTEX);
  ASSERT(ret == 0);

  /* Initialize MP message queue,
   * On the worker side, 3rd argument is ignored.
   */

  ret = mpmq_init(&mq, ASMP_TEST_MQ, 0);
  ASSERT(ret == 0);

  /* Initialize MP shared memory */

  ret = mpshm_init(&shm, ASMP_TEST_SHM, 1024);
  ASSERT(ret == 0);

  /* Map shared memory to virtual space */

  buf = (char *)mpshm_attach(&shm, 0);
  ASSERT(buf);

  /* Receive message from supervisor */

  for (int i = 0; i < 5; i++)
  {
    ret = mpmq_receive(&mq, &msgdata);
    ASSERT(ret == MSG_ID_TEST);

    /* Copy hello message to shared memory */

    mpmutex_lock(&mutex);
    strcopy(buf, "sup");
    mpmutex_unlock(&mutex);
    /* Send done message to supervisor */

    ret = mpmq_send(&mq, MSG_ID_TEST, msgdata);
  }

  /* Free virtual space */

  mpshm_detach(&shm);

  ASSERT(ret == 0);

  return 0;
}
