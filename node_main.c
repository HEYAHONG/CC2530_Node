#include "contiki.h"
#include "contiki-conf.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include "models.h"
#include "dev/adc-sensor.h"
#include "includes.h"


#define DEBUG 1

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
/* We overwrite (read as annihilate) all output functions here */
#define PRINTF(...)
#endif /* DEBUG */
/*---------------------------------------------------------------------------*/
//多跳相关函数
struct neighbor
{
    struct neighbor *next;
    linkaddr_t addr;
    struct ctimer ctimer;
};
#define NEIGHBOR_TIMEOUT 60 * CLOCK_SECOND
#define MAX_NEIGHBORS 16
LIST(neighbor_table);
MEMB(neighbor_mem, struct neighbor, MAX_NEIGHBORS);
static void
remove_neighbor(void *n)
{
    struct neighbor *e = n;

    list_remove(neighbor_table, e);
    memb_free(&neighbor_mem, e);
}
static void
received_announcement(struct announcement *a,
                      const linkaddr_t *from,
                      uint16_t id, uint16_t value)
{
    struct neighbor *e;

    /*  printf("Got announcement from %d.%d, id %d, value %d\n",
        from->u8[0], from->u8[1], id, value);*/

    /* We received an announcement from a neighbor so we need to update
       the neighbor list, or add a new entry to the table. */
    for(e = list_head(neighbor_table); e != NULL; e = e->next)
    {
        if(linkaddr_cmp(from, &e->addr))
        {
            /* Our neighbor was found, so we update the timeout. */
            ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
            return;
        }
    }

    /* The neighbor was not found in the list, so we add a new entry by
       allocating memory from the neighbor_mem pool, fill in the
       necessary fields, and add it to the list. */
    e = memb_alloc(&neighbor_mem);
    if(e != NULL)
    {
        linkaddr_copy(&e->addr, from);
        list_add(neighbor_table, e);
        ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
    }
}

static struct announcement _announcement;

static void
recv(struct multihop_conn *c, const linkaddr_t *sender,
     const linkaddr_t *prevhop,
     uint8_t hops)
{
    PRINTF("multihop message received '%s'\n", (char *)packetbuf_dataptr());
}
static linkaddr_t *
forward(struct multihop_conn *c,
        const linkaddr_t *originator, const linkaddr_t *dest,
        const linkaddr_t *prevhop, uint8_t hops)
{
    /* Find a random neighbor to send to. */
    int num, i;
    struct neighbor *n;

    if(list_length(neighbor_table) > 0)
    {
        num = random_rand() % list_length(neighbor_table);
        i = 0;
        for(n = list_head(neighbor_table); n != NULL && i != num; n = n->next)
        {
            ++i;
        }
        if(n != NULL)
        {
            PRINTF("%d.%d: Forwarding packet to %d.%d (%d in list), hops %d\n",
                   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                   n->addr.u8[0], n->addr.u8[1], num,
                   packetbuf_attr(PACKETBUF_ATTR_HOPS));
            return &n->addr;
        }
    }
    PRINTF("%d.%d: did not find a neighbor to foward to\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    return NULL;
}
static const struct multihop_callbacks multihop_call = {recv, forward};
static struct multihop_conn multihop;
#define MultiHop_Rime_Channel 135
#define MultiHop_Test
/*---------------------------------------------------------------------------*/
//广播相关函数
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d: '%s'\n\r",
           from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
#define BroadCast_Rime_Channel 129
#define BroadCast_Test
/*---------------------------------------------------------------------------*/
//Node main主进程
PROCESS(node_main_process, "Node main Process");
AUTOSTART_PROCESSES(&node_main_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_main_process, ev, data)
{
    static struct etimer et;

    /* Sensor Values */
    static int rv;
    static const struct sensors_sensor *sensor;
    static float sane = 0;
    static int dec;
    static float frac;

    //广播
    PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

    //多跳
    PROCESS_EXITHANDLER(multihop_close(&multihop);)

    PROCESS_BEGIN();

    //启动广播接收
    broadcast_open(&broadcast,BroadCast_Rime_Channel, &broadcast_call);

    //启动多跳接收
    {
        /* Initialize the memory for the neighbor table entries. */
        memb_init(&neighbor_mem);

        /* Initialize the list used for the neighbor table. */
        list_init(neighbor_table);

        /* Open a multihop connection on Rime channel CHANNEL. */
        multihop_open(&multihop, MultiHop_Rime_Channel, &multihop_call);

        /* Register an announcement with the same announcement ID as the
           Rime channel we use to open the multihop connection above. */
        announcement_register(&_announcement,
                              MultiHop_Rime_Channel,
                              received_announcement);

        /* Set a dummy value to start sending out announcments. */
        announcement_set_value(&_announcement, 0);

    }

    /* Set an etimer. We take sensor readings when it expires and reset it. */
    etimer_set(&et, CLOCK_SECOND * 2);

    while(1)
    {

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        /*
         * Request some ADC conversions
         * Return value -1 means sensor not available or turned off in conf
         */
        sensor = sensors_find(ADC_SENSOR);
        if(sensor)
        {
            PRINTF("------------------\n\r");
            leds_toggle(LEDS_RED);
            /*
             * Temperature:
             * Using 1.25V ref. voltage (1250mV).
             * Typical AD Output at 25°C: 1480
             * Typical Co-efficient     : 4.5 mV/°C
             *
             * Thus, at 12bit decimation (and ignoring the VDD co-efficient as well
             * as offsets due to lack of calibration):
             *
             *          AD - 1480
             * T = 25 + ---------
             *              4.5
             */
            rv = sensor->value(ADC_SENSOR_TYPE_TEMP);
            if(rv != -1)
            {
                sane = 25 + ((rv - 1480) / 4.5);
                dec = sane;
                frac = sane - dec;
                PRINTF("  Temp=%d.%02u C (%d)\n\r", dec, (unsigned int)(frac*100), rv);
            }
            /*
             * Power Supply Voltage.
             * Using 1.25V ref. voltage.
             * AD Conversion on VDD/3
             *
             * Thus, at 12bit resolution:
             *
             *          ADC x 1.25 x 3
             * Supply = -------------- V
             *               2047
             */
            rv = sensor->value(ADC_SENSOR_TYPE_VDD);
            if(rv != -1)
            {
                sane = rv * 3.75 / 2047;
                dec = sane;
                frac = sane - dec;
                PRINTF("Supply=%d.%02u V (%d)\n\r", dec, (unsigned int)(frac*100), rv);
                /* Store rv temporarily in dec so we can use it for the battery */
                dec = rv;
            }
            /*
             * Battery Voltage - ToDo
             *   rv = sensor->value(ADC_SENSOR_TYPE_BATTERY);
             */
             #ifdef BroadCast_Test
            {
                //测试广播
                packetbuf_copyfrom("Hello BroadCast Message for test!", sizeof("Hello BroadCast Message for test!"));
                broadcast_send(&broadcast);

            }
            #endif // BroadCast_Test
            #ifdef MultiHop_Test
            {
                //测试多跳

                static linkaddr_t to;
                packetbuf_copyfrom("Hello MultiHop Message for test!", sizeof("Hello MultiHop Message for test!"));

                /* Set the Rime address of the final receiver of the packet to
                   1.0. This is a value that happens to work nicely in a Cooja
                   simulation (because the default simulation setup creates one
                   node with address 1.0). */
                to.u8[0] = 1;
                to.u8[1] = 0;

                /* Send the packet. */
                multihop_send(&multihop, &to);


            }
            #endif // MultiHop_Test

        }
        etimer_reset(&et);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
