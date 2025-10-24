#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_android.h>

#include <android/log.h>
#include <stdio.h>

#define LOG_TAG "MyNativeLib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct touch_event {
    float x;
    float y;
    float dx;
    float dy;
};

int main(int argc, char **argv) {
    al_init();
    al_install_touch_input();

    ALLEGRO_DISPLAY *display = al_create_display(0, 0);
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1 / 60.0);
    ALLEGRO_FONT* font = al_create_builtin_font();

    int redraw = 1;
    struct touch_event last_touched = { 0 };

    ALLEGRO_EVENT_SOURCE* touchy = al_get_touch_input_event_source();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, touchy);

    al_start_timer(timer);

    LOGI("Hello world!");

    ALLEGRO_EVENT event;
    while (1) {

        al_wait_for_event(queue, &event);

        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                redraw = 1;
                break;
            case ALLEGRO_EVENT_TOUCH_BEGIN:
            case ALLEGRO_EVENT_TOUCH_MOVE:
            case ALLEGRO_EVENT_TOUCH_END:
                last_touched.x = event.touch.x;
                last_touched.y = event.touch.y;
                last_touched.dx = event.touch.dx;
                last_touched.dy = event.touch.dy;
                break;
            case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
                LOGI("Display going to sleep!");
                al_stop_timer(timer);
                al_uninstall_touch_input();
                al_unregister_event_source(queue, touchy);
                al_acknowledge_drawing_halt(display);
                break;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                LOGI("Display switching out!");
                al_uninstall_touch_input();
                al_unregister_event_source(queue, touchy);
                break;
            case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
                LOGI("Display awakening!");
                al_acknowledge_drawing_resume(display);
                al_rest(0.05);
                al_start_timer(timer);
                // al_install_touch_input();
                al_install_touch_input();
                touchy = al_get_touch_input_event_source();
                al_register_event_source(queue, touchy);
                redraw = 0;
                break;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
                LOGI("Display refocused!");
                al_install_touch_input();
                touchy = al_get_touch_input_event_source();
                al_register_event_source(queue, touchy);
                break;
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                al_acknowledge_resize(display);
            default:
                LOGI("%d\n", event.type);
        }

        if (redraw) {
            if (!al_event_queue_is_empty(queue)) continue;
            al_clear_to_color(al_map_rgb_f(0, al_get_time() - (int) (al_get_time()), 1));
            al_draw_textf(font, al_map_rgb(255, 255, 255), 50, 50, 0, "%.2f %.2f %.2f %.2f",
                          last_touched.x, last_touched.y, last_touched.dx, last_touched.dy);
            al_draw_filled_circle(last_touched.x, last_touched.y, 7,
                                  al_map_rgb(0, 0, 0));
            al_flip_display();
            redraw = 0;
        }
    }
    return 0;
}