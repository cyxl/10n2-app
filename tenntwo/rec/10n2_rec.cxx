
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <arch/board/board.h>
#include <nuttx/clock.h>
#include <nuttx/arch.h>
#include <10n2_rec.h>
#include <10n2_imu.h>
#include <10n2_aud.h>
#include <10n2_btn.h>
#include <10n2_cam.h>
#include <10n2_gnss.h>
#include <10n2_dp.h>
#include <10n2_tf_pi.h>

static bool rec_running = true;
#define REC_QUEUE_NAME "/rec_queue" /* Queue name. */
#define REC_QUEUE_PERMS ((int)(0644))
#define REC_QUEUE_MAXMSG 16               /* Maximum number of messages. */
#define REC_QUEUE_MSGSIZE sizeof(rec_req) /* Length of message. */
#define REC_QUEUE_ATTR_INITIALIZER ((struct mq_attr){REC_QUEUE_MAXMSG, REC_QUEUE_MSGSIZE, 0, 0})

static pthread_t rec_th;
static struct mq_attr rec_attr_mq = REC_QUEUE_ATTR_INITIALIZER;

#define POS_SAVE_DIR "/mnt/sd0/pos"
#define KML_SAVE_DIR "/mnt/sd0/kml"

#define KML_NUM_POINTS_IN_SEGMENT 10

const char *TNT_KML_HEADER =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<kml xmlns=\"http://earth.google.com/kml/2.0\"> <Document>";

const char *TNT_KML_FOOTER =
    "</Document> </kml>";

const char *TNT_KML_FAIL_LINESTYLE =
    "<Style id=\"FAIL\">"
    "<LineStyle>"
    "<color>cf0000ff</color>"
    "<width>8</width>"
    "<gx:labelVisibility>1</gx:labelVisibility>"
    "</LineStyle>"
    "</Style>";

const char *TNT_KML_BAD_LINESTYLE =
    "<Style id=\"BAD\">"
    "<LineStyle>"
    "<color>cf0080ff</color>"
    "<width>8</width>"
    "<gx:labelVisibility>1</gx:labelVisibility>"
    "</LineStyle>"
    "</Style>";

const char *TNT_KML_WARN_LINESTYLE =
    "<Style id=\"WARN\">"
    "<LineStyle>"
    "<color>cf00cfff</color>"
    "<width>8</width>"
    "<gx:labelVisibility>1</gx:labelVisibility>"
    "</LineStyle>"
    "</Style>";

const char *TNT_KML_GOOD_LINESTYLE =
    "<Style id=\"GOOD\">"
    "<LineStyle>"
    "<color>cf33cc33</color>"
    "<width>8</width>"
    "<gx:labelVisibility>1</gx:labelVisibility>"
    "</LineStyle>"
    "</Style>";

const char *TNT_KML_EXCEPTIONAL_LINESTYLE =
    "<Style id=\"GOOD\">"
    "<LineStyle>"
    "<color>cfff0000</color>"
    "<width>8</width>"
    "<gx:labelVisibility>1</gx:labelVisibility>"
    "</LineStyle>"
    "</Style>";
const char *TNT_KML_ROUTE_PLACEMARK_HEADER =
    "<Placemark>"
    "<styleUrl>%s</styleUrl>"
    "<MultiGeometry>"
    "<LineString>"
    "<extrude>1</extrude>"
    "<tessellate>1</tessellate>"
    "<coordinates>";

const char *TNT_KML_ROUTE_PLACEMARK_FOOTER =
    "</coordinates>"
    "</LineString>"
    "</MultiGeometry>"
    "</Placemark>";

const char *TNT_KML_POINT_PLACEMARK_HEADER =
    "<Placemark>"
    "<Point>"
    "<coordinates>%lf,%lf"
    "</coordinates>"
    "</Point>"
    "<ExtendedData>";

const char *TNT_KML_POINT_PLACEMARK_FOOTER =
    "</ExtendedData>"
    "</Placemark>";

const char *TNT_KML_EXTENDED_DATA =
    "<Data name=\"%s\">"
    "<value>%i</value>"
    "</Data>";
const char *TNT_KML_SINGLE_POINT =
    "%lf,%lf\n";

FILE *pos_pf = NULL;
FILE *kml_pf = NULL;

const char *FAIL_STYLE = "FAIL";
const char *BAD_STYLE = "BAD";
const char *WARN_STYLE = "WARN";
const char *GOOD_STYLE = "GOOD";
const char *EXCEPTIONAL_STYLE = "EXCEPTIONAL";
const char *VNAME_CELL = "Cell";
const char *VNAME_NOHANDS = "No Hands";
const char *VNAME_BADHANDS = "Bad Hands";
const char *VNAME_ACCEL = "Accel";
const char *VNAME_DECEL = "Decel";
const char *VNAME_LEFT = "Left";
const char *VNAME_RIGHT = "Right";
const char *VNAME_POTHOLE = "Pot Hole";

uint32_t kml_current_seg_cnt = 0;
uint32_t kml_current_geo_cnt = 0;
float kml_current_seg_score = 100.;
std::vector<struct gnss_data> gnss_points;

#define V_CELL 0
#define V_NOHANDS 1
#define V_BADHANDS 2
#define V_ACCEL 3
#define V_DECEL 4
#define V_LEFT 5
#define V_RIGHT 6
#define V_POTHOLE 7
#define V_NUM 8

uint16_t violations[V_NUM] = {0};

void write_kml_fd(const char *s, int n, ...)
{
    va_list args;
    va_start(args, n);
    if (kml_pf != NULL)
    {
        fprintf(kml_pf, s, args);
        fflush(kml_pf);
    }
    va_end(args);
}
void write_kml_fd_si(const char *f, const char *s, int i)
{
    if (kml_pf != NULL)
    {
        fprintf(kml_pf, f, s, i);
        fflush(kml_pf);
    }
}

void write_kml_fd_ff(const char *fmt, float f, float f2)
{
    if (kml_pf != NULL)
    {
        fprintf(kml_pf, fmt, f, f2);
        fflush(kml_pf);
    }
}

void close_pos_fd()
{
    if (pos_pf != NULL)
    {
        if (fclose(pos_pf) != 0)
        {
            printf("Unable to close pos file! :%s\n", strerror(errno));
        }
        else
        {
            printf("success!  closed pos output file\n");
        }
        pos_pf = NULL;
    }
}
void close_kml_fd()
{
    if (kml_pf != NULL)
    {
        fprintf(kml_pf, TNT_KML_FOOTER);
        fflush(kml_pf);
        if (fclose(kml_pf) != 0)
        {
            printf("Unable to close kml file! :%s\n", strerror(errno));
        }
        else
        {
            printf("success!  closed kml output file\n");
        }
        kml_pf = NULL;
    }
}

void open_pos_fd(uint8_t cnt, bool verbose)
{

    if (pos_pf != NULL)
    {
        printf("Closing open file\n");
        close_pos_fd();
    }
    unsigned curr_time = clock();
    char namebuf[128];
    const char *type = verbose ? "verbose" : "terse";
    snprintf(namebuf, 128, "%s/tf-data-%s-%i-%i.%s", POS_SAVE_DIR, type, cnt, curr_time, "csv");
    printf("opening :%s\n", namebuf);
    pos_pf = fopen(namebuf, "wb+");
    if (pos_pf == NULL)
    {
        printf("Unable to open pos! :%s\n", strerror(errno));
    }
    else
    {
        printf("success!  opened pos output file\n");
    }

    if (verbose)
        fprintf(pos_pf, "t,slopex,slopey,slopez,inf,conf,acx,acy,acz,gyx,gyy,gyz,y,M,d,h,m,s,us,type,lat,lon\n");
    else
        fprintf(pos_pf, "t,inf,conf,imu,y,M,d,h,m,s,us,type,lat,lon\n");
    fflush(pos_pf);
}

void open_kml_fd()
{

    if (kml_pf != NULL)
    {
        printf("Closing open file\n");
        close_kml_fd();
    }
    unsigned curr_time = clock();
    char namebuf[128];
    snprintf(namebuf, 128, "%s/10n2-report-%i.%s", KML_SAVE_DIR, curr_time, "kml");
    printf("opening :%s\n", namebuf);
    kml_pf = fopen(namebuf, "wb+");
    if (kml_pf == NULL)
    {
        printf("Unable to open kml! :%s\n", strerror(errno));
    }
    else
    {
        printf("success!  opened kml output file\n");
    }

    kml_current_seg_cnt = 0;
    kml_current_geo_cnt = 0;
    for (int i = 0; i < V_NUM; i++)
        violations[i] = 0;

    gnss_points.clear();
    write_kml_fd(TNT_KML_HEADER, 0);
    write_kml_fd(TNT_KML_WARN_LINESTYLE, 0);
    write_kml_fd(TNT_KML_BAD_LINESTYLE, 0);
    write_kml_fd(TNT_KML_GOOD_LINESTYLE, 0);
    fflush(kml_pf);
}

float calculate_violation_score(float scale)
{
    float calc_violations = 0;

    calc_violations = violations[V_CELL] * 10;
    calc_violations += violations[V_NOHANDS] * 5;
    calc_violations += violations[V_BADHANDS] * 5;
    calc_violations += violations[V_ACCEL] * 2;
    calc_violations += violations[V_DECEL] * 2;
    calc_violations += violations[V_LEFT] * 1;
    calc_violations += violations[V_RIGHT] * 1;
    calc_violations += violations[V_POTHOLE] * .5;

    return calc_violations * scale;
}

void *_rec_run(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
                /* Initialize the queue attributes */
    int cpu = up_cpu_index();
    printf("REC CPU %d\n", cpu);

    mqd_t r_mq = mq_open(REC_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, REC_QUEUE_PERMS, &rec_attr_mq);
    if (r_mq < 0)
    {
        fprintf(stderr, "[rec CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }
    printf("[rec CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);
    unsigned int prio;
    ssize_t bytes_read;
    char buffer[REC_QUEUE_MSGSIZE];
    struct timespec poll_sleep;

    bool recording = false;

    while (rec_running)
    {
        bytes_read = mq_receive(r_mq, buffer, REC_QUEUE_MSGSIZE, &prio);
        rec_req *r = (rec_req *)buffer;
        if (bytes_read >= 0)
        {
            if (r->act == rec_open)
            {
                open_pos_fd(r->f_id, r->type == rec_verbose);
                open_kml_fd();
                recording = true;
            }
            else if (r->act == rec_close)
            {
                close_pos_fd();
                close_kml_fd();
                recording = false;
            }
        }
        if (recording && r->type == rec_verbose)
        {
            unsigned curr_time = clock();
            fprintf(pos_pf, "%i,%f,%f,%f,%i,%f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%lf,%lf\n",
                    curr_time,
                    current_x_slope,
                    current_y_slope,
                    current_z_slope,
                    current_inf,
                    current_conf,
                    current_pmu.ac_x,
                    current_pmu.ac_y,
                    current_pmu.ac_z,
                    current_pmu.gy_x,
                    current_pmu.gy_y,
                    current_pmu.gy_z,
                    current_gnss.date.year,
                    current_gnss.date.month,
                    current_gnss.date.day,
                    current_gnss.time.hour,
                    current_gnss.time.minute,
                    current_gnss.time.sec,
                    current_gnss.time.usec,
                    current_gnss.type,
                    current_gnss.latitude,
                    current_gnss.longitude);
            int rc = fflush(pos_pf);
            printf("recording verbose : %d!\n", rc);
        }
        else if (recording && r->type == rec_terse)
        {
            unsigned curr_time = clock();
            fprintf(pos_pf, "%i,%i,%f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%lf,%lf\n",
                    curr_time,
                    current_inf,
                    current_conf,
                    current_imu_bit,
                    current_gnss.date.year,
                    current_gnss.date.month,
                    current_gnss.date.day,
                    current_gnss.time.hour,
                    current_gnss.time.minute,
                    current_gnss.time.sec,
                    current_gnss.time.usec,
                    current_gnss.type,
                    current_gnss.latitude,
                    current_gnss.longitude);
            int rc = fflush(pos_pf);
            printf("recording terse : %d!\n", rc);
        }

        if (recording) // KML
        {
            if (current_inf == CELL_IDX)
                violations[V_CELL] += 1;
            if (current_inf == NONE_IDX)
                violations[V_NOHANDS] += 1;
            if (current_inf == BAD_IDX)
                violations[V_BADHANDS] += 1;
            if (current_imu_bit & ACCEL_BIT)
                violations[V_ACCEL] += 1;
            if (current_imu_bit & DECEL_BIT)
                violations[V_DECEL] += 1;
            if (current_imu_bit & LEFT_BIT)
                violations[V_LEFT] += 1;
            if (current_imu_bit & RIGHT_BIT)
                violations[V_RIGHT] += 1;
            if (current_imu_bit & POTHOLE_BIT)
                violations[V_POTHOLE] += 1;

            // KML
            kml_current_seg_cnt += 1;
            if (current_gnss.data_exists)
            {
                gnss_points.push_back(current_gnss);
                printf("got gnss %i %s\n", current_gnss.type, VNAME_ACCEL);
                kml_current_geo_cnt += 1;

                if ((kml_current_geo_cnt % KML_NUM_POINTS_IN_SEGMENT) == 0)
                {
                    float violation_score = calculate_violation_score(1. / kml_current_seg_cnt);
                    kml_current_seg_cnt = 0;
                    float score = 100 - violation_score;

                    if (score < 50.)
                    {
                        write_kml_fd_si(TNT_KML_ROUTE_PLACEMARK_HEADER, FAIL_STYLE, 0);
                    }
                    else if (score < 60.)
                    {
                        write_kml_fd_si(TNT_KML_ROUTE_PLACEMARK_HEADER, BAD_STYLE, 0);
                    }
                    else if (score < 70.)
                    {
                        write_kml_fd_si(TNT_KML_ROUTE_PLACEMARK_HEADER, WARN_STYLE, 0);
                    }
                    else if (score < 80.)
                    {
                        write_kml_fd_si(TNT_KML_ROUTE_PLACEMARK_HEADER, GOOD_STYLE, 0);
                    }
                    else
                    {
                        write_kml_fd_si(TNT_KML_ROUTE_PLACEMARK_HEADER, EXCEPTIONAL_STYLE, 0);
                    }

                    for (struct gnss_data gd : gnss_points)
                    {
                        write_kml_fd_ff(TNT_KML_SINGLE_POINT, gd.longitude, gd.latitude);
                    }
                    write_kml_fd(TNT_KML_ROUTE_PLACEMARK_FOOTER, 0);

                    write_kml_fd_ff(TNT_KML_POINT_PLACEMARK_HEADER, gnss_points.back().longitude, gnss_points.back().latitude);

                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_CELL, violations[V_CELL]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_NOHANDS, violations[V_NOHANDS]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_BADHANDS, violations[V_BADHANDS]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_ACCEL, violations[V_ACCEL]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_DECEL, violations[V_DECEL]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_LEFT, violations[V_LEFT]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_RIGHT, violations[V_RIGHT]);
                    write_kml_fd_si(TNT_KML_EXTENDED_DATA, VNAME_POTHOLE, violations[V_POTHOLE]);

                    write_kml_fd(TNT_KML_POINT_PLACEMARK_FOOTER, 0);
                    gnss_points.erase(gnss_points.begin(), gnss_points.end() - 1);

                    // clear violations
                    for (int i = 0; i < V_NUM; i++)
                        violations[i] = 0;
                }           // segment
            }               // if current_gns
        }                   // if recording (kml)
        usleep(1000 * 1e3); // 1 hz recording samples
    }                       // while running
    printf("rec cleaning mq\n");
    mq_close(r_mq);
    return NULL;
}

bool send_rec_req(struct rec_req req)
{
    mqd_t mq = mq_open(REC_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
    if (mq < 0)
    {
        fprintf(stderr, "[rec sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, REC_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[rec sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}
bool rec_init(void)
{
    printf("menu handler init\n");
    rec_running = true;

    cpu_set_t cpuset = 1 << 3;
    pthread_create(&rec_th, NULL, &_rec_run, NULL);
    int rc;
    rc = pthread_setaffinity_np(rec_th, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }

    return true;
}

bool rec_teardown(void)
{
    printf("menu handler teardown\n");
    rec_running = false;
    pthread_join(rec_th, NULL);
    return true;
}
