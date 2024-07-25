# SmartMarine pH Keeper

SmartMarine pH Keeper is an IoT system designed to maintain water pH stability and provide automatic fish feeding at specific times. This system uses various components including an aquarium, pH sensor, turbidity sensor, servo motor, ESP32, filter box, pH powder, cotton, and rocks. The system is integrated with BLYNK, allowing users to monitor water pH and turbidity and control fish feeding remotely.

## Features

- **Automatic Fish Feeding:** Automatically feeds fish at scheduled times.
- **pH Regulation:** Checks the pH level every 15 minutes, and if the pH is unstable, the servo motor dispenses pH powder to stabilize it.
- **Real-time Monitoring:** Displays pH and turbidity levels on the Blynk app.
- **Water Clarity Maintenance:** Maintains water clarity in the aquarium with a filter box using cotton and gravel.

## Installation

Follow these steps to set up the SmartMarine pH Keeper system:

1. **Assemble the Prototype:**
    - Assemble the hardware components according to the provided prototype diagram.
    - Ensure all connections are secure and correct.

2. **Upload Code:**
    - Copy the `.ino` code file from this repository.
    - Replace the placeholders with your Wi-Fi SSID and password.
    - Upload the code to the ESP32 using the Arduino IDE.

    ```cpp
    #include <WiFi.h>
    #include <BlynkSimpleEsp32.h>

    // Your WiFi credentials.
    // Set password to "" for open networks.
    char ssid[] = "YourNetworkName";
    char pass[] = "YourPassword";

    // Your Auth Token from Blynk
    char auth[] = "YourBlynkAuthToken";

    void setup()
    {
      // Debug console
      Serial.begin(9600);

      Blynk.begin(auth, ssid, pass);
    }

    void loop()
    {
      Blynk.run();
    }
    ```

3. **Integrate with Blynk:**
    - Create a new project in the Blynk app.
    - Add necessary widgets to monitor pH and turbidity and control the fish feeder.
    - Obtain the Auth Token and insert it into your `.ino` file.
    - Upload the code to your ESP32.

## Usage

1. **Fill with pH Powder:**
    - Add pH stabilizing powder to the designated compartment to maintain water pH.

2. **Add Fish Feed:**
    - Fill the fish feeder compartment with fish feed to ensure your fish stay healthy.

3. **Monitor and Control:**
    - Use the Blynk app to monitor real-time pH and turbidity levels.
    - Manually feed fish or adjust settings as needed through the app.

## Preview
![Screenshot 2024-07-25 224952](https://github.com/user-attachments/assets/8bbbf5ea-a15a-4210-9efd-15a3f390fb1d)
![1717328636286](https://github.com/user-attachments/assets/d4dca962-0217-4f99-86eb-d006dd270991)
https://github.com/user-attachments/assets/5377741a-9dc8-4009-8429-6f7bc6a1b6bd

