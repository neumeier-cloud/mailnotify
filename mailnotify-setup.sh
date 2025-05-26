#!/bin/bash

# Prüfe auf notwendige Abhängigkeiten
if ! command -v whiptail &> /dev/null; then
  echo "whiptail wird installiert..."
  sudo apt update && sudo apt install -y whiptail
fi

# Hauptmenü anzeigen
CHOICE=$(whiptail --title "Mailnotify Installer" --menu "Wähle eine Option:" 15 60 4 \
"1" "Installieren von Mailnotify" \
"2" "Deinstallation von Mailnotify" \
"3" "Beenden" 3>&1 1>&2 2>&3)

EXIT_STATUS=$?
if [ $EXIT_STATUS != 0 ]; then
  echo "Abgebrochen."
  exit 1
fi

case $CHOICE in
  1)
    REPO_URL="https://github.com/neumeier-cloud/mailnotify.git"
    INSTALL_DIR="/usr/local/bin"
    CONFIG_PATH="/etc/mailnotify.conf"

    whiptail --title "Installation" --msgbox "Lade Repository von $REPO_URL ..." 8 50

    if ! command -v git &> /dev/null; then
      whiptail --title "Fehler" --msgbox "Git ist nicht installiert. Bitte mit: sudo apt install git" 8 50
      exit 1
    fi

    # Verzeichnis vorab löschen, wenn vorhanden
    [ -d /tmp/mailnotify ] && rm -rf /tmp/mailnotify

    git clone "$REPO_URL" /tmp/mailnotify || { whiptail --msgbox "❌ Fehler beim Klonen" 8 40; exit 1; }

    cd /tmp/mailnotify || exit 1

    sudo apt install -y libcurl4-openssl-dev

    gcc mailnotify.c -o mailnotify -lcurl || { whiptail --msgbox "❌ Kompilierung fehlgeschlagen" 8 40; exit 1; }

    sudo cp mailnotify "$INSTALL_DIR/"
    sudo chmod +x "$INSTALL_DIR/mailnotify"

    sudo cp -n mailnotify.conf "$CONFIG_PATH"
    sudo chmod 600 "$CONFIG_PATH"

    whiptail --title "Fertig" --msgbox "✅ Mailnotify wurde installiert.\n\nTest mit:\nmailnotify --to test@example.com --subject 'Test' --body '<h1>OK</h1>' --html" 12 60
    ;;

  2)
    sudo rm -f /usr/local/bin/mailnotify
    sudo rm -f /etc/mailnotify.conf
    sudo rm -f /etc/systemd/system/mailnotify.service
    sudo rm -f /etc/systemd/system/mailnotify.timer
    sudo systemctl daemon-reexec

    whiptail --title "Deinstallation" --msgbox "✅ Mailnotify wurde entfernt." 8 50
    ;;

  3)
    clear
    exit 0
    ;;
esac
