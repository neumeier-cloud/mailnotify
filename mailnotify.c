
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>

#define MAX_ATTACHMENTS 10
#define MAX_RECIPIENTS 10

typedef struct {
    char *to;
    char *subject;
    char *body;
    char *from_name;
    int html;
    char *attachments[MAX_ATTACHMENTS];
    int attach_count;
    char *cc[MAX_RECIPIENTS];
    int cc_count;
    char *bcc[MAX_RECIPIENTS];
    int bcc_count;
    char *priority;
} MailConfig;

void print_help() {
    printf("mailnotify - Einfacher SMTP-Mailer für Linux mit MIME-Support\n\n");
    printf("Nutzung:\n");
    printf("  mailnotify [OPTIONEN]\n\n");
    printf("Pflichtoptionen:\n");
    printf("  --to <EMAIL>              Zieladresse\n");
    printf("  --subject <TEXT>          Betreff\n");
    printf("  --body <TEXT/HTML>        Inhalt\n\n");
    printf("Optionale Flags:\n");
    printf("  --html                    Inhalt ist HTML\n");
    printf("  --from-name <NAME>        Absendername\n");
    printf("  --attach <DATEI>          Anhang hinzufügen (mehrfach möglich)\n");
    printf("  --cc <EMAIL>              Cc-Empfänger (mehrfach möglich)\n");
    printf("  --bcc <EMAIL>             Bcc-Empfänger (mehrfach möglich)\n");
    printf("  --priority <low|normal|high>  E-Mail-Priorität setzen\n");
    printf("  -h, --help                Diese Hilfe anzeigen\n\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    MailConfig cfg = {0};

    static struct option long_options[] = {
        {"to", required_argument, 0, 0},
        {"subject", required_argument, 0, 0},
        {"body", required_argument, 0, 0},
        {"html", no_argument, 0, 0},
        {"from-name", required_argument, 0, 0},
        {"attach", required_argument, 0, 0},
        {"cc", required_argument, 0, 0},
        {"bcc", required_argument, 0, 0},
        {"priority", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        if (opt == 'h') print_help();
        if (opt == 0) {
            const char *optname = long_options[option_index].name;
            if (!strcmp(optname, "to")) cfg.to = optarg;
            else if (!strcmp(optname, "subject")) cfg.subject = optarg;
            else if (!strcmp(optname, "body")) cfg.body = optarg;
            else if (!strcmp(optname, "html")) cfg.html = 1;
            else if (!strcmp(optname, "from-name")) cfg.from_name = optarg;
            else if (!strcmp(optname, "attach") && cfg.attach_count < MAX_ATTACHMENTS)
                cfg.attachments[cfg.attach_count++] = optarg;
            else if (!strcmp(optname, "cc") && cfg.cc_count < MAX_RECIPIENTS)
                cfg.cc[cfg.cc_count++] = optarg;
            else if (!strcmp(optname, "bcc") && cfg.bcc_count < MAX_RECIPIENTS)
                cfg.bcc[cfg.bcc_count++] = optarg;
            else if (!strcmp(optname, "priority")) cfg.priority = optarg;
        }
    }

    if (!cfg.to || !cfg.subject || !cfg.body) {
        fprintf(stderr, "❌ Pflichtoption fehlt. Nutze --help für Details.\n");
        return 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "❌ curl konnte nicht initialisiert werden.\n");
        return 1;
    }

    struct curl_slist *recipients = NULL;
    char url[256], from[256], header_priority[64];
    char *smtp_server = "smtp://mail.your-server.de:465";
    char *smtp_user = "no-reply@neumeier.cloud";
    char *smtp_pass = "DEIN_PASSWORT";
    char *mail_from = smtp_user;

    if (cfg.from_name)
        snprintf(from, sizeof(from), "%s <%s>", cfg.from_name, mail_from);
    else
        snprintf(from, sizeof(from), "%s", mail_from);

    curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_pass);
    curl_easy_setopt(curl, CURLOPT_URL, smtp_server);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from);

    recipients = curl_slist_append(recipients, cfg.to);
    for (int i = 0; i < cfg.cc_count; i++) recipients = curl_slist_append(recipients, cfg.cc[i]);
    for (int i = 0; i < cfg.bcc_count; i++) recipients = curl_slist_append(recipients, cfg.bcc[i]);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    part = curl_mime_addpart(mime);
    curl_mime_data(part, cfg.body, CURL_ZERO_TERMINATED);
    curl_mime_type(part, cfg.html ? "text/html" : "text/plain");

    for (int i = 0; i < cfg.attach_count; i++) {
        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, cfg.attachments[i]);
    }

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    struct curl_slist *headers = NULL;
    char subject_header[256];
    snprintf(subject_header, sizeof(subject_header), "Subject: %s", cfg.subject);
    headers = curl_slist_append(headers, subject_header);

    if (cfg.priority) {
        if (strcmp(cfg.priority, "high") == 0)
            headers = curl_slist_append(headers, "X-Priority: 1 (Highest)");
        else if (strcmp(cfg.priority, "low") == 0)
            headers = curl_slist_append(headers, "X-Priority: 5 (Lowest)");
        else
            headers = curl_slist_append(headers, "X-Priority: 3 (Normal)");
    }

    headers = curl_slist_append(headers, from);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "❌ Fehler beim Senden: %s\n", curl_easy_strerror(res));
    else
        printf("✅ E-Mail gesendet an %s\n", cfg.to);

    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);
    return 0;
}
