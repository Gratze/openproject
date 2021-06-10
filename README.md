# OPEN PROJECT - HEALTH

Open Project ist ein freies Wahlpflichtfach im Fachbereich Medien an der Hochschule Düsseldorf. In dem Projekt geht es um die digitale bzw. virtuelle Unterstützung der Lehre im medizinischen Bereich durch Tangibles. Dazu wurde ein 3D-Objekt als greifbares Interface zur Veranschaulichung des menschlichen Herzens entwickelt und prototypisch realisiert. 

## MOTIVATION

Hier vielleicht kurze Motivation wieso wir das Projekt gemacht haben und worum es nochmal genau geht

## OBJECTIVE

Ziel des Projektes stellt die prototypische Realisierung eines greifbaren Interfaces zur Unterstützung der Lehre in einem iterativen Prozess dar. Der Prototyp soll Studierenden der Medizin die Anatomie des Herzens vermitteln. Dazu soll mittels LEDs und Vibration ein Herzschlag simuliert werden. Zusätzlich soll das Tangible in Unity angesprochen und verschiedene Funktionen des Tangibles über Unity ausgeführt werden können.

## APPROACH

In einem iterativen Prozess mit insgesamt 4 Sprints wurde die Umsetzung des Prototypen geplant und realisiert. In der ersten Iteration wurde konzeptionell gearbeitet. Dabei wurden verschiedene Tangibles und die entsprechende Umsetzung recherchiert. Der Fokus lag auf den benötigten Komponenten und den verschiedenen Möglichkeiten ein Tangible zu entwickeln. Aus der Recherche ergaben sich teilweise die Anforderungen an den Prototypen sowie die benötigte Schnittstelle zwischen Unity und dem Tangible. In den zweiten Iteration wurde die Schnittstelle zwschen Unity und dem Tangible (Arduino) den Anforderungen entsprechend angepasst sowie eine erste Version des Tangibles mittels eines 3D-Druckers gedruckt. In den weiteren Iterationen lag der Fokus auf einem passenden 3D-Modell für das Herz sowie den entsprechenden Anpassungen für die verbaute Elektronik sowie den 3D-Druck. Weitere Funktionen bzw. softwareseitige Veränderungen entsprachen in diesen Iterationen eher kosmetischer Natur. </br>
</br>
Die folgende Abbildung zeigt die verschiedenen Komponenten sowie die entsprechenden Funktionen des Tangibles. Diese Komponenten werden in den folgenden Unterkapitel genauer betrachtet. </br>

![Open Health Components](./docs/component_diagram.png)

### UNITY

- Standard Unity Projekt
- SteamVR hinzugefügt
- Zwei Skripte, um mit ViveTracker und Arduino zu kommunizieren

```bash
├── ViveTrackerTest
│   ├── Assets
│   │   ├── Scenes
│   │   │   ├── SampleScene.unity # Scene with example buttons and outputs
│   │   ├── Scripts
│   │   │   ├── ArduinoConnector.cs # Communication with Arduino
│   │   │   └── TrackerInput.cs # Communication with ViveTracker. Uses ArduinoConnector to send messages
│   │   ├── SteamVR
└── └── ... # Standard Unity project files
```

### ARDUINO

- Ein Adafruit, der an den PC per USB angeschlossen ist → Transmitter
- Ein Adafruit, der im Herz eingebaut ist → Receiver
- Kommunikation über 433 MHz Radio

```bash
├── arduino
│   ├── receiver
│   │   ├── receiver_led.ino # Receive messages on Adafruit connected to the heart
│   ├── transmitter
└── └── └── transmitter.ino # Send messages from Adafruit connected to PC
```

- TODO: Schaltplan

### SENDING AND RECEIVING DATA

Zum Senden an den Adafruit receiver, welcher sich im Herzen befindet, gibt es ein Skript innerhalb Unity, welches die Informationen über einen COM-Port via USB an den Transmitter übermittelt und dieser dann das Signal an den Receiver weiterleitet.</br>
</br>
In der Unity-Szene ist das Skript dem Würfel hinzugefügt. Dort kann der COM-Port an dem der Transmitter am Computer angeschlossen ist, festgelegt werden. Ebenso kann die BaudRate angepasst werden.</br>
</br>
Das Skript "ArduinoConnector.cs" verbindet sich beim Start der Szene über den COM-Port mit dem Adafruit-Transmitter. In diesem Skript gibt es verschiedene Funktionen zum verschicken oder empfangen von Daten aus dem Transmitter. In unserem Beispiel verwenden wir nur die SendToArduino(string command) Funktion, welche den Befehl in eine Queue setzt. Diese Queue wird in einer Loop überprüft und falls etwas darin steht, wird über die Funktion WriteToArduino(string message) versucht über den SerialPort die Nachricht an den Transmitter zu übergeben. Im Transmitter wird die SerialCommands Library verwendet, mit der man Events erstellen kann, welche auf Strings reagieren. Der Vorteil ist, dass dadurch weitere Events einfach hinzugefügt werden können.</br>
</br>
Dadurch können wir im TrackerInput.cs Script, wenn ein Button gedrückt wird bspw. die Nachricht (als String) "on" oder "off" an den Transmitter schicken. Die jeweiligen Events werten den String aus und falls ein Event auf den String passt, wird eine Funktion ausgeführt, welche eine neue Nachricht(auch als String) über die 433MHz Radio-Frequenz an den Receiver schickt. Bei uns wird bspw. die "on"-Nachricht erkannt und dann eine Nachricht mit dem Wert "1" an den receiver geschickt. Im Receiver wird über die RHReliableDatagram-Library der rf69_manager initialisiert, welcher erkennt ob Nachrichten empfangen werden. In der Loop wird dann überprüft ob im Buffer des rf69_manager eine Nachricht liegt. Ist dies der Fall, wird dieser ausgelesen und in einen Char umgewandelt. Dieser Char setzt dann in der Loop über einen einfachen Switch-Case den led_on-Bool auf true oder false. Je nach Zustand werden in der Loop dann die Leds und die Vibration an- oder ausgeschaltet.



### 3D MODEL

Hier Eigenschaften, Besonderheiten des Modells zeigen

- **Software für die Bearbeitung**</br>
Für das Modell wurde ein lizensiertes Herzmodell verwendet, welches nicht auf Github hochgeladen werden darf.
Das vorgegebene Modell war bereits eine STL-Datei weshalb sie nur aus Dreiecken bestand und dadurch schwieriger zu bearbeiten war, als eine normale OBJ- oder FBX-Datei. Für die Bearbeitung und Optimierung des Modells, damit dieses später auch im 3D-Drucker gedruckt werden konnte, wurde die Open-Source Software Blender verwendet. Die Software hatte den Vorteil, dass man sich "Non-Manifold"-Bereiche, also Bereiche welche später in der Slicer Software nicht korrekt verarbeitet werden können, selektiert und hervorgehoben hat. Dadurch konnte das Modell bereits in der Software perfekt für den Slicer vorbereitet werden, ohne später Überraschungen zu haben.

- **Wieso bearbeitet?**</br>
Das Modell musste für die Anforderungen und den 3D-Druck angepasst werden. Bei der eigentlichen Bearbeitung wurde das Modell in 4 Teile geschnitten. 2 Teile davon waren Venen oberhalb des Herzens, welche das Modell für den Druck nur unnötig kompliziert gemacht hatte und somit abgeschnitten und später nicht mehr weiterverwendet wurde. Die untere Hälfte wurde ausgehölt, damit dort genug Platz für die Elektronik war. Ebenso wurden dort noch 3 Steckplätze für lang gezogene Pyramiden ähnliche Stecker hinzugefügt, damit das Herz immer gleich zusammengesteckt werden kann. Es wurde ein Teil der äußeren Wand ausgeschnitten und daraus später die Klappe gemacht. In dieser Aussparung wurde auch ein Loch für den Button gemacht, damit dieser von der Innenseite, durch das Loch ragt und somit hinter der Klappe interagierbar wird. Die Klappe wurde etwas kleiner gemacht, damit diese etwas besser in die Aussparung passt. An die Klappe wurde ein kleiner Griff befestigt, damit diese leichter abnehmbar ist.</br>
An der oberen Hälfte musste weniger bearbeitet werden, da dort nur ein Aufsatz für den Tracker und ein Kabelkanal für die Anschlüsse an den Tracker gemacht werden musste. Diese Hälfte ist deshalb auch etwas massiver. Dazu wurde in die obere Hälfte auch noch eine Aussparung für einen Button hinzugefügt, damit dieser an der Seite des Herzens und von außen interagierbar ist. 

- **3D-Druck**</br>
Für den 3D-Druck wurde die Slicer-Software "Ultimaker Cura" verwendet, welche das Objekt in eine GCODE-Datei umwandelt, damit der 3D-Drucker das Modell auch drucken kann. Das Filament ist von der Marke "PrimaSelect" und ist ein 1.75mm PETG Filament, welches Trasparent ist. Am Drucker wurde eine 0.4mm Nozzle verwendet, da ab der größe von 0.6mm zuviel Details bei den äußeren Adern verloren gehen. Die erste Schicht wurde mit einer Temperatur von 220 °C gedruckt und alle folgenden mit einer Temperatur von 210 °C. Als Schichtdicke wurde 0.2mm, als Infill 20% und für die Wall-Thickness 1.6mm verwendet. Der Print-Speed wurde auf 80mm/s eingestellt. Der Fan-Speed wurde auf maximale 30% eingestellt. Beide Drucke haben jeweils knapp 1 Tag gedauert.

- **Zusätzliche Teile**</br>
3 Spikes welche etwas kleiner gedruckt wurden und an der oberen Hälfte befestigt wurden, damit diese beim Zusammenstecken in die Steckplätze der unteren Hälfte passen. Der Vive-Tracker Schiene und Steckslot. Alle zusätzlichen Teile wurden nach dem Druck an das Objekt geklebt.

- **Nachbearbeitung**</br>
Da man häufig erst ein Gefühl für das Modell bekommt wenn es gedruckt wurde, wir aber nicht soviel Plastikmüll generieren wollte, haben wir uns dafür entschieden an bestimmten Stellen das Modell mit der Hand und einem Dremel zu bearbeiten. Beispielsweise wurde an der unteren und oberen Hälfte etwas knapp mit dem Button-Slot geplant, deshalb wurde dieser dann etwas größer gefräst. Ähnlich war es beim Tracker Schiene, da wir dort noch die Kabel und Stecker unterbringen mussten, wurde hier auch gefräst und später die Stecker mit Heißkleber in der Schiene befestigt. Da manche Kanten durch den 3D-Druck nicht ganz so rund oder fein wurden, wurde auch hier etwas geschliffen, bis es besser gepasst hat.

## OUTCOME

Hier fertigen Prototypen zeigen und kurz drauf eingehen

## ISSUES

Issues faced during development

## FUTURE WORK / OUTLOOK

Hier kurz Weiterentwicklungsmöglichkeiten, Optimierungsmöglichkeiten darlegen
