#include <xc.h>
#include "clcd.h"
#include "matrix_keypad.h"
#include "timer.h"

// Disable the Watchdog Timer so it does not reset the controller unexpectedly.
#pragma config WDTE = OFF

/* Operating modes */
// Tracks the selected operating mode: 0=menu, 1=Micro, 2=Grill, 3=Convection, 4=Quick Start.
static unsigned char mode = 0;
/* 0 = Menu, 1 = Micro, 2 = Grill,
   3 = Convection, 4 = Quick Start */

/* Cooking time */
// Stores the four digits entered for cooking time in MM:SS format.
static unsigned char digits[4] = {0, 0, 0, 0};
static unsigned char digit_count = 0;
// Becomes 1 after the user confirms a valid cooking time with #.
static unsigned char time_set = 0;

static unsigned char minutes = 0;
static unsigned char seconds = 0;

// Controls whether the cooking countdown is actively decreasing.
static unsigned char countdown_running = 0;
// Records that the countdown was paused; the remaining time is preserved.
static unsigned char countdown_paused = 0;

/* Quick Start */
// Indicates that Quick Start is the active mode.
static unsigned char quick_start_active = 0;

/* Convection */
// Confirmed Convection temperature in degrees Celsius; 0 means not confirmed.
static unsigned char temperature = 0;
// Stores up to three digits for the temperature entry.
static unsigned char temp_digits[3] = {0, 0, 0};
static unsigned char temp_digit_count = 0;

/* Convection states:
   0 = Enter temperature
   1 = Preheating
   2 = Enter cooking time
   3 = Cooking */
// Convection state: 0=temperature entry, 1=preheat, 2=time entry, 3=cooking.
static unsigned char conv_state = 0;
// Simulated preheating duration, counted down from 60 seconds.
static unsigned char preheat_seconds = 60;

/* Time-entry blinking */
static unsigned char blink_visible = 1;
static unsigned char last_digit_count = 255;

static unsigned char quick_start_just_started = 0;


/* Initialize peripherals */
// Initializes the LCD, matrix keypad, and Timer2 timebase.
static void init_config(void)
{
    init_clcd();
    init_matrix_keypad();
    init_timer2();
}


/* Display main menu */
// Clears the LCD and displays the four available oven modes.
static void display_menu(void)
{
    clcd_clear();

    clcd_print("1. Micro", LINE1(0));
    clcd_print("2. Grill", LINE2(0));
    clcd_print("3. Convection", LINE3(0));
    clcd_print("4. Quick Start", LINE4(0));
}


/* Reset cooking-time entry */
// Clears all entered cooking-time digits and resets the blinking-entry state.
static void reset_time_entry(void)
{
    digit_count = 0;
    time_set = 0;
    last_digit_count = 255;
    blink_visible = 1;

    for (unsigned char i = 0; i < 4; i++)
    {
        digits[i] = 0;
    }
}


/* Set cooking time in MM:SS format */
// Handles MM:SS entry: digits append, * clears, and # confirms if seconds are below 60.
static void set_time(unsigned char key)
{
    unsigned char position;

    clcd_print("SET TIME <MM:SS>", LINE1(0));
    clcd_print("TIME: ", LINE2(0));
    clcd_print("*:CLEAR  #:ENTER", LINE4(0));

    if (last_digit_count != digit_count)
    {
        clcd_putch(digits[0] + '0', LINE2(6));
        clcd_putch(digits[1] + '0', LINE2(7));
        clcd_putch(':', LINE2(8));
        clcd_putch(digits[2] + '0', LINE2(9));
        clcd_putch(digits[3] + '0', LINE2(10));

        last_digit_count = digit_count;
        blink_visible = 1;
    }

    if (digit_count < 4)
    {
        position = (digit_count < 2) ? 6 + digit_count : 7 + digit_count;

        if (blink_flag)
        {
            blink_flag = 0;
            blink_visible = !blink_visible;

            if (blink_visible)
                clcd_putch(digits[digit_count] + '0', LINE2(position));
            else
                clcd_putch(' ', LINE2(position));
        }
    }

    if (key != ALL_RELEASED)
    {
        if (key <= 9 && digit_count < 4)
        {
            digits[digit_count] = key;
            digit_count++;
            last_digit_count = 255;
        }
        else if (key == '*')
        {
            reset_time_entry();
        }
        else if (key == '#' && digit_count == 4)
        {
            unsigned char entered_seconds = digits[2] * 10 + digits[3];

            if (entered_seconds < 60)
            {
                minutes = digits[0] * 10 + digits[1];
                seconds = entered_seconds;
                time_set = 1;

                countdown_running = 0;
                countdown_paused = 0;

                clcd_clear();

                clcd_print("TIME :", LINE1(1));
                clcd_putch(minutes / 10 + '0', LINE1(7));
                clcd_putch(minutes % 10 + '0', LINE1(8));
                clcd_putch(':', LINE1(9));
                clcd_putch(seconds / 10 + '0', LINE1(10));
                clcd_putch(seconds % 10 + '0', LINE1(11));

                clcd_print("4 - Start/Resume", LINE2(0));
                clcd_print("5 - Pause", LINE3(0));
                clcd_print("6 - Stop", LINE4(0));
            }
            else
            {
                clcd_clear();
                clcd_print("Invalid Seconds", LINE1(0));
                __delay_ms(500);
                reset_time_entry();
            }
        }
    }
}


/* Enter convection temperature */
// Handles Convection temperature entry; accepts values from 25 to 180 C.
// Press * to clear the entry and # to confirm and begin preheating.
static unsigned char temp_blink_visible = 1;
static unsigned char last_temp_digit_count = 255;

static void set_temperature(unsigned char key)
{
    unsigned char position;

    clcd_print("SET TEMP (C)", LINE1(0));
    clcd_print("TEMP: ", LINE2(0));
    clcd_print("*:CLEAR  #:ENTER", LINE4(0));

    /* Update the temperature digits when input changes */
    if (last_temp_digit_count != temp_digit_count)
    {
        clcd_putch(temp_digits[0] + '0', LINE2(6));
        clcd_putch(temp_digits[1] + '0', LINE2(7));
        clcd_putch(temp_digits[2] + '0', LINE2(8));

        last_temp_digit_count = temp_digit_count;
        temp_blink_visible = 1;
    }

    /* Blink only the digit currently being entered */
    if (temp_digit_count < 3)
    {
        position = 6 + temp_digit_count;

        if (blink_flag)
        {
            blink_flag = 0;
            temp_blink_visible = !temp_blink_visible;

            if (temp_blink_visible)
            {
                clcd_putch(temp_digits[temp_digit_count] + '0',
                           LINE2(position));
            }
            else
            {
                clcd_putch(' ', LINE2(position));
            }
        }
    }

    if (key == ALL_RELEASED)
        return;

    /* Accept the next temperature digit */
    if (key <= 9 && temp_digit_count < 3)
    {
        temp_digits[temp_digit_count] = key;
        temp_digit_count++;

        last_temp_digit_count = 255;
        temp_blink_visible = 1;
    }
    else if (key == '*')
    {
        /* Clear all entered digits */
        temp_digit_count = 0;

        for (unsigned char i = 0; i < 3; i++)
        {
            temp_digits[i] = 0;
        }

        last_temp_digit_count = 255;
        temp_blink_visible = 1;
    }
    else if (key == '#' && temp_digit_count > 0)
    {
        unsigned int entered_temp =
            temp_digits[0] * 100 +
            temp_digits[1] * 10 +
            temp_digits[2];

        if (entered_temp >= 25 && entered_temp <= 180)
        {
            temperature = entered_temp;

            conv_state = 1;
            preheat_seconds = 60;

            countdown_running = 0;
            countdown_paused = 0;

            clcd_clear();

            clcd_print("PREHEATING...", LINE1(0));
            clcd_print("TEMP: ", LINE2(0));

            clcd_putch(temperature / 100 + '0', LINE2(6));
            clcd_putch((temperature / 10) % 10 + '0',
                       LINE2(7));
            clcd_putch(temperature % 10 + '0', LINE2(8));

            clcd_print("TIME LEFT:", LINE3(0));
            clcd_putch('0', LINE3(11));
            clcd_putch('6', LINE3(12));
            clcd_putch('0', LINE3(13));
            clcd_print("6:Stop", LINE4(0));
        }
        else
        {
            clcd_clear();
            clcd_print("INVALID TEMP", LINE1(2));
            clcd_print("Range: 25-180 C", LINE2(0));

            __delay_ms(1000);

            temp_digit_count = 0;

            for (unsigned char i = 0; i < 3; i++)
            {
                temp_digits[i] = 0;
            }

            last_temp_digit_count = 255;
            temp_blink_visible = 1;
            clcd_clear();
        }
    }
}


/* Update the displayed cooking time */
// Writes the current minutes and seconds to the LCD time field.
static void display_cooking_time(void)
{
    clcd_putch(minutes / 10 + '0', LINE1(7));
    clcd_putch(minutes % 10 + '0', LINE1(8));
    clcd_putch(':', LINE1(9));
    clcd_putch(seconds / 10 + '0', LINE1(10));
    clcd_putch(seconds % 10 + '0', LINE1(11));
}


/* Return to menu and reset mode-specific state */
// Stops any countdown, resets mode-specific state, and returns to the main menu.
static void stop_and_return_to_menu(void)
{
    countdown_running = 0;
    countdown_paused = 0;

    quick_start_active = 0;
    conv_state = 0;
    temperature = 0;
    temp_digit_count = 0;

    minutes = 0;
    seconds = 0;

    reset_time_entry();

    mode = 0;
    display_menu();
}

// Shows a short power-on animation before the main menu appears.
static void startup_animation(void)
{
    unsigned char i;

    clcd_clear();

    /* Initial animation blocks */
    for (i = 0; i < 16; i++)
    {
        clcd_putch(0xFF, LINE1(i));
        __delay_ms(60);
    }

    clcd_print("Powering ON", LINE2(2));
    clcd_print("Microwave Oven", LINE3(1));

    /* Animate blocks on the bottom row */
    for (i = 0; i < 16; i++)
    {
        clcd_putch(0xFF, LINE4(i));
        __delay_ms(60);
    }

    __delay_ms(1000);

    clcd_clear();
}

// Program entry point: initializes hardware, shows startup animation, then services keypad and timers.
void main(void)
{
    unsigned char key;

    init_config();
    
    startup_animation();
    
    display_menu();

    // Main loop continuously scans the keypad and processes mode/timer events.
    while (1)
    {
        // Read a newly pressed keypad key; ALL_RELEASED means no new key press.
        key = read_matrix_keypad(STATE);

        /* =====================================
           MAIN MENU
           ===================================== */
        if (mode == 0)
        {
            if (key >= 1 && key <= 4)
            {
                mode = key;

                countdown_running = 0;
                countdown_paused = 0;
                quick_start_active = 0;

                minutes = 0;
                seconds = 0;

                reset_time_entry();

                temperature = 0;
                temp_digit_count = 0;
                conv_state = 0;
                preheat_seconds = 60;

                for (unsigned char i = 0; i < 3; i++)
                {
                    temp_digits[i] = 0;
                }

                if (mode == 4)
                {
                    /* Quick Start begins immediately */
                    quick_start_active = 1;
                    quick_start_just_started = 1;

                    seconds = 30;
                    countdown_running = 1;
                    time_set = 1;

                    clcd_clear();
                    clcd_print("QUICK START", LINE1(0));
                    clcd_print("TIME: 00:30", LINE2(0));
                    clcd_print("4:+30s 5:Pause", LINE3(0));
                    clcd_print("6:Stop", LINE4(0));
                }
                else
                {
                    clcd_clear();
                }
            }
        }

        /* =====================================
           MICRO AND GRILL: TIME ENTRY
           ===================================== */
        else if ((mode == 1 || mode == 2) && !time_set)
        {
            set_time(key);
        }

        /* =====================================
           CONVECTION: TEMPERATURE ENTRY
           ===================================== */
        else if (mode == 3 && conv_state == 0)
        {
            set_temperature(key);
        }

        /* =====================================
           CONVECTION: PREHEATING
           ===================================== */
        if (mode == 3 && conv_state == 1)
        {
            if (key == 6)
            {
                stop_and_return_to_menu();
            }
        }

        /* =====================================
           CONVECTION: COOKING TIME ENTRY
           ===================================== */
        if (mode == 3 && conv_state == 2 && !time_set)
        {
            set_time(key);

            if (time_set)
            {
                conv_state = 3;
            }
        }

        /* =====================================
           MICRO AND GRILL CONTROLS
           ===================================== */
        if ((mode == 1 || mode == 2) && time_set)
        {
            if (key == 4)
            {
                countdown_running = 1;
                countdown_paused = 0;
            }
            else if (key == 5)
            {
                countdown_running = 0;
                countdown_paused = 1;
            }
            else if (key == 6)
            {
                stop_and_return_to_menu();
            }
        }

        /* =====================================
           CONVECTION: COOKING CONTROLS
           ===================================== */
        if (mode == 3 && conv_state == 3 && time_set)
        {
            if (key == 4)
            {
                countdown_running = 1;
                countdown_paused = 0;
            }
            else if (key == 5)
            {
                countdown_running = 0;
                countdown_paused = 1;
            }
            else if (key == 6)
            {
                stop_and_return_to_menu();
            }
        }

        /* =====================================
           QUICK START CONTROLS
           ===================================== */
           if (mode == 4 && quick_start_active)
           {
               if (quick_start_just_started)
               {
                   quick_start_just_started = 0;
               }
               else if (key == 4)
               {
                   if (countdown_running)
                   {
                       seconds += 30;

                       if (seconds >= 60)
                       {
                           minutes += seconds / 60;
                           seconds %= 60;
                       }

                       clcd_putch(minutes / 10 + '0', LINE2(6));
                       clcd_putch(minutes % 10 + '0', LINE2(7));
                       clcd_putch(':', LINE2(8));
                       clcd_putch(seconds / 10 + '0', LINE2(9));
                       clcd_putch(seconds % 10 + '0', LINE2(10));
                   }
                   else
                   {
                       countdown_running = 1;
                       countdown_paused = 0;
                   }
               }
               else if (key == 5)
               {
                   countdown_running = 0;
                   countdown_paused = 1;
               }
               else if (key == 6)
               {
                   stop_and_return_to_menu();
               }
           }

        /* =====================================
           PROCESS ONE-SECOND TIMER EVENT
           timer_1s_flag is set by the Timer2 interrupt logic.
           The flag is cleared here after consuming the event.
           ===================================== */
        if (timer_1s_flag)
        {
            timer_1s_flag = 0;

            /* Convection preheating countdown: reduce the simulated warm-up time and refresh its LCD digits. */
            if (mode == 3 && conv_state == 1)
            {
                if (preheat_seconds > 0)
                {
                    preheat_seconds--;
                }

                clcd_putch((preheat_seconds / 100) % 10 + '0',
                           LINE3(11));
                clcd_putch((preheat_seconds / 10) % 10 + '0',
                           LINE3(12));
                clcd_putch(preheat_seconds % 10 + '0',
                           LINE3(13));

                if (preheat_seconds == 0)
                {
                    conv_state = 2;

                    reset_time_entry();

                    clcd_clear();
                    clcd_print("PREHEAT COMPLETE", LINE1(0));
                    clcd_print("SET COOKING TIME", LINE2(0));
                }
            }

            /* Cooking countdown: Micro, Grill, Convection cooking, or Quick Start.
               Decrease the remaining time by one second and update the appropriate LCD field. */
            else if (countdown_running &&
                     ((mode == 1) ||
                      (mode == 2) ||
                      (mode == 3 && conv_state == 3) ||
                      (mode == 4 && quick_start_active)))
            {
                if (seconds > 0)
                {
                    seconds--;
                }
                else if (minutes > 0)
                {
                    minutes--;
                    seconds = 59;
                }

                /* Update correct LCD positions */
                if (mode == 4 && quick_start_active)
                {
                    clcd_putch(minutes / 10 + '0', LINE2(6));
                    clcd_putch(minutes % 10 + '0', LINE2(7));
                    clcd_putch(':', LINE2(8));
                    clcd_putch(seconds / 10 + '0', LINE2(9));
                    clcd_putch(seconds % 10 + '0', LINE2(10));
                }
                else
                {
                    display_cooking_time();
                }

                /* Cooking completed: stop the timer, show the completion message, then return to the menu. */
                if (minutes == 0 && seconds == 0)
                {
                    countdown_running = 0;
                    countdown_paused = 0;
                    time_set = 0;

                    quick_start_active = 0;
                    conv_state = 0;
                    temperature = 0;

                    mode = 0;

                    clcd_clear();
                    clcd_print("TIME UP!!!", LINE2(3));
                    clcd_print("Enjoy your meal!", LINE3(0));

                    __delay_ms(2000);
                    display_menu();
                }
            }
        }
    }
}