/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#include <components.h>

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#include <rt_spi_flash_device.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#include <touch_setup.h>

#endif

#ifdef  RT_USING_FINSH
#include "shell.h"
#endif

#include "led.h"
extern void rtgui_touch_hw_init(void);
extern void gui_application_init(void);
extern void calibration(void);
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;

    rt_hw_led_init();

    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(0);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}


void rt_init_thread_entry(void* parameter)
{
	
		/* initialization RT-Thread Components */
		rt_components_init();
		
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
		/* mount SPI flash as root directory */ 
    if (dfs_mount("flash0", "/", "elm", 0, 0) == 0)
    {      
				rt_kprintf("flash0 mount to /.\n");				
    }
    else
        rt_kprintf("flash0 mount to / failed.\n");
		/* mount SD Card as /dev/sd directory */ 
    if (dfs_mount("sd0", "/dev", "elm", 0, 0) == 0)
    {
        rt_kprintf("sd0 mount to /dev.\n");
    }
    else		
        rt_kprintf("sd0 mount to /dev failed.\n");	
#endif /* RT_USING_DFS */		
				       
#ifdef RT_USING_RTGUI
		{
				rt_device_t lcd; 	
				/* find lcd device */
				lcd = rt_device_find("lcd");
				/* set lcd device as rtgui graphic driver */
				rtgui_graphic_set_device(lcd);
				rtgui_system_server_init();
				gui_application_init();
#ifdef RTGUI_USING_CALIBRATION
				calibration_set_restore(calibration_restore);//initialize the pointer to load user data
				calibration_set_after(calibration_store); //initialize the poiter to save user data
				calibration_init();		
#endif /* RTGUI_USING_CALIBRATION */	
		}
#endif /* RT_USING_RTGUI */


#ifdef RT_USING_USB_HOST
    /* register stm32 usb host controller driver */
    rt_hw_susb_init();
#endif

		rt_thread_delay(50);
    rt_device_init_all();
		
}

int rt_application_init(void)
{
    rt_thread_t init_thread;

    rt_err_t result;
		
    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}

/*@}*/
