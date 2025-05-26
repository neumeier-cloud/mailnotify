# Mailnotify

**mailnotify** ist ein moderner, eigenständiger SMTP-Mailclient in C für Linux/Proxmox.  
Unterstützt HTML-Mails, beliebige Anhänge, freie Absendernamen, Priorität, Mehrfachempfänger und Konfigdatei.

---

## ✅ Features

- SMTP-Versand via `libcurl` (SMTPS/SSL)
- Konfigurierbar über `/etc/mailnotify.conf`
- Empfänger, Cc, Bcc: mehrere Adressen (Komma-getrennt)
- Anhänge (`--attach`)
- HTML oder Text
- Freier Absendername (`--from-name`)
- Priorität (`--priority`)
- TUI-Installer (`mailnotify-setup.sh`)
- Kompatibel mit Proxmox, Debian, Ubuntu

---

## 🔧 Konfiguration

Pfad: `/etc/mailnotify.conf`

```ini
smtp_server = mail.your-server.de
smtp_port = 465
smtp_user = no-reply@neumeier.cloud
smtp_pass = DEIN_PASSWORT
from       = no-reply@neumeier.cloud
```

**Berechtigung sichern:**

```bash
sudo chmod 600 /etc/mailnotify.conf
```

---

## 🛠️ Kompilierung & Installation

### 1. Abhängigkeit installieren

```bash
sudo apt update
sudo apt install libcurl4-openssl-dev build-essential
```

### 2. Build ausführen (mit Build-Skript)

```bash
chmod +x build.sh
./build.sh
```

### Alternativ manuell kompilieren:

```bash
gcc mailnotify.c -o mailnotify -lcurl
```

### 3. Binary installieren

```bash
sudo cp mailnotify /usr/local/bin/
sudo chmod +x /usr/local/bin/mailnotify
```

### 4. Konfigurationsdatei einrichten

```bash
sudo cp mailnotify.conf /etc/
sudo chmod 600 /etc/mailnotify.conf
```

---

## 🧰 Installieren/Deinstallieren mit grafischem Menü (TUI)

```bash
curl -o mailnotify-setup.sh https://raw.githubusercontent.com/neumeier-cloud/mailnotify/main/mailnotify-setup.sh
chmod +x mailnotify-setup.sh
./mailnotify-setup.sh
```

---

## 📤 Beispielnutzung

### Einfache Mail

```bash
mailnotify \
  --to "admin@example.com" \
  --subject "Backup abgeschlossen" \
  --body "<h1>Backup OK</h1><p>Alle Systeme normal.</p>" \
  --html
```

### Mehrere Empfänger, CC und BCC

```bash
mailnotify \
  --to "user1@firma.de,user2@domain.de" \
  --cc "boss@firma.de,teamlead@firma.de" \
  --bcc "audit@domain.de" \
  --subject "Status" \
  --body "Der nächtliche Report ist angehängt." \
  --attach /tmp/report.pdf \
  --priority high \
  --from-name "Proxmox Backup"
```

---

## 🔁 Automatisierung mit systemd

### 1. Service

`/etc/systemd/system/mailnotify.service`  
```ini
[Unit]
Description=Mailnotify: täglicher Bericht

[Service]
Type=oneshot
ExecStart=/usr/local/bin/mailnotify --to user1@domain.de,user2@domain.de --subject "Status" --body "Alles OK." --html
```

### 2. Timer

`/etc/systemd/system/mailnotify.timer`  
```ini
[Unit]
Description=Täglicher Mailnotify-Aufruf

[Timer]
OnCalendar=*-*-* 08:00:00
Persistent=true

[Install]
WantedBy=timers.target
```

### 3. Aktivieren

```bash
sudo systemctl daemon-reexec
sudo systemctl enable --now mailnotify.timer
```

---

## 🧪 Manuelle Test-Mail

```bash
mailnotify --to "test@example.com" --subject "Test" --body "<h1>Mailnotify Test</h1>" --html
```

---

## 📖 Hilfe

Alle Optionen auf einen Blick:

```bash
mailnotify --help
```

```
mailnotify - Einfacher SMTP-Mailer für Linux mit MIME-Support

Nutzung:
  mailnotify [OPTIONEN]

Pflichtoptionen:
  --to <EMAIL[,EMAIL...]>         Zieladresse(n), Komma-getrennt
  --subject <TEXT>                Betreff
  --body <TEXT/HTML>              Inhalt

Optionale Flags:
  --html                          Inhalt ist HTML
  --from-name "<NAME>"            Absendername
  --attach <DATEI>                Anhang (mehrfach möglich)
  --cc <EMAIL[,EMAIL...]>         Cc-Empfänger (mehrfach möglich)
  --bcc <EMAIL[,EMAIL...]>        Bcc-Empfänger (mehrfach möglich)
  --priority <low|normal|high>    E-Mail-Priorität
  -h, --help                      Hilfe anzeigen
```

---

## 🛡️ Sicherheitshinweis

- Passwörter liegen im Klartext in der Konfigurationsdatei.  
- Nur auf abgesicherten Systemen verwenden.  
- TLS-Zertifikatsprüfung ist deaktiviert – nur für interne Nutzung empfohlen.

---

## 📁 Projektstruktur

```
mailnotify/
├── mailnotify.c
├── build.sh
├── mailnotify.conf
├── mailnotify.service
├── mailnotify.timer
├── mailnotify-setup.sh
└── README.md
```
