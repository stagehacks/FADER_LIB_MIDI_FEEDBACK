# FADER_8_MIDI_FEEDBACK
 
Example code to turn a [Stage Hacks FADER_8](https://www.stagehacks.com/store/p/5yt9wdkipr0sifzmb9u53mt62kkpkz) into a generic 8-channel MIDI fader bank with feedback.

- Sends MIDI Control Change messages on Channel 1, Controls 0-7
- Responds to MIDI Control Change messages on Channel 16, Controls 0-7

This code will not compile unless the USB Type is set to MIDI. In Teensyduino software, navigate to **Tools** > **USB Type** and select **MIDI**
