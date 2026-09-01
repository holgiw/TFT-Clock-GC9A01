#ifndef TRANSLATION_H
#define TRANSLATION_H


    // Uebersetzungen: liegen als 'static const' Tabelle (translationTable) direkt im Flash - der ESP32 liest Flash ueber den Cache wie normalen
    // Speicher, kein PROGMEM/Umkopieren noetig. translate() sucht direkt darin, ohne Sprachwechsel-Aufbau - fruehere Initialisierungsliste in
    // std::map<String,String> verursachte einen Stack-Overflow-Absturz.

    // English: Translations live in a 'static const' table (translationTable) directly in flash - the ESP32 reads
    // flash via the cache like normal memory, no PROGMEM/copying needed. translate() searches it directly, no
    // language-switch setup - an earlier std::map<String,String> init list caused a stack overflow crash.

    // Bekannte Sprachen (fuer die Validierung in /setLanguage) - bewusst NUR
    // die Sprachcodes, nicht die vollen Uebersetzungstabellen.

    // English: known languages (for validation in /setLanguage) - deliberately
    // only the language codes, not the full translation tables.
    const std::set<String> availableLanguages = {"de", "fr"};


    // Uebersetzungstabelle (Erklaerung siehe Kommentar am Dateianfang)
    // English: translation table (see comment at top of file for explanation)
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
        { "Not connected", "Nicht verbunden", "Non connect&eacute;" },
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
        { "Open", "Offen", "Ouvert" },
        { "Secured", "Gesichert", "S&eacute;curis&eacute;" },
        { "System", "System", "Syst&egrave;me" },
        { "Manage", "Verwalten", "G&eacute;rer" },
        { "full details", "alle Details", "tous les d&eacute;tails" },
        { "No WiFi network configured yet, or the last known network is unavailable - the clock created its own WiFi network. Enter your home WiFi details below, save, and the clock will restart and try to connect", "Es ist noch kein WLAN eingerichtet, oder das zuletzt bekannte WLAN ist gerade nicht erreichbar - die Uhr hat ein eigenes WLAN erstellt. Trage unten dein Heim-WLAN ein, speichere, und die Uhr startet neu und versucht sich zu verbinden", "Aucun r&eacute;seau WiFi n&#39;est configur&eacute; pour le moment, ou le dernier r&eacute;seau connu est indisponible - l&#39;horloge a cr&eacute;&eacute; son propre r&eacute;seau WiFi. Indiquez ci-dessous les d&eacute;tails de votre WiFi domestique, enregistrez, et l&#39;horloge red&eacute;marrera pour tenter de s&#39;y connecter" },
        { "Storage used", "Speicher belegt", "Stockage utilis&eacute;" },
        { "Presets used", "Presets belegt", "Pr&eacute;r&eacute;glages utilis&eacute;s" },
        { "Free", "Frei", "Libre" },
        { "Version", "Version", "Version" },
        { "Not applicable to this file type", "Nicht anwendbar auf diesen Dateityp", "Non applicable &agrave; ce type de fichier" },
        { "Test", "Testen", "Tester" },
        { "Testing", "Teste", "Test en cours" },
        { "Server not reachable", "Server nicht erreichbar", "Serveur inaccessible" },
        { "The second hand rushes ahead slightly and briefly rests at 60, like a classic train station clock", "Der Sekundenzeiger eilt etwas voraus und verweilt kurz auf der 60, wie bei einer klassischen Bahnhofsuhr", "La trotteuse avance l&eacute;g&egrave;rement et s&#39;arr&ecirc;te bri&egrave;vement sur 60, comme une horloge de gare classique" },
        { "Shows or hides the second hand on the clock face", "Zeigt oder verbirgt den Sekundenzeiger auf dem Zifferblatt", "Affiche ou masque la trotteuse sur le cadran" },
        { "The minute hand moves smoothly instead of jumping in 1-minute steps", "Der Minutenzeiger bewegt sich gleichm&auml;&szlig;ig, statt in 1-Minuten-Schritten zu springen", "L&#39;aiguille des minutes se d&eacute;place en douceur au lieu d&#39;avancer minute par minute" },
        { "Server used to periodically check the internet connection", "Server, der regelm&auml;&szlig;ig zur Pr&uuml;fung der Internetverbindung angefragt wird", "Serveur utilis&eacute; pour v&eacute;rifier p&eacute;riodiquement la connexion internet" },
        { "Automatically tries to reconnect if the WiFi connection is lost", "Versucht automatisch, die WLAN-Verbindung bei Verbindungsverlust wiederherzustellen", "Tente automatiquement de se reconnecter en cas de perte de la connexion WiFi" },
        { "Writes up to 9 log files to LittleFS for troubleshooting", "Schreibt bis zu 9 Logdateien zur Fehlersuche in LittleFS", "&Eacute;crit jusqu&#39;&agrave; 9 fichiers journaux sur LittleFS pour le d&eacute;pannage" },
        { "Rotates the clock face by the selected number of degrees, useful if the display is mounted rotated in its housing", "Dreht das Zifferblatt um den gew&auml;hlten Winkel - n&uuml;tzlich, falls das Display gedreht im Geh&auml;use verbaut ist", "Fait pivoter le cadran selon l&#39;angle choisi - utile si l&#39;&eacute;cran est mont&eacute; en rotation dans son bo&icirc;tier" },
        { "Rotation Display 1", "Rotation Display 1", "Rotation &eacute;cran 1" },
        { "Rotation Display 2", "Rotation Display 2", "Rotation &eacute;cran 2" },
        { "Rotates Display 2's (CS2) clock face independently of Display 1", "Dreht das Zifferblatt von Display 2 (CS2) unabh&auml;ngig von Display 1", "Fait pivoter le cadran de l&#39;&eacute;cran 2 (CS2) ind&eacute;pendamment de l&#39;&eacute;cran 1" },
        { "Adds a new network via WPS - press the WPS button on your router when prompted. The clock's connection may be lost for about 3 minutes while this happens", "F&uuml;gt ein neues Netzwerk per WPS hinzu - dr&uuml;cken Sie bei Aufforderung die WPS-Taste an Ihrem Router. Die Verbindung zur Uhr kann dabei f&uuml;r ca. 3 Minuten verloren gehen", "Ajoute un nouveau r&eacute;seau via WPS - appuyez sur le bouton WPS de votre routeur lorsque vous y &ecirc;tes invit&eacute;. La connexion &agrave; l&#39;horloge peut &ecirc;tre interrompue pendant environ 3 minutes" },
        { "Scans for available WiFi networks again and refreshes the dropdown lists below", "Sucht erneut nach verf&uuml;gbaren WLAN-Netzwerken und aktualisiert die Auswahllisten darunter", "Recherche &agrave; nouveau les r&eacute;seaux WiFi disponibles et actualise les listes d&eacute;roulantes ci-dessous" },
        { "The clock can also be reached at http://\"hostname\".local instead of its IP address, e.g.", "Die Uhr ist statt &uuml;ber die IP-Adresse auch &uuml;ber http://\"hostname\".local erreichbar, z.B.", "L&#39;horloge est &eacute;galement accessible via http://\"hostname\".local au lieu de son adresse IP, par ex." },
        { "Up to", "Bis zu", "Jusqu&#39;&agrave;" },
        { "WiFi networks can be stored", "WLAN-Netzwerke k&ouml;nnen hinterlegt werden", "r&eacute;seaux WiFi peuvent &ecirc;tre enregistr&eacute;s" },
        { "A restart is required for a changed hostname to take effect. Not all routers support hostname resolution", "Ein Neustart ist erforderlich, damit ein ge&auml;nderter Hostname wirksam wird. Nicht alle Router unterst&uuml;tzen die Aufl&ouml;sung von Hostnamen", "Un red&eacute;marrage est n&eacute;cessaire pour qu&#39;un nom d&#39;h&ocirc;te modifi&eacute; prenne effet. Tous les routeurs ne prennent pas en charge la r&eacute;solution des noms d&#39;h&ocirc;te" },
        { "Automatically adjusts brightness based on ambient light measured by the photoresistor", "Passt die Helligkeit automatisch anhand des vom Fotowiderstand gemessenen Umgebungslichts an", "Ajuste automatiquement la luminosit&eacute; en fonction de la lumi&egrave;re ambiante mesur&eacute;e par la photor&eacute;sistance" },
        { "Reverses the brightness sensor reading - use if the display gets darker in bright light instead of brighter", "Kehrt den Messwert des Helligkeitssensors um - verwenden, falls das Display bei hellem Licht dunkler statt heller wird", "Inverse la lecture du capteur de luminosit&eacute; - &agrave; utiliser si l&#39;&eacute;cran s&#39;assombrit au lieu de s&#39;&eacute;claircir en pr&eacute;sence de lumi&egrave;re vive" },
        { "Below this ambient light percentage, the display uses minimum brightness", "Unterhalb dieses Umgebungslicht-Prozentwerts verwendet das Display die Mindesthelligkeit", "En dessous de ce pourcentage de lumi&egrave;re ambiante, l&#39;&eacute;cran utilise la luminosit&eacute; minimale" },
        { "Above this ambient light percentage, the display uses maximum brightness", "Oberhalb dieses Umgebungslicht-Prozentwerts verwendet das Display die Maximalhelligkeit", "Au-dessus de ce pourcentage de lumi&egrave;re ambiante, l&#39;&eacute;cran utilise la luminosit&eacute; maximale" },
        { "Display brightness used at or below the low threshold", "Displayhelligkeit, die bei oder unterhalb der unteren Schwelle verwendet wird", "Luminosit&eacute; de l&#39;&eacute;cran utilis&eacute;e au niveau ou en dessous du seuil bas" },
        { "Display brightness used at or above the high threshold", "Displayhelligkeit, die bei oder oberhalb der oberen Schwelle verwendet wird", "Luminosit&eacute; de l&#39;&eacute;cran utilis&eacute;e au niveau ou au-dessus du seuil haut" },
        { "Start of the daily time window during which the display always uses full brightness, regardless of ambient light", "Beginn des t&auml;glichen Zeitfensters, in dem das Display unabh&auml;ngig vom Umgebungslicht immer volle Helligkeit nutzt", "D&eacute;but de la plage horaire quotidienne pendant laquelle l&#39;&eacute;cran utilise toujours la luminosit&eacute; maximale, ind&eacute;pendamment de la lumi&egrave;re ambiante" },
        { "End of the daily time window during which the display always uses full brightness, regardless of ambient light", "Ende des t&auml;glichen Zeitfensters, in dem das Display unabh&auml;ngig vom Umgebungslicht immer volle Helligkeit nutzt", "Fin de la plage horaire quotidienne pendant laquelle l&#39;&eacute;cran utilise toujours la luminosit&eacute; maximale, ind&eacute;pendamment de la lumi&egrave;re ambiante" },
        { "Adjusts how brightness ramps between minimum and maximum - higher values keep the display darker for longer before brightening", "Passt an, wie die Helligkeit zwischen Minimum und Maximum ansteigt - h&ouml;here Werte halten das Display l&auml;nger dunkler, bevor es heller wird", "Ajuste la mani&egrave;re dont la luminosit&eacute; augmente entre le minimum et le maximum - des valeurs plus &eacute;lev&eacute;es maintiennent l&#39;&eacute;cran plus sombre plus longtemps avant qu&#39;il ne s&#39;&eacute;claircisse" },
        { "Total", "Gesamt", "Total" },
        { "Used", "Belegt", "Utilis&eacute;" },
        { "Save", "speichern", "enregistrer" },
        { "Brightness", "Helligkeit", "Luminosit&eacute;" },
        { "Language", "Sprache", "Langue" },
        { "Time Settings", "Zeiteinstellungen", "Param&egrave;tres de l&#39;heure" },
        { "NTP Server 1", "NTP Server 1", "Serveur NTP 1" },
        { "NTP Server 2", "NTP Server 2", "Serveur NTP 2" },
        { "Timezone", "Zeitzone", "Fuseau horaire" },
        { "Clock Settings", "Uhreinstellungen", "Param&egrave;tres de l&#39;horloge" },
        { "Handset", "Zeigersatz", "Aiguilles" },
        { "Background", "Hintergrund", "Fond" },
        { "Display Settings", "Anzeigeeinstellungen", "Param&egrave;tres d&#39;affichage" },
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
        { "Load Presets from File", "Presets aus Datei laden", "Charger les pr&eacute;r&eacute;glages depuis un fichier" },
        { "This will replace all currently saved presets", "Dies ersetzt alle aktuell gespeicherten Presets", "Cela remplacera tous les pr&eacute;r&eacute;glages actuellement enregistr&eacute;s" },
        { "Continue", "Fortfahren", "Continuer" },
        { "Presets imported successfully", "Presets erfolgreich importiert", "Pr&eacute;r&eacute;glages import&eacute;s avec succ&egrave;s" },
        { "Import failed - please check the file", "Import fehlgeschlagen - bitte Datei pr&uuml;fen", "&Eacute;chec de l&#39;importation - veuillez v&eacute;rifier le fichier" },
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
        { "Hand&nbsp;Set", "Zeiger", "Jeu&nbsp;d&#39;aiguilles" },
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
        { "Manage Hand Set Files", "Zeigersatz Dateien verwalten", "G&eacute;rer les jeux d&#39;aiguilles" },
        { "Rename", "Umbenennen", "Renommer" },
        { "Enter new name for", "Neuen Namen eingeben f&uuml;r", "Entrez le nouveau nom pour" },
        { "New name", "Neuer Name", "Nouveau nom" },
        { "New Name", "Neuer Name", "Nouveau nom" },
        { "File renamed successfully", "Datei erfolgreich umbenannt", "Fichier renomm&eacute; avec succ&egrave;s" },
        { "File rename failed", "Datei-Umbenennung fehlgeschlagen", "&Eacute;chec du renommage" },
        { "Download Additional Clock Faces", "Zus&auml;tzliche Zifferbl&auml;tter herunterladen", "T&eacute;l&eacute;charger des cadrans suppl&eacute;mentaires" },
        { "You can download a ZIP file containing additional clock faces and hand sets from the following link: (use 'view raw')", "Sie k&ouml;nnen eine ZIP-Datei mit zus&auml;tzlichen Zifferbl&auml;ttern und Zeigers&auml;tzen von folgendem Link herunterladen: (verwenden Sie &#39;view raw&#39;)", "Vous pouvez t&eacute;l&eacute;charger un fichier ZIP contenant des cadrans et jeux d&#39;aiguilles suppl&eacute;mentaires depuis le lien suivant&nbsp;: (utilisez &#39;view raw&#39;)" },
        { "Unzip the file and upload the contents to the /clockfaces and /handsets directories using the File Manager", "Entpacken Sie die Datei und laden Sie den Inhalt mit dem Dateimanager in die Verzeichnisse /clockfaces und /handsets hoch", "D&eacute;compressez le fichier et t&eacute;l&eacute;chargez le contenu dans les dossiers /clockfaces et /handsets &agrave; l&#39;aide du gestionnaire de fichiers" },
        { "After downloading, upload the extracted BMP files using the form below", "Nach dem Herunterladen k&ouml;nnen Sie die extrahierten BMP-Dateien mit dem untenstehenden Formular hochladen", "Apr&egrave;s le t&eacute;l&eacute;chargement, envoyez les fichiers BMP extraits &agrave; l&#39;aide du formulaire ci-dessous" },
        { "Upload New Clock Face", "neue Zifferbl&auml;tter hochladen", "T&eacute;l&eacute;charger un nouveau cadran" },
        { "Requirements", "Anforderungen", "Exigences" },
        { "instead of the IP address for better reliability", "anstelle der IP Adresse f&uuml;r bessere Erreichbarkeit", "au lieu de l&#39;adresse IP pour une meilleure fiabilit&eacute;" },
        { "name must start with face_ and end with .bmp", "Name muss mit face_ beginnen und mit .bmp enden", "le nom doit commencer par face_ et se terminer par .bmp" },
        { "size must be", "Gr&ouml;&szlig;e muss sein", "la taille doit &ecirc;tre" },
        { "Clock Setup", "Uhr Einstellungen", "Param&egrave;tres de l&#39;horloge" },
        { "name must start with", "Name muss beginnen mit", "le nom doit commencer par" },
        { "Manage Clock Hand Sets", "Zeigers&auml;tze verwalten", "G&eacute;rer les jeux d&#39;aiguilles" },
        { "Preview/Set", "Vorschau/Setzen", "Aper&ccedil;u/D&eacute;finir" },
        { "Upload New Hand Set", "Neuen Zeigersatz hochladen", "T&eacute;l&eacute;charger un nouveau jeu d&#39;aiguilles" },
        { "Upload to Set", "Set hochladen", "T&eacute;l&eacute;charger vers le jeu" },
        { "Color (RGB hex, e.g. FF0000 = Red, 000000 = Black, EC0016 = DB red)", "Farbe (RGB hex, z.B. FF0000 = Rot, 000000 = Schwarz, EC0016 = DB rot)", "Couleur (hex RGB, ex. FF0000 = Rouge, 000000 = Noir, EC0016 = rouge DB)" },
        { "Centre point", "Mittelpunkt", "Point central" },
        { "Size", "Gr&ouml;&szlig;e", "Taille" },
        { "Warning: Not enough free space to upload new hand sets!Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zeiger! Bitte zuerst Speicherplatz freigeben", "Attention&nbsp;: espace insuffisant pour t&eacute;l&eacute;charger de nouveaux jeux d&#39;aiguilles&nbsp;! Lib&eacute;rez d&#39;abord de l&#39;espace" },
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
        { "Full brightness until (hour, 0-23)", "volle Helligkeit bis Stunde (0-23)", "Luminosit&eacute; maximale jusqu&#39;&agrave; (heure, 0-23)" },
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
        { "Are you sure you want to reboot?", "Sind Sie sicher, das Sie die Uhr neu starten wollen?", "&Ecirc;tes-vous s&ucirc;r de vouloir red&eacute;marrer l&#39;horloge&nbsp;?" },
        { "Are you sure you want to reset to factory settings?", "Sind Sie absolut sicher, das Sie die Uhr auf Werkseinstellung setzen wollen?", "&Ecirc;tes-vous absolument s&ucirc;r de vouloir r&eacute;initialiser l&#39;horloge aux param&egrave;tres d&#39;usine&nbsp;?" },
        { "Reset Everything", "Alles zur&uuml;cksetzen", "Tout r&eacute;initialiser" },
        { "Resets WiFi, all settings and deletes all files - the clock restarts afterwards", "Setzt WLAN, alle Einstellungen zur&uuml;ck und l&ouml;scht alle Dateien - die Uhr startet danach neu", "R&eacute;initialise le WiFi, tous les param&egrave;tres et supprime tous les fichiers - l&#39;horloge red&eacute;marre ensuite" },
        { "Deletes all saved WiFi networks - other settings remain unchanged", "L&ouml;scht alle gespeicherten WLAN-Netzwerke - andere Einstellungen bleiben unver&auml;ndert", "Supprime tous les r&eacute;seaux WiFi enregistr&eacute;s - les autres param&egrave;tres restent inchang&eacute;s" },
        { "Delete Clock Faces (except default)", "Zifferbl&auml;tter l&ouml;schen (au&szlig;er Standard)", "Supprimer les cadrans (sauf le cadran par d&eacute;faut)" },
        { "Deletes all uploaded clock faces - the built-in default remains", "L&ouml;scht alle hochgeladenen Zifferbl&auml;tter - der eingebaute Standard bleibt erhalten", "Supprime tous les cadrans t&eacute;l&eacute;charg&eacute;s - le cadran par d&eacute;faut int&eacute;gr&eacute; est conserv&eacute;" },
        { "Delete Hand Sets (except default)", "Zeigers&auml;tze l&ouml;schen (au&szlig;er Standard)", "Supprimer les jeux d&#39;aiguilles (sauf celui par d&eacute;faut)" },
        { "Deletes all uploaded hand sets - the built-in default remains", "L&ouml;scht alle hochgeladenen Zeigers&auml;tze - der eingebaute Standard bleibt erhalten", "Supprime tous les jeux d&#39;aiguilles t&eacute;l&eacute;charg&eacute;s - celui par d&eacute;faut int&eacute;gr&eacute; est conserv&eacute;" },
        { "Delete Presets", "Uhren Sets l&ouml;schen", "Supprimer les pr&eacute;r&eacute;glages" },
        { "Deletes all saved presets", "L&ouml;scht alle gespeicherten Uhren Sets", "Supprime tous les pr&eacute;r&eacute;glages enregistr&eacute;s" },
        { "Are you sure you want to delete all clock faces except the default one?", "Sind Sie sicher, dass Sie alle Zifferbl&auml;tter au&szlig;er dem Standard l&ouml;schen wollen?", "&Ecirc;tes-vous s&ucirc;r de vouloir supprimer tous les cadrans sauf celui par d&eacute;faut&nbsp;?" },
        { "Are you sure you want to delete all hand sets except the default one?", "Sind Sie sicher, dass Sie alle Zeigers&auml;tze au&szlig;er dem Standard l&ouml;schen wollen?", "&Ecirc;tes-vous s&ucirc;r de vouloir supprimer tous les jeux d&#39;aiguilles sauf celui par d&eacute;faut&nbsp;?" },
        { "Are you sure you want to delete all presets?", "Sind Sie sicher, dass Sie alle Uhren Sets l&ouml;schen wollen?", "&Ecirc;tes-vous s&ucirc;r de vouloir supprimer tous les pr&eacute;r&eacute;glages&nbsp;?" },
        { "Clock faces deleted", "Zifferbl&auml;tter gel&ouml;scht", "Cadrans supprim&eacute;s" },
        { "Hand sets deleted", "Zeigers&auml;tze gel&ouml;scht", "Jeux d&#39;aiguilles supprim&eacute;s" },
        { "Presets deleted", "Presets gel&ouml;scht", "Pr&eacute;r&eacute;glages supprim&eacute;s" },
        { "Network deleted", "Netzwerk gel&ouml;scht", "R&eacute;seau supprim&eacute;" },
        { "Rename File", "Datei umbenennen", "Renommer le fichier" },
        { "Rename Preset", "Preset umbenennen", "Renommer le pr&eacute;r&eacute;glage" },
        { "Maximum number of presets reached - delete an existing preset first", "Maximale Anzahl an Presets erreicht - bitte zuerst ein bestehendes Preset l&ouml;schen", "Nombre maximal de pr&eacute;r&eacute;glages atteint - veuillez d&#39;abord supprimer un pr&eacute;r&eacute;glage existant" },
        { "Copy link", "Link kopieren", "Copier le lien" },
        { "Download Additional Clock Faces from GitHub", "Weitere Zifferbl&auml;tter von GitHub laden", "T&eacute;l&eacute;charger d&#39;autres cadrans depuis GitHub" },
        { "Download Additional Hand Sets from GitHub", "Weitere Zeigers&auml;tze von GitHub laden", "T&eacute;l&eacute;charger d&#39;autres jeux d&#39;aiguilles depuis GitHub" },
        { "Checking GitHub for new files", "Suche nach neuen Dateien auf GitHub", "Recherche de nouveaux fichiers sur GitHub" },
        { "Scanning for WiFi networks - the page will reload automatically in 10 seconds", "Suche nach WLAN-Netzwerken - die Seite l&auml;dt sich in 10 Sekunden automatisch neu", "Recherche de r&eacute;seaux WiFi - la page se rechargera automatiquement dans 10 secondes" },
        { "All files already up to date", "Alle Dateien sind bereits aktuell", "Tous les fichiers sont d&eacute;j&agrave; &agrave; jour" },
        { "All presets already up to date", "Alle Presets sind bereits aktuell", "Tous les pr&eacute;r&eacute;glages sont d&eacute;j&agrave; &agrave; jour" },
        { "No presets found. Load recommended presets from GitHub?", "Keine Presets vorhanden. Empfohlene Presets von GitHub laden?", "Aucun pr&eacute;r&eacute;glage trouv&eacute;. Charger les pr&eacute;r&eacute;glages recommand&eacute;s depuis GitHub&nbsp;?" },
        { "Downloading", "Lade herunter", "T&eacute;l&eacute;chargement de" },
        { "Converting", "Konvertiere", "Conversion de" },
        { "Done - reloading", "Fertig - lade neu", "Termin&eacute; - rechargement" },
        { "Failed to reach GitHub - check your internet connection", "GitHub nicht erreichbar - Internetverbindung pr&uuml;fen", "Impossible de joindre GitHub - v&eacute;rifiez votre connexion internet" },
        { "Load Presets from GitHub", "Presets von GitHub laden", "Charger les pr&eacute;r&eacute;glages depuis GitHub" },
        { "Language updated", "Sprache ge&auml;ndert", "Langue mise &agrave; jour" },
        { "Preset created", "Preset erstellt", "Pr&eacute;r&eacute;glage cr&eacute;&eacute;" },
        { "Preset applied", "Preset angewendet", "Pr&eacute;r&eacute;glage appliqu&eacute;" },
        { "File renamed", "Datei umbenannt", "Fichier renomm&eacute;" },
        { "Clock face uploaded", "Zifferblatt hochgeladen", "Cadran t&eacute;l&eacute;charg&eacute;" },
        { "Clock face selected", "Zifferblatt ausgew&auml;hlt", "Cadran s&eacute;lectionn&eacute;" },
        { "Preset deleted", "Preset gel&ouml;scht", "Pr&eacute;r&eacute;glage supprim&eacute;" },
        { "Preset renamed", "Preset umbenannt", "Pr&eacute;r&eacute;glage renomm&eacute;" },
        { "Hand set uploaded", "Zeigersatz hochgeladen", "Jeu d&#39;aiguilles t&eacute;l&eacute;charg&eacute;" },
        { "Hand set selected", "Zeigersatz ausgew&auml;hlt", "Jeu d&#39;aiguilles s&eacute;lectionn&eacute;" },
        { "Hand set deleted", "Zeigersatz gel&ouml;scht", "Jeu d&#39;aiguilles supprim&eacute;" },
        { "Enter a name for the new preset (leave empty for automatic naming)", "Namen f&uuml;r das neue Preset eingeben (leer lassen f&uuml;r automatische Benennung)", "Entrez un nom pour le nouveau pr&eacute;r&eacute;glage (laissez vide pour un nommage automatique)" },
        { "Timezone updated", "Zeitzone aktualisiert", "Fuseau horaire mis &agrave; jour" },
        { "Settings saved", "Einstellungen gespeichert", "Param&egrave;tres enregistr&eacute;s" },
        { "Please connect to your home network and go to the ESP website at", "Bitte mit dem Heimnetzwerk verbinden und die ESP-Webseite aufrufen unter", "Veuillez vous connecter &agrave; votre r&eacute;seau domestique et acc&eacute;der au site de l&#39;ESP &agrave;" },
        { "Time synced", "Zeit synchronisiert", "Heure synchronis&eacute;e" },
        { "Returning to main page in 3 seconds", "Zur&uuml;ck zur Hauptseite in 3 Sekunden", "Retour &agrave; la page principale dans 3 secondes" },
        { "System Status", "Systemstatus", "&Eacute;tat du syst&egrave;me" },
        { "Upload Failed", "Upload fehlgeschlagen", "&Eacute;chec du t&eacute;l&eacute;chargement" },
        { "Upload failed", "Upload fehlgeschlagen", "&Eacute;chec du t&eacute;l&eacute;chargement" },
        { "Only .bmp files starting with", "Nur .bmp-Dateien, die beginnen mit", "Seuls les fichiers .bmp commen&ccedil;ant par" },
        { "or", "oder", "ou" },
        { "are accepted", "werden akzeptiert", "sont accept&eacute;s" },
        { "are accepted for handset upload", "werden f&uuml;r den Zeigersatz-Upload akzeptiert", "sont accept&eacute;s pour le t&eacute;l&eacute;chargement de jeux d&#39;aiguilles" },
        { "Please also check the available space", "Bitte auch den verf&uuml;gbaren Speicherplatz pr&uuml;fen", "Veuillez &eacute;galement v&eacute;rifier l&#39;espace disponible" },
        { "Try again", "Erneut versuchen", "R&eacute;essayer" },
        { "Return to the main page in 10 seconds or refresh the website when the ESP is online again", "Kehren Sie in 10 Sekunden zur Hauptseite zur&uuml;ck oder aktualisieren Sie die Website, wenn der ESP wieder online ist", "Retour &agrave; la page principale dans 10 secondes, ou actualisez le site lorsque l&#39;ESP est &agrave; nouveau en ligne" },
        { "Enable Logging", "Logging aktivieren", "Activer la journalisation" },
        { "Reconnect WiFi", "Wifi neu verbinden", "Reconnecter le WiFi" },
        { "JavaScript is disabled.This page requires JavaScript to work properly!", "JavaScript ist deaktiviert.Diese Seite ben&ouml;tigt JavaScript, um richtig zu funktionieren!", "JavaScript est d&eacute;sactiv&eacute;. Cette page n&eacute;cessite JavaScript pour fonctionner correctement&nbsp;!" },
        { "Are you sure you want to reset all saved WiFi networks?", "Sind Sie sicher, dass Sie alle gespeicherten WLAN Netzwerke zur&uuml;cksetzen m&ouml;chten?", "&Ecirc;tes-vous s&ucirc;r de vouloir r&eacute;initialiser tous les r&eacute;seaux WiFi enregistr&eacute;s&nbsp;?" },
        { "Reset WiFi Networks", "WLAN Netzwerke zur&uuml;cksetzen", "R&eacute;initialiser les r&eacute;seaux WiFi" },
        { "WiFi networks reset. Rebooting...", "WLAN Netzwerke zur&uuml;ckgesetzt. Starte neu...", "R&eacute;seaux WiFi r&eacute;initialis&eacute;s. Red&eacute;marrage..." },
        { "DCF77 detected", "DCF77 erkannt", "DCF77 d&eacute;tect&eacute;" },
        { "Waiting", "warte", "Attente" },
        { "Back", "Zur&uuml;ck", "Retour" },
        { "Check RTC", "RTC pr&uuml;fen", "V&eacute;rifier le RTC" },
        { "Download", "Herunterladen", "T&eacute;l&eacute;charger" },
        { "Failed to scale BMP", "Skalierung des BMP fehlgeschlagen", "&Eacute;chec du redimensionnement du BMP" },
        { "Gamma Correction", "Gamma-Korrektur", "Correction gamma" },
        { "Hostname", "Hostname", "Nom d&#39;h&ocirc;te" },
        { "Hostname saved - requires a reboot to take effect", "Hostname gespeichert - Neustart erforderlich, damit die &Auml;nderung wirksam wird", "Nom d&#39;h&ocirc;te enregistr&eacute; - un red&eacute;marrage est n&eacute;cessaire pour appliquer le changement" },
        { "No valid hostname could be derived from the input - falling back to the automatic name based on the MAC address", "Aus der Eingabe konnte kein g&uuml;ltiger Hostname gebildet werden - R&uuml;ckfall auf den automatischen, aus der MAC-Adresse gebildeten Namen", "Aucun nom d&#39;h&ocirc;te valide n&#39;a pu &ecirc;tre form&eacute; &agrave; partir de la saisie - retour au nom automatique bas&eacute; sur l&#39;adresse MAC" },
        { "Ping Server", "Ping-Server", "Serveur de ping" },
        { "Reset Saved Networks", "Gespeicherte Netzwerke zur&uuml;cksetzen", "R&eacute;initialiser les r&eacute;seaux enregistr&eacute;s" },
        { "Add Network via WPS", "Netzwerk per WPS hinzuf&uuml;gen", "Ajouter un r&eacute;seau via WPS" },
        { "WPS active - press the WPS button on your router now (within 2 minutes)", "WPS aktiv - jetzt die WPS-Taste am Router dr&uuml;cken (innerhalb von 2 Minuten)", "WPS actif - appuyez maintenant sur le bouton WPS de votre routeur (dans les 2 minutes)" },
        { "Reset WLan...", "WLAN zur&uuml;cksetzen...", "R&eacute;initialisation du WiFi..." },
        { "Saved as", "Gespeichert als", "Enregistr&eacute; sous" },
        { "Scaling successful", "Skalierung erfolgreich", "Redimensionnement r&eacute;ussi" },
        { "View", "Anzeigen", "Voir" },
        { "Warning: Not enough free space to upload new clock faces! Free up some space first", "Warnung: Nicht gen&uuml;gend Speicherplatz zum Hochladen neuer Zifferbl&auml;tter! Bitte zuerst Speicherplatz freigeben", "Attention&nbsp;: espace insuffisant pour t&eacute;l&eacute;charger de nouveaux cadrans&nbsp;! Lib&eacute;rez d&#39;abord de l&#39;espace" },
        { "Use the host name", "Benutze den Hostnamen", "Utilisez le nom d&#39;h&ocirc;te" },
    };
    static const size_t translationTableSize = sizeof(translationTable) / sizeof(translationTable[0]);


    // Laedt die zuletzt gespeicherte Spracheinstellung aus den Preferences
    // (wird einmalig in setup() aufgerufen). Es muss nichts mehr aufgebaut
    // werden - translate() liest direkt aus der flash-residenten Tabelle.

    // English: loads the last saved language setting from Preferences
    // (called once in setup()). Nothing needs to be built anymore -
    // translate() reads directly from the flash-resident table.

    void loadLanguage() {
        currentLanguage = preferences.getString(PK_LANGUAGE, "en");
    }


    // Setzt die aktive Sprache und speichert sie dauerhaft in den Preferences
    // English: sets the active language and persists it in Preferences

    void saveLanguage(String lang) {
        currentLanguage = lang;
        preferences.putString(PK_LANGUAGE, lang);
    }


    // Uebersetzt einen englischen Schluessel-String in die aktuell aktive Sprache
    // (Fallback: Schluessel selbst bei Englisch/fehlendem Eintrag). Durchsucht
    // direkt die flash-residente translationTable - kein Heap-Umkopieren noetig.

    // English: translates an English key string into the currently active
    // language (fallback: the key itself for English/missing entry). Searches
    // the flash-resident translationTable directly - no heap copying needed.

    String translate(const String& key) {

        if (currentLanguage == "en") return key;  // Englisch: Schlüssel zurückgeben
                                                  // English: return the key

        for (size_t i = 0; i < translationTableSize; i++) {
            if (key == translationTable[i].key) {
                if (currentLanguage == "de") return String(translationTable[i].de);
                if (currentLanguage == "fr") return String(translationTable[i].fr);
                return key;
            }
        }

        return key; // Fallback: Schlüssel zurückgeben
                    // English: fallback - return the key
    }


#endif // TRANSLATION_H
