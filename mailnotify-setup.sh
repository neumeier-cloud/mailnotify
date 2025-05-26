#!/bin/bash

# Sicherstellen, dass whiptail verfügbar ist
if ! command -v whiptail &> /dev/null; then
  echo "📦 Installiere whiptail..."
  sudo apt update
  sudo apt install -y whiptail
fi

# Menü
CHOICE=$(whiptail --title "Mailnotify Installer" --menu "Wähle eine Option:" 15 60 4 \
"1" "Installieren von Mailnotify" \
"2" "Deinstallation von Mailnotify" \
"3" "Beenden" 3>&1 1>&2 2>&3)

[ $? != 0 ] && echo "Abgebrochen." && exit 1

case $CHOICE in
  1)
    REPO_URL="https://github.com/neumeier-cloud/mailnotify.git"
    INSTALL_DIR="/usr/local/bin"
    CONFIG_PATH="/etc/mailnotify.conf"

    whiptail --title "Abhängigkeiten prüfen" --infobox "Installiere Abhängigkeiten..." 8 40
    sudo apt update
    sudo apt install -y git build-essential libcurl4-openssl-dev

    # Verzeichnis vorbereiten
    [ -d /tmp/mailnotify ] && rm -rf /tmp/mailnotify

    if ! git clone "$REPO_URL" /tmp/mailnotify; then
      whiptail --msgbox "❌ Fehler beim Klonen des Repos." 8 50
      exit 1
    fi

    cd /tmp/mailnotify || exit 1

    if ! gcc mailnotify.c -o mailnotify -lcurl; then
      whiptail --msgbox "❌ Kompilierung fehlgeschlagen. Bitte 'mailnotify.c' prüfen." 8 60
      exit 1
    fi

    sudo cp mailnotify "$INSTALL_DIR/"
    sudo chmod +x "$INSTALL_DIR/mailnotify"
    sudo cp -n mailnotify.conf "$CONFIG_PATH"
    sudo chmod 600 "$CONFIG_PATH"

    whiptail --msgbox "✅ Mailnotify wurde installiert.\n\nTeste mit:\nmailnotify --to user@example.com --subject 'Test' --body '<h1>OK</h1>' --html" 12 60
    ;;

  2)
    sudo rm -f /usr/local/bin/mailnotify
    sudo rm -f /etc/mailnotify.conf
    sudo rm -f /etc/systemd/system/mailnotify.service
    sudo rm -f /etc/systemd/system/mailnotify.timer
    sudo systemctl daemon-reexec

    whiptail --msgbox "✅ Mailnotify wurde vollständig entfernt." 8 50
    ;;

  3)
    clear
    exit 0
    ;;
esac
