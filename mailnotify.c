#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define CONFIG_PATH "/etc/mailnotify.conf"

typedef struct {
    char smtp_server[256];
    int smtp_port;
    char smtp_user[256];
    char smtp_pass[256];
    char from[256];
} Config;

int load_config(Config *cfg) {
    FILE *file = fopen(CONFIG_PATH, "r");
    if (!file) {
        perror("Konfigurationsdatei fehlt");
        return 0;
    }
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "smtp_server")) sscanf(line, "smtp_server = %255s", cfg->smtp_server);
        if (strstr(line, "smtp_port"))   sscanf(line, "smtp_port = %d", &cfg->smtp_port);
        if (strstr(line, "smtp_user"))   sscanf(line, "smtp_user = %255s", cfg->smtp_user);
        if (strstr(line, "smtp_pass"))   sscanf(line, "smtp_pass = %255s", cfg->smtp_pass);
        if (strstr(line, "from"))        sscanf(line, "from = %255s", cfg->from);
    }
    fclose(file);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Nutzung: %s --to <empfaenger> --subject <betreff> --body <nachricht> [--html]\n", argv[0]);
        return 1;
    }

    const char *to = NULL, *subject = NULL, *body = NULL;
    int html = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--to")) to = argv[++i];
        else if (!strcmp(argv[i], "--subject")) subject = argv[++i];
        else if (!strcmp(argv[i], "--body")) body = argv[++i];
        else if (!strcmp(argv[i], "--html")) html = 1;
    }

    if (!to || !subject || !body) {
        fprintf(stderr, "Fehlende Parameter.\n");
        return 1;
    }

    Config cfg;
    if (!load_config(&cfg)) return 1;

    CURL *curl = curl_easy_init();
    if (!curl) return 1;

    struct curl_slist *recipients = NULL;
    char payload[4096];
    snprintf(payload, sizeof(payload),
             "To: %s\r\nFrom: %s\r\nSubject: %s\r\n%s\r\n%s\r\n",
             to, cfg.from, subject,
             html ? "Content-Type: text/html" : "Content-Type: text/plain",
             body);

    curl_easy_setopt(curl, CURLOPT_USERNAME, cfg.smtp_user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, cfg.smtp_pass);
    char url[300];
    snprintf(url, sizeof(url), "smtps://%s:%d", cfg.smtp_server, cfg.smtp_port);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, cfg.from);
    recipients = curl_slist_append(NULL, to);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_READDATA, fmemopen(payload, strlen(payload), "r"));
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "Fehler: %s\n", curl_easy_strerror(res));
    else
        printf("✅ Mail erfolgreich gesendet an: %s\n", to);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return 0;
}
