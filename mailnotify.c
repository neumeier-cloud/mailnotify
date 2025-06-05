#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>

#define MAX_ATTACHMENTS 20
#define MAX_RECIPIENTS 40
#define MAX_LINE 4096

typedef struct {
    char *smtp_server;
    int smtp_port;
    char *smtp_user;
    char *smtp_pass;
    char *from_mail;
} SmtpConfig;

typedef struct {
    char *to[MAX_RECIPIENTS];
    int to_count;
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

// --- Hilfsfunktionen ---

void print_help() {
    printf("mailnotify - SMTP-Mailer mit MIME, Attachments, CC/BCC, Freitext-Absender\n\n");
    printf("Nutzung:\n");
    printf("  mailnotify [OPTIONEN]\n\n");
    printf("Pflichtoptionen:\n");
    printf("  --to <EMAIL[,EMAIL2,...]>     Zieladresse(n), Komma-getrennt\n");
    printf("  --subject <TEXT>              Betreff\n");
    printf("  --body <TEXT/HTML>            Inhalt\n\n");
    printf("Optionale Flags:\n");
    printf("  --html                        Inhalt ist HTML\n");
    printf("  --from-name <NAME>            Absendername (frei)\n");
    printf("  --attach <DATEI>              Anhang (mehrfach möglich)\n");
    printf("  --cc <EMAIL[,EMAIL2,...]>     CC (mehrfach möglich)\n");
    printf("  --bcc <EMAIL[,EMAIL2,...]>    BCC (mehrfach möglich)\n");
    printf("  --priority <low|normal|high>  E-Mail-Priorität\n");
    printf("  -h, --help                    Diese Hilfe anzeigen\n\n");
    printf("Beispiel:\n");
    printf("  mailnotify --to alice@example.com,bob@example.com \\\n");
    printf("    --subject \"Backup-Status\" --body \"<b>OK</b>\" --html \\\n");
    printf("    --from-name \"Backup Bot\" --cc carol@example.com \\\n");
    printf("    --attach /tmp/log.txt --priority high\n");
    exit(0);
}

// Trimme Whitespace am Zeilenende/-anfang
char *trim(char *str) {
    char *end;
    while(*str == ' ' || *str == '\t' || *str == '\n') str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end+1) = 0;
    return str;
}

// Zerlege Komma-getrennte Empfänger
int split_emails(const char *in, char **out, int max) {
    int n = 0;
    char *copy = strdup(in);
    if(!copy) return 0;

    char *p = strtok(copy, ",");
    while (p && n < max) {
        out[n++] = strdup(trim(p));
        p = strtok(NULL, ",");
    }

    free(copy);
    return n;
}

// Konfigurationsdatei lesen
int read_config(SmtpConfig *conf, const char *filename) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        fprintf(stderr, "❌ Konfigurationsdatei %s nicht lesbar!\n", filename);
        return 0;
    }
    char line[MAX_LINE];
    while(fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if(!eq) continue;
        *eq = 0;
        char *key = trim(line), *val = trim(eq+1);
        if(strcmp(key,"smtp_server")==0) conf->smtp_server = strdup(val);
        else if(strcmp(key,"smtp_port")==0) conf->smtp_port = atoi(val);
        else if(strcmp(key,"smtp_user")==0) conf->smtp_user = strdup(val);
        else if(strcmp(key,"smtp_pass")==0) conf->smtp_pass = strdup(val);
        else if(strcmp(key,"from")==0) conf->from_mail = strdup(val);
    }
    fclose(f);
    return conf->smtp_server && conf->smtp_port && conf->smtp_user && conf->smtp_pass && conf->from_mail;
}

int main(int argc, char *argv[]) {
    MailConfig cfg = {0};
    SmtpConfig smtp = {0};

    // --- Optionen parsen
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
            if(!strcmp(optname, "to") && cfg.to_count < MAX_RECIPIENTS) {
                cfg.to_count = split_emails(optarg, cfg.to, MAX_RECIPIENTS);
            } else if(!strcmp(optname, "subject")) cfg.subject = strdup(optarg);
            else if(!strcmp(optname, "body")) cfg.body = strdup(optarg);
            else if(!strcmp(optname, "html")) cfg.html = 1;
            else if(!strcmp(optname, "from-name")) cfg.from_name = strdup(optarg);
            else if(!strcmp(optname, "attach") && cfg.attach_count < MAX_ATTACHMENTS) {
                cfg.attachments[cfg.attach_count++] = strdup(optarg);
            } else if(!strcmp(optname, "cc") && cfg.cc_count < MAX_RECIPIENTS) {
                cfg.cc_count = split_emails(optarg, cfg.cc, MAX_RECIPIENTS);
            } else if(!strcmp(optname, "bcc") && cfg.bcc_count < MAX_RECIPIENTS) {
                cfg.bcc_count = split_emails(optarg, cfg.bcc, MAX_RECIPIENTS);
            } else if(!strcmp(optname, "priority")) cfg.priority = strdup(optarg);
        }
    }

    if(cfg.to_count < 1 || !cfg.subject || !cfg.body) {
        fprintf(stderr, "❌ Pflichtoption fehlt. Nutze --help für Details.\n");
        return 1;
    }

    // --- Konfiguration einlesen
    if(!read_config(&smtp, "/etc/mailnotify.conf")) {
        fprintf(stderr, "❌ Konnte SMTP-Konfiguration nicht laden!\n");
        return 2;
    }

    CURL *curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "❌ curl konnte nicht initialisiert werden.\n");
        return 2;
    }

    // SMTP-URL bauen (SMTPS für Port 465)
    char url[512];
    snprintf(url, sizeof(url), "%s://%s:%d",
             (smtp.smtp_port==465) ? "smtps" : "smtp",
             smtp.smtp_server, smtp.smtp_port);

    curl_easy_setopt(curl, CURLOPT_USERNAME, smtp.smtp_user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp.smtp_pass);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (smtp.smtp_port==465) ? CURLUSESSL_ALL : CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, smtp.from_mail);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // für interne Netze
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // für interne Netze
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // für Debug

    // Empfängerliste
    struct curl_slist *recipients = NULL;
    for(int i=0;i<cfg.to_count;i++) recipients = curl_slist_append(recipients, cfg.to[i]);
    for(int i=0;i<cfg.cc_count;i++) recipients = curl_slist_append(recipients, cfg.cc[i]);
    for(int i=0;i<cfg.bcc_count;i++) recipients = curl_slist_append(recipients, cfg.bcc[i]);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // Header vorbereiten
    struct curl_slist *headers = NULL;
    char subject_header[1024], from_header[1024], cc_header[2048], prio_header[64];
    snprintf(subject_header, sizeof(subject_header), "Subject: %s", cfg.subject);

    // From: mit optionalem Namen
    if(cfg.from_name)
        snprintf(from_header, sizeof(from_header), "From: %s <%s>", cfg.from_name, smtp.from_mail);
    else
        snprintf(from_header, sizeof(from_header), "From: %s", smtp.from_mail);

    // Cc-Header
    cc_header[0]=0;
    if(cfg.cc_count > 0) {
        strcat(cc_header, "Cc: ");
        for(int i=0;i<cfg.cc_count;i++) {
            strcat(cc_header, cfg.cc[i]);
            if(i<cfg.cc_count-1) strcat(cc_header, ", ");
        }
    }

    headers = curl_slist_append(headers, subject_header);
    headers = curl_slist_append(headers, from_header);
    if(cfg.cc_count > 0) headers = curl_slist_append(headers, cc_header);

    // Priorität
    if(cfg.priority) {
        if(!strcmp(cfg.priority,"high")) snprintf(prio_header,sizeof(prio_header),"X-Priority: 1 (Highest)");
        else if(!strcmp(cfg.priority,"low")) snprintf(prio_header,sizeof(prio_header),"X-Priority: 5 (Lowest)");
        else snprintf(prio_header,sizeof(prio_header),"X-Priority: 3 (Normal)");
        headers = curl_slist_append(headers, prio_header);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // MIME-Mail zusammenbauen
    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    // Body
    part = curl_mime_addpart(mime);
    curl_mime_data(part, cfg.body, CURL_ZERO_TERMINATED);
    curl_mime_type(part, cfg.html ? "text/html" : "text/plain");

    // Anhänge
    for(int i=0;i<cfg.attach_count;i++) {
        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, cfg.attachments[i]);
    }

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "❌ Fehler beim Senden: %s\n", curl_easy_strerror(res));
    else
        printf("✅ E-Mail erfolgreich gesendet!\n");

    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return (res==CURLE_OK)?0:1;
}
