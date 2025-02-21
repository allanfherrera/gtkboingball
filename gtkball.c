#include <gtk/gtk.h>
#include <math.h>
#include <gst/gst.h>

#define BALL_RADIUS 64
#define SPEED 6.0
#define GRID_SIZE 64
#define WIDTH 800
#define HEIGHT 600
typedef struct {
    double x, y;
    double dx, dy;
    GstElement *sound;
    int window_width;
    int window_height;
} Ball;

static Ball ball = {400, 300, SPEED, SPEED, NULL, WIDTH, HEIGHT};

// Initialize sound
static void init_sound() {
    gst_init(NULL, NULL);
    ball.sound = gst_element_factory_make("playbin", "playbin");
    if (!ball.sound) {
        g_print("Failed to create sound element\n");
        return;
    }
    g_object_set(ball.sound, "uri", "file:///usr/share/sounds/freedesktop/stereo/bell.oga", NULL);
}

// Play bump sound
static void play_bump() {
    if (ball.sound) {
        gst_element_set_state(ball.sound, GST_STATE_NULL);
        gst_element_set_state(ball.sound, GST_STATE_PLAYING);
    }
}

// Handle window resize
static void on_window_resize(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    ball.window_width = allocation->width;
    ball.window_height = allocation->height;
    
    // Keep ball within new bounds
    if (ball.x < BALL_RADIUS) ball.x = BALL_RADIUS;
    if (ball.x > ball.window_width - BALL_RADIUS) ball.x = ball.window_width - BALL_RADIUS;
    if (ball.y < BALL_RADIUS) ball.y = BALL_RADIUS;
    if (ball.y > ball.window_height - BALL_RADIUS) ball.y = ball.window_height - BALL_RADIUS;
}

// Toggle fullscreen
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_F11) {
        static gboolean fullscreen = FALSE;
        fullscreen = !fullscreen;
        if (fullscreen) {
            gtk_window_fullscreen(GTK_WINDOW(widget));
        } else {
            gtk_window_unfullscreen(GTK_WINDOW(widget));
        }
        return TRUE;
    }
    return FALSE;
}

// Drawing function
static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    // Draw grid background
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_line_width(cr, 1);
    
    for (int x = 0; x <= ball.window_width; x += GRID_SIZE) {
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, ball.window_height);
    }
    
    for (int y = 0; y <= ball.window_height; y += GRID_SIZE) {
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, ball.window_width, y);
    }
    cairo_stroke(cr);

    // Draw the ball
    cairo_save(cr);
    cairo_translate(cr, ball.x, ball.y);
    
    cairo_arc(cr, 0, 0, BALL_RADIUS, 0, 2 * M_PI);
    cairo_clip(cr);
    
    int segments = 12;
    for (int i = 0; i < segments; i++) {
        for (int j = 0; j < segments; j++) {
            double angle1 = (i * 2 * M_PI) / segments;
            double angle2 = ((i + 1) * 2 * M_PI) / segments;
            double v1 = (j * M_PI) / segments - M_PI/2;
            double v2 = ((j + 1) * M_PI) / segments - M_PI/2;
            
            if ((i + j) % 2 == 0) {
                cairo_set_source_rgb(cr, 1, 0, 0);
            } else {
                cairo_set_source_rgb(cr, 1, 1, 1);
            }
            
            cairo_move_to(cr, 
                BALL_RADIUS * cos(angle1) * cos(v1),
                BALL_RADIUS * sin(v1));
            cairo_line_to(cr,
                BALL_RADIUS * cos(angle2) * cos(v1),
                BALL_RADIUS * sin(v1));
            cairo_line_to(cr,
                BALL_RADIUS * cos(angle2) * cos(v2),
                BALL_RADIUS * sin(v2));
            cairo_line_to(cr,
                BALL_RADIUS * cos(angle1) * cos(v2),
                BALL_RADIUS * sin(v2));
            cairo_close_path(cr);
            cairo_fill(cr);
        }
    }
    
    cairo_reset_clip(cr);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 2);
    cairo_arc(cr, 0, 0, BALL_RADIUS, 0, 2 * M_PI);
    cairo_stroke(cr);
    
    cairo_restore(cr);
    return FALSE;
}

// Animation update
static gboolean update(gpointer data) {
    GtkWidget *widget = GTK_WIDGET(data);
    gboolean bounced = FALSE;
    
    ball.x += ball.dx;
    ball.y += ball.dy;
    
    if (ball.x - BALL_RADIUS < 0 || ball.x + BALL_RADIUS > ball.window_width) {
        ball.dx = -ball.dx;
        bounced = TRUE;
    }
    if (ball.y - BALL_RADIUS < 0 || ball.y + BALL_RADIUS > ball.window_height) {
        ball.dy = -ball.dy;
        bounced = TRUE;
    }
    
    if (ball.x < BALL_RADIUS) ball.x = BALL_RADIUS;
    if (ball.x > ball.window_width - BALL_RADIUS) ball.x = ball.window_width - BALL_RADIUS;
    if (ball.y < BALL_RADIUS) ball.y = BALL_RADIUS;
    if (ball.y > ball.window_height - BALL_RADIUS) ball.y = ball.window_height - BALL_RADIUS;
    
    if (bounced) {
        play_bump();
    }
    
    gtk_widget_queue_draw(widget);
    return TRUE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *drawing_area;

    gtk_init(&argc, &argv);
    init_sound();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Boing Ball Demo");
    gtk_window_set_default_size(GTK_WINDOW(window), ball.window_width, ball.window_height);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "size-allocate", G_CALLBACK(on_window_resize), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);

    g_timeout_add(16, update, drawing_area);

    gtk_widget_show_all(window);
    gtk_main();

    if (ball.sound) {
        gst_element_set_state(ball.sound, GST_STATE_NULL);
        g_object_unref(ball.sound);
    }
    return 0;
}
