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
