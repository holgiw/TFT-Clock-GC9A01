#!/usr/bin/env bash
# Flasht die Uhr unter Linux. Aufruf: ./flashESP.sh 0  (fuer /dev/ttyACM0),
# auch ./flashESP.sh ttyACM0 oder ./flashESP.sh /dev/ttyACM0.
# Ohne Parameter werden alle seriellen Schnittstellen aufgelistet und
# angeschlossene Uhren (ESP32-S2, USB-Kennung 303a) erkannt - bei genau einer
# Uhr wird diese ohne Rueckfrage verwendet.
#
# Der ESP32-S2 meldet sich mit ZWEI verschiedenen Schnittstellen: laufend
# (Arduino USB-CDC) und im Download-Modus (ROM, USB-Kennung 303a:0002). Laeuft
# die Uhr, wird sie wie in der Arduino IDE per 1200-Baud-Signal in den
# Download-Modus neu gestartet und die dann neu erscheinende Schnittstelle
# verwendet.
#
# Optional als zweiter Parameter der Displaytyp: ./flashESP.sh 0 GC9D01
# (1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341). Ohne Parameter: zuerst die Uhr
# suchen, eine laufende Uhr nach ihrem Displaytyp fragen (Vorwahl), dann
# Displaytyp und WLAN abfragen, danach flashen und beides per USB an die Uhr senden (siehe
# handleSerialCommands() in display.h), zuletzt die Uhrzeit des PCs. Das
# WLAN-Passwort wird verdeckt eingegeben und nirgends gespeichert.
#
# Nur die Uhrzeit des PCs an die laufende Uhr senden (ohne Flashen):
# ./flashESP.sh --time [Schnittstelle]  bzw.  ./setTime.sh [Schnittstelle]
#
# Flashes the clock on Linux. Usage: ./flashESP.sh 0  (for /dev/ttyACM0),
# also ./flashESP.sh ttyACM0 or ./flashESP.sh /dev/ttyACM0.
# Without a parameter all serial ports are listed and connected clocks
# (ESP32-S2, USB id 303a) are detected - with exactly one clock it is used
# without asking.
#
# The ESP32-S2 shows up with TWO different ports: running (Arduino USB-CDC)
# and in download mode (ROM, USB id 303a:0002). If the clock is running, it is
# restarted into download mode via the 1200 baud signal like the Arduino IDE
# does, and the port that then appears is used.
#
# Optionally the display type as second parameter: ./flashESP.sh 0 GC9D01
# (1/GC9A01, 2/GC9A01_WITH_BACKLIGHT, 3/GC9D01, 4/ILI9341). Without a parameter: first
# find the clock, ask a running clock for its display type (preselection),
# then ask for display type and WiFi, afterwards flash and send both to the clock via USB (see handleSerialCommands() in
# display.h), finally the PC's time. The WiFi password is entered hidden and
# stored nowhere.
#
# Only send the PC's time to the running clock (without flashing):
# ./flashESP.sh --time [port]  or  ./setTime.sh [port]

cd "$(dirname "$0")" || exit 1

# USB-Kennung "vid:pid" einer Schnittstelle (klein geschrieben), leer wenn unbekannt
# USB id "vid:pid" of a port (lower case), empty if unknown
usb_id() {
    local dev="$1" vid="" pid="" dir
    if command -v udevadm >/dev/null 2>&1; then
        vid=$(udevadm info -q property -n "$dev" 2>/dev/null | sed -n 's/^ID_VENDOR_ID=//p')
        pid=$(udevadm info -q property -n "$dev" 2>/dev/null | sed -n 's/^ID_MODEL_ID=//p')
    fi
    if [ -z "$vid" ]; then
        dir=$(readlink -f "/sys/class/tty/$(basename "$dev")/device" 2>/dev/null)
        while [ -n "$dir" ] && [ "$dir" != "/" ]; do
            if [ -f "$dir/idVendor" ]; then
                vid=$(cat "$dir/idVendor"); pid=$(cat "$dir/idProduct"); break
            fi
            dir=$(dirname "$dir")
        done
    fi
    [ -n "$vid" ] && echo "${vid,,}:${pid,,}"
}

# Zustand: download, running, nots2 (303a:1001 = USB-Serial-JTAG von ESP32-S3/C3/C6) oder other
# State: download, running, nots2 (303a:1001 = USB serial JTAG of ESP32-S3/C3/C6) or other
port_state() {
    local id
    id=$(usb_id "$1")
    case "$id" in
        303a:0002) echo download ;;
        303a:1001) echo nots2 ;;
        303a:*)    echo running ;;
        *)         echo other ;;
    esac
}

list_ports() {
    local dev
    for dev in /dev/ttyACM* /dev/ttyUSB*; do
        [ -e "$dev" ] && echo "$dev"
    done
}

find_download_port() {
    local dev
    for dev in $(list_ports); do
        [ "$(port_state "$dev")" = download ] && { echo "$dev"; return 0; }
    done
    return 1
}

find_running_port() {
    local dev
    for dev in $(list_ports); do
        [ "$(port_state "$dev")" = running ] && { echo "$dev"; return 0; }
    done
    return 1
}

# Text -> Hex (UTF-8-Bytes), fuer WLAN-Name/-Passwort per USB
# text -> hex (UTF-8 bytes), for WiFi name/password via USB
to_hex() {
    printf '%s' "$1" | od -An -tx1 | tr -d ' \n'
}

# Fragt das WLAN fuer die Uhr ab. nmcli (falls vorhanden) sucht neu und listet
# die sichtbaren 2,4-GHz-Netze (Kanal 1-14), nach Signalstaerke sortiert.
# Setzt WIFI_SSID/WIFI_PASS; Enter = ueberspringen.
# Asks for the WiFi for the clock. nmcli (if available) rescans and lists the
# visible 2.4 GHz networks (channel 1-14), sorted by signal strength.
# Sets WIFI_SSID/WIFI_PASS; Enter = skip.
ask_wifi() {
    local nets=() current="" answer p1 p2 i mark
    if command -v nmcli >/dev/null 2>&1; then
        echo "Suche WLANs ... / scanning for WiFi networks ..."
        # SSID als letztes Feld und ohne Escaping: Doppelpunkte im Namen bleiben erhalten
        # SSID as the last field and without escaping: colons in the name are kept
        mapfile -t nets < <(nmcli -t -e no -f CHAN,SIGNAL,SSID dev wifi list --rescan yes 2>/dev/null |
            awk -F: '$1 <= 14 { ssid = substr($0, length($1) + length($2) + 3); if (ssid != "") print $2 "\t" ssid }' |
            sort -t$'\t' -k1,1nr | awk -F'\t' '!seen[$2]++ { print $2 }')
        current=$(nmcli -t -e no -f active,ssid dev wifi 2>/dev/null | sed -n 's/^yes://p' | head -n 1)
    fi
    echo
    echo "WLAN fuer die Uhr einrichten? Die Uhr kann nur 2,4-GHz-Netze, nur diese werden gezeigt."
    echo "Set up WiFi for the clock? The clock only supports 2.4 GHz networks, only those are shown."
    for i in "${!nets[@]}"; do
        mark=""
        [ "${nets[$i]}" = "$current" ] && mark="  (PC ist verbunden / PC connected)"
        echo "  $((i + 1)) = ${nets[$i]}$mark"
    done

    # PC haengt an einem 2,4-GHz-Netz: SSID und gespeichertes Passwort anbieten
    # (nmcli -s zeigt das Passwort der aktiven Verbindung, sofern erlaubt)
    # PC is on a 2.4 GHz network: offer its SSID and stored password
    # (nmcli -s shows the password of the active connection, if permitted)
    answer=""
    if [ -n "$current" ] && printf '%s\n' "${nets[@]}" | grep -qxF -- "$current"; then
        local take conn key
        read -r -p "Der PC ist mit '$current' (2,4 GHz) verbunden - WLAN-Name und Passwort uebernehmen? / PC is connected to '$current' (2.4 GHz) - take over WiFi name and password? [J/n, Y/n] " take
        if [ -z "$take" ] || [[ "$take" =~ ^[JjYy] ]]; then
            conn=$(nmcli -t -f NAME,TYPE connection show --active 2>/dev/null | awk -F: '$NF == "802-11-wireless" { sub(/:[^:]*$/, ""); print; exit }')
            key=""
            [ -n "$conn" ] && key=$(nmcli -s -g 802-11-wireless-security.psk connection show "$conn" 2>/dev/null)
            if [ "${#key}" -ge 8 ] && [ "${#key}" -le 63 ]; then
                WIFI_SSID="$current"
                WIFI_PASS="$key"
                echo "WLAN '$current' mit dem gespeicherten Passwort des PCs uebernommen / WiFi '$current' taken over with the PC's stored password."
                return 0
            fi
            echo "Kein gespeichertes Passwort lesbar - bitte eingeben / no stored password readable - please enter it."
            answer="$current"
        fi
    fi

    if [ -z "$answer" ]; then
        if [ "${#nets[@]}" -eq 0 ]; then
            # Keine Liste (kein nmcli, kein WLAN-Adapter, WLAN aus oder keine Rechte): Name eintippen
            # No list (no nmcli, no WiFi adapter, WiFi off or no permission): type the name
            echo "  Keine WLAN-Liste verfuegbar - WLAN-Name bitte eintippen oder Enter = ueberspringen"
            echo "  No WiFi list available - please type the WiFi name or Enter = skip"
        else
            echo "  Nummer, WLAN-Name eingeben oder Enter = ueberspringen (Einrichtung spaeter per WPS/Access Point)"
            echo "  number, WiFi name or Enter = skip (set up later via WPS/access point)"
        fi
        read -r -p "WLAN / WiFi: " answer
    fi
    [ -z "$answer" ] && return 0
    if [[ "$answer" =~ ^[0-9]+$ ]] && [ "$answer" -ge 1 ] && [ "$answer" -le "${#nets[@]}" ]; then
        WIFI_SSID="${nets[$((answer - 1))]}"
    else
        WIFI_SSID="$answer"
    fi
    while true; do
        read -r -s -p "Passwort fuer / password for '$WIFI_SSID' (Enter = offenes Netz / open network): " p1; echo
        read -r -s -p "Passwort wiederholen / repeat password: " p2; echo
        if [ "$p1" != "$p2" ]; then echo "Die Passwoerter stimmen nicht ueberein / passwords do not match"; continue; fi
        if [ -n "$p1" ] && { [ "${#p1}" -lt 8 ] || [ "${#p1}" -gt 63 ]; }; then
            echo "WLAN-Passwoerter haben 8-63 Zeichen / WiFi passwords have 8-63 characters"; continue
        fi
        WIFI_PASS="$p1"
        break
    done
}

# Andere Programme, die die Schnittstelle offen haben (z.B. ein serieller
# Monitor). Unter Linux duerfen mehrere Programme dieselbe Schnittstelle
# gleichzeitig oeffnen - es gibt also keinen Fehler, aber der Monitor kann die
# Antworten der Uhr wegschnappen. Gibt "Name (PID)"-Liste aus, leer wenn frei
# oder fuser (Paket psmisc) fehlt. Nur VOR dem eigenen Oeffnen aufrufen, sonst
# erscheint dieses Skript selbst in der Liste.
# Other programs that have the port open (e.g. a serial monitor). On Linux
# several programs may open the same port at the same time - so there is no
# error, but the monitor can snatch the clock's replies. Prints a "name (PID)"
# list, empty if free or fuser (package psmisc) is missing. Only call BEFORE
# opening the port ourselves, otherwise this script shows up in the list.
port_users() {
    local pid name out=""
    command -v fuser >/dev/null 2>&1 || return 0
    for pid in $(fuser "$1" 2>/dev/null); do
        pid=${pid//[!0-9]/}
        [ -z "$pid" ] || [ "$pid" = "$$" ] && continue
        name=$(ps -o comm= -p "$pid" 2>/dev/null)
        [ -z "$name" ] && continue # inzwischen beendet / exited meanwhile
        out="$out $name ($pid)"
    done
    echo "${out# }"
}

# Meldet einmal (BUSY_SHOWN), wenn die Schnittstelle belegt ist; merkt sich in
# BUSY_PORT, ob sie beim letzten Blick belegt war.
# Reports once (BUSY_SHOWN) if the port is in use; remembers in BUSY_PORT
# whether it was in use at the last check.
BUSY_SHOWN=""
BUSY_PORT=""
check_port_busy() {
    local users
    users=$(port_users "$1")
    if [ -n "$users" ]; then
        BUSY_PORT="$1"
        if [ "$BUSY_SHOWN" != "$1" ]; then
            BUSY_SHOWN="$1"
            echo "$1 ist von einem anderen Programm geoeffnet: $users"
            echo "  z.B. serieller Monitor (Arduino IDE, screen, minicom) - bitte schliessen, sonst gehen Antworten der Uhr verloren."
            echo "$1 is opened by another program: $users"
            echo "  e.g. a serial monitor (Arduino IDE, screen, minicom) - please close it, otherwise the clock's replies get lost."
        fi
    else
        BUSY_PORT=""
    fi
}

# Sendet einen Befehl und wartet bis 3 s auf "UHR4 OK ..." bzw. "UHR4 ERROR ...",
# Ergebnis in REPLY_LINE (leer = keine Antwort).
# Sends a command and waits up to 3 s for "UHR4 OK ..." or "UHR4 ERROR ...",
# result in REPLY_LINE (empty = no reply).
send_command() {
    local line until
    REPLY_LINE=""
    printf '%s\n' "$1" >&3 || return 1
    until=$((SECONDS + 3))
    while [ $SECONDS -lt $until ]; do
        IFS= read -r -t 1 line <&3 || continue
        line=${line%$'\r'}
        case "$line" in
            *"UHR4 OK"*|*"UHR4 ERROR"*) REPLY_LINE="UHR4 ${line#*UHR4 }"; return 0 ;;
        esac
    done
    return 1
}

# Wartet nach dem Flashen auf die gestartete Uhr und sendet WLAN (WIFI_SSID/
# WIFI_PASS) und Displaytyp ($1), danach bei Bedarf einen Neustart.
# Waits after flashing for the started clock and sends WiFi (WIFI_SSID/
# WIFI_PASS) and display type ($1), then a restart if needed.
send_setup() {
    local display="$1" start=$SECONDS hinted=0 dev wifi_done=1 restarting port_seen=""
    [ -n "$WIFI_SSID" ] && wifi_done=0
    echo
    echo "Einstellungen werden an die Uhr gesendet - warte auf die gestartete Uhr ..."
    echo "Sending settings to the clock - waiting for the started clock ..."
    while [ $((SECONDS - start)) -lt 150 ]; do
        if ! dev=$(find_running_port); then
            if [ "$hinted" = 0 ] && [ $((SECONDS - start)) -gt 20 ]; then
                echo "Die Uhr startet nicht von selbst? Einmal die Reset-Taste druecken."
                echo "Clock does not start by itself? Press the Reset button once."
                hinted=1
            fi
            sleep 0.5
            continue
        fi
        # Uhr da, antwortet aber 45 s lang nicht: Firmware ohne Einrichtung per
        # USB (aelter als diese flashESP-Version) - aufgeben.
        # Clock present but no reply for 45 s: firmware without USB setup
        # (older than this flashESP version) - give up.
        [ -z "$port_seen" ] && port_seen=$SECONDS
        if [ -z "$BUSY_PORT" ] && [ $((SECONDS - port_seen)) -gt 45 ]; then
            WIFI_PASS=""
            echo "Die Uhr antwortet nicht - ihre Firmware kennt die Einrichtung per USB noch nicht."
            echo "Displaytyp und WLAN dann in der Weboberflaeche einstellen."
            echo "The clock does not reply - its firmware does not know the setup via USB yet."
            echo "Set display type and WiFi in the web interface then."
            return 1
        fi
        # -hupcl: beim Schliessen DTR nicht fallen lassen (kein Neustart der Uhr)
        # -hupcl: do not drop DTR on close (no restart of the clock)
        check_port_busy "$dev"
        if ! { stty -F "$dev" 115200 raw -echo -hupcl && exec 3<>"$dev"; } 2>/dev/null; then
            sleep 0.5
            continue
        fi
        if [ "$wifi_done" = 0 ]; then
            if ! send_command "UHR4 WIFI $(to_hex "$WIFI_SSID") $(to_hex "$WIFI_PASS")"; then
                exec 3>&-; sleep 0.5; continue
            fi
            case "$REPLY_LINE" in
                "UHR4 OK"*) echo "WLAN '$WIFI_SSID' gespeichert / WiFi '$WIFI_SSID' saved."; wifi_done=1 ;;
                *) exec 3>&-; echo "Die Uhr meldet einen Fehler / the clock reports an error: $REPLY_LINE"; return 1 ;;
            esac
        fi
        restarting=0
        if [ -n "$display" ]; then
            if ! send_command "UHR4 DISPLAY $display"; then
                exec 3>&-; sleep 0.5; continue
            fi
            case "$REPLY_LINE" in
                "UHR4 OK"*RESTART*) echo "Displaytyp $display eingestellt / display type $display set."; restarting=1 ;;
                "UHR4 OK"*) echo "Displaytyp $display war bereits eingestellt / display type $display was already set." ;;
                *) exec 3>&-; echo "Die Uhr meldet einen Fehler / the clock reports an error: $REPLY_LINE"; return 1 ;;
            esac
        fi
        if [ "$restarting" = 0 ] && [ -n "$WIFI_SSID" ]; then
            # WLAN wird erst nach einem Neustart verwendet / WiFi is used only after a restart
            send_command "UHR4 RESTART"
            restarting=1
        fi
        exec 3>&-
        [ "$restarting" = 1 ] && echo "Die Uhr startet neu / the clock restarts."
        SETUP_RESTARTING=$restarting
        WIFI_PASS=""
        return 0
    done
    WIFI_PASS=""
    if [ -n "$BUSY_PORT" ]; then
        echo "$BUSY_PORT ist weiterhin von einem anderen Programm geoeffnet - bitte schliessen und flashESP.sh erneut starten."
        echo "$BUSY_PORT is still opened by another program - please close it and run flashESP.sh again."
    fi
    echo "Keine Antwort von der Uhr. Displaytyp und WLAN dann in der Weboberflaeche einstellen"
    echo "oder flashESP.sh erneut starten. Programme mit offener Schnittstelle (z.B. seriellen Monitor) schliessen."
    echo "No reply from the clock. Set display type and WiFi in the web interface"
    echo "or run flashESP.sh again. Close programs using the port (e.g. serial monitor)."
    return 1
}

# Sendet die Uhrzeit des PCs ("UHR4 TIME <Unix-Sekunden UTC>.<ms>", erst direkt
# vor dem Senden bestimmt) an die laufende Uhr - an $1 oder die gefundene.
# Wartet bis $2 Sekunden (Standard 60) auf die Uhr, z.B. waehrend sie startet.
# Sends the PC's time ("UHR4 TIME <unix seconds UTC>.<ms>", determined only
# right before sending) to the running clock - to $1 or the one found. Waits
# up to $2 seconds (default 60) for the clock, e.g. while it is starting.
send_time() {
    local fixed="$1" wait="${2:-60}" start=$SECONDS port_seen="" hinted=0 dev
    while [ $((SECONDS - start)) -lt "$wait" ]; do
        dev="$fixed"
        [ -n "$dev" ] || dev=$(find_running_port) || dev=""
        if [ -z "$dev" ]; then
            if [ "$hinted" = 0 ] && [ $((SECONDS - start)) -gt 20 ]; then
                echo "Keine laufende Uhr gefunden - startet sie nicht von selbst? Einmal die Reset-Taste druecken."
                echo "No running clock found - does it not start by itself? Press the Reset button once."
                hinted=1
            fi
            sleep 0.5
            continue
        fi
        # Uhr da, antwortet aber 20 s lang nicht: Firmware ohne "UHR4 TIME"
        # Clock present but no reply for 20 s: firmware without "UHR4 TIME"
        [ -z "$port_seen" ] && port_seen=$SECONDS
        [ -z "$BUSY_PORT" ] && [ $((SECONDS - port_seen)) -gt 20 ] && break
        check_port_busy "$dev"
        if ! { stty -F "$dev" 115200 raw -echo -hupcl && exec 3<>"$dev"; } 2>/dev/null; then
            sleep 0.5
            continue
        fi
        if send_command "UHR4 TIME $(date +%s.%3N)"; then
            exec 3>&-
            case "$REPLY_LINE" in
                "UHR4 OK TIME "*) echo "Uhrzeit gesetzt / time set: ${REPLY_LINE#UHR4 OK TIME }"; return 0 ;;
                *) echo "Die Uhr meldet einen Fehler / the clock reports an error: $REPLY_LINE"; return 1 ;;
            esac
        fi
        exec 3>&-
        sleep 0.5
    done
    if [ -n "$BUSY_PORT" ]; then
        echo "Uhrzeit nicht gesetzt: $BUSY_PORT ist weiterhin von einem anderen Programm geoeffnet - bitte schliessen und erneut starten."
        echo "Time not set: $BUSY_PORT is still opened by another program - please close it and start again."
        return 1
    fi
    echo "Uhrzeit nicht gesetzt: keine Antwort (Firmware ohne diese Funktion?). Programme mit offener Schnittstelle schliessen."
    echo "Time not set: no reply (firmware without this function?). Close programs using the port."
    return 1
}

# Displayname aus Nummer oder Name, leer bei ungueltig
# Display name from number or name, empty if invalid
display_name() {
    case "${1^^}" in
        1|GC9A01) echo GC9A01 ;;
        2|GC9A01_WITH_BACKLIGHT) echo GC9A01_WITH_BACKLIGHT ;;
        3|GC9D01) echo GC9D01 ;;
        4|ILI9341) echo ILI9341 ;;
    esac
}

# Fragt eine laufende Uhr VOR dem Flashen nach ihrem Displaytyp ("UHR4 INFO",
# siehe handleSerialInfo() in display.h). Setzt CLOCK_NAME/CLOCK_SET, leer
# wenn keine laufende Uhr da ist oder sie nicht antwortet (uhr3, aeltere uhr4).
# Asks a running clock BEFORE flashing for its display type ("UHR4 INFO", see
# handleSerialInfo() in display.h). Sets CLOCK_NAME/CLOCK_SET, empty if there
# is no running clock or it does not reply (uhr3, older uhr4).
CLOCK_NAME=""
CLOCK_SET=""
query_clock_info() {
    local dev="$1"
    [ -n "$dev" ] || dev=$(find_running_port) || return 1
    echo "Frage die Uhr auf $dev nach ihrem Displaytyp ... / asking the clock on $dev for its display type ..."
    check_port_busy "$dev"
    { stty -F "$dev" 115200 raw -echo -hupcl && exec 3<>"$dev"; } 2>/dev/null || return 1
    if send_command "UHR4 INFO"; then
        case "$REPLY_LINE" in
            "UHR4 OK INFO "*)
                set -- $REPLY_LINE
                CLOCK_NAME="$4"
                CLOCK_SET="$5" ;;
        esac
    fi
    exec 3>&-
    [ -n "$CLOCK_NAME" ]
}

# Displaytyp abfragen: gespeicherter Typ vorgewaehlt (Enter behaelt ihn), uhr4
# ohne gespeicherten Typ -> Angabe noetig, keine Antwort -> Enter = unveraendert.
# Setzt DISP (leer = nichts senden).
# Ask for the display type: stored type preselected (Enter keeps it), uhr4
# without stored type -> answer required, no reply -> Enter = unchanged.
# Sets DISP (empty = send nothing).
ask_display() {
    local k mark answer choice
    local labels=("" "GC9A01 (240x240) ohne Hintergrundbeleuchtung (BL) / without backlight (BL)" "GC9A01 (240x240) mit Hintergrundbeleuchtung (BL) an Pin 3 / with backlight (BL) on pin 3" "GC9D01 (160x160)" "ILI9341 (240x320) mit Uhrzeit und Datum unter der Uhr / with time and date below the clock")
    local names=("" GC9A01 GC9A01_WITH_BACKLIGHT GC9D01 ILI9341)
    echo
    echo "Welches Display hat die Uhr? / Which display does the clock have?"
    for k in 1 2 3 4; do
        mark=""
        [ "$CLOCK_SET" = 1 ] && [ "${names[$k]}" = "$CLOCK_NAME" ] && mark="   <- eingestellt / configured"
        echo "  $k = ${labels[$k]}$mark"
    done
    if [ "$CLOCK_SET" = 1 ]; then
        echo "  Enter = $CLOCK_NAME beibehalten / keep $CLOCK_NAME"
    elif [ -n "$CLOCK_NAME" ]; then
        echo "  Die Uhr hat noch keinen Displaytyp eingestellt - bitte angeben."
        echo "  The clock has no display type configured yet - please choose one."
    else
        echo "  Displaytyp der Uhr nicht feststellbar (neue Uhr, uhr3 oder aeltere Firmware)."
        echo "  Enter = unveraendert lassen - bei einer neuen Uhr bitte angeben."
        echo "  Display type of the clock not detectable (new clock, uhr3 or older firmware)."
        echo "  Enter = keep unchanged - please choose one for a new clock."
    fi
    while true; do
        read -r -p "Auswahl / choice: " answer
        if [ -z "$answer" ]; then
            # Angabe noetig, wenn die Uhr noch keinen Typ hat / answer required if the clock has no type yet
            [ -n "$CLOCK_NAME" ] && [ "$CLOCK_SET" != 1 ] && continue
            DISP=""
            break
        fi
        choice=$(display_name "$answer")
        if [ -n "$choice" ]; then
            # Gleicher Typ wie eingestellt: nichts senden / same as configured: send nothing
            if [ "$CLOCK_SET" = 1 ] && [ "$choice" = "$CLOCK_NAME" ]; then DISP=""; else DISP="$choice"; fi
            break
        fi
        echo "Bitte 1, 2, 3 oder 4 eingeben / please enter 1, 2, 3 or 4"
    done
    echo
}

# 0 -> /dev/ttyACM0, ttyACM0 -> /dev/ttyACM0
normalize_port() {
    case "$1" in
        /dev/*) echo "$1" ;;
        *[!0-9]*) echo "/dev/$1" ;;
        *) echo "/dev/ttyACM$1" ;;
    esac
}

# Nur Uhrzeit setzen / only set the time
if [ "$1" = "--time" ]; then
    dev=""
    if [ -n "$2" ]; then
        dev=$(normalize_port "$2")
    elif ! dev=$(find_running_port); then
        echo "Keine laufende Uhr gefunden (USB-Kabel? Uhr im Download-Modus: Reset druecken)."
        echo "No running clock found (USB cable? clock in download mode: press Reset)."
        exit 1
    fi
    if [ ! -w "$dev" ]; then
        echo "Keine Schreibrechte auf $dev (oder nicht vorhanden) - User zur Gruppe dialout hinzufuegen und neu anmelden:"
        echo "No write permission on $dev (or not present) - add the user to the dialout group and log in again:"
        echo "  sudo usermod -a -G dialout \$USER"
        exit 1
    fi
    send_time "$dev" 30
    exit $?
fi

# Alle Dateien da? Sonst wurde das Zip nicht komplett ausgepackt.
# All files present? Otherwise the zip was not fully unpacked.
missing=""
for f in uhr4.ino.bootloader.bin uhr4.ino.partitions.bin uhr4.ino.bin; do
    [ -f "$f" ] || missing="$missing $f"
done
if [ -n "$missing" ]; then
    echo "Fehlende Dateien / missing files:$missing"
    echo "Das Zip zuerst komplett in einen Ordner auspacken und flashESP.sh dort starten."
    echo "First unpack the whole zip into a folder and start flashESP.sh there."
    exit 1
fi

PORT="$1"
DISP="$2"

if [ -z "$PORT" ]; then
    echo "Serielle Schnittstellen / serial ports:"
    clocks=()
    any=0
    for dev in $(list_ports); do
        any=1
        state=$(port_state "$dev")
        case "$state" in
            download) text="UHR - Download-Modus / download mode"; clocks+=("$dev") ;;
            running)  text="UHR - laeuft / running"; clocks+=("$dev") ;;
            nots2)    text="ESP32-S3/C3/C6 - kein ESP32-S2 / not an ESP32-S2" ;;
            *)        text="-" ;;
        esac
        echo "  $dev  $text  [$(usb_id "$dev")]"
    done
    [ "$any" = 0 ] && echo "  keine / none"
    echo

    if [ "${#clocks[@]}" -eq 1 ]; then
        PORT="${clocks[0]}"
        echo "Uhr auf / clock on $PORT"
    else
        if [ "${#clocks[@]}" -gt 1 ]; then
            echo "Mehrere Uhren gefunden - bitte nur eine anschliessen oder die Schnittstelle angeben."
            echo "Several clocks found - connect only one or enter the port."
        else
            echo "Keine Uhr erkannt. Bitte pruefen:"
            echo "  * Taucht die Uhr nach dem Anstecken mit 'dmesg | tail -n 20' bzw. 'lsusb'"
            echo "    (Espressif 303a) auf und hat sie eine Schnittstelle /dev/ttyACM... bekommen?"
            echo "  * Ist das verwendete USB-Kabel ein Datenkabel oder nur ein Ladekabel?"
            echo "  * Neuer ESP32-S2 ohne Programm (oder mit fremdem Programm)? Er meldet sich erst im Boot-Modus:"
            echo "    Boot-Taste halten, dann USB einstecken - danach flashESP.sh erneut starten."
            echo "No clock detected. Please check:"
            echo "  * Does the clock show up after plugging in with 'dmesg | tail -n 20' or 'lsusb'"
            echo "    (Espressif 303a) and has it been assigned a port /dev/ttyACM...?"
            echo "  * Is the USB cable a data cable or just a charging cable?"
            echo "  * New ESP32-S2 without a program (or with a foreign program)? It only shows up in boot mode:"
            echo "    hold the Boot button, then plug in USB - then start flashESP.sh again."
            echo
        fi
        read -r -p "Schnittstelle / port (z.B./e.g. 0 = /dev/ttyACM0, Enter = Abbruch/abort): " PORT
        [ -z "$PORT" ] && exit 0
    fi
fi

PORT=$(normalize_port "$PORT")

if [ ! -e "$PORT" ]; then
    echo "$PORT nicht vorhanden / not present."
    exit 1
fi

state=$(port_state "$PORT")
if [ "$state" = nots2 ]; then
    echo "$PORT ist ein ESP32-S3/C3/C6 - uhr4 laeuft nur auf dem ESP32-S2 (Lolin S2 Pico)."
    echo "$PORT is an ESP32-S3/C3/C6 - uhr4 only runs on the ESP32-S2 (Lolin S2 Pico)."
    exit 1
fi
if [ "$state" = other ]; then
    echo "Hinweis: an $PORT wurde keine Uhr erkannt - es wird trotzdem versucht."
    echo "Note: no clock detected on $PORT - trying anyway."
fi

if [ ! -w "$PORT" ]; then
    echo "Keine Schreibrechte auf $PORT - User zur Gruppe dialout hinzufuegen und neu anmelden:"
    echo "No write permission on $PORT - add the user to the dialout group and log in again:"
    echo "  sudo usermod -a -G dialout \$USER"
    exit 1
fi

# Fragen erst NACH der Portauswahl, aber VOR dem Umschalten in den Download-
# Modus: eine laufende Uhr meldet so noch ihren Displaytyp (Vorwahl). Danach
# laufen Flashen und Einrichten ohne Eingriff.
# Questions only AFTER the port selection, but BEFORE switching into download
# mode: a running clock can thus still report its display type (preselection).
# Flashing and setup then run without intervention.
WIFI_SSID=""
WIFI_PASS=""
if [ -z "$1" ]; then
    CLOCK_NAME=""
    CLOCK_SET=""
    [ "$state" = running ] && query_clock_info "$PORT"
    ask_display
    ask_wifi
elif [ -n "$DISP" ]; then
    d=$(display_name "$DISP")
    [ -z "$d" ] && echo "Ungueltiger Displaytyp - bleibt unveraendert / invalid display type - stays unchanged: $DISP"
    DISP="$d"
fi

if [ "$state" = running ]; then
    echo "Uhr laeuft auf $PORT - Neustart in den Download-Modus ..."
    echo "Clock running on $PORT - restarting into download mode ..."
    # Oeffnen mit 1200 Baud und Schliessen (DTR faellt) loest den Neustart aus.
    # Opening at 1200 baud and closing (DTR drops) triggers the restart.
    stty -F "$PORT" 1200 hupcl 2>/dev/null || \
        echo "$PORT konnte nicht geoeffnet werden (belegt?) / could not be opened (in use?) - warte / waiting ..."
    PORT=""
    for _ in $(seq 1 30); do
        sleep 0.5
        PORT=$(find_download_port) && break
    done
    if [ -z "$PORT" ]; then
        echo "Kein Download-Modus erkannt. Programme mit offener Schnittstelle (z.B. seriellen Monitor) schliessen"
        echo "oder Boot-Taste halten, USB neu einstecken und erneut starten."
        echo "No download mode detected. Close programs using the port (e.g. serial monitor)"
        echo "or hold the Boot button, replug USB and start again."
        exit 1
    fi
    echo "Download-Modus auf / download mode on $PORT"
fi

# esptool ab v5 heisst "esptool" (write-flash), aeltere Versionen "esptool.py" (write_flash)
# esptool v5+ is called "esptool" (write-flash), older versions "esptool.py" (write_flash)
if command -v esptool >/dev/null 2>&1; then
    ESPTOOL=esptool; WRITE=write-flash
elif command -v esptool.py >/dev/null 2>&1; then
    ESPTOOL=esptool.py; WRITE=write_flash
else
    echo "esptool nicht gefunden / not found - installieren / install: sudo pip3 install esptool --break-system-packages"
    exit 1
fi

if ! "$ESPTOOL" --chip esp32s2 -p "$PORT" -b 460800 "$WRITE" \
    0x1000 uhr4.ino.bootloader.bin 0x8000 uhr4.ino.partitions.bin 0x10000 uhr4.ino.bin; then
    echo
    echo "Flashen fehlgeschlagen - siehe Meldung von esptool oben. Haeufige Ursachen:"
    echo "  * ESP nicht im Bootmodus: Boot-Taste druecken und halten, erst DANACH den USB anstecken"
    echo "    ODER bei angestecktem USB: Reset und Boot druecken, Reset loslassen, Boot kurz danach loslassen."
    echo "  * 'Wrong --chip' / 'This chip is ...': kein ESP32-S2 - uhr4 laeuft nur auf dem ESP32-S2 (Lolin S2 Pico)."
    echo "  * 'could not open port' / Permission denied: Port belegt oder keine Rechte (Gruppe dialout)."
    echo "  * USB-Hub oder Frontanschluss: die Uhr direkt an einen USB-Anschluss am PC stecken."
    echo "Danach flashESP.sh erneut starten."
    echo "Flashing failed - see the esptool message above. Common causes:"
    echo "  * ESP not in boot mode: press and hold the Boot button, only THEN plug in USB"
    echo "    OR with USB connected: press Reset and Boot, release Reset, release Boot shortly after."
    echo "  * 'Wrong --chip' / 'This chip is ...': not an ESP32-S2 - uhr4 only runs on the ESP32-S2 (Lolin S2 Pico)."
    echo "  * 'could not open port' / permission denied: port in use or no rights (group dialout)."
    echo "  * USB hub or front port: plug the clock directly into a USB port of the PC."
    echo "Then run flashESP.sh again."
    exit 1
fi

SETUP_RESTARTING=0
if [ -n "$DISP" ] || [ -n "$WIFI_SSID" ]; then
    send_setup "$DISP" || exit 1
fi

# Uhrzeit des PCs senden - nach einem Neustart erst, wenn die Uhr wieder laeuft
# (die Wartezeit ueberbrueckt die 1 s bis zum Neustart)
# Send the PC's time - after a restart only once the clock runs again (the
# wait bridges the 1 s until the restart)
echo
echo "Uhrzeit des PCs wird an die Uhr gesendet ... / sending the PC time to the clock ..."
[ "$SETUP_RESTARTING" = 1 ] && sleep 3
send_time "" 60
exit 0
