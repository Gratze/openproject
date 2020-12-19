using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Valve.VR;


public class TrackerInput : MonoBehaviour
{
    public SteamVR_Input_Sources hand;

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
        if(SteamVR_Input.GetStateDown("Btn1", hand)){
            Debug.Log("Button 1 pressed");
        }
        
        if(SteamVR_Input.GetStateDown("Btn2", hand)){
            Debug.Log("Button 2 pressed");
        }

        if(SteamVR_Input.GetStateDown("Btn3", hand)){
            Debug.Log("Button 3 pressed");
        }

        if(SteamVR_Input.GetStateDown("Btn4", hand)){
            Debug.Log("Button 4 pressed");
            if (debugData) {
                sendData();
            }
        }
    }

    // Thread beenden wenn Unity beendet wird
    void OnApplicationQuit() {
        arduinoConnector.isThreadActive = false;
    }

    public void sendData(){
        if( arduinoConnector != null) {
            arduinoConnector.SendToArduino("receive");
            Debug.Log("Data sent");
        }
    }
}

