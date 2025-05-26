# Mailnotify

**mailnotify** ist ein kompakter, eigenständiger Mailclient in C für Linux/Proxmox, der SMTP-Nachrichten mit HTML-Unterstützung und Konfigurationsdatei versendet – ohne Python oder externe Abhängigkeiten.

---

## ✅ Features

- SMTP-Versand via `libcurl` (SSL / SMTPS)
- Konfigurierbare Zugangsdaten über `/etc/mailnotify.conf`
- Unterstützung für:
  - `--to` Empfänger
  - `--subject` Betreff
  - `--body` E-Mail-Inhalt (Text oder HTML)
  - `--html` HTML-Format aktivieren
- Kompatibel mit z. B. Proxmox VE 8, Debian 12

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
sudo apt install libcurl4-openssl-dev
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

## 📤 Beispielnutzung

```bash
mailnotify \
  --to admin@example.com \
  --subject "Backup abgeschlossen" \
  --body "<h1>Backup OK</h1><p>Alle Systeme normal.</p>" \
  --html
```

---

## 🔁 Automatisierung mit systemd

### 1. systemd-Service

Pfad: `/etc/systemd/system/mailnotify.service`

```ini
[Unit]
Description=Mailnotify: täglicher Bericht

[Service]
Type=oneshot
ExecStart=/usr/local/bin/mailnotify \
  --to admin@example.com \
  --subject "Systemstatus" \
  --body "<h1>Status OK</h1>" \
  --html
```

### 2. systemd-Timer

Pfad: `/etc/systemd/system/mailnotify.timer`

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
mailnotify \
  --to admin@example.com \
  --subject "Test erfolgreich" \
  --body "<h1>Mailnotify funktioniert</h1>" \
  --html
```

---

## 🧰 Installieren/Deinstallieren mit grafischem Menü (TUI)

Du kannst Mailnotify mit einem interaktiven Menü (TUI) über `whiptail` installieren oder deinstallieren.

### Ausführen:

```bash
chmod +x mailnotify-setup.sh
./mailnotify-setup.sh
```

### Voraussetzungen:

```bash
sudo apt update
sudo apt install -y whiptail git gcc libcurl4-openssl-dev
```

### Funktionen:

- 📥 Klonen des Projekts per Git
- 🔨 Kompilieren der Binary (`gcc`)
- ⚙️ Installation nach `/usr/local/bin`
- 🛠️ Beispiel-Konfiguration nach `/etc/`
- 🧹 Deinstallation aller Dateien über Menü

Das Skript fragt dich im Menü nach der gewünschten Aktion:

```
Mailnotify Installer
--------------------
1) Installieren von Mailnotify
2) Deinstallation von Mailnotify
3) Beenden
```

---

## 🛡️ Sicherheitshinweis

- Passwörter liegen im Klartext in der Konfigurationsdatei.  
- Nur auf abgesicherten Systemen verwenden.  
- TLS-Zertifikatsprüfung ist deaktiviert – nur für interne Nutzung empfohlen.

---

## ℹ️ Zusätzliche Hinweise

- Stelle sicher, dass dein SMTP-Server TLS/SSL auf Port 465 unterstützt.  
- Konfigurationsdatei sollte **nur für root lesbar** sein.  
- `mailnotify` läuft synchron – prüfe Exit-Codes bei Skriptnutzung.  
- Für produktive Sicherheit ggf. TLS-Verifikation im Code aktivieren.  
- Diese Software ist experimentell und sollte vor dem produktiven Einsatz getestet werden.

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
