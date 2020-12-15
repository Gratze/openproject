using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Valve.VR
{
public class TrackerInput : MonoBehaviour
{
    public SteamVR_Input_Sources hand;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
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
            SteamVR_Action_Vibration[] actions = SteamVR_Input.actionsVibration;
            actions[0].Execute(0.0f, 3.0f, 360.0f, 1.0f, hand);
        }
        
    }
}
}
