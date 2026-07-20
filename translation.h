#ifndef TRANSLATION_H
#define TRANSLATION_H


    // ####################################################################
    // ### Uebersetzungen #################################################
    // ####################################################################
    // Alle Uebersetzungen liegen als 'static const' Tabelle (translationTable,
    // siehe unten) direkt im Flash-Speicher. Der ESP32 kann Flash ueber den
    // Cache wie normalen Speicher lesen, daher ist - anders als z.B. bei
    // AVR-Chips mit PROGMEM/pgm_read - kein Umkopieren in den RAM noetig, um
    // darauf zuzugreifen. translate() durchsucht die Tabelle bei jedem
    // Aufruf direkt und kopiert nur den einen gefundenen Treffer in ein
    // String-Objekt. Ein Sprachwechsel (/setLanguage) muss dadurch nichts
    // mehr aufbauen/umkopieren - das war zuvor (grosse Initialisierungsliste
    // in einer std::map<String,String>) Ursache eines Stack-Overflow-Absturzes.

    // Bekannte Sprachen (fuer die Validierung in /setLanguage) - bewusst NUR
    // die Sprachcodes, nicht die vollen Uebersetzungstabellen.
    const std::set<String> availableLanguages = {"de", "fr"};


    // Uebersetzungstabelle (Erklaerung siehe Kommentar am Dateianfang)
    struct TranslationEntry {
        const char* key;
        const char* de;
        const char* fr;
    };

    static const TranslationEntry translationTable[] = {
        { "Main", "Start", "Accueil" },
        { "Connected to", "Verbunden mit", "Connect&eacute; &agrave;" },
        { "Connecting to", "Verbinde mit", "Connexion &agrave;" },
        { "Connection failed", "Verbindung fehlgeschlagen", "&Eacute;chec de la connexion" },
        { "IP Address", "IP Adresse", "Adresse IP" },
        { "SSID", "SSID", "SSID" },
        { "Password", "Passwort", "Mot de passe" },
        { "Alternative WiFi", "alternatives WiFi", "WiFi alternatif" },
        { "WiFi Settings", "WLAN Einstellungen", "Param&egrave;tres WiFi" },
        { "Save Settings", "Einstellungen speichern", "Enregistrer les param&egrave;tres" },
        { "WiFi Networks", "WLAN Netzwerke", "R&eacute;seaux WiFi" },
        { "Scan", "Scannen", "Scanner" },
        { "Refresh", "Aktualisieren", "Actualiser" },
        { "Available Networks", "Verf&uuml;gbare Netzwerke", "R&eacute;seaux disponibles" },
        { "Signal Strength", "Signalst&auml;rke", "Force du signal" },
        { "Encryption", "Verschl&uuml;sselung", "Chiffrement" },
        { "Storage used", "Speicher belegt", "Stockage utilis&eacute;" },
        { "Presets used", "Presets belegt", "Pr&eacute;r&eacute;glages utilis&eacute;s" },
        { "Free", "Frei", "Libre" },
        { "Total", "Gesamt", "Total" },
        { "Used", "Belegt", "Utilis&eacute;" },
        { "Save", "speichern", "enregistrer" },
        { "Brightness", "Helligkeit", "Luminosit&eacute;" },
        { "Language", "Sprache", "Langue" },
        { "Time Settings", "Zeiteinstellungen", "Param&egrave;tres de l'heure" },
        { "NTP Server 1", "NTP Server 1", "Serveur NTP 1" },
        { "NTP Server 2", "NTP Server 2", "Serveur NTP 2" },
        { "Timezone", "Zeitzone", "Fuseau horaire" },
        { "Clock Settings", "Uhreinstellungen", "Param&egrave;tres de l'horloge" },
        { "Handset", "Zeigersatz", "Aiguilles" },
        { "Background", "Hintergrund", "Fond" },
        { "Display Settings", "Anzeigeeinstellungen", "Param&egrave;tres d'affichage" },
        { "Station Mode", "Station Modus", "Mode gare" },
        { "Show Second Hand", "Sekundenzeiger anzeigen", "Afficher la trotteuse" },
        { "Smooth Minute Hand", "Sanfter Minutenzeiger", "Aiguille des minutes fluide" },
        { "Photoresistor Settings", "Fotowiderstand Einstellungen", "Param&egrave;tres de la photor&eacute;sistance" },
        { "Min Brightness", "Minimale Helligkeit", "Luminosit&eacute; minimale" },
        { "Max Brightness", "Maximale Helligkeit", "Luminosit&eacute; maximale" },
        { "Low Light Threshold", "Schwellenwert f&uuml;r dunkles Licht", "Seuil de faible luminosit&eacute;" },
        { "High Light Threshold", "Schwellenwert f&uuml;r helles Licht", "Seuil de forte luminosit&eacute;" },
        { "Center Color", "Nabenfarbe", "Couleur du moyeu" },
        { "Center Size", "Nabengr&ouml;&szlig;e", "Taille du moyeu" },
        { "Rebooting...", "Starte neu...", "Red&eacute;marrage..." },
        { "Reboot", "Neustart", "Red&eacute;marrer" },
        { "Factory&nbsp;Reset", "Werkseinstellungen", "R&eacute;initialisation&nbsp;usine" },
        { "File&nbsp;Manager", "Dateimanager", "Gestionnaire&nbsp;de&nbsp;fichiers" },
        { "Upload File", "Datei hochladen", "T&eacute;l&eacute;charger un fichier" },
        { "Choose File", "Datei ausw&auml;hlen", "Choisir un fichier" },
        { "No file selected", "Keine Datei ausgew&auml;hlt", "Aucun fichier s&eacute;lectionn&eacute;" },
        { "Upload", "Hochladen", "T&eacute;l&eacute;charger" },
        { "Delete", "L&ouml;schen", "Supprimer" },
        { "active", "aktiv", "actif" },
        { "Status", "Status", "&Eacute;tat" },
        { "Info", "Info", "Infos" },
        { "pixels", "Pixel", "pixels" },
        { "Backup / Restore Presets", "Presets sichern / wiederherstellen", "Sauvegarder / restaurer les pr&eacute;r&eacute;glages" },
        { "Save Presets to File", "Presets als Datei speichern", "Enregistrer les pr&eacute;r&eacute;glages dans un fichier" },
        { "Save the current clock settings as a new preset?", "Aktuelle Uhr-Einstellungen als neues Preset speichern?", "Enregistrer les param&egrave;tres actuels comme nouveau pr&eacute;r&eacute;glage&nbsp;?" },
        { "Restore Presets from File", "Presets aus Datei wiederherstellen", "Restaurer les pr&eacute;r&eacute;glages depuis un fichier" },
        { "This will replace all currently saved presets", "Dies ersetzt alle aktuell gespeicherten Presets", "Cela remplacera tous les pr&eacute;r&eacute;glages actuellement enregistr&eacute;s" },
        { "Continue", "Fortfahren", "Continuer" },
        { "Import Presets", "Presets importieren", "Importer les pr&eacute;r&eacute;glages" },
        { "Presets imported successfully", "Presets erfolgreich importiert", "Pr&eacute;r&eacute;glages import&eacute;s avec succ&egrave;s" },
        { "Import failed - please check the file", "Import fehlgeschlagen - bitte Datei pr&uuml;fen", "&Eacute;chec de l'importation - veuillez v&eacute;rifier le fichier" },
        { "File deleted", "Datei gel&ouml;scht", "Fichier supprim&eacute;" },
        { "File upload failed", "Datei-Upload fehlgeschlagen", "&Eacute;chec du t&eacute;l&eacute;chargement" },
        { "File uploaded successfully", "Datei erfolgreich hochgeladen", "Fichier t&eacute;l&eacute;charg&eacute; avec succ&egrave;s" },
        { "File already exists", "Datei existiert bereits", "Le fichier existe d&eacute;j&agrave;" },
        { "File too large", "Datei zu gro&szlig;", "Fichier trop volumineux" },
        { "rename", "Umbenennen", "renommer" },
        { "Disconnected", "Getrennt", "D&eacute;connect&eacute;" },
        { "Settings", "Einstellungen", "Param&egrave;tres" },
        { "settings", "Einstellungen", "Param&egrave;tres" },
        { "save", "Speichern", "Enregistrer" },
        { "brightness", "Helligkeit", "Luminosit&eacute;" },
        { "timezone", "Zeitzone", "Fuseau horaire" },
        { "NTP&nbsp;Timezone", "NTP&nbsp;Zeitzone", "Fuseau&nbsp;horaire&nbsp;NTP" },
        { "Select File", "Datei ausw&auml;hlen", "Choisir un fichier" },
        { "No File Selected", "Keine Datei ausgew&auml;hlt", "Aucun fichier s&eacute;lectionn&eacute;" },
        { "Delete File", "Datei l&ouml;schen", "Supprimer le fichier" },
        { "Are you sure you want to delete", "M&ouml;chten Sie wirklich l&ouml;schen", "&Ecirc;tes-vous s&ucirc;r de vouloir supprimer" },
        { "Cancel", "Abbrechen", "Annuler" },
        { "Confirm", "Best&auml;tigen", "Confirmer" },
        { "Clock&nbsp;Face", "Zifferblatt", "Cadran" },
        { "Hand&nbsp;Set", "Zeiger", "Jeu&nbsp;d'aiguilles" },
        { "Use Touch Control", "Touch verwenden", "Utiliser le tactile" },
        { "Presets", "Uhren Sets", "Pr&eacute;r&eacute;glages" },
        { "Select Preset", "Set ausw&auml;hlen", "Choisir un pr&eacute;r&eacute;glage" },
        { "Default", "Standard", "Par d&eacute;faut" },
        { "Clock Seetup", "Einstellungen", "Param&egrave;tres" },
        { "Rescan Networks", "WLan Netzwerke neu scannen", "Rescanner les r&eacute;seaux" },
        { "Connect", "Verbinden", "Connecter" },
        { "Primary WiFi", "Prim&auml;res WLAN Netzwerk", "WiFi principal" },
        { "Alternative Network", "Alternatives WLAN Netzwerk", "R&eacute;seau alternatif" },
        { "Password is hidden.Leave empty to keep current", "Das Passwort ist ausgeblendet. Lassen Sie das Feld leer, um das aktuelle Passwort beizubehalten", "Le mot de passe est masqu&eacute;. Laissez le champ vide pour conserver le mot de passe actuel" },
        { "You can also enter an SSID manually", "Sie k&ouml;nnen auch eine SSID manuell eingeben", "Vous pouvez aussi saisir un SSID manuellement" },
        { "No WiFi networks found", "Keine WLAN Netzwerke gefunden", "Aucun r&eacute;seau WiFi trouv&eacute;" },
        { "Save WiFi settings", "WLAN Einstellungen speichern", "Enregistrer les param&egrave;tres WiFi" },
        { "Train Station Mode", "Bahnhof Modus", "Mode gare" },
        { "Show Seconds", "Sekundenzeiger anzeigen", "Afficher la trotteuse" },
        { "Enable Touch", "Touch Pin verwenden", "Activer le tactile" },
        { "Apply", "Anwenden", "Appliquer" },
        { "select network", "Netzwerk ausw&auml;hlen", "choisir un r&eacute;seau" },
        { "Manage Clock Face Files", "Zifferbl&auml;tter verwalten", "G&eacute;rer les cadrans" },
        { "Manage Hand Set Files", "Zeigersatz Dateien verwalten", "G&eacute;rer les jeux d'aiguilles" },
        { "Rename", "Umbenennen", "Renommer" },
        { "Enter new name for", "Neuen Namen eingeben f&uuml;r", "Entrez le nouveau nom pour" },
        { "New name", "Neuer Name", "Nouveau nom" },
        { "New Name", "Neuer Name", "Nouveau nom" },
        { "File renamed successfully", "Datei erfolgreich umbenannt", "Fichier renomm&eacute; avec succ&egrave;s" },
        { "File rename failed", "Datei-Umbenennung fehlgeschlagen", "&Eacute;chec du renommage" },
        { "Download Additional Clock Faces", "Zus&auml;tzliche Zifferbl&auml;tter herunterladen", "T&eacute;l&eacute;charger des cadrans suppl&eacute;mentaires" },
        { "You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')", "Sie k&ouml;nnen eine ZIP-Datei mit zus&auml;tzlichen Zifferbl&auml;ttern und Zeigers&auml;tzen von folgendem Link herunterladen: (verwenden Sie 'view raw')", "Vous pouvez t&eacute;l&eacute;charger un fichier ZIP contenant des cadrans et jeux d'aiguilles suppl&eacute;mentaires depuis le lien suivant : (utilisez 'view raw')" },
        { "Unzip the file and upload the contents to the /clockfaces and /handsets directories using the File Manager", "Entpacken Sie die Datei und laden Sie den Inhalt mit dem Dateimanager in die Verzeichnisse /clockfaces und /handsets hoch", "D&eacute;compressez le fichier et t&eacute;l&eacute;chargez le contenu dans les dossiers /clockfaces et /handsets &agrave; l'aide du gestionnaire de fichiers" },
        { "After downloading, upload the extracted BMP files using the form below", "Nach dem Herunterladen k&ouml;nnen Sie die extrahierten BMP-Dateien mit dem untenstehenden Formular hochladen", "Apr&egrave;s le t&eacute;l&eacute;chargement, envoyez les fichiers BMP extraits &agrave; l'aide du formulaire ci-dessous" },
        { "Upload New Clock Face", "neue Zifferbl&auml;tter hochladen", "T&eacute;l&eacute;charger un nouveau cadran" },
        { "Requirements", "Anforderungen", "Exigences" },
        { "instead of the IP address for better reliability", "anstelle der IP Adresse f&uuml;r bessere Erreichbarkeit", "au lieu de l'adresse IP pour une meilleure fiabilit&eacute;" },
        { "name must start with face_ and end with .bmp", "Name muss mit face_ beginnen und mit .bmp enden", "le nom doit commencer par face_ et se terminer par .bmp" },
        { "size must be", "Gr&ouml;&szlig;e muss sein", "la taille doit &ecirc;tre" },
        { "Clock Setup", "Uhr Einstellungen", "Param&egrave;tres de l'horloge" },
        { "name must start with", "Name muss beginnen mit", "le nom doit commencer par" },
        { "Manage Clock Hand Sets", "Zeigers&auml;tze verwalten", "G&eacute;rer les jeux d'aiguilles" },
        { "Preview/Set", "Vorschau/Setzen", "Aper&ccedil;u/D&eacute;finir" },
        { "Upload New Hand Set", "Neuen Zeigersatz hochladen", "T&eacute;l&eacute;charger un nouveau jeu d'aiguilles" },
        { "Upload to Set", "Set hochladen", "T&eacute;l&eacute;charger vers le jeu" },
        { "Color (RGB hex, e.g. FF0000 = Red, 000000 = Black, EC0016 = DB red)", "Farbe (RGB hex, z.B. FF0000 = Rot, 000000 = Schwarz, EC0016 = DB rot)", "Couleur (hex RGB, ex. FF0000 = Rouge, 000000 = Noir, EC0016 = rouge DB)" },
        { "Centre point", "Mittelpunkt", "Point central" },
        { "Size", "Gr&ouml;&szlig;e", "Taille" },
        { "Warning: Not enough free space to upload new hand sets!Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zeiger! Bitte zuerst Speicherplatz freigeben", "Attention : espace insuffisant pour t&eacute;l&eacute;charger de nouveaux jeux d'aiguilles ! Lib&eacute;rez d'abord de l'espace" },
        { "Pivot point", "Drehpunkt bei", "Point de pivot" },
        { "Manage Presets", "Manage Uhren Sets", "G&eacute;rer les pr&eacute;r&eacute;glages" },
        { "Preset Name", "Set Name", "Nom du pr&eacute;r&eacute;glage" },
        { "Create New Preset", "Erstelle neues Set", "Cr&eacute;er un nouveau pr&eacute;r&eacute;glage" },
        { "Create Preset from Current Settings", "Erzeuge ein Set aus den aktuellen Einstellungen", "Cr&eacute;er un pr&eacute;r&eacute;glage &agrave; partir des param&egrave;tres actuels" },
        { "For custom timezones, select a preset or enter your own value above", "F&uuml;r benutzerdefinierte Zeitzonen w&auml;hlen Sie eine Voreinstellung aus oder geben Sie oben Ihren eigenen Wert ein", "Pour un fuseau horaire personnalis&eacute;, choisissez un pr&eacute;r&eacute;glage ou saisissez votre propre valeur ci-dessus" },
        { "Save Timezone", "Zeitzone speichern", "Enregistrer le fuseau horaire" },
        { "NTP Server / Timezone (DST String)", "NTP Server / Zeitzone (DST)", "Serveur NTP / Fuseau horaire (cha&icirc;ne DST)" },
        { "Low Threshold", "untere Helligkeitsschwelle", "Seuil bas" },
        { "High Threshold", "obere Helligkeitsschwelle", "Seuil haut" },
        { "Full brightness from (hour, 0-23)", "volle Helligkeit ab Stunde (0-23)", "Luminosit&eacute; maximale &agrave; partir de (heure, 0-23)" },
        { "Full brightness until (hour, 0-23)", "volle Helligkeit bis Stunde (0-23)", "Luminosit&eacute; maximale jusqu'&agrave; (heure, 0-23)" },
        { "Current ADC Value", "aktueller ADC Wert", "Valeur ADC actuelle" },
        { "Current Brightness", "aktuelle Helligkeit", "Luminosit&eacute; actuelle" },
        { "Light (for Threshold)", "Licht (f&uuml;r Helligkeitsschwelle", "Luminosit&eacute; (pour le seuil)" },
        { "Brightness Settings", "Helligkeit Einstellungen", "Param&egrave;tres de luminosit&eacute;" },
        { "Enable Auto Brightness", "automatische Einstellung", "Activer la luminosit&eacute; automatique" },
        { "Invert ADC Reading", "invertiere ADC Werte", "Inverser la lecture ADC" },
        { "All Files on LittleFS", "alle Dateien im FileSystem", "Tous les fichiers sur LittleFS" },
        { "Scale and Save BMP", "skaliere und speichere das BMP", "Redimensionner et enregistrer le BMP" },
        { "Filename", "Dateiname", "Nom du fichier" },
        { "Preview", "Vorschau", "Aper&ccedil;u" },
        { "No BMP files found in /", "Keine BMP-Dateien im Dateisystem gefunden", "Aucun fichier BMP trouv&eacute; dans le syst&egrave;me de fichiers" },
        { "Size(bytes)", "Gr&ouml;&szlig;e (Bytes)", "Taille (octets)" },
        { "Action", "Aktion", "Action" },
        { "Scale", "skalieren", "Redimensionner" },
        { "Source", "Quelle", "Source" },
        { "Target", "Ziel", "Cible" },
        { "Width", "Breite", "Largeur" },
        { "Height", "H&ouml;he", "Hauteur" },
        { "Scale and Save", "skalieren und speichern", "Redimensionner et enregistrer" },
        { "Are you sure you want to reboot?", "Sind Sie sicher, das Sie die Uhr neu starten wollen?", "&Ecirc;tes-vous s&ucirc;r de vouloir red&eacute;marrer l'horloge ?" },
        { "Are you sure you want to reset to factory settings?", "Sind Sie absolut sicher, das Sie die Uhr auf Werkseinstellung setzen wollen?", "&Ecirc;tes-vous absolument s&ucirc;r de vouloir r&eacute;initialiser l'horloge aux param&egrave;tres d'usine ?" },
        { "Rename File", "Datei umbenennen", "Renommer le fichier" },
        { "Rename Preset", "Preset umbenennen", "Renommer le pr&eacute;r&eacute;glage" },
        { "Maximum number of presets reached - delete an existing preset first", "Maximale Anzahl an Presets erreicht - bitte zuerst ein bestehendes Preset l&ouml;schen", "Nombre maximal de pr&eacute;r&eacute;glages atteint - veuillez d'abord supprimer un pr&eacute;r&eacute;glage existant" },
        { "Return to the main page in 10 seconds or refresh the website when the ESP is online again", "Kehren Sie in 10 Sekunden zur Hauptseite zur&uuml;ck oder aktualisieren Sie die Website, wenn der ESP wieder online ist", "Retour &agrave; la page principale dans 10 secondes, ou actualisez le site lorsque l'ESP est &agrave; nouveau en ligne" },
        { "Enable Logging", "Logging aktivieren", "Activer la journalisation" },
        { "Reconnect WiFi", "Wifi neu verbinden", "Reconnecter le WiFi" },
        { "JavaScript is disabled.This page requires JavaScript to work properly!", "JavaScript ist deaktiviert.Diese Seite ben&ouml;tigt JavaScript, um richtig zu funktionieren!", "JavaScript est d&eacute;sactiv&eacute;. Cette page n&eacute;cessite JavaScript pour fonctionner correctement !" },
        { "Are you sure you want to reset all saved WiFi networks?", "Sind Sie sicher, dass Sie alle gespeicherten WLAN Netzwerke zur&uuml;cksetzen m&ouml;chten?", "&Ecirc;tes-vous s&ucirc;r de vouloir r&eacute;initialiser tous les r&eacute;seaux WiFi enregistr&eacute;s ?" },
        { "Reset WiFi Networks", "WLAN Netzwerke zur&uuml;cksetzen", "R&eacute;initialiser les r&eacute;seaux WiFi" },
        { "WiFi networks reset. Rebooting...", "WLAN Netzwerke zur&uuml;ckgesetzt. Starte neu...", "R&eacute;seaux WiFi r&eacute;initialis&eacute;s. Red&eacute;marrage..." },
        { "DCF77 detected", "DCF77 erkannt", "DCF77 d&eacute;tect&eacute;" },
        { "Waiting", "warte", "Attente" },
        { "Back", "Zur&uuml;ck", "Retour" },
        { "Check RTC", "RTC pr&uuml;fen", "V&eacute;rifier le RTC" },
        { "Download", "Herunterladen", "T&eacute;l&eacute;charger" },
        { "Failed to scale BMP", "Skalierung des BMP fehlgeschlagen", "&Eacute;chec du redimensionnement du BMP" },
        { "Gamma Correction", "Gamma-Korrektur", "Correction gamma" },
        { "Hostname", "Hostname", "Nom d'h&ocirc;te" },
        { "Ping Server", "Ping-Server", "Serveur de ping" },
        { "Reset Saved Networks", "Gespeicherte Netzwerke zur&uuml;cksetzen", "R&eacute;initialiser les r&eacute;seaux enregistr&eacute;s" },
        { "Reset WLan...", "WLAN zur&uuml;cksetzen...", "R&eacute;initialisation du WiFi..." },
        { "Saved as", "Gespeichert als", "Enregistr&eacute; sous" },
        { "Scaling successful", "Skalierung erfolgreich", "Redimensionnement r&eacute;ussi" },
        { "View", "Anzeigen", "Voir" },
        { "Warning: Not enough free space to upload new clock faces! Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zifferbl&auml;tter! Bitte zuerst Speicherplatz freigeben", "Attention : espace insuffisant pour t&eacute;l&eacute;charger de nouveaux cadrans ! Lib&eacute;rez d'abord de l'espace" },
        { "Use the host name", "Benutze den Hostnamen", "Utilisez le nom d'h&ocirc;te" },
    };
    static const size_t translationTableSize = sizeof(translationTable) / sizeof(translationTable[0]);


    // Laedt die zuletzt gespeicherte Spracheinstellung aus den Preferences
    // (wird einmalig in setup() aufgerufen). Es muss nichts mehr aufgebaut
    // werden - translate() liest direkt aus der flash-residenten Tabelle.
    void loadLanguage() {
        currentLanguage = preferences.getString(PK_LANGUAGE, "en");
    }


    // Setzt die aktive Sprache und speichert sie dauerhaft in den Preferences
    void saveLanguage(String lang) {
        currentLanguage = lang;
        preferences.putString(PK_LANGUAGE, lang);
    }


    // Uebersetzt einen englischen Schluessel-String in die aktuell aktive Sprache;
    // gibt bei Englisch bzw. fehlendem Eintrag einfach den Schluessel selbst zurueck.
    // Durchsucht dafuer direkt die flash-residente translationTable - kein
    // Heap-Umkopieren beim Sprachwechsel mehr noetig.
    String translate(const String& key) {

        if (currentLanguage == "en") return key;  // Englisch: Schlüssel zurückgeben

        for (size_t i = 0; i < translationTableSize; i++) {
            if (key == translationTable[i].key) {
                if (currentLanguage == "de") return String(translationTable[i].de);
                if (currentLanguage == "fr") return String(translationTable[i].fr);
                return key;
            }
        }

        return key; // Fallback: Schlüssel zurückgeben
    }


#endif // TRANSLATION_H
