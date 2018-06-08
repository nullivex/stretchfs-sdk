#include <stdio.h>

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

int main(int argc, char *argv[]){
    printf("Hello, World!\n");

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL; /* http headers to send with request */

    json_object *json;                                      /* json post body */
    enum json_tokener_error jerr = json_tokener_success;    /* json parse error */

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {
        curl_slist_append(headers, "Accept: application/json");
        curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, "http://some.site:8161/user/login");
        /* Now specify the POST data */
        json = json_object_new_object();
        json_object_object_add(json, "username", json_object_new_string("fusetest"));
        json_object_object_add(json, "password", json_object_new_string("SuPerSeCreT"));
        printf("%s\n",json_object_to_json_string_ext(json,JSON_C_TO_STRING_PRETTY));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(json,JSON_C_TO_STRING_PLAIN));

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    printf("Goodbye, World!\n");
    return 0;
}