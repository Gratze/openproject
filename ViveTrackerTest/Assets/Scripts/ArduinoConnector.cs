using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO.Ports;
using System.Threading;

public class ArduinoConnector {
    
    private string dataString;
    public int arduinoDelay = 10;
    
    private Queue outputQueue;  // Queue Unity zu Arduino
    private Queue inputQueue;   // Queue Arduino zu Unity
     
    private Thread arduinioThread;
    private int timeoutRead = 10;
    public bool isThreadActive;

    [SerializeField]
    private string serialPort = "";
    [SerializeField]
    private int baudRate = 0;
    private SerialPort stream;

    // Konstruktor für Konfigdaten der serielle Schnittstelle
    public ArduinoConnector(int baudRate) {
        serialPort = "COM8";//GetFirstSerialPort();
        this.baudRate = baudRate;
        StartThread();
    }
   
    public void StartThread () {
        isThreadActive = true;
        // Warteschlangen instanziieren
        outputQueue = Queue.Synchronized(new Queue());
        inputQueue = Queue.Synchronized(new Queue());
        // ThreadLoop als Thread starten
        arduinioThread = new Thread(ThreadLoop);
        arduinioThread.Start();
    }   
	
    // Threadfunktion zum Auslesen der Handschuhdaten
    public void ThreadLoop() {
        // Prüfen der Seriellen Verbindungsdaten (Konstruktor), aufbau der seriellen Verbindung
        Debug.Log(serialPort);
        if (!serialPort.Equals("") && baudRate != 0){
            try {
                stream = new SerialPort(serialPort, baudRate);
                
                stream.ReadTimeout = arduinoDelay;
                stream.Open();
            } catch (Exception e) {
                Debug.LogError(e.Message);
            }

            // Solange isThreadActive = true (kann von WriteAndRead-Skript gesetzt werden) bleibt Serielle Verbindung für das Auslesen geöffnet
            while (isThreadActive) {
                // Warteschlange auf existierende Anfragen prüfen
                if (outputQueue.Count != 0) {
                    // Anfrage aus Warteschlange entnehmen und an Arduino schicken
                    string command = (string) outputQueue.Dequeue();    
                    WriteToArduino(command);
                }
                // Auf Antwort warten
                string result = ReadFromArduino(timeoutRead);

                // Antwort in (IN)Warteschlange plazieren (Daten an WriteAndRead MonoBehavior)
                if (result != null){
                    inputQueue.Enqueue(result);
                }
            }
            stream.Close();
        }
    }
    // Anfrage in (OUT)Warteschlange plazieren (Daten von WriteAndRead MonoBehavior)
    public void SendToArduino(string command) {
        outputQueue.Enqueue(command);
    }

    // Antwort aus der Warteschlange (sofern Antwort besteht) entnehmen und zurückgeben (für WriteAndRead MonoBehavior)
    public string ReadFromArduino() {
        if (inputQueue.Count == 0) {
            return null;
        }
        return (string)inputQueue.Dequeue();
    }

    // Read Funktion um eingehende Arduino-Daten (serielle Verbindung) entgegen zu nehmen / zurück zu geben
    public string ReadFromArduino(int timeout = 0) {
        stream.ReadTimeout = timeout;
        try
        {
            string readBuffer = stream.ReadLine();
            WriteToArduino("flush");
            return readBuffer;
        }
        catch (TimeoutException e)
        {
            //Debug.LogError(e.Message);
            return null;
        }
    }

    // Senden der Anfrage an den Arduino über serielle Verbindung
    public void WriteToArduino(string message) {
        try {
            stream.WriteLine(message);
            stream.BaseStream.Flush();
        }
        catch (TimeoutException e) {
            Debug.LogError(e.Message);
            dataString = null;
        }        
    }

    private String GetFirstSerialPort() {
        string[] ports = SerialPort.GetPortNames();
        if (ports == null) {
            throw new Exception("No COM-Port found");
        }
        return ports[0];
    }
}
