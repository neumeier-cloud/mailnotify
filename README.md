# Mailnotify

**Mailnotify** ist ein moderner, eigenständiger SMTP-Mailclient in C für Linux/Proxmox.  
Er unterstützt HTML-Mails, beliebige Anhänge, freie Absendernamen, Priorität, Mehrfachempfänger und eine Konfigurationsdatei.

---

## ✅ Features

- **SMTP-Versand** via `libcurl` (SMTPS/SSL)
- **Konfigurierbar** über `/etc/mailnotify.conf`
- **Empfänger**, **Cc**, **Bcc**: mehrere Adressen (Komma-getrennt)
- **Anhänge** (`--attach`)
- **HTML** oder **Text**
- **Freier Absendername** (`--from-name`)
- **Priorität** (`--priority`)
- **TUI-Installer** (`mailnotify-setup.sh`)
- Kompatibel mit **Proxmox**, **Debian**, **Ubuntu**

---

## ⚙️ Konfiguration

Pfad: `/etc/mailnotify.conf`

```ini
smtp_server = mail.your-server.de
smtp_port = 465
smtp_user = no-reply@neumeier.cloud
smtp_pass = DEIN_PASSWORT
from       = no-reply@neumeier.cloud
```

**Hinweis:** Stelle sicher, dass die Datei nur für den Benutzer lesbar ist:

```bash
sudo chmod 600 /etc/mailnotify.conf
```

---

## 🛠️ Installation

### 1. Abhängigkeiten installieren

```bash
sudo apt update
sudo apt install -y libcurl4-openssl-dev build-essential git
```

### 2. Repository klonen und kompilieren

```bash
git clone https://github.com/neumeier-cloud/mailnotify.git
cd mailnotify
gcc mailnotify.c -o mailnotify -lcurl
sudo cp mailnotify /usr/local/bin/
sudo chmod +x /usr/local/bin/mailnotify
sudo cp mailnotify.conf /etc/
sudo chmod 600 /etc/mailnotify.conf
```

---

## 🧰 Komfort-Installer

Alternativ mit grafischem Menü (whiptail):

```bash
curl -o mailnotify-setup.sh https://raw.githubusercontent.com/neumeier-cloud/mailnotify/main/mailnotify-setup.sh
chmod +x mailnotify-setup.sh
./mailnotify-setup.sh
```

Der Installer kann:

- Installieren
- Rebuild
- Deinstallieren (alles löschen)

---

## 📤 Beispiel-Nutzung

### Einfache HTML-Mail

```bash
mailnotify \
  --to "alice@example.com" \
  --subject "Testmail" \
  --body "<h1>Test erfolgreich!</h1>" \
  --html
```

### Mit Anhang, CC, BCC, From-Name, hoher Priorität

```bash
mailnotify \
  --to "alice@example.com,bob@example.com" \
  --cc "carol@example.com" \
  --bcc "daniel@example.com" \
  --subject "Backup abgeschlossen" \
  --body "<h2>Backup-Status: OK</h2><p>Siehe Anhang.</p>" \
  --html \
  --from-name "Backup Service" \
  --attach /var/log/backup.log \
  --priority high
```

---

## 🔄 Automatisierung (systemd)

### Service (`/etc/systemd/system/mailnotify.service`)

```ini
[Unit]
Description=Mailnotify: täglicher Bericht

[Service]
Type=oneshot
ExecStart=/usr/local/bin/mailnotify \
  --to alice@example.com,bob@example.com \
  --cc carol@example.com \
  --subject "Täglicher Status" \
  --body "Der tägliche Statusbericht." \
  --html \
  --from-name "Proxmox Notifier"
```

### Timer (`/etc/systemd/system/mailnotify.timer`)

```ini
[Unit]
Description=Täglicher Mailnotify-Aufruf

[Timer]
OnCalendar=*-*-* 08:00:00
Persistent=true

[Install]
WantedBy=timers.target
```

Aktivieren mit:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now mailnotify.timer
```

---

## 🧪 Manuelle Test-Mail

```bash
mailnotify --to "alice@example.com" --subject "Test" --body "Testmail von Mailnotify"
```

---

## 🆘 Hilfe/Optionen

```bash
mailnotify --help
```

```
mailnotify - SMTP-Mailer mit MIME, Attachments, CC/BCC, Freitext-Absender

Nutzung:
  mailnotify [OPTIONEN]

Pflichtoptionen:
  --to <EMAIL[,EMAIL2,...]>     Zieladresse(n), Komma-getrennt
  --subject <TEXT>              Betreff
  --body <TEXT/HTML>            Inhalt

Optionale Flags:
  --html                        Inhalt ist HTML
  --from-name <NAME>            Absendername (frei)
  --attach <DATEI>              Anhang (mehrfach möglich)
  --cc <EMAIL[,EMAIL2,...]>     CC (mehrfach möglich)
  --bcc <EMAIL[,EMAIL2,...]>    BCC (mehrfach möglich)
  --priority <low|normal|high>  E-Mail-Priorität
  -h, --help                    Diese Hilfe anzeigen

Beispiel:
  mailnotify --to alice@example.com,bob@example.com --subject "Report" --body "<b>Fertig!</b>" --html --attach /tmp/log.txt
```

---

## 🔐 Sicherheitshinweis

- Passwörter liegen **im Klartext** in der Konfigurationsdatei.
- `/etc/mailnotify.conf` **unbedingt** auf 600 setzen.
- Für interne Nutzung – TLS/SSL wird verwendet, aber Verifizierung ist für maximale Kompatibilität deaktiviert.

---

## 📂 Verzeichnisstruktur

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

---

## ❓ Support

Fehler, Fragen oder Erweiterungswünsche?  
Einfach Issue auf GitHub eröffnen oder Support-Mail an: `admin@example.com`  
Natürlich kannst du auch hier im Chat jederzeit nachfragen!

---

**Happy Mailing!**
