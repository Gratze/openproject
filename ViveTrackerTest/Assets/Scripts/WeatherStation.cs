using UnityEngine;
using System;
using UnityEngine.XR;
using System.Collections;
public class WeatherStation : MonoBehaviour {

    // ArduinoConnector Thread Setup
#if !UNITY_EDITOR && UNITY_WSA_10_0
    private ArduinoConnectorUWP arduinoConnector;
#else
    private ArduinoConnector arduinoConnector;
#endif
    

    private string arduinoMessage;

    [SerializeField]
    private int baudRate = 115200;
    
    [SerializeField]
    private bool debugData = false;

    void Start () {

        arduinoConnector = new ArduinoConnector(baudRate);
    }

    void Update() {
            if (debugData) {
            if( arduinoConnector != null) {
                arduinoMessage = arduinoConnector.ReadFromArduino();
                Debug.Log("Message:  " + arduinoMessage);
                if (arduinoMessage != null && !arduinoMessage.Equals("Sending failed (no ack)")) {
                    String[] values = arduinoMessage.Split(';');
                    
                }
            }
        }
    }

    // Thread beenden wenn Unity beendet wird
    void OnApplicationQuit() {
        arduinoConnector.isThreadActive = false;
    }

    public void sendDate(){
        if( arduinoConnector != null) {
            arduinoConnector.SendToArduino("receive");
            Debug.Log("Data sent");
        }
    }

}
