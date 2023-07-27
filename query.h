#include <json-c/json.h>
#include <string.h>
#include <time.h>

struct json_object* fetchData(char* stopid, int limit);

typedef struct {
    wchar_t lineName[60];
    wchar_t direction[400];
    wchar_t realTime[100];
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

        if(json_object_object_get_ex(Departure, "LineName", &LineName) && LineName != NULL) {
            mbstowcs(departures[i].lineName, json_object_get_string(LineName), sizeof(departures[i].lineName)/sizeof(wchar_t));
        }
        if(json_object_object_get_ex(Departure, "Direction", &Direction) && Direction != NULL) {
            mbstowcs(departures[i].direction, json_object_get_string(Direction), sizeof(departures[i].direction)/sizeof(wchar_t));
        }
        if(json_object_object_get_ex(Departure, "RealTime", &RealTime) && RealTime != NULL) {
            mbstowcs(departures[i].realTime, json_object_get_string(RealTime), sizeof(departures[i].realTime)/sizeof(wchar_t));
        }
        wprintf(L"%ls, %ls, %ls\n", departures[i].lineName, departures[i].direction, departures[i].realTime);
    }
}

wchar_t* extract_timestamp(const wchar_t* realTime) {
    wchar_t* timestamp = (wchar_t*)malloc(sizeof(wchar_t) * 14);  // Allocate memory for timestamp
    wcsncpy(timestamp, realTime + 6, 13);  // Copy the 7th to 19th characters
    timestamp[13] = L'\0';  // Null-terminate the new string


    // No need to check for errors in this case
    
    return timestamp;
}






long long get_current_unix_timestamp_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000LL + spec.tv_nsec / 1e6;  // Convert to milliseconds
}


long long calculate_time_offset(const wchar_t* realTime) {

    wchar_t* timestamp_str = extract_timestamp(realTime);
    if (timestamp_str == NULL) {
    printf("Error: timestamp_str is NULL\n");
    return -1;
}


    long long timestamp_ms = wcstoll(timestamp_str, NULL, 10);  // Convert to long long
    free(timestamp_str);  // Free allocated memory

    long long current_timestamp_ms = get_current_unix_timestamp_ms();

    long long diff_ms = timestamp_ms - current_timestamp_ms;
    long long diff_min = diff_ms / (60 * 1000);  // Convert to minutes


    if(diff_min < 0) diff_min = 0;  // If negative, set to 0

    return diff_min;
}


