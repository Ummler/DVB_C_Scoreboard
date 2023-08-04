#include <gtk/gtk.h>
#include <stdbool.h>
#include <ctype.h>
#include <wchar.h>
#include "letters.h"
#include "query.h"
#include <locale.h>

// Set locale at the beginning of your program

#define CELL_SIZE 3  
#define GRID_ROWS 400
#define GRID_COLS 500
#define CIRCLE_RADIUS 1
#define NUM_LETTERS (sizeof(letters)/sizeof(letters[0]))
#define WIDTH (CELL_SIZE * GRID_COLS)
#define HEIGHT (CELL_SIZE * GRID_ROWS)
#define ROW_HEIGHT 13
#define MAX_DIR_LENGTH (GRID_COLS - 75)
#define MAX_LINE_LENGTH 30
#define MAX_TIME_LENGTH 40


// Fügen Sie diese Zeilen vor der main Funktion hinzu
void DrawHeader(int row, const wchar_t* string);
void DrawLine(int row, DepartureInfo* departures);
gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data);


bool grid[GRID_ROWS][GRID_COLS] = {false};
GtkWidget *window;
GtkWidget *drawing_area;

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    gtk_widget_set_size_request(widget, WIDTH, HEIGHT);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);  // Setze den Hintergrund auf schwarz

    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            if (grid[i][j]) {
                cairo_set_source_rgb(cr, 1, 1, 0);
                int center_x = j * CELL_SIZE + CELL_SIZE / 2;
                int center_y = i * CELL_SIZE + CELL_SIZE / 2;
                cairo_arc(cr, center_x, center_y, CIRCLE_RADIUS, 0, 2 * G_PI);
                cairo_fill(cr);
            }
        }
    }
    return FALSE;
}


int getLength(const wchar_t* string) {
    int totalWidth = 0;
    int len = wcslen(string);

    for(int i = 0; i < len; i++) {
        wchar_t character = string[i];

        // Find the width of the character from the mapping
        for(int j = 0; j < NUM_LETTERS; j++) {
            if(letters[j].c == character) {
                totalWidth += letters[j].width;
                break;
            }
        }
        totalWidth += len-1; //Für den Platz zwischen den Buchstaben        
    }

    return totalWidth;
}



void DrawLetter(int x, int y, wchar_t character) {
    for (int i = 0; i < NUM_LETTERS; i++) {
        if (letters[i].c == character) {
            for (int j = 0; j < letters[i].height; j++) {
                for (int k = 0; k < letters[i].width; k++) {
                    if (letters[i].map[j][k] == 1) {
                        grid[y + j][x + k] = true;
                    }
                }
            }
            break;
        }
    }
}



void DrawString(int x, int y, const wchar_t* string) {
    int len = wcslen(string);

    // Create a new string that includes original characters and spaces
    wchar_t* spaced_string = malloc((2 * len + 1) * sizeof(wchar_t));  // Allocate memory for new string
    for(int i = 0; i < len; i++) {
        spaced_string[2*i] = string[i];   // Copy original character
        spaced_string[2*i + 1] = L' ';     // Insert space
    }
    spaced_string[2 * len] = L'\0';  // Null-terminate the new string
    
    len = wcslen(spaced_string);  // Update length for the new string

    for (int i = 0; i < len; i++) {
        // Check if the current character is a space
        if(spaced_string[i] == L' '){
            x += 1;  // Update x to the next position; add space width
            continue;
        }
        
        // get the corresponding letter
        LetterInfo letterInfo;
        for(int j = 0; j < NUM_LETTERS; j++) {
            if(letters[j].c == spaced_string[i]) {
                letterInfo = letters[j];
                break;
            }
        }

        DrawLetter(x, y, spaced_string[i]);

        // update x to the next position; add letter width and space between letters
        x += letterInfo.width + 1;
    }

    free(spaced_string);  // Free the allocated memory
}





void DrawHeader(int row, const wchar_t* string) {
    DrawString(0, row * ROW_HEIGHT, string);
    DrawString(0, (row +1) * ROW_HEIGHT + 1, L"Linie");
    DrawString(GRID_COLS - 32, (row+1) * ROW_HEIGHT, L"in Min");
}

void DrawLine(int row, DepartureInfo* departures) {
    DrawString(0, (row) * ROW_HEIGHT, departures->lineName);
    DrawString(45, (row) * ROW_HEIGHT, departures->direction);
    DrawString(GRID_COLS - getLength(departures->realTime)-1, (row) * ROW_HEIGHT, departures->realTime);
}


void shortenAttributes(DepartureInfo* departures){
    int len = getLength(departures->direction);
    if(len > MAX_DIR_LENGTH){
        wcsncpy(departures->direction, departures->direction, MAX_DIR_LENGTH);
        departures->direction[MAX_DIR_LENGTH] = L'\0';  // Null-Terminate the new string
    }
    len = getLength(departures->lineName);
    if (len > MAX_LINE_LENGTH){
        wcsncpy(departures->lineName, departures->lineName, MAX_LINE_LENGTH);
        departures->lineName[MAX_LINE_LENGTH] = L'\0';  // Null-Terminate the new string
    }
    len = getLength(departures->realTime);
    if (len > MAX_TIME_LENGTH){
        wcsncpy(departures->realTime, departures->realTime, MAX_TIME_LENGTH);
        departures->realTime[MAX_TIME_LENGTH] = L'\0';  // Null-Terminate the new string
    }
}



gboolean repeat_function(gpointer user_data) {
    // Hier setzen wir das gesamte Raster auf 'false'
    memset(grid, false, sizeof grid);

    const int limit = 23;
    DepartureInfo departures[limit];
    struct json_object* json_data = fetchData("33000016", limit);
    parse_and_store_data(json_data, departures, limit);

    for (int i = 0; i < limit; i++) {
        long long offset = calculate_time_offset(departures[i].realTime);
        swprintf(departures[i].realTime, sizeof(departures[i].realTime)/sizeof(wchar_t), L"%lld", offset);
    }

    DrawHeader(0, L"Bahnhof Neustadt");
    for (int i = 0; i < limit; i++) {
        wprintf(L"%ls %ls %ls\n", departures[i].lineName, departures[i].direction, departures[i].realTime);
        DrawLine(2+i, &departures[i]);
    }

    gtk_widget_queue_draw(window);
    return TRUE;  // return TRUE here to keep the timer running
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    gtk_init(&argc, &argv);



window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_window_set_title(GTK_WINDOW(window), "Anzeigetafel");
gtk_widget_set_size_request (GTK_WIDGET(window), 1920, 1080);
//gtk_window_fullscreen(GTK_WINDOW(window));  // Add this line for fullscreen
g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

drawing_area = gtk_drawing_area_new();
gtk_container_add(GTK_CONTAINER(window), drawing_area);
g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_event), NULL);

    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
    GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    const gchar *myCss = "GtkDrawingArea { background-color: #000000; }";
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider), myCss, -1, NULL);

    gtk_widget_show_all(window);

    repeat_function(NULL);


    // Den Timer starten

    g_timeout_add_seconds(60, repeat_function, NULL);

    gtk_main();

    return 0;
}



