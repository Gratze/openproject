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

    private bool toggle = false;

    public GameObject cube;

    void Start () {

        arduinoConnector = new ArduinoConnector(baudRate);
    }

    void Update() {
        if(SteamVR_Input.GetStateDown("Btn1", hand)){
            Debug.Log("Button 1 pressed");
                Debug.Log("Button 1 pressed: OFF");
                //sendData("off");
                cube.GetComponent<Renderer>().material.SetColor("_Color", Color.red);

        }

        if(SteamVR_Input.GetStateUp("Btn1", hand)){
                Debug.Log("Button 1 pressed: ON");
                //sendData("on");
                cube.GetComponent<Renderer>().material.SetColor("_Color", Color.green);

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
                
            }
        }
    }

    // Thread beenden wenn Unity beendet wird
    void OnApplicationQuit() {
        arduinoConnector.isThreadActive = false;
    }

    public void sendData(String message){
        if( arduinoConnector != null) {
            arduinoConnector.SendToArduino(message);
            Debug.Log("Data sent");
        }
    }

    public void toggleButton(){
         if(!toggle){
                Debug.Log("Button 1 pressed: ON");
                sendData("on");
                toggle = true;
            }
            else{
                Debug.Log("Button 1 pressed: OFF");
                sendData("off");
                toggle = false;
            }
    }
}

