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
    private string serialPort = "";
    
    [SerializeField]
    private bool debugData = false;

    private bool toggle = false;

    public GameObject cube;

    void Start () {

        arduinoConnector = new ArduinoConnector(baudRate);
        arduinoConnector.serialPort = serialPort;
    }

    void Update() {
        if(SteamVR_Input.GetStateDown("Btn2", hand)){
            Debug.Log("Door closed");
            //sendData("off");
            cube.GetComponent<Renderer>().material.SetColor("_Color", Color.red);

        }

        if(SteamVR_Input.GetStateUp("Btn2", hand)){
                Debug.Log("Door open");
                //sendData("on");
                cube.GetComponent<Renderer>().material.SetColor("_Color", Color.green);

        }
    

        if(SteamVR_Input.GetStateDown("Btn1", hand)){
            Debug.Log("Button on Heart-Top pressed");
            cube.GetComponent<Renderer>().material.SetColor("_Color", Color.blue);
        }

        if(SteamVR_Input.GetStateUp("Btn1", hand)){
                //sendData("on");
                Debug.Log("Button on Heart-Top released");
                cube.GetComponent<Renderer>().material.SetColor("_Color", Color.red);

        }

        if(SteamVR_Input.GetStateDown("Btn4", hand)){
            Debug.Log("Button behind Door pressed");
            toggleButton();
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

