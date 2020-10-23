#include "contiki.h"
#include "contiki-conf.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include "models.h"
#include "dev/adc-sensor.h"

#define DEBUG 1

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
/* We overwrite (read as annihilate) all output functions here */
#define PRINTF(...)
#endif /* DEBUG */

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


    PROCESS_BEGIN();

    //启动广播接收
    broadcast_open(&broadcast,BroadCast_Rime_Channel, &broadcast_call);



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

        }
        etimer_reset(&et);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
