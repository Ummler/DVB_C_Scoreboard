#include <json-c/json.h>
#include <string.h>
#include <time.h>

struct json_object* fetchData(char* stopid, int limit);

typedef struct {
    char lineName[20];
    char direction[100];
    char realTime[50];
} DepartureInfo;

void parse_and_store_data(struct json_object *parsed_json, DepartureInfo* departures, int limit) {
    struct json_object *Departures;
    struct json_object *Departure;
    struct json_object *LineName;
    struct json_object *Direction;
    struct json_object *RealTime;

    json_object_object_get_ex(parsed_json, "Departures", &Departures);
    size_t len = json_object_array_length(Departures);

    for (size_t i = 0; i < len && i < limit; i++) {
        Departure = json_object_array_get_idx(Departures, i);

        json_object_object_get_ex(Departure, "LineName", &LineName);
        json_object_object_get_ex(Departure, "Direction", &Direction);
        json_object_object_get_ex(Departure, "RealTime", &RealTime);

        strncpy(departures[i].lineName, json_object_get_string(LineName), 20);
        strncpy(departures[i].direction, json_object_get_string(Direction), 100);
        strncpy(departures[i].realTime, json_object_get_string(RealTime), 50);
    }
}

char* extract_timestamp(const char* realTime) {
    char* timestamp = (char*)malloc(sizeof(char) * 14);  // Allocate memory for timestamp
    printf("%s\n", "Test");
    sscanf(realTime, "/Date(%13[0-9]-0000)/", timestamp);
    return timestamp;
}

long long get_current_unix_timestamp_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000LL + spec.tv_nsec / 1e6;  // Convert to milliseconds
}

long long calculate_time_offset(const char* realTime) {
    char* timestamp_str = extract_timestamp(realTime);
    long long timestamp_ms = atoll(timestamp_str);  // Convert to long long
    free(timestamp_str);  // Free allocated memory

    long long current_timestamp_ms = get_current_unix_timestamp_ms();

    long long diff_ms = timestamp_ms - current_timestamp_ms;
    long long diff_min = diff_ms / (60 * 1000);  // Convert to minutes

    if(diff_min < 0) diff_min = 0;  // If negative, set to 0

    return diff_min;
}
